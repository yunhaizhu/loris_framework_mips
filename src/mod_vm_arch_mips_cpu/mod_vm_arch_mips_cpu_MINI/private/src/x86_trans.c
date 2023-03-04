/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    x86_trans.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
//
// Created by yun on 1/18/22.
//
#include "x86_trans.h"
#include "cvm_arch_mips_cpu.h"
#include "mips64_jit.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_memory.h"


#ifdef _USE_JIT_
struct mips64_jit_desc mips_jit[];
struct mips64_jit_desc mips_spec_jit[];
struct mips64_jit_desc mips_bcond_jit[];
struct mips64_jit_desc mips_cop0_jit[];
struct mips64_jit_desc mips_mad_jit[];
struct mips64_jit_desc mips_tlb_jit[];

extern mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;

std_int_t fastcall mips64_exec_single_instruction(cvm_arch_mips_cpu_t *cpu, mips_insn_t instruction);

#define REG_OFFSET(reg) (OFFSET(cvm_arch_mips_cpu_t, gpr[(reg)]))
#define MEMOP_OFFSET(op) (OFFSET(cvm_arch_mips_cpu_t, mem_op_fn[(op)]))

/* Load a 64 bit immediate value */
std_void_t mips64_load_imm(mips64_jit_tcb_t *b, std_uint_t reg,
                     std_u64_t value)
{
    if (value > 0xffffffffULL)
        amd64_mov_reg_imm_size(b->jit_ptr, reg, value, 8);
    else
        amd64_mov_reg_imm(b->jit_ptr, reg, value);
}

/* Set the Pointer Counter (PC) register */
std_void_t mips64_set_pc(mips64_jit_tcb_t *b, m_va_t new_pc)
{
    mips64_load_imm(b, AMD64_RAX, new_pc);
    amd64_mov_membase_reg(b->jit_ptr,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, pc),
                          AMD64_RAX, 8);
}

/* Set the Return Address (RA) register */
std_void_t mips64_set_ra(mips64_jit_tcb_t *b, m_va_t ret_pc)
{
    mips64_load_imm(b, AMD64_RAX, ret_pc);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15,
                          REG_OFFSET(MIPS_GPR_RA),
                          AMD64_RAX, 8);
}

/*
 * Try to branch directly to the specified JIT block without returning to
 * main loop.
 */
static std_void_t mips64_try_direct_far_jump(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *b,
                                       m_va_t new_pc)
{
    std_u64_t new_page;
    std_u32_t pc_hash, pc_offset;
    u_char *test1, *test2, *test3;

    new_page = new_pc & MIPS_MIN_PAGE_MASK;
    pc_offset = (new_pc & MIPS_MIN_PAGE_IMASK) >> 2;
    pc_hash = mips64_jit_get_pc_hash(cpu, new_pc);

    /* Get JIT block info in %rdx */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RBX,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, exec_blk_map), 8);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX,
                          AMD64_RBX, pc_hash * sizeof(std_void_t *), 8);

    /* no JIT block found ? */
    amd64_test_reg_reg(b->jit_ptr, AMD64_RDX, AMD64_RDX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_Z, 0, 1);

    /* Check block IA */
    mips64_load_imm(b, AMD64_RAX, new_page);
    amd64_alu_reg_membase_size(b->jit_ptr, X86_CMP, X86_EAX, AMD64_RDX,
                               OFFSET(mips64_jit_tcb_t, start_pc), 4);
    test2 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_NE, 0, 1);

    /* Jump to the code */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RSI,
                          AMD64_RDX, OFFSET(mips64_jit_tcb_t, jit_insn_ptr), 8);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RBX,
                          AMD64_RSI, pc_offset * sizeof(std_void_t *), 8);

    amd64_test_reg_reg(b->jit_ptr, AMD64_RBX, AMD64_RBX);
    test3 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_Z, 0, 1);
    amd64_jump_reg(b->jit_ptr, AMD64_RBX);

    /* Returns to caller... */
    amd64_patch(test1, b->jit_ptr);
    amd64_patch(test2, b->jit_ptr);
    amd64_patch(test3, b->jit_ptr);

    mips64_set_pc(b, new_pc);
    mips64_jit_tcb_push_epilog(b);
}

/* Set Jump */
static std_void_t mips64_set_jump(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *b,
                            m_va_t new_pc, std_int_t local_jump)
{
    std_int_t return_to_caller = FALSE;
    u_char *jump_ptr;

    if (!local_jump)
        return_to_caller = TRUE;

    if (!return_to_caller && mips64_jit_tcb_local_addr(b, new_pc, &jump_ptr)) {
        if (jump_ptr) {
            amd64_jump_code(b->jit_ptr, jump_ptr);
        } else {
            /* Never jump directly to code in a delay slot */
            if (mips64_jit_is_delay_slot(b, new_pc)) {
                mips64_set_pc(b, new_pc);
                mips64_jit_tcb_push_epilog(b);
                return;
            }

            mips64_jit_tcb_record_patch(b, b->jit_ptr, new_pc);
            amd64_jump32(b->jit_ptr, 0);
        }
    } else {
        if (cpu->exec_blk_direct_jump) {
            /* Block lookup optimization */
            mips64_try_direct_far_jump(cpu, b, new_pc);
        } else {
            mips64_set_pc(b, new_pc);
            mips64_jit_tcb_push_epilog(b);
        }
    }
}

/* Basic C call */
std_void_t forced_inline mips64_emit_basic_c_call(mips64_jit_tcb_t *b, std_void_t *f)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RCX, f);
    amd64_call_reg(b->jit_ptr, AMD64_RCX);
}

/* Emit a simple call to a C function without any parameter */
static std_void_t mips64_emit_c_call(mips64_jit_tcb_t *b, std_void_t *f)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RCX, f);
    amd64_call_reg(b->jit_ptr, AMD64_RCX);
}

/* Single-step operation */
std_void_t mips64_emit_single_step(mips64_jit_tcb_t *b, mips_insn_t insn)
{
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    mips64_emit_basic_c_call(b, mips64_exec_single_instruction);
}

/* Fast memory operation prototype */
typedef std_void_t (*memop_fast_access)(mips64_jit_tcb_t *b, std_int_t target);

/* Fast LW */
STD_CALL static std_void_t mips64_memop_fast_lw(mips64_jit_tcb_t *b, std_int_t target)
{
    amd64_mov_reg_memindex(b->jit_ptr, AMD64_RAX, AMD64_RBX, 0, AMD64_RSI, 0, 4);
//    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RDX, X86_EAX);

    /* Save value in register */
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(target), AMD64_RAX, 8);
}

/* Fast SW */
STD_CALL static std_void_t mips64_memop_fast_sw(mips64_jit_tcb_t *b, std_int_t target)
{
    /* Load value from register */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(target), 4);
    amd64_mov_memindex_reg(b->jit_ptr, AMD64_RBX, 0, AMD64_RSI, 0, AMD64_RAX, 4);
}


static std_void_t mips64_emit_memop_fast32(mips64_jit_tcb_t *b, std_int_t write_op,
                                     std_int_t opcode, std_int_t base, std_int_t offset,
                                     std_int_t target, std_int_t keep_ll_bit,
                                     memop_fast_access op_handler)
{
    std_u32_t val = sign_extend(offset, 16);
    u_char *test1, *p_exit;

    /* ESI = GPR[base] + sign-extended offset */
    amd64_mov_reg_imm(b->jit_ptr, X86_ESI, val);
    amd64_alu_reg_membase_size(b->jit_ptr, X86_ADD,
                               X86_ESI, AMD64_R15, REG_OFFSET(base), 4);

    /* RBX = mts32_entry index */
    amd64_mov_reg_reg_size(b->jit_ptr, X86_EBX, X86_ESI, 4);
    amd64_shift_reg_imm_size(b->jit_ptr, X86_SHR, X86_EBX, MTS32_HASH_SHIFT, 4);
    amd64_alu_reg_imm_size(b->jit_ptr, X86_AND, X86_EBX, MTS32_HASH_MASK, 4);

    /* RCX = mts32 entry */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX,
                          AMD64_R15,
                          OFFSET(cvm_arch_mips_cpu_t, mts_u.mts32_cache), 8);
    amd64_shift_reg_imm(b->jit_ptr, X86_SHL, AMD64_RBX, 5); /* TO FIX */
    amd64_alu_reg_reg(b->jit_ptr, X86_ADD, AMD64_RCX, AMD64_RBX);

    /* Compare virtual page address (EAX = vpage) */
    amd64_mov_reg_reg(b->jit_ptr, X86_EAX, X86_ESI, 4);
    amd64_alu_reg_imm(b->jit_ptr, X86_AND, X86_EAX, MIPS_MIN_PAGE_MASK);

    amd64_alu_reg_membase_size(b->jit_ptr, X86_CMP, X86_EAX, AMD64_RCX,
                               OFFSET(mts32_entry_t, gvpa), 4);
    test1 = b->jit_ptr;
    x86_branch8(b->jit_ptr, X86_CC_NZ, 0, 1);


    /* ESI = offset in page, RBX = Host Page Address */
    amd64_alu_reg_imm(b->jit_ptr, X86_AND, X86_ESI, MIPS_MIN_PAGE_IMASK);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RBX,
                          AMD64_RCX, OFFSET(mts32_entry_t, hpa), 8);

    /* Memory access */
    op_handler(b, target);

    p_exit = b->jit_ptr;
    amd64_jump8(b->jit_ptr, 0);

    /* === Slow lookup === */
    amd64_patch(test1, b->jit_ptr);

    /* Save PC for exception handling */
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));

    /* Sign-extend virtual address */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RSI, X86_ESI);

    /* RDX = target register */
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RDX, target);

    /* RDI = CPU instance */
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);

    /* Call memory access function */
    amd64_call_membase(b->jit_ptr, AMD64_R15, MEMOP_OFFSET(opcode));

    amd64_patch(p_exit, b->jit_ptr);
}


/* Fast memory operation */
STD_CALL static std_void_t mips64_emit_memop_fast(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *b,
                                            std_int_t write_op, std_int_t opcode,
                                            std_int_t base, std_int_t offset,
                                            std_int_t target, std_int_t keep_ll_bit,
                                            memop_fast_access op_handler)
{
    switch (cpu->addr_mode) {
        case 32:
                mips64_emit_memop_fast32(b, write_op, opcode, base, offset, target,
                                         keep_ll_bit, op_handler);

            break;
        default:
            break;
    }
}


/* Memory operation */
/*we use EAX EDX ECX to transfer parameter. yajin.
Makesure memory operation DONOT have more than 3 parameters*/
static std_void_t mips64_emit_memop(mips64_jit_tcb_t *b, std_int_t op, std_int_t base, std_int_t offset,
                              std_int_t target, std_int_t keep_ll_bit)
{
    std_u64_t val = sign_extend(offset, 16);

    /* Save PC for exception handling */
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));

    /* RDI = CPU instance */
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);

    if (!keep_ll_bit) {
        amd64_clear_reg(b->jit_ptr, AMD64_RCX);
        amd64_mov_membase_reg(b->jit_ptr, AMD64_RDI, OFFSET(cvm_arch_mips_cpu_t, ll_bit),
                              X86_ECX, 4);
    }

    /* RSI = GPR[base] + sign-extended offset */
    mips64_load_imm(b, AMD64_RSI, val);
    amd64_alu_reg_membase(b->jit_ptr, X86_ADD,
                          AMD64_RSI, AMD64_RDI, REG_OFFSET(base));

    /* RDX = target register */
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RDX, target);

    /* Call memory access function */
    amd64_call_membase(b->jit_ptr, AMD64_RDI, MEMOP_OFFSET(op));
}


/* Unknown opcode handler */
static asmlinkage std_void_t mips64_unknown_opcode(cvm_arch_mips_cpu_t *cpu, std_u32_t opcode)
{
    printf("MIPS64: unhandled opcode 0x%8.8x at 0x%" LL "x (ra=0x%" LL "x)\n",
           opcode, cpu->pc, cpu->gpr[MIPS_GPR_RA]);

    //mips64_dump_regs(cpu);
}

/* Emit unhandled instruction code */
static std_int_t mips64_emit_unknown(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *b, mips_insn_t opcode)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, opcode);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);

    mips64_emit_c_call(b, mips64_unknown_opcode);
    return 0;
}

/* Invalid delay slot handler */
static fastcall std_void_t mips64_invalid_delay_slot(cvm_arch_mips_cpu_t *cpu)
{
    printf("MIPS64: invalid instruction in delay slot at 0x%" LL "x (ra=0x%" LL "x)\n",
           cpu->pc, cpu->gpr[MIPS_GPR_RA]);

    //mips64_dump_regs(cpu);

    /* Halt the virtual CPU */
    cpu->pc = 0;
}

/* Emit unhandled instruction code */
std_int_t mips64_emit_invalid_delay_slot(mips64_jit_tcb_t *b)
{
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_c_call(b, mips64_invalid_delay_slot);
    return 0;
}




/* Trigger IRQs */
std_void_t fastcall mips64_trigger_irq()
{
    if (mod_vm_arch_mips_cp0_irq_op(p_global_vm_arch_mips_cp0, OP_IRQ_UPDATE, 0)) {
        mod_vm_arch_mips_cp0_trigger_exception(p_global_vm_arch_mips_cp0, MIPS_CP0_CAUSE_INTERRUPT, 0);
    }
}

/* Check if there are pending IRQ */
std_void_t mips64_check_pending_irq(mips64_jit_tcb_t *b)
{
    std_uchar_t *test1;

    /* Check the pending IRQ flag */
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, p_global_vm_arch_mips_cp0);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX,
                          AMD64_RSI, OFFSET(mod_vm_arch_mips_cp0_t, irq_pending), 4);

    amd64_test_reg_reg_size(b->jit_ptr, AMD64_RAX, AMD64_RAX, 4);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_Z, 0, 1);

    /* Update PC */
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));

    /* Trigger the IRQ */
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_trigger_irq);
    mips64_jit_tcb_push_epilog(b);

    amd64_patch(test1, b->jit_ptr);
}


/**
 * mips_emit_add
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_add(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    /* TODO: Exception handling */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_ADD, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/* ADDI */
static std_int_t mips_emit_addi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);

    /* TODO: Exception handling */

    mips64_load_imm(b, AMD64_RAX, val);
    amd64_alu_reg_membase(b->jit_ptr, X86_ADD, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rs));

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RAX, 8);
    return 0;
}

/* ADDIU */
static std_int_t mips_emit_addiu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);

    mips64_load_imm(b, AMD64_RAX, val);

    if (rs != 0) {
        amd64_alu_reg_membase(b->jit_ptr, X86_ADD, AMD64_RAX,
                              AMD64_R15, REG_OFFSET(rs));
    }

//    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RDX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RAX, 8);
    return 0;
}

/* ADDu */
static std_int_t mips_emit_addu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_ADD, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}


/* AND */
static std_int_t mips_emit_and(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_AND, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/* ANDI */
static std_int_t mips_emit_andi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);

    mips64_load_imm(b, AMD64_RAX, imm);

    amd64_alu_reg_membase(b->jit_ptr, X86_AND, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rs));

    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_bcond
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_bcond(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    uint16_t special_func = bits(insn, 16, 20);
    return mips_bcond_jit[special_func].emit_func(cpu, b, insn);
}



/* BEQ (Branch On Equal) */
static std_int_t mips_emit_beq(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /*
    * compare gpr[rs] and gpr[rt].
    */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_CMP, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rt));
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_NE, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BEQL (Branch On Equal Likely) */
static std_int_t mips_emit_beql(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /*
    * compare gpr[rs] and gpr[rt].
    */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_CMP, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rt));
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_NE, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/* BGEZ (Branch On Greater or Equal Than Zero) */
static std_int_t mips_emit_bgez(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* If sign bit is set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_S, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BGEZAL (Branch On Greater or Equal Than Zero And Link) */
static std_int_t mips_emit_bgezal(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    mips64_set_ra(b, b->start_pc + ((b->mips_trans_pos + 1) << 2));

    /* If sign bit is set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_S, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BGEZALL (Branch On Greater or Equal Than Zero and Link Likely) */
static std_int_t mips_emit_bgezall(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    mips64_set_ra(b, b->start_pc + ((b->mips_trans_pos + 1) << 2));

    /* If sign bit is set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_S, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/* BGEZL (Branch On Greater or Equal Than Zero Likely) */
static std_int_t mips_emit_bgezl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* If sign bit is set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_S, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/* BGTZ (Branch On Greater Than Zero) */
static std_int_t mips_emit_bgtz(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* compare reg to zero */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RCX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_LE, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}


/* BGTZL (Branch On Greater Than Zero Likely) */
static std_int_t mips_emit_bgtzl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* compare reg to zero */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RCX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_LE, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/* BLEZ (Branch On Less or Equal Than Zero) */
static std_int_t mips_emit_blez(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* compare reg to zero */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RCX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_GT, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BLEZL (Branch On Less or Equal Than Zero Likely) */
static std_int_t mips_emit_blezl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* compare reg to zero */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RCX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_GT, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}


/* BLTZ (Branch On Less Than Zero) */
static std_int_t mips_emit_bltz(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* If sign bit isn't set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_NS, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BLTZAL (Branch On Less Than Zero And Link) */
static std_int_t mips_emit_bltzal(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    mips64_set_ra(b, b->start_pc + ((b->mips_trans_pos + 1) << 2));

    /* If sign bit isn't set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_NS, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BLTZALL (Branch On Less Than Zero And Link Likely) */
static std_int_t mips_emit_bltzall(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    mips64_set_ra(b, b->start_pc + ((b->mips_trans_pos + 1) << 2));

    /* If sign bit isn't set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_NS, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}


/* BLTZL (Branch On Less Than Zero Likely) */
static std_int_t mips_emit_bltzl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /* If sign bit isn't set, don't take the branch */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RAX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_NS, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}


/* BNE (Branch On Not Equal) */
static std_int_t mips_emit_bne(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /*
    * compare gpr[rs] and gpr[rt].
    */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_CMP, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rt));
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_E, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 2);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);

    /* if the branch is not taken, we have to execute the delay slot too */
    mips64_jit_fetch_and_emit(cpu, b, 1);
    return 0;
}

/* BNEL (Branch On Not Equal Likely) */
static std_int_t mips_emit_bnel(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);
    u_char *test1;
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc += sign_extend(offset << 2, 18);

    /*
    * compare gpr[rs] and gpr[rt].
    */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_CMP, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rt));
    test1 = b->jit_ptr;
    amd64_branch32(b->jit_ptr, X86_CC_E, 0, 1);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/* Execute BREAK instruction */
std_void_t fastcall mips64_exec_break(IN cvm_arch_mips_cpu_t *cpu, IN std_uint_t code)
{
    //    mips64_dump_regs(cpu);
    printf("exec break cpu->pc %x\n", cpu->pc);

    /* XXX : Branch Delay slot */
    mod_vm_arch_mips_cp0_trigger_exception(p_global_vm_arch_mips_cp0, MIPS_CP0_CAUSE_BP, 0);
}
/* BREAK */
static std_int_t mips_emit_break(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_uint_t code = bits(insn, 6, 25);

    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, code);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_break);
    x86_alu_reg_imm(b->jit_ptr, X86_ADD, AMD64_RSP, 12);
    mips64_jit_tcb_push_epilog(b);
    return 0;
}

/* CACHE */
static std_int_t mips_emit_cache(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t op = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_CACHE, base, offset, op, FALSE);
    return 0;
}

/**
 * mips_emit_cfc0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_cfc0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * clz_emu
 * @brief
 * @param   cpu
 * @param   insn
 * @return  static std_int_t fastcall
 */
static std_int_t fastcall clz_emu(cvm_arch_mips_cpu_t *cpu, mips_insn_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rd = bits(insn, 11, 15);
    std_int_t i;
    std_u32_t val;
    val = 32;
    for (i = 31; i >= 0; i--) {
        if (cpu->gpr[rs] & (1 << i)) {
            val = 31 - i;
            break;
        }
    }
    cpu->gpr[rd] = val;
    return 0;
}

/**
 * mips_emit_clz
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_clz(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);

    mips64_emit_basic_c_call(b, clz_emu);
    return 0;
}

/**
 * mips_emit_cop0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_cop0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    uint16_t special_func = bits(insn, 21, 25);
    return mips_cop0_jit[special_func].emit_func(cpu, b, insn);
}

fastcall std_int_t mips64_exec_COP1(cvm_arch_mips_cpu_t *cpu, std_u32_t insn);
/**
 * mips_emit_cop1
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_cop1(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
#if SOFT_FPU
    /* Save PC for exception handling */
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_COP1);
    mips64_jit_tcb_push_epilog(b);
    return 0;
#else
    mips64_emit_unknown(cpu, b, insn);
    return 0;
#endif
}
/**
 * mips_emit_cop1x
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_cop1x(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
#if SOFT_FPU
    mips64_emit_unknown(cpu, b, insn);
    return 0;
#endif
}

/**
 * mips_emit_cop2
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_cop2(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dadd
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dadd(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_daddi
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_daddi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_daddiu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_daddiu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_daddu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_daddu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_ddiv
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ddiv(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_ddivu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ddivu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_div
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_div(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);

    /* eax = gpr[rs] */
    amd64_clear_reg(b->jit_ptr, AMD64_RDX);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 4);

    /* ecx = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rt), 4);

    /* eax = quotient (LO), edx = remainder (HI) */
    amd64_div_reg_size(b->jit_ptr, AMD64_RCX, 1, 4);

    /* store LO */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, lo),
                          AMD64_RAX, 8);

    /* store HI */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RDX, X86_EDX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, hi),
                          AMD64_RDX, 8);
    return 0;
}


/**
 * mips_emit_divu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_divu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);

    /* eax = gpr[rs] */
    amd64_clear_reg(b->jit_ptr, AMD64_RDX);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 4);

    /* ecx = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rt), 4);

    /* eax = quotient (LO), edx = remainder (HI) */
    amd64_div_reg_size(b->jit_ptr, AMD64_RCX, 0, 4);

    /* store LO */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, lo),
                          AMD64_RAX, 8);

    /* store HI */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RDX, X86_EDX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, hi),
                          AMD64_RDX, 8);
    return 0;
}
/**
 * mips_emit_dmfc0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dmfc0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dmtc0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dmtc0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dmult
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dmult(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dmultu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dmultu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsll
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsll(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}
/**
 * mips_emit_dsllv
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsllv(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsrlv
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsrlv(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsrav
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsrav(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsub
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsub(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsubu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsubu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsrl
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsrl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsra
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsra(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsll32
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsll32(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsrl32
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsrl32(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_dsra32
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_dsra32(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

fastcall std_int_t mips64_exec_ERET(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
/**
 * mips_emit_eret
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_eret(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_ERET);
    mips64_jit_tcb_push_epilog(b);

    return 0;
}

/* J (Jump) */
static std_int_t mips_emit_j(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_uint_t instr_index = bits(insn, 0, 25);
    m_va_t new_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc &= ~((1 << 28) - 1);
    new_pc |= instr_index << 2;

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 1);
    return 0;
}

/**
 * mips_emit_jal
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_jal(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_uint_t instr_index = bits(insn, 0, 25);
    m_va_t new_pc, ret_pc;

    /* compute the new pc */
    new_pc = b->start_pc + (b->mips_trans_pos << 2);
    new_pc &= ~((1 << 28) - 1);
    new_pc |= instr_index << 2;

    /* set the return address (instruction after the delay slot) */
    ret_pc = b->start_pc + ((b->mips_trans_pos + 1) << 2);
    mips64_set_ra(b, ret_pc);

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc in cpu structure */
    mips64_set_jump(cpu, b, new_pc, 0);
    return 0;
}

/**
 * mips_emit_jalr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_jalr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rd = bits(insn, 11, 15);
    std_u64_t ret_pc;

    /* set the return pc (instruction after the delay slot) in GPR[rd] */
    ret_pc = b->start_pc + ((b->mips_trans_pos + 1) << 2);
    mips64_load_imm(b, AMD64_RAX, ret_pc);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);

    /* get the new pc */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_R14, AMD64_R15, REG_OFFSET(rs), 8);

#if DEBUG_JR0
    {
        u_char *test1;

        amd64_test_reg_reg(b->jit_ptr, AMD64_R14, AMD64_R14);
        test1 = b->jit_ptr;
        amd64_branch8(b->jit_ptr, X86_CC_NZ, 0, 1);
        amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
        mips64_emit_c_call(b, mips64_debug_jr0);
        amd64_patch(test1, b->jit_ptr);
    }
#endif

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc */
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, pc),
                          AMD64_R14, 8);

    /* returns to the caller which will determine the next path */
    mips64_jit_tcb_push_epilog(b);
    return 0;
}

/**
 * mips_emit_jr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_jr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);

    /* get the new pc */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_R14, AMD64_R15, REG_OFFSET(rs), 8);

#if DEBUG_JR0
    {
        u_char *test1;

        amd64_test_reg_reg(b->jit_ptr, AMD64_RCX, AMD64_RCX);
        test1 = b->jit_ptr;
        amd64_branch8(b->jit_ptr, X86_CC_NZ, 0, 1);
        amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
        mips64_emit_c_call(b, mips64_debug_jr0);
        amd64_patch(test1, b->jit_ptr);
    }
#endif

    /* insert the instruction in the delay slot */
    mips64_jit_fetch_and_emit(cpu, b, 1);

    /* set the new pc */
    amd64_mov_membase_reg(b->jit_ptr,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, pc),
                          AMD64_R14, 8);

    /* returns to the caller which will determine the next path */
    mips64_jit_tcb_push_epilog(b);
    return 0;
}

/**
 * mips_emit_lb
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lb(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LB, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_lbu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lbu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LBU, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_ld
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ld(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_ldc1
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ldc1(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_ldc2
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ldc2(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_ldl
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ldl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_ldr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ldr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_lh
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lh(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LH, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_lhu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lhu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LHU, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_ll
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ll(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LL, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_lld
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lld(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_lui
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lui(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16) << 16;

#if 1
    mips64_load_imm(b, AMD64_RCX, val);
#else
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RCX, imm);
    amd64_shift_reg_imm(b->jit_ptr, X86_SHL, AMD64_RCX, 48);
    amd64_shift_reg_imm(b->jit_ptr, X86_SAR, AMD64_RCX, 32);
#endif

    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RCX, 8);
    return 0;
}

/**
 * mips_emit_lw
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lw(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    if (cpu->fast_memop) {
        mips64_emit_memop_fast(cpu, b, 0, MIPS_MEMOP_LW, base, offset, rt, TRUE,
                               mips64_memop_fast_lw);
    } else {
        mips64_emit_memop(b, MIPS_MEMOP_LW, base, offset, rt, TRUE);
    }
    return 0;
}

/**
 * mips_emit_lwc1
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lwc1(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
#if SOFT_FPU

    mips64_emit_unknown(cpu, b, insn);
    return 0;
#endif
}

/**
 * mips_emit_lwc2
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lwc2(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_lwl
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lwl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LWL, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_lwr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lwr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LWR, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_lwu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_lwu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_LWU, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_mad
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mad(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t index = bits(insn, 0, 5);
    return mips_mad_jit[index].emit_func(cpu, b, insn);
}

/**
 * madd_emu
 * @brief
 * @param   cpu
 * @param   insn
 * @return  static std_int_t fastcall
 */
static std_int_t fastcall madd_emu(cvm_arch_mips_cpu_t *cpu, mips_insn_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_64_t val, temp;

    val = (std_32_t) cpu->gpr[rs];
    val *= (std_32_t) cpu->gpr[rt];

    temp = cpu->hi;
    temp = temp << 32;
    temp += cpu->lo;
    val += temp;

    cpu->lo = sign_extend(val, 32);
    cpu->hi = sign_extend(val >> 32, 32);
    return 0;
}

/**
 * mips_emit_madd
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_madd(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, madd_emu);
    return 0;
}


/**
 * maddu_emu
 * @brief
 * @param   cpu
 * @param   insn
 * @return  static std_int_t fastcall
 */
static std_int_t fastcall maddu_emu(cvm_arch_mips_cpu_t *cpu, mips_insn_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_64_t val, temp;

    val = (std_u32_t) (std_u32_t) cpu->gpr[rs];
    val *= (std_u32_t) (std_u32_t) cpu->gpr[rt];

    temp = cpu->hi;
    temp = temp << 32;
    temp += cpu->lo;
    val += temp;

    cpu->lo = sign_extend(val, 32);
    cpu->hi = sign_extend(val >> 32, 32);
    return 0;
}

/**
 * mips_emit_maddu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_maddu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, maddu_emu);
    return 0;
}

fastcall std_int_t mips64_exec_MFC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
/**
 * mips_emit_mfc0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mfc0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);

    mips64_emit_basic_c_call(b, mips64_exec_MFC0);
    return 0;
}

/**
 * mips_emit_mfhi
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mfhi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, hi), 8);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RDX, 8);
    return 0;
}

/**
 * mips_emit_mflo
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mflo(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rd = bits(insn, 11, 15);

    if (!rd) return 0;

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, lo), 8);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RDX, 8);
    return 0;
}

/**
 * mips_emit_movc
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_movc(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_movz
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_movz(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rd = bits(insn, 11, 15);
    std_int_t rt = bits(insn, 16, 20);
    u_char *test1;

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rt), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RDX, AMD64_RDX);
    /*goto end if !=0*/
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_NZ, 0, 1);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RDX, 8);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/**
 * mips_emit_movn
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_movn(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rd = bits(insn, 11, 15);
    std_int_t rt = bits(insn, 16, 20);
    u_char *test1;

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rt), 8);
    amd64_test_reg_reg(b->jit_ptr, AMD64_RDX, AMD64_RDX);
    /*goto end if ==0*/
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_Z, 0, 1);
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RDX, 8);

    amd64_patch(test1, b->jit_ptr);
    return 0;
}


/**
 * msub_emu
 * @brief
 * @param   cpu
 * @param   insn
 * @return  static std_int_t fastcall
 */
static std_int_t fastcall msub_emu(cvm_arch_mips_cpu_t *cpu, mips_insn_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_64_t val, temp;

    val = (std_32_t) cpu->gpr[rs];
    val *= (std_32_t) cpu->gpr[rt];

    temp = cpu->hi;
    temp = temp << 32;
    temp += cpu->lo;

    temp -= val;
    //val += temp;

    cpu->lo = sign_extend(temp, 32);
    cpu->hi = sign_extend(temp >> 32, 32);
    return 0;
}

/**
 * mips_emit_msub
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_msub(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, msub_emu);
    return 0;
}


/**
 * msubu_emu
 * @brief
 * @param   cpu
 * @param   insn
 * @return  static std_int_t fastcall
 */
static std_int_t fastcall msubu_emu(cvm_arch_mips_cpu_t *cpu, mips_insn_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_64_t val;
    std_64_t temp;

    val = (std_u32_t) (std_u32_t) cpu->gpr[rs];
    val *= (std_u32_t) (std_u32_t) cpu->gpr[rt];

    temp = cpu->hi;
    temp = temp << 32;
    temp += cpu->lo;

    temp -= val;

    cpu->lo = sign_extend(temp, 32);
    cpu->hi = sign_extend(temp >> 32, 32);
    return 0;
}
/**
 * mips_emit_msubu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_msubu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, msubu_emu);
    return 0;
}

fastcall std_int_t mips64_exec_MTC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
/**
 * mips_emit_mtc0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mtc0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_MTC0);
    return 0;
}


/**
 * mips_emit_mthi
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mthi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);

    amd64_mov_membase_reg(b->jit_ptr,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, hi), AMD64_RDX, 8);
    return 0;
}
/**
 * mips_emit_mtlo
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mtlo(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);

    amd64_mov_membase_reg(b->jit_ptr,
                          AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, lo), AMD64_RDX, 8);
    return 0;
}

/* MUL */
static std_int_t mips_emit_mul(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    /* eax = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 4);

    /* ecx = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rt), 4);

    amd64_mul_reg_size(b->jit_ptr, AMD64_RCX, 1, 4);

    /* store result in gpr[rd] */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_mult
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_mult(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);

    /* eax = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 4);

    /* ecx = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rt), 4);

    amd64_mul_reg_size(b->jit_ptr, AMD64_RCX, 1, 4);

    /* store LO */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, lo),
                          AMD64_RAX, 8);

    /* store HI */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RDX, X86_EDX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, hi),
                          AMD64_RDX, 8);
    return 0;
}
/**
 * mips_emit_multu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_multu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);

    /* eax = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 4);

    /* ecx = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rt), 4);

    amd64_mul_reg_size(b->jit_ptr, AMD64_RCX, 0, 4);

    /* store LO */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, lo),
                          AMD64_RAX, 8);

    /* store HI */
    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RDX, X86_EDX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, OFFSET(cvm_arch_mips_cpu_t, hi),
                          AMD64_RDX, 8);
    return 0;
}


/**
 * mips_emit_nor
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_nor(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_OR, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));
    amd64_not_reg(b->jit_ptr, AMD64_RAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_or
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_or(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_OR, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_ori
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_ori(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);

    mips64_load_imm(b, AMD64_RAX, imm);

    amd64_alu_reg_membase(b->jit_ptr, X86_OR, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rs));

    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_pref
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_pref(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    return 0;
}

/**
 * mips_emit_sb
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sb(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_SB, base, offset, rt, FALSE);
    return 0;
}

/**
 * mips_emit_sc
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sc(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_SC, base, offset, rt, TRUE);
    return 0;
}

/**
 * mips_emit_scd
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_scd(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_sd
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sd(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_sdc1
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sdc1(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
#if SOFT_FPU
    /* Save PC for exception handling */
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_COP1);
    mips64_jit_tcb_push_epilog(b);
    return 0;
#else
    mips64_emit_unknown(cpu, b, insn);
    return 0;
#endif
}

/**
 * mips_emit_sdc2
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sdc2(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_sdl
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sdl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_sdr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sdr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_sh
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sh(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_SH, base, offset, rt, FALSE);
    return 0;
}

/**
 * mips_emit_sll
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sll(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);
    std_int_t sa = bits(insn, 6, 10);

    if (rd != 0) {
        amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 4);
        amd64_shift_reg_imm(b->jit_ptr, X86_SHL, AMD64_RAX, sa);

        amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
        amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    }

    return 0;
}

/**
 * mips_emit_sllv
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sllv(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rs), 4);
    amd64_alu_reg_imm(b->jit_ptr, X86_AND, AMD64_RCX, 0x1f);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 4);
    amd64_shift_reg(b->jit_ptr, X86_SHL, AMD64_RAX);

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

#if 0
/**
 * mips_emit_slt
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_slt(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);
    u_char *test1;

    /* RDX = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);

    /* RAX = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 8);

    /* we set rd to 1 when gpr[rs] < gpr[rt] */
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RCX, 8);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RDX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_GE, 0, 1);

    amd64_inc_membase(b->jit_ptr, AMD64_R15, REG_OFFSET(rd));

    /* end */
    amd64_patch(test1, b->jit_ptr);
    return 0;
}
#else

/**
 * mips_emit_slt
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_slt(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);
    u_char *test1;

    /* RDX = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);

    /* RAX = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 8);

    amd64_mov_membase_imm(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), 0, 8);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RDX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_GE, 0, 1);

    amd64_mov_membase_imm(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), 1, 8);
    /* end */
    amd64_patch(test1, b->jit_ptr);

    return 0;
}
#endif

/**
 * mips_emit_slti
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_slti(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);
    u_char *test1;

    /* RDX = val */
    mips64_load_imm(b, AMD64_RDX, val);

    /* RAX = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);

    /* we set rt to 1 when gpr[rs] < val */
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RCX, 8);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RDX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_GE, 0, 1);

    amd64_inc_membase(b->jit_ptr, AMD64_R15, REG_OFFSET(rt));

    /* end */
    amd64_patch(test1, b->jit_ptr);
    return 0;
}


/**
 * mips_emit_sltiu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sltiu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);
    u_char *test1;

    /* RDX = val */
    mips64_load_imm(b, AMD64_RDX, val);

    /* RAX = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);

    /* we set rt to 1 when gpr[rs] < val */
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RCX, 8);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RDX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_AE, 0, 0);

    amd64_inc_membase(b->jit_ptr, AMD64_R15, REG_OFFSET(rt));

    /* end */
    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/**
 * mips_emit_sltu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sltu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);
    u_char *test1;

    /* RDX = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RDX, AMD64_R15, REG_OFFSET(rs), 8);

    /* RAX = gpr[rt] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 8);

    /* we set rd to 1 when gpr[rs] < gpr[rt] */
    amd64_clear_reg(b->jit_ptr, AMD64_RCX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RCX, 8);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RDX, AMD64_RAX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_AE, 0, 0);

    amd64_inc_membase(b->jit_ptr, AMD64_R15, REG_OFFSET(rd));

    /* end */
    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/**
 * mips_emit_spec
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_spec(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    uint16_t special_func = bits(insn, 0, 5);
    return mips_spec_jit[special_func].emit_func(cpu, b, insn);
}

/**
 * mips_emit_sra
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sra(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);
    std_int_t sa = bits(insn, 6, 10);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 4);
    amd64_shift_reg_imm_size(b->jit_ptr, X86_SAR, AMD64_RAX, sa, 4);

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}


/**
 * mips_emit_srav
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_srav(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rs), 4);
    amd64_alu_reg_imm(b->jit_ptr, X86_AND, AMD64_RCX, 0x1f);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 4);
    amd64_shift_reg_size(b->jit_ptr, X86_SAR, AMD64_RAX, 4);

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_srl
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_srl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);
    std_int_t sa = bits(insn, 6, 10);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 4);
    amd64_shift_reg_imm(b->jit_ptr, X86_SHR, AMD64_RAX, sa);

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_srlv
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_srlv(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RCX, AMD64_R15, REG_OFFSET(rs), 4);
    amd64_alu_reg_imm(b->jit_ptr, X86_AND, AMD64_RCX, 0x1f);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rt), 4);
    amd64_shift_reg(b->jit_ptr, X86_SHR, AMD64_RAX);

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}
/**
 * mips_emit_sub
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sub(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    /* TODO: Exception handling */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_SUB, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}
/**
 * mips_emit_subu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_subu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_SUB, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));

    amd64_movsxd_reg_reg(b->jit_ptr, AMD64_RAX, X86_EAX);
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_sw
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sw(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    if (cpu->fast_memop) {
        mips64_emit_memop_fast(cpu, b, 1, MIPS_MEMOP_SW, base, offset, rt, FALSE,
                               mips64_memop_fast_sw);
    } else {
        mips64_emit_memop(b, MIPS_MEMOP_SW, base, offset, rt, FALSE);
    }
    return 0;
}

/**
 * mips_emit_swc1
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_swc1(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
#if SOFT_FPU
    mips64_emit_unknown(cpu, b, insn);
    return 0;
#endif
}
/**
 * mips_emit_swc2
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_swc2(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_swl
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_swl(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_SWL, base, offset, rt, FALSE);
    return 0;
}
/**
 * mips_emit_swr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_swr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t base = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t offset = bits(insn, 0, 15);

    mips64_emit_memop(b, MIPS_MEMOP_SWR, base, offset, rt, FALSE);
    return 0;
}
/**
 * mips_emit_sync
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_sync(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    return 0;
}

std_void_t fastcall mips64_exec_syscall(cvm_arch_mips_cpu_t *cpu);
/**
 * mips_emit_syscall
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_syscall(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_syscall);
    mips64_jit_tcb_push_epilog(b);
    return 0;
}
std_void_t fastcall mips64_trigger_trap_exception(cvm_arch_mips_cpu_t *cpu);
/**
 * mips_emit_teq
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_teq(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    u_char *test1;

    /*
    * compare gpr[rs] and gpr[rt].
    */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_CMP, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rt));
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_NE, 0, 1);

    /* Generate trap exception */
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_c_call(b, mips64_trigger_trap_exception);
    mips64_jit_tcb_push_epilog(b);

    /* end */
    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/**
 * mips_emit_teqi
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_teqi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);
    u_char *test1;

    /* RDX = val */
    mips64_load_imm(b, AMD64_RDX, val);

    /* RAX = gpr[rs] */
    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);

    amd64_alu_reg_reg(b->jit_ptr, X86_CMP, AMD64_RAX, AMD64_RDX);
    test1 = b->jit_ptr;
    amd64_branch8(b->jit_ptr, X86_CC_NE, 0, 1);

    /* Generate trap exception */
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_c_call(b, mips64_trigger_trap_exception);
    mips64_jit_tcb_push_epilog(b);

    /* end */
    amd64_patch(test1, b->jit_ptr);
    return 0;
}

/**
 * mips_emit_tlb
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tlb(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    uint16_t func = bits(insn, 0, 5);
    return mips_tlb_jit[func].emit_func(cpu, b, insn);
}


fastcall std_int_t mips64_exec_TLBP(cvm_arch_mips_cpu_t *cpu, std_u32_t insn);
/**
 * mips_emit_tlbp
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tlbp(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_TLBP);
    return 0;
}

fastcall std_int_t mips64_exec_TLBR(cvm_arch_mips_cpu_t *cpu, std_u32_t insn);
/**
 * mips_emit_tlbr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  int
 */
std_int_t mips_emit_tlbr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_TLBR);
    return 0;
}
fastcall std_int_t mips64_exec_TLBWI(cvm_arch_mips_cpu_t *cpu, std_u32_t insn);
/**
 * mips_emit_tlbwi
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  int
 */
std_int_t mips_emit_tlbwi(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_TLBWI);
    return 0;
}

fastcall std_int_t mips64_exec_TLBWR(cvm_arch_mips_cpu_t *cpu, std_u32_t insn);
/**
 * mips_emit_tlbwr
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tlbwr(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_set_pc(b, b->start_pc + ((b->mips_trans_pos - 1) << 2));
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_TLBWR);
    return 0;
}


/**
 * mips_emit_tge
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tge(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_tgei
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tgei(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_tgeu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tgeu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_tgeiu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tgeiu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_tlt
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tlt(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_tlti
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tlti(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}
/**
 * mips_emit_tltiu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tltiu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}
/**
 * mips_emit_tltu
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tltu(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * tne_emu
 * @brief
 * @param   cpu
 * @param   insn
 * @return  static std_int_t fastcall
 */
static std_int_t fastcall tne_emu(cvm_arch_mips_cpu_t *cpu, mips_insn_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);

    if ((std_64_t) cpu->gpr[rs] != (std_64_t) cpu->gpr[rt]) {
        /*take a trap */
        mips64_trigger_trap_exception(cpu);
        return 1;
    } else
        return 0;
}

/**
 * mips_emit_tne
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tne(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, tne_emu);
    return 0;
}


/**
 * mips_emit_tnei
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_tnei(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}
fastcall std_int_t mips64_exec_WAIT(cvm_arch_mips_cpu_t *cpu, std_u32_t insn);
/**
 * mips_emit_wait
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_wait(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    amd64_mov_reg_imm(b->jit_ptr, AMD64_RSI, insn);
    amd64_mov_reg_reg(b->jit_ptr, AMD64_RDI, AMD64_R15, 8);
    mips64_emit_basic_c_call(b, mips64_exec_WAIT);
    return 0;
}
/**
 * mips_emit_xor
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_xor(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t rd = bits(insn, 11, 15);

    amd64_mov_reg_membase(b->jit_ptr, AMD64_RAX, AMD64_R15, REG_OFFSET(rs), 8);
    amd64_alu_reg_membase(b->jit_ptr, X86_XOR, AMD64_RAX, AMD64_R15,
                          REG_OFFSET(rt));
    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rd), AMD64_RAX, 8);
    return 0;
}

/**
 * mips_emit_xori
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_xori(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    std_int_t rs = bits(insn, 21, 25);
    std_int_t rt = bits(insn, 16, 20);
    std_int_t imm = bits(insn, 0, 15);

    mips64_load_imm(b, AMD64_RAX, imm);

    amd64_alu_reg_membase(b->jit_ptr, X86_XOR, AMD64_RAX,
                          AMD64_R15, REG_OFFSET(rs));

    amd64_mov_membase_reg(b->jit_ptr, AMD64_R15, REG_OFFSET(rt), AMD64_RAX, 8);
    return 0;
}


/**
 * mips_emit_undef
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_undef(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_unknownBcond
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_unknownBcond(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_unknowncop0
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_unknowncop0(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_unknownmad
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_unknownmad(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/**
 * mips_emit_unknownSpec
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_unknownSpec(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}

/**
 * mips_emit_unknowntlb
 * @brief
 * @param   cpu
 * @param   b
 * @param   insn
 * @return  static int
 */
static std_int_t mips_emit_unknowntlb(IN cvm_arch_mips_cpu_t *cpu, IN mips64_jit_tcb_t *b, IN std_uint_t insn)
{
    mips64_emit_unknown(cpu, b, insn);
    return 0;
}


/*instruction table*/

struct mips64_jit_desc mips_jit[] = {
        {"spec", mips_emit_spec, 0x00, 0x99},
        {"bcond", mips_emit_bcond, 0x01, 0x99},
        {"j", mips_emit_j, 0x02, 0x0, 11},
        {"jal", mips_emit_jal, 0x03, 0x0, 11},
        {"beq", mips_emit_beq, 0x04, 0x0, 8},
        {"bne", mips_emit_bne, 0x05, 0x0, 8},
        {"blez", mips_emit_blez, 0x06, 0x0, 9},
        {"bgtz", mips_emit_bgtz, 0x07, 0x0, 9},
        {"addi", mips_emit_addi, 0x08, 0x1, 6},
        {"addiu", mips_emit_addiu, 0x09, 0x1, 6},
        {"slti", mips_emit_slti, 0x0A, 0x1, 5},
        {"sltiu", mips_emit_sltiu, 0x0B, 0x1, 5},
        {"andi", mips_emit_andi, 0x0C, 0x1, 5},
        {"ori", mips_emit_ori, 0x0D, 0x1, 5},
        {"xori", mips_emit_xori, 0x0E, 0x1, 5},
        {"lui", mips_emit_lui, 0x0F, 0x1, 16},
        {"cop0", mips_emit_cop0, 0x10, 0x99},
        {"cop1", mips_emit_cop1, 0x11, 0x1},
        {"cop2", mips_emit_cop2, 0x12, 0x1},
        {"cop1x", mips_emit_cop1x, 0x13, 0x1},
        {"beql", mips_emit_beql, 0x14, 0x0, 8},
        {"bnel", mips_emit_bnel, 0x15, 0x0, 8},
        {"blezl", mips_emit_blezl, 0x16, 0x0, 9},
        {"bgtzl", mips_emit_bgtzl, 0x17, 0x0, 9},
        {"daddi", mips_emit_daddi, 0x18, 0x1},
        {"daddiu", mips_emit_daddiu, 0x19, 0x1, 5},
        {"ldl", mips_emit_ldl, 0x1A, 0x1, 2},
        {"ldr", mips_emit_ldr, 0x1B, 0x1, 2},
        {"undef", mips_emit_mad, 0x1C, 0x99},
        {"undef", mips_emit_undef, 0x1D, 0x1},
        {"undef", mips_emit_undef, 0x1E, 0x1},
        {"undef", mips_emit_undef, 0x1F, 0x1},
        {"lb", mips_emit_lb, 0x20, 0x1, 2},
        {"lh", mips_emit_lh, 0x21, 0x1, 2},
        {"lwl", mips_emit_lwl, 0x22, 0x1, 2},
        {"lw", mips_emit_lw, 0x23, 0x1, 2},
        {"lbu", mips_emit_lbu, 0x24, 0x1, 2},
        {"lhu", mips_emit_lhu, 0x25, 0x1, 2},
        {"lwr", mips_emit_lwr, 0x26, 0x1, 2},
        {"lwu", mips_emit_lwu, 0x27, 0x1, 2},
        {"sb", mips_emit_sb, 0x28, 0x1, 2},
        {"sh", mips_emit_sh, 0x29, 0x1, 2},
        {"swl", mips_emit_swl, 0x2A, 0x1, 2},
        {"sw", mips_emit_sw, 0x2B, 0x1, 2},
        {"sdl", mips_emit_sdl, 0x2C, 0x1, 2},
        {"sdr", mips_emit_sdr, 0x2D, 0x1, 2},
        {"swr", mips_emit_swr, 0x2E, 0x1, 2},
        {"cache", mips_emit_cache, 0x2F, 0x1, 2},
        {"ll", mips_emit_ll, 0x30, 0x1, 2},
        {"lwc1", mips_emit_lwc1, 0x31, 0x1},
        {"lwc2", mips_emit_lwc2, 0x32, 0x1},
        {"pref", mips_emit_pref, 0x33, 0x1},
        {"lld", mips_emit_lld, 0x34, 0x1},
        {"ldc1", mips_emit_ldc1, 0x35, 0x1, 2},
        {"ldc2", mips_emit_ldc2, 0x36, 0x1},
        {"ld", mips_emit_ld, 0x37, 0x1, 2},
        {"sc", mips_emit_sc, 0x38, 0x1, 2},
        {"swc1", mips_emit_swc1, 0x39, 0x1},
        {"swc2", mips_emit_swc2, 0x3A, 0x1},
        {"undef", mips_emit_undef, 0x3B, 0x1},
        {"scd", mips_emit_scd, 0x3C, 0x1},
        {"sdc1", mips_emit_sdc1, 0x3D, 0x1},
        {"sdc2", mips_emit_sdc2, 0x3E, 0x1},
        {"sd", mips_emit_sd, 0x3F, 0x1, 2},
};

/* Based on the func field of spec opcode */
struct mips64_jit_desc mips_spec_jit[] = {
        {"sll", mips_emit_sll, 0x00, 0x1, 7},
        {"movc", mips_emit_movc, 0x01, 0x1},
        {"srl", mips_emit_srl, 0x02, 0x1, 7},
        {"sra", mips_emit_sra, 0x03, 0x1, 7},
        {"sllv", mips_emit_sllv, 0x04, 0x1, 4},
        {"unknownSpec", mips_emit_unknownSpec, 0x05, 0x1},
        {"srlv", mips_emit_srlv, 0x06, 0x1, 4},
        {"srav", mips_emit_srav, 0x07, 0x1, 4},
        {"jr", mips_emit_jr, 0x08, 0x0, 13},
        {"jalr", mips_emit_jalr, 0x09, 0x0, 15},
        {"movz", mips_emit_movz, 0x0A, 0x1, 3},
        {"movn", mips_emit_movn, 0x0B, 0x1, 3},
        {"syscall", mips_emit_syscall, 0x0C, 0x1},
        {"break", mips_emit_break, 0x0D, 0x1},
        {"spim", mips_emit_unknownSpec, 0x0E, 0x1},
        {"sync", mips_emit_sync, 0x0F, 0x1},
        {"mfhi", mips_emit_mfhi, 0x10, 0x1, 14},
        {"mthi", mips_emit_mthi, 0x11, 0x1, 13},
        {"mflo", mips_emit_mflo, 0x12, 0x1, 14},
        {"mtlo", mips_emit_mtlo, 0x13, 0x1, 13},
        {"dsllv", mips_emit_dsllv, 0x14, 0x1},
        {"unknownSpec", mips_emit_unknownSpec, 0x15, 0x1},
        {"dsrlv", mips_emit_dsrlv, 0x16, 0x1},
        {"dsrav", mips_emit_dsrav, 0x17, 0x1},
        {"mult", mips_emit_mult, 0x18, 0x1, 17},
        {"multu", mips_emit_multu, 0x19, 0x1, 17},
        {"div", mips_emit_div, 0x1A, 0x1, 17},
        {"divu", mips_emit_divu, 0x1B, 0x1, 17},
        {"dmult", mips_emit_dmult, 0x1C, 0x1},
        {"dmultu", mips_emit_dmultu, 0x1D, 0x1},
        {"ddiv", mips_emit_ddiv, 0x1E, 0x1},
        {"ddivu", mips_emit_ddivu, 0x1F, 0x1},
        {"add", mips_emit_add, 0x20, 0x1, 3},
        {"addu", mips_emit_addu, 0x21, 0x1, 3},
        {"sub", mips_emit_sub, 0x22, 0x1, 3},
        {"subu", mips_emit_subu, 0x23, 0x1, 3},
        {"and", mips_emit_and, 0x24, 0x1, 3},
        {"or", mips_emit_or, 0x25, 0x1, 3},
        {"xor", mips_emit_xor, 0x26, 0x1, 3},
        {"nor", mips_emit_nor, 0x27, 0x1, 3},
        {"unknownSpec", mips_emit_unknownSpec, 0x28, 0x1},
        {"unknownSpec", mips_emit_unknownSpec, 0x29, 0x1},
        {"slt", mips_emit_slt, 0x2A, 0x1, 3},
        {"sltu", mips_emit_sltu, 0x2B, 0x1, 3},
        {"dadd", mips_emit_dadd, 0x2C, 0x1},
        {"daddu", mips_emit_daddu, 0x2D, 0x1},
        {"dsub", mips_emit_dsub, 0x2E, 0x1},
        {"dsubu", mips_emit_dsubu, 0x2F, 0x1},
        {"tge", mips_emit_tge, 0x30, 0x1},
        {"tgeu", mips_emit_tgeu, 0x31, 0x1},
        {"tlt", mips_emit_tlt, 0x32, 0x1},
        {"tltu", mips_emit_tltu, 0x33, 0x1},
        {"teq", mips_emit_teq, 0x34, 0x1},
        {"unknownSpec", mips_emit_unknownSpec, 0x35, 0x1},
        {"tne", mips_emit_tne, 0x36, 0x1},
        {"unknownSpec", mips_emit_unknownSpec, 0x37, 0x1},
        {"dsll", mips_emit_dsll, 0x38, 0x1},
        {"unknownSpec", mips_emit_unknownSpec, 0x39, 0x1},
        {"dsrl", mips_emit_dsrl, 0x3A, 0x1},
        {"dsra", mips_emit_dsra, 0x3B, 0x1},
        {"dsll32", mips_emit_dsll32, 0x3C, 0x1},
        {"unknownSpec", mips_emit_unknownSpec, 0x3D, 0x1},
        {"dsrl32", mips_emit_dsrl32, 0x3E, 0x1},
        {"dsra32", mips_emit_dsra32, 0x3F, 0x1}};


/* Based on the rt field of bcond opcodes */
struct mips64_jit_desc mips_bcond_jit[] = {
        {"bltz", mips_emit_bltz, 0x00, 0x0, 9},
        {"bgez", mips_emit_bgez, 0x01, 0x0, 9},
        {"bltzl", mips_emit_bltzl, 0x02, 0x0, 9},
        {"bgezl", mips_emit_bgezl, 0x03, 0x0, 9},
        {"spimi", mips_emit_unknownBcond, 0x04, 0x0},
        {"unknownBcond", mips_emit_unknownBcond, 0x05, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x06, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x07, 0x1},
        {"tgei", mips_emit_tgei, 0x08, 0x1},
        {"tgeiu", mips_emit_tgeiu, 0x09, 0x1},
        {"tlti", mips_emit_tlti, 0x0A, 0x1},
        {"tltiu", mips_emit_tltiu, 0x0B, 0x1},
        {"teqi", mips_emit_teqi, 0x0C, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x0D, 0x1},
        {"tnei", mips_emit_tnei, 0x0E, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x0F, 0x1},
        {"bltzal", mips_emit_bltzal, 0x10, 0x0},
        {"bgezal", mips_emit_bgezal, 0x11, 0x0},
        {"bltzall", mips_emit_bltzall, 0x12, 0x0},
        {"bgezall", mips_emit_bgezall, 0x13, 0x0},
        {"unknownBcond", mips_emit_unknownBcond, 0x14, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x15, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x16, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x17, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x18, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x19, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x1A, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x1B, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x1C, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x1D, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x1E, 0x1},
        {"unknownBcond", mips_emit_unknownBcond, 0x1F, 0x1}};


struct mips64_jit_desc mips_cop0_jit[] = {
        {"mfc0", mips_emit_mfc0, 0x0, 0x1, 18},
        {"dmfc0", mips_emit_dmfc0, 0x1, 0x1},
        {"cfc0", mips_emit_cfc0, 0x2, 0x1, 18},
        {"unknowncop0", mips_emit_unknowncop0, 0x3, 0x1},
        {"mtc0", mips_emit_mtc0, 0x4, 0x1, 18},
        {"dmtc0", mips_emit_dmtc0, 0x5, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x6, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x7, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x8, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x9, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0xa, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0xb, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0xc, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0xd, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0xe, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0xf, 0x1},
        {"tlb", mips_emit_tlb, 0x10, 0x1, 1},
        {"unknowncop0", mips_emit_unknowncop0, 0x11, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x12, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x13, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x14, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x15, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x16, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x17, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x18, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x19, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x1a, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x1b, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x1c, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x1d, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x1e, 0x1},
        {"unknowncop0", mips_emit_unknowncop0, 0x1f, 0x1},

};


struct mips64_jit_desc mips_mad_jit[] = {
        {"mad", mips_emit_madd, 0x0, 0x1},
        {"maddu", mips_emit_maddu, 0x1, 0x1},
        {"mul", mips_emit_mul, 0x2, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3, 0x1},
        {"msub", mips_emit_msub, 0x4, 0x1},
        {"msubu", mips_emit_msubu, 0x5, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x6, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x7, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x8, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x9, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0xa, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0xb, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0xc, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0xd, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0xe, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0xf, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x10, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x11, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x12, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x13, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x14, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x15, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x16, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x17, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x18, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x19, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x1a, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x1b, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x1c, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x1d, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x1e, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x1f, 0x1},
        {"clz", mips_emit_clz, 0x20, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x21, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x22, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x23, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x24, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x25, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x26, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x27, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x28, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x29, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x2a, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x2b, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x2c, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x2d, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x2e, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x2f, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x30, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x31, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x32, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x33, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x34, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x35, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x36, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x37, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x38, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x39, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3a, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3b, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3c, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3d, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3e, 0x1},
        {"unknownmad_op", mips_emit_unknownmad, 0x3f, 0x1},

};


struct mips64_jit_desc mips_tlb_jit[] = {
        {"unknowntlb_op", mips_emit_unknowntlb, 0x0, 0x1},
        {"tlbr", mips_emit_tlbr, 0x1, 0x1, 1},
        {"tlbwi", mips_emit_tlbwi, 0x2, 0x1, 1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x4, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x5, 0x1},
        {"tlbwi", mips_emit_tlbwr, 0x6, 0x1, 1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x7, 0x1},
        {"tlbp", mips_emit_tlbp, 0x8, 0x1, 1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x9, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0xa, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0xb, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0xc, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0xd, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0xe, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0xf, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x10, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x11, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x12, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x13, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x14, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x15, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x16, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x17, 0x1},
        {"eret", mips_emit_eret, 0x18, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x19, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x1a, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x1b, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x1c, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x1d, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x1e, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x1f, 0x1},
        {"wait", mips_emit_wait, 0x20, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x21, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x22, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x23, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x24, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x25, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x26, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x27, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x28, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x29, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x2a, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x2b, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x2c, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x2d, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x2e, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x2f, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x30, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x31, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x32, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x33, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x34, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x35, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x36, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x37, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x38, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x39, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3a, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3b, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3c, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3d, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3e, 0x1},
        {"unknowntlb_op", mips_emit_unknowntlb, 0x3f, 0x1},

};

#endif