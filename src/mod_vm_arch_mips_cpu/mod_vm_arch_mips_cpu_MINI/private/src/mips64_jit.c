/*
 * Copyright (C) yajin 2008 <yajinzhou@gmail.com >
 *     
 * This file is part of the virtualmips distribution. 
 * See LICENSE file for terms of the license. 
 *
 */

#include "mips64_jit.h"
#include "cvm_arch_mips_cpu.h"
#include "dump_insn.h"
#include "std_common.h"
#include "x86_trans.h"
#include <assert.h>
#include <setjmp.h>
#include <sys/mman.h>


/* Number of instructions per page */
#define MIPS_INSN_PER_PAGE (MIPS_MIN_PAGE_SIZE / sizeof(mips_insn_t))


#ifdef _USE_JIT_

extern struct mips64_jit_desc mips_jit[];
extern struct mips64_jit_desc mips_spec_jit[];
extern struct mips64_jit_desc mips_bcond_jit[];
extern struct mips64_jit_desc mips_cop0_jit[];


/**
 * m_memalign
 * @brief   
 * @param   boundary
 * @param   size
 * @return  static void *
 */
static void *m_memalign(size_t boundary, size_t size)
{
    void *p;

#ifdef __linux__
    if (posix_memalign((void *) &p, boundary, size))
#else
#if defined(__CYGWIN__) || defined(SUNOS)
    if (!(p = memalign(boundary, size)))
#else
    if (!(p = malloc(size)))
#endif
#endif
        return NULL;

    assert(((unsigned long) p & (boundary - 1)) == 0);
    return p;
}


/* Initialize the JIT structure */
int mips64_jit_init(cvm_arch_mips_cpu_t *cpu)
{
    insn_exec_page_t *cp;
    u_char *cp_addr;
    u_int area_size;
    size_t len;
    int i;

    /* Physical mapping for executable pages */
    len = MIPS_JIT_PC_HASH_SIZE * sizeof(void *);
    cpu->exec_blk_map = m_memalign(4096, len);
    memset(cpu->exec_blk_map, 0, len);

    /* Get area size */
    area_size = MIPS_EXEC_AREA_SIZE;

    /* Create executable page area */
    cpu->exec_page_area_size = area_size * 1048576;
    cpu->exec_page_area = mmap(NULL, cpu->exec_page_area_size,
                               PROT_EXEC | PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, (off_t) 0);

    if (!cpu->exec_page_area) {
        fprintf(stderr,
                "mips64_jit_init: unable to create exec area (size %lu)\n",
                cpu->exec_page_area_size);
        return (-1);
    }

    /* Carve the executable page area */
    cpu->exec_page_count = cpu->exec_page_area_size / MIPS_JIT_BUFSIZE;

    cpu->exec_page_array = calloc(cpu->exec_page_count,
                                  sizeof(insn_exec_page_t));

    if (!cpu->exec_page_array) {
        fprintf(stderr, "mips64_jit_init: unable to create exec page array\n");
        return (-1);
    }

    for (i = 0, cp_addr = cpu->exec_page_area; i < cpu->exec_page_count; i++) {
        cp = &cpu->exec_page_array[i];

        cp->ptr = cp_addr;
        cp_addr += MIPS_JIT_BUFSIZE;

        cp->next = cpu->exec_page_free_list;
        cpu->exec_page_free_list = cp;
    }

    printf("CPU%u: carved JIT exec zone of %lu Mb into %lu pages of %u Kb.\n",
           cpu->id,
           (u_long) (cpu->exec_page_area_size / 1048576),
           (u_long) cpu->exec_page_count, MIPS_JIT_BUFSIZE / 1024);
    return 0;
}

/* Flush the JIT */
int mips64_jit_flush(cvm_arch_mips_cpu_t *cpu, std_u32_t threshold)
{
    mips64_jit_tcb_t *p, *next;
    std_u32_t pc_hash;
    u_int count = 0;
    std_u32_t flush_threadhold = 0;

    if (threshold == 0)
        flush_threadhold = (std_u32_t) (0xffffffff);
    for (p = cpu->tcb_list; p; p = next) {
        next = p->next;
        if ((std_u32_t) p->acc_count <= flush_threadhold) {
            pc_hash = mips64_jit_get_pc_hash(cpu, p->start_pc);
            cpu->exec_blk_map[pc_hash] = NULL;
            mips64_jit_tcb_free(cpu, p, TRUE);
            count++;
        }
    }

    cpu->compiled_pages -= count;
    return (count);
}

/* Shutdown the JIT */
void mips64_jit_shutdown(cvm_arch_mips_cpu_t *cpu)
{
    mips64_jit_tcb_t *p, *next;

    /* Flush the JIT */
    mips64_jit_flush(cpu, 0);

    /* Free the instruction blocks */
    for (p = cpu->tcb_free_list; p; p = next) {
        next = p->next;
        free(p);
    }

    /* Unmap the executable page area */
    if (cpu->exec_page_area)
        munmap(cpu->exec_page_area, cpu->exec_page_area_size);

    /* Free the exec page array */
    free(cpu->exec_page_array);

    /* Free physical mapping for executable pages */
    free(cpu->exec_blk_map);
}

/* Allocate an exec page */
static forced_inline insn_exec_page_t *exec_page_alloc(cvm_arch_mips_cpu_t *cpu)
{
    insn_exec_page_t *p;


    /* If the free list is empty, flush JIT */
    if (unlikely(!cpu->exec_page_free_list)) {
        if (cpu->jit_flush_method) {
            mips64_jit_flush(cpu, 0);
        } else {
            mips64_jit_flush(cpu, 100);
            if (!cpu->exec_page_free_list)
                mips64_jit_flush(cpu, 0);
        }

        /* Use both methods alternatively */
        cpu->jit_flush_method = 1 - cpu->jit_flush_method;
    }

    if (unlikely(!(p = cpu->exec_page_free_list))) {
        return NULL;
    }


    cpu->exec_page_free_list = p->next;
    cpu->exec_page_alloc++;
    return p;
}

/* Free an exec page and returns it to the pool */
static forced_inline void exec_page_free(cvm_arch_mips_cpu_t *cpu, insn_exec_page_t *p)
{
    if (p) {
        p->next = cpu->exec_page_free_list;
        cpu->exec_page_free_list = p;
        cpu->exec_page_alloc--;
    }
}


/* Fetch a MIPS instruction */
static forced_inline mips_insn_t insn_fetch(mips64_jit_tcb_t *b)
{
    return (vmtoh32(b->mips_code[b->mips_trans_pos]));
}

std_int_t global_debug = TRUE;

#ifdef DEBUG_JIT
/**
 * jit_debug
 * @brief   
 * @param   cpu
 * @param   insn
 * @param   block
 * @return  void fastcall
 */
void fastcall jit_debug(cvm_arch_mips_cpu_t *cpu, std_u32_t insn, mips64_jit_tcb_t *block)
{
    if (global_debug) {
        mips64_dump_insn_with_buffer(cpu->jit_pc, insn);
    }

}

#endif

/*What is the meaning of delay_slot?
Search the whole project and you will find delay_slot can be 0/1/2.

0: we are translating the instruction not in delay slot.
1: we are translating the instruction in delay and update mips_trans_pos.
2: we are translating the instruction in delay and NOT update mips_trans_pos.

*/

/* Basic C call */
void forced_inline mips64_emit_basic_c_call(mips64_jit_tcb_t *b, void *f)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RCX, f);
    amd64_call_reg(b->jit_ptr, AMD64_RCX);
}


void mips64_load_imm(mips64_jit_tcb_t *b, u_int reg,
                     std_u64_t value);


/**
 * mips64_check_cpu_pausing
 * @brief   
 * @param   b
 */
void mips64_check_cpu_pausing(mips64_jit_tcb_t *b)
{
    u_char *test1;

    /* Check pause_request */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, pause_request), 4);
    amd64_alu_reg_imm(b->jit_ptr, X86_AND, AMD64_RAX, CPU_INTERRUPT_EXIT);
    amd64_alu_reg_imm(b->jit_ptr, X86_CMP, AMD64_RAX, CPU_INTERRUPT_EXIT);
    test1 = b->jit_ptr;
    x86_branch32(b->jit_ptr, X86_CC_NE, 0, 1);
    /*if (cpu->pause_request)&CPU_INTERRUPT_EXIT==CPU_INTERRUPT_EXIT,
  		set cpu->state and return to main loop*/
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_membase_imm(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, state), CPU_STATE_PAUSING, 4);
    mips64_jit_tcb_push_epilog(b);
    /*else do noting*/
    x86_patch(test1, b->jit_ptr);
}

#if 0
/* Fetch a MIPS instruction and emit corresponding translated code */
int mips64_jit_fetch_and_emit(cvm_arch_mips_cpu_t *cpu,
                                                  mips64_jit_tcb_t *block,
                                                  int delay_slot)
{
   mips_insn_t code;
	register uint op;

   code = insn_fetch(block);
   op = MAJOR_OP(code);

   /* Branch-delay slot is in another page: slow exec */
  if ((block->mips_trans_pos == (MIPS_INSN_PER_PAGE-1))&&(insn_is_jmp(code)) ) {
		block->jit_insn_ptr[block->mips_trans_pos] = block->jit_ptr;
      mips64_set_pc(block,block->start_pc + (block->mips_trans_pos << 2));
      mips64_emit_single_step(block,code); 
      mips64_jit_tcb_push_epilog(block);
      block->mips_trans_pos++;
      return (0) ;
   }

   if (!delay_slot)
      block->jit_insn_ptr[block->mips_trans_pos] = block->jit_ptr;

  if (delay_slot==0 )
      block->mips_trans_pos++;


#ifdef DEBUG_JIT
   std_u32_t  jit_pc;
if (delay_slot == 0)
	jit_pc=block->start_pc + ((block->mips_trans_pos-1) << 2);
else
	jit_pc=block->start_pc + ((block->mips_trans_pos) << 2);

    amd64_mov_reg_imm(block->jit_ptr,AMD64_RDX,block);
    amd64_mov_reg_imm(block->jit_ptr,AMD64_RSI,code);
    amd64_mov_reg_reg(block->jit_ptr,AMD64_RDI,AMD64_R15,8);
    amd64_mov_membase_imm(block->jit_ptr,AMD64_RDI,OFFSET(cvm_arch_mips_cpu_t,jit_pc),jit_pc,8);

    mips64_emit_basic_c_call(block,jit_debug);
#endif
   




 if (delay_slot && insn_is_jmp(code)) { 
       if (delay_slot==1 )
      		block->mips_trans_pos++;
       return (0) ;
   }

   if (!delay_slot) {
       struct mips64_jit_desc *tag;
       std_16_t special_func;
       if (op == 0){
           special_func = bits(code, 0, 5);
           tag = &mips_spec_jit[special_func];
       }else if (op == 1){
           special_func = bits(code, 16, 20);
           tag = &mips_bcond_jit[special_func];
       }else if (op == 16){
           special_func = bits(code, 21, 25);
           tag = &mips_cop0_jit[special_func];
       }else {
           tag = &mips_jit[op];
       }
       /* Check for IRQs + Increment count register before jumps */
       if (!tag->delay_slot) {
           mips64_check_pending_irq(block);
       }

   }

//   if (!delay_slot) {
//      /* Check for IRQs and cpu pausing before jumps */
//      if (insn_is_jmp(code)){
//          mips64_check_pending_irq(block);
//      	}
//   }


///*set is_in_bdslot*/
//if ((delay_slot==1)||(delay_slot==2)){
//    mips64_load_imm(block,AMD64_RAX,1);
//    amd64_mov_membase_reg(block->jit_ptr,
//                          AMD64_R15,OFFSET(cvm_arch_mips_cpu_t,is_in_bdslot),
//                          AMD64_RAX,4);
//}

mips_jit[op].emit_func(cpu,block, code);



///*clear is_in_bdslot*/
//if ((delay_slot==1)||(delay_slot==2)){
//    mips64_load_imm(block,AMD64_RAX,0);
//    amd64_mov_membase_reg(block->jit_ptr,
//                          AMD64_R15,OFFSET(cvm_arch_mips_cpu_t,is_in_bdslot),
//                          AMD64_RAX,4);
//}

  if (delay_slot==1 )
      block->mips_trans_pos++;
 
	return 0;
}
#else
int mips64_jit_fetch_and_emit(cvm_arch_mips_cpu_t *cpu,
                              mips64_jit_tcb_t *block,
                              int delay_slot)
{
    mips_insn_t code;
    register uint op;

    code = insn_fetch(block);
    op = MAJOR_OP(code);

#if 0
    STD_LOG(DISPLAY, "delay_slot: %d\n", delay_slot);
    mips64_dump_insn_with_buffer(block->mips_trans_pos*4, code);
#endif

    struct mips64_jit_desc *tag;
    std_16_t special_func;
    if (op == 0) {
        special_func = bits(code, 0, 5);
        tag = &mips_spec_jit[special_func];
    } else if (op == 1) {
        special_func = bits(code, 16, 20);
        tag = &mips_bcond_jit[special_func];
    } else if (op == 16) {
        special_func = bits(code, 21, 25);
        tag = &mips_cop0_jit[special_func];
    } else {
        tag = &mips_jit[op];
    }
    /* Branch-delay slot is in another page: slow exec */
    if ((block->mips_trans_pos == (MIPS_INSN_PER_PAGE - 1)) && !tag->delay_slot) {
        block->jit_insn_ptr[block->mips_trans_pos] = block->jit_ptr;
        mips64_set_pc(block, block->start_pc + (block->mips_trans_pos << 2));
        mips64_emit_single_step(block, code);
        mips64_jit_tcb_push_epilog(block);
        block->mips_trans_pos++;
        return 0;
    }

    if (!delay_slot)
        block->jit_insn_ptr[block->mips_trans_pos] = block->jit_ptr;

    if (delay_slot == 0)
        block->mips_trans_pos++;

#ifdef DEBUG_JIT
    std_u32_t jit_pc;
    if (delay_slot == 0)
        jit_pc = block->start_pc + ((block->mips_trans_pos - 1) << 2);
    else
        jit_pc = block->start_pc + ((block->mips_trans_pos) << 2);

    amd64_mov_reg_imm(block->jit_ptr, AMD64_RDX, block);
    amd64_mov_reg_imm(block->jit_ptr, AMD64_RSI, code);
    amd64_mov_reg_reg(block->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    amd64_mov_membase_imm(block->jit_ptr, AMD64_RDI, OFFSET(cvm_arch_mips_cpu_t, jit_pc), jit_pc, 8);

    mips64_emit_basic_c_call(block, jit_debug);
#endif

    if (delay_slot && !tag->delay_slot) {
        if (delay_slot == 1){
            block->mips_trans_pos++;
        }
        return 0;
    }
#if 0
    if (!delay_slot) {
        /* Check for IRQs + Increment count register before jumps */
        if (!tag->delay_slot) {
            mips64_check_cpu_pausing(block);
            mips64_check_pending_irq(block);
        }
    }

    /*set is_in_bdslot*/
    if ((delay_slot == 1) || (delay_slot == 2)) {
        mips64_load_imm(block, AMD64_RAX, 1);
        amd64_mov_membase_reg(block->jit_ptr,
                              AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, is_in_bdslot),
                              AMD64_RAX, 4);
    }
#endif
    mips_jit[op].emit_func(cpu, block, code);
#if 0
    /*clear is_in_bdslot*/
    if ((delay_slot == 1) || (delay_slot == 2)) {
        mips64_load_imm(block, AMD64_RAX, 0);
        amd64_mov_membase_reg(block->jit_ptr,
                              AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, is_in_bdslot),
                              AMD64_RAX, 4);
    }
#endif
    if (delay_slot == 1)
        block->mips_trans_pos++;

    return 0;
}
#endif

/* Add end of JIT block */
static forced_inline void mips64_jit_tcb_add_end(mips64_jit_tcb_t *b)
{
    mips64_set_pc(b, b->start_pc + (b->mips_trans_pos << 2));
    mips64_jit_tcb_push_epilog(b);
}

/* Record a patch to apply in a compiled block */
int mips64_jit_tcb_record_patch(mips64_jit_tcb_t *block, u_char *jit_ptr,
                                m_va_t vaddr)
{
    struct mips64_jit_patch_table *ipt = block->patch_table;
    struct mips64_insn_patch *patch;

    /* pc must be 32-bit aligned */
    if (vaddr & 0x03) {
        fprintf(stderr, "Block 0x%8.8x: trying to record an invalid PC "
                        "(0x%8.8x) - mips_trans_pos=%d.\n",
                block->start_pc, vaddr, block->mips_trans_pos);
        return (-1);
    }

    if (!ipt || (ipt->cur_patch >= MIPS64_INSN_PATCH_TABLE_SIZE)) {
        /* full table or no table, create a new one */
        ipt = malloc(sizeof(*ipt));
        if (!ipt) {
            fprintf(stderr, "Block 0x%8.8x: unable to create patch table.\n",
                    block->start_pc);
            return (-1);
        }

        memset(ipt, 0, sizeof(*ipt));
        ipt->next = block->patch_table;
        block->patch_table = ipt;
    }

    patch = &ipt->patches[ipt->cur_patch];
    patch->jit_insn = jit_ptr;
    patch->mips_pc = vaddr;
    ipt->cur_patch++;
    return 0;
}

/* Apply all patches */
static int mips64_jit_tcb_apply_patches(cvm_arch_mips_cpu_t *cpu,
                                        mips64_jit_tcb_t *block)
{
    struct mips64_jit_patch_table *ipt;
    struct mips64_insn_patch *patch;
    u_char *jit_dst;
    int i;

    for (ipt = block->patch_table; ipt; ipt = ipt->next)
        for (i = 0; i < ipt->cur_patch; i++) {
            patch = &ipt->patches[i];
            jit_dst = mips64_jit_tcb_get_host_ptr(block, patch->mips_pc);

            if (jit_dst) {
                mips64_jit_tcb_set_patch(patch->jit_insn, jit_dst);
            }
        }

    return 0;
}

/* Free the patch table */
static void mips64_jit_tcb_free_patches(mips64_jit_tcb_t *block)
{
    struct mips64_jit_patch_table *p, *next;

    for (p = block->patch_table; p; p = next) {
        next = p->next;
        free(p);
    }

    block->patch_table = NULL;
}

/* Adjust the JIT buffer if its size is not sufficient */
static int mips64_jit_tcb_adjust_buffer(cvm_arch_mips_cpu_t *cpu,
                                        mips64_jit_tcb_t *block)
{
    insn_exec_page_t *new_buffer;

    if ((block->jit_ptr - block->jit_buffer->ptr) <= (MIPS_JIT_BUFSIZE - 512))
        return 0;

    if (block->jit_chunk_pos >= MIPS_JIT_MAX_CHUNKS) {
        fprintf(stderr, "Block 0x%x: too many JIT chunks.\n", block->start_pc);
        return (-1);
    }

    if (!(new_buffer = exec_page_alloc(cpu)))
        return (-1);

    /* record the new exec page */
    block->jit_chunks[block->jit_chunk_pos++] = block->jit_buffer;
    block->jit_buffer = new_buffer;

    /* jump to the new exec page (link) */
    mips64_jit_tcb_set_jump(block->jit_ptr, new_buffer->ptr);
    block->jit_ptr = new_buffer->ptr;


    return 0;
}

/* Allocate an instruction block */
static inline mips64_jit_tcb_t *mips64_jit_tcb_alloc(cvm_arch_mips_cpu_t *cpu)
{
    mips64_jit_tcb_t *p;

    if (cpu->tcb_free_list) {
        p = cpu->tcb_free_list;
        cpu->tcb_free_list = p->next;
    } else {
        if (!(p = malloc(sizeof(*p)))) {
            return NULL;
        }
    }

    memset(p, 0, sizeof(*p));
    return p;
}

/* Free an instruction block */
void mips64_jit_tcb_free(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *block,
                         int list_removal)
{
    int i;

    if (block) {
        if (list_removal) {
            /* Remove the block from the linked list */
            if (block->next)
                block->next->prev = block->prev;
            else
                cpu->tcb_last = block->prev;

            if (block->prev)
                block->prev->next = block->next;
            else
                cpu->tcb_list = block->next;
        }

        /* Free the patch tables */
        mips64_jit_tcb_free_patches(block);

        /* Free code pages */
        for (i = 0; i < MIPS_JIT_MAX_CHUNKS; i++)
            exec_page_free(cpu, block->jit_chunks[i]);

        /* Free the current JIT buffer */
        exec_page_free(cpu, block->jit_buffer);

        /* Free the MIPS-to-native code mapping */
        free(block->jit_insn_ptr);

        /* Make the block return to the free list */
        block->next = cpu->tcb_free_list;
        cpu->tcb_free_list = block;
    }
}
#ifdef DEBUG_JIT
/*get the tcb count*/
//STD_CALL static void mips64_jit_count_tcb(cvm_arch_mips_cpu_t *cpu)
//{
//
//   unsigned int i=0;
//   insn_exec_page_t *p1;
//   p1=cpu->exec_page_free_list ;
//   while (p1!=NULL)
//   	{
//   		p1=p1->next;
//   		i++;
//   	}
//	printf("FREE PAGES %x  \n",i);
//
//	i=0;
//	mips64_jit_tcb_t * tcb1;
//	tcb1=cpu->tcb_list;
//	 while (tcb1!=NULL)
//   	{
//   		tcb1=tcb1->next;
//   		i++;
//   	}
//	printf("tcb list  %x  \n",i);
//
//	i=0;
//	tcb1=cpu->tcb_free_list;
//	 while (tcb1!=NULL)
//   	{
//   		tcb1=tcb1->next;
//   		i++;
//   	}
//	printf("tcb free list  %x  \n",i);
//
//}
#endif

#if 0
static mips64_jit_tcb_t *mips64_jit_tcb_create(cvm_arch_mips_cpu_t *cpu,
                                               m_va_t vaddr)
{
    mips64_jit_tcb_t *block = NULL;

    if (!(block = mips64_jit_tcb_alloc(cpu)))
        goto err_block_alloc;

    block->start_pc = vaddr;

    /* Allocate the first JIT buffer */
    if (!(block->jit_buffer = exec_page_alloc(cpu)))
        goto err_jit_alloc;

    block->jit_ptr = block->jit_buffer->ptr;
    block->mips_code = mod_vm_memory_lookup(p_global_vm_memory, block->start_pc);

    if (!block->mips_code) {
        fprintf(stderr,"%% No memory map for code execution at 0x%x\n",
                block->start_pc);
        goto err_lookup;
    }

    return block;

err_lookup:
err_jit_alloc:
    mips64_jit_tcb_free(cpu,block,FALSE);
err_block_alloc:
    fprintf(stderr,"%% Unable to create instruction block for vaddr=0x%x\n",
            vaddr);
    return NULL;
}
#else
/* Create an instruction block */
static mips64_jit_tcb_t *mips64_jit_tcb_create(cvm_arch_mips_cpu_t *cpu,
                                               m_va_t vaddr)
{
    mips64_jit_tcb_t *block = NULL;
    std_u32_t asid;

    if (!(block = mips64_jit_tcb_alloc(cpu)))
        goto err_block_alloc;

    block->start_pc = vaddr;

    int zone = (vaddr >> 29) & 0x7;
    if ((zone == 0x4) || (zone == 0x5)) {

    } else {
        asid = mod_vm_arch_mips_cp0_mfc_op(p_global_vm_arch_mips_cp0, MIPS_CP0_TLB_HI, 0) & MIPS_TLB_ASID_MASK;
        block->asid = asid;
    }

    /* Allocate the first JIT buffer */
    if (!(block->jit_buffer = exec_page_alloc(cpu)))
        goto err_jit_alloc;


    block->jit_ptr = block->jit_buffer->ptr;
    block->mips_code = mod_vm_memory_lookup(p_global_vm_memory, block->start_pc);

    if (!block->mips_code) {
        goto err_lookup;
    }

    return block;

err_lookup:
err_jit_alloc:
    mips64_jit_tcb_free(cpu, block, FALSE);
err_block_alloc:
    fprintf(stderr, "%% Unable to create instruction block for vaddr=0x%x\n",
            vaddr);
    return NULL;
}
#endif


/* Compile a MIPS instruction page */
static forced_inline mips64_jit_tcb_t *mips64_jit_tcb_compile(cvm_arch_mips_cpu_t *cpu, m_va_t vaddr)
{
    mips64_jit_tcb_t *block;
    std_u64_t page_addr;
    size_t len;

    page_addr = vaddr & ~(std_u64_t) MIPS_MIN_PAGE_IMASK;

    if (unlikely(!(block = mips64_jit_tcb_create(cpu, page_addr)))) {
        fprintf(stderr, "insn_page_compile: unable to create JIT block.\n");
        return NULL;
    }

    /* Allocate the array used to convert MIPS code ptr to native code ptr */
    len = MIPS_MIN_PAGE_SIZE / sizeof(mips_insn_t);

    if (!(block->jit_insn_ptr = calloc(len, sizeof(u_char *)))) {
        fprintf(stderr, "insn_page_compile: unable to create JIT mappings.\n");
        goto error;
    }

    /* Emit native code for each instruction */
    block->mips_trans_pos = 0;

    while (block->mips_trans_pos < MIPS_INSN_PER_PAGE) {
        if (unlikely((mips64_jit_fetch_and_emit(cpu, block, 0) == -1))) {
            fprintf(stderr, "insn_page_compile: unable to fetch instruction.\n");
            goto error;
        }

        mips64_jit_tcb_adjust_buffer(cpu, block);
    }

    mips64_jit_tcb_add_end(block);
    mips64_jit_tcb_apply_patches(cpu, block);
    mips64_jit_tcb_free_patches(block);

    /* Add the block to the linked list */
    block->next = cpu->tcb_list;
    block->prev = NULL;

    if (cpu->tcb_list)
        cpu->tcb_list->prev = block;
    else
        cpu->tcb_last = block;

    cpu->tcb_list = block;

    cpu->compiled_pages++;
    return block;

error:
    mips64_jit_tcb_free(cpu, block, FALSE);
    return NULL;
}

/* Run a compiled MIPS instruction block */
static forced_inline void mips64_jit_tcb_run(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *block)
{
    /* Execute JIT compiled code */
    mips64_jit_tcb_exec(cpu, block);
}


/**
 * jit_fetch_and_exec
 * @brief   
 * @param   cpu
 * @return  std_void_t
 */
std_void_t jit_fetch_and_exec(cvm_arch_mips_cpu_t *cpu)
{
    std_u32_t pc_hash;
    mips64_jit_tcb_t *block;

    setjmp((cpu)->exec_loop_env);

    cpu->gpr[0] = 0;

    pc_hash = mips64_jit_get_pc_hash(cpu, cpu->pc);
    block = cpu->exec_blk_map[pc_hash];

    /* No block found, compile the page */
    if (unlikely(!block) || unlikely(!mips64_jit_tcb_match(cpu, block, cpu->pc))) {

        if (block != NULL) {
            mips64_jit_tcb_free(cpu, block, TRUE);
            cpu->exec_blk_map[pc_hash] = NULL;
        }

        block = mips64_jit_tcb_compile(cpu, cpu->pc);
        if (unlikely(!block)) {
            fprintf(stderr,
                    "VM : unable to compile block for CPU PC=0x%lx\n",
                    cpu->pc);
            return;
        }
        block->acc_count++;
        cpu->exec_blk_map[pc_hash] = block;
    }
    mips64_jit_tcb_run(cpu, block);
}


#endif
