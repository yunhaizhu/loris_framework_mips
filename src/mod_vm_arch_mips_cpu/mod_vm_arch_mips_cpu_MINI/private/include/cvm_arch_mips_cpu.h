/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    cvm_arch_mips_cpu.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#ifndef MIPS_CPU_H
#define MIPS_CPU_H

#include <setjmp.h>
#include "std_common.h"
#include "mod_vm_memory.h"
#include "mod_vm_arch_mips_cp0.h"

extern mod_vm_memory_t *p_global_vm_memory;

/* 
 * MIPS General Purpose Registers 
 */
#define MIPS_GPR_ZERO        0  /*  zero  */
#define MIPS_GPR_AT          1  /*  at  */
#define MIPS_GPR_V0          2  /*  v0  */
#define MIPS_GPR_V1          3  /*  v1  */
#define MIPS_GPR_A0          4  /*  a0  */
#define MIPS_GPR_A1          5  /*  a1  */
#define MIPS_GPR_A2          6  /*  a2  */
#define MIPS_GPR_A3          7  /*  a3  */
#define MIPS_GPR_T0          8  /*  t0  */
#define MIPS_GPR_T1          9  /*  t1  */
#define MIPS_GPR_T2          10 /*  t2  */
#define MIPS_GPR_T3          11 /*  t3  */
#define MIPS_GPR_T4          12 /*  t4  */
#define MIPS_GPR_T5          13 /*  t5  */
#define MIPS_GPR_T6          14 /*  t6  */
#define MIPS_GPR_T7          15 /*  t7  */
#define MIPS_GPR_S0          16 /*  s0  */
#define MIPS_GPR_S1          17 /*  s1  */
#define MIPS_GPR_S2          18 /*  s2  */
#define MIPS_GPR_S3          19 /*  s3  */
#define MIPS_GPR_S4          20 /*  s4  */
#define MIPS_GPR_S5          21 /*  s5  */
#define MIPS_GPR_S6          22 /*  s6  */
#define MIPS_GPR_S7          23 /*  s7  */
#define MIPS_GPR_T8          24 /*  t8  */
#define MIPS_GPR_T9          25 /*  t9  */
#define MIPS_GPR_K0          26 /*  k0  */
#define MIPS_GPR_K1          27 /*  k1  */
#define MIPS_GPR_GP          28 /*  gp  */
#define MIPS_GPR_SP          29 /*  sp  */
#define MIPS_GPR_FP          30 /*  fp  */
#define MIPS_GPR_RA          31 /*  ra  */

/* Number of GPR (general purpose registers) */
#define MIPS64_GPR_NR  32

/* Memory operations */
enum
{
   MIPS_MEMOP_LOOKUP = 0,

   MIPS_MEMOP_LB,
   MIPS_MEMOP_LBU,
   MIPS_MEMOP_LH,
   MIPS_MEMOP_LHU,
   MIPS_MEMOP_LW,
   MIPS_MEMOP_LWU,
   MIPS_MEMOP_LD,
   MIPS_MEMOP_SB,
   MIPS_MEMOP_SH,
   MIPS_MEMOP_SW,
   MIPS_MEMOP_SD,

   MIPS_MEMOP_LWL,
   MIPS_MEMOP_LWR,
   MIPS_MEMOP_LDL,
   MIPS_MEMOP_LDR,
   MIPS_MEMOP_SWL,
   MIPS_MEMOP_SWR,
   MIPS_MEMOP_SDL,
   MIPS_MEMOP_SDR,

   MIPS_MEMOP_LL,
   MIPS_MEMOP_SC,

   MIPS_MEMOP_LDC1,
   MIPS_MEMOP_SDC1,

   MIPS_MEMOP_CACHE,
   MIPS_MEMOP_MAX,
};

typedef struct cvm_arch_mips_cpu cvm_arch_mips_cpu_t;
typedef struct mips64_jit_tcb mips64_jit_tcb_t;
typedef struct insn_exec_page insn_exec_page_t;

/**
 * std_uint_t
 * @brief   
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  typedef fastcall
 */
typedef fastcall std_uint_t (*mips_memop_fn) (struct cvm_arch_mips_cpu *cpu, std_u32_t vaddr, std_uint_t reg);

/* Virtual CPU states */
enum
{
    CPU_STATE_RUNNING = 0,       /*cpu is running */
    CPU_STATE_HALTED,
    CPU_STATE_SUSPENDED,         /*CPU is SUSPENDED */
    CPU_STATE_RESTARTING,        /*cpu is restarting */
    CPU_STATE_PAUSING,           /*cpu is pausing for timer */
};

#define CPU_INTERRUPT_EXIT   0x01       /* wants exit from main loop */
#define CPU_INTERRUPT_HARD   0x02       /* hardware interrupt pending */
#define CPU_INTERRUPT_EXITTB 0x04       /* exit the current TB (use for x86 a20 case) */
#define CPU_INTERRUPT_TIMER  0x08       /* internal timer exception pending */
#define CPU_INTERRUPT_FIQ    0x10       /* Fast interrupt pending.  */
#define CPU_INTERRUPT_HALT   0x20       /* CPU halt wanted */
#define CPU_INTERRUPT_SMI    0x40       /* (x86 only) SMI interrupt pending */


/* mips CPU definition */
struct cvm_arch_mips_cpu{
    jmp_buf exec_loop_env;

	std_u32_t state;
    std_u32_t ll_bit;
    std_u64_t pc;
	std_u64_t gpr[MIPS64_GPR_NR];
	std_u64_t lo;
	std_u64_t hi;
    std_u64_t ret_pc;

    std_uint_t fast_memop;

	/* Address mode (32 or 64 bits) */
	std_uint_t addr_mode;
	std_int_t is_in_bdslot;


	/* Current exec page (non-JIT) info */
	std_u32_t njm_exec_page;
	std_u32_t *njm_exec_ptr;

	mips_memop_fn mem_op_fn[MIPS_MEMOP_MAX];

	/* "Idle" loop management */
	std_uint_t idle_count,idle_max,idle_sleep_time;
	pthread_mutex_t idle_mutex;
	pthread_cond_t idle_cond;
	
	std_u32_t idle_pc;

    /*pause request. INTERRUPT will pause cpu*/
    std_u32_t pause_request;

    std_u32_t cpu_thread_running;



#ifdef _USE_JIT_
    std_u64_t jit_pc;

    /* JIT flush method */
    u_int jit_flush_method;

    /* Number of compiled pages */
    u_int compiled_pages;

    /* Code page translation cache */
    mips64_jit_tcb_t **exec_blk_map;
    void *exec_page_area;
    size_t exec_page_area_size;  /*M bytes*/
    size_t exec_page_count,exec_page_alloc;
    insn_exec_page_t *exec_page_free_list;
    insn_exec_page_t *exec_page_array;
    /* Current and free lists of translated code blocks */
    mips64_jit_tcb_t *tcb_list,*tcb_last,*tcb_free_list;
    /* Direct block jump.Optimization */
    u_int exec_blk_direct_jump;

#endif
    union
    {
        struct mts32_entry *mts32_cache;
    } mts_u;

    std_u32_t *p_cp0_reg;
};




/**
 * mips64_exec_fetch
 * @brief   
 * @param   cpu
 * @param   pc
 * @param   insn
 * @return  forced_inline std_int_t
 */
forced_inline std_int_t mips64_exec_fetch(cvm_arch_mips_cpu_t *cpu, std_u32_t pc, std_u32_t *insn)
{
  std_u32_t exec_page;
  std_u32_t offset;

  exec_page = pc & ~(std_u32_t) MIPS_MIN_PAGE_IMASK;
  if (unlikely(exec_page != cpu->njm_exec_page)){
    cpu->njm_exec_ptr = mod_vm_memory_lookup(p_global_vm_memory, exec_page);
  }
  if (cpu->njm_exec_ptr == NULL) {
    //exception when fetching instruction
    return 1;
  }

  cpu->njm_exec_page = exec_page;
  offset = (pc & MIPS_MIN_PAGE_IMASK) >> 2;
  *insn = vmtoh32(cpu->njm_exec_ptr[offset]);

  return 0;
}

/**
 * mips64_exec_single_instruction
 * @brief   
 * @param   cpu
 * @param   instruction
 * @return  std_int_t fastcall
 */
std_int_t fastcall mips64_exec_single_instruction(cvm_arch_mips_cpu_t *cpu, std_u32_t instruction);

/**
 * cpu_idle_break_wait
 * @brief   
 * @param   cpu
 * @return  std_void_t
 */
std_void_t cpu_idle_break_wait(cvm_arch_mips_cpu_t *cpu);

#endif
