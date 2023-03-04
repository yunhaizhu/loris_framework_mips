/*
 * Cisco router simulation platform.
 * Copyright (c) 2005,2006 Christophe Fillot (cf@utc.fr)
 */
/*
 * Copyright (C) yajin 2008 <yajinzhou@gmail.com >
 *     
 * This file is part of the virtualmips distribution. 
 * See LICENSE file for terms of the license. 
 *
 */

#ifndef __MIPS64_X86_TRANS_H__
#define __MIPS64_X86_TRANS_H__

#include "amd64-codegen.h"
#include "mips64_jit.h"
#include "std_common.h"
#ifdef _USE_JIT_


/* Manipulate bitmasks atomically */
static forced_inline void x86_atomic_or(std_u32_t *v, std_u32_t m)
{
    __asm__ __volatile__("lock; orl %1,%0"
                         : "=m"(*v)
                         : "ir"(m), "m"(*v));
}

/**
 * x86_atomic_and
 * @brief   
 * @param   v
 * @param   m
 * @return  static forced_inline void
 */
static forced_inline void x86_atomic_and(std_u32_t *v, std_u32_t m)
{
    __asm__ __volatile__("lock; andl %1,%0"
                         : "=m"(*v)
                         : "ir"(m), "m"(*v));
}

/* Wrappers to x86-codegen functions */
#define mips64_jit_tcb_set_patch amd64_patch
#define mips64_jit_tcb_set_jump amd64_jump_code


/* Push epilog for an x86 instruction block */
static forced_inline void mips64_jit_tcb_push_epilog(mips64_jit_tcb_t *block)
{
    amd64_ret(block->jit_ptr);
}

/* Translated block function pointer */
typedef void (*insn_tblock_fptr)(void);
/**
 * mips64_exec_single_instruction
 * @brief   
 * @param   cpu
 * @param   instruction
 * @return  std_int_t fastcall
 */
std_int_t fastcall mips64_exec_single_instruction(cvm_arch_mips_cpu_t *cpu, mips_insn_t instruction);

extern mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;

/* Execute JIT code */
static forced_inline void mips64_jit_tcb_exec(cvm_arch_mips_cpu_t *cpu, mips64_jit_tcb_t *block)
{
    insn_tblock_fptr jit_code;
    std_u32_t offset;

    offset = (cpu->pc & MIPS_MIN_PAGE_IMASK) >> 2;
    jit_code = (insn_tblock_fptr) block->jit_insn_ptr[offset];

    if (unlikely(!jit_code)) {
        int res = mips64_exec_single_instruction(cpu, vmtoh32(block->mips_code[offset]));
        if (!res) {
            cpu->pc += 4;
        }
        return;
    }

    asm volatile(
            "movq %0,%%r15\n"
            :
            : "r"(cpu)
            : "r13", "r14", "r15", "rax", "rbx", "rcx", "rdx", "rdi", "rsi");
    jit_code();
}

/**
 * mips64_set_pc
 * @brief   
 * @param   b
 * @param   new_pc
 */
void mips64_set_pc(mips64_jit_tcb_t *b, m_va_t new_pc);
/**
 * mips64_emit_single_step
 * @brief   
 * @param   b
 * @param   insn
 */
void mips64_emit_single_step(mips64_jit_tcb_t *b, mips_insn_t insn);
/**
 * mips64_check_cpu_pausing
 * @brief   
 * @param   b
 */
void mips64_check_cpu_pausing(mips64_jit_tcb_t *b);
/**
 * mips64_check_pending_irq
 * @brief   
 * @param   b
 */
void mips64_check_pending_irq(mips64_jit_tcb_t *b);

/**
 * amd64_patch
 * @brief   
 * @param   code
 * @param   target
 * @return  static inline void
 */
static inline void amd64_patch(u_char *code, u_char *target)
{
    /* Skip REX */
    if ((code[0] >= 0x40) && (code[0] <= 0x4f))
        code += 1;

    if ((code[0] & 0xf8) == 0xb8) {
        /* amd64_set_reg_template */
        *(std_u64_t *) (code + 1) = (std_u64_t) target;
    } else if (code[0] == 0x8b) {
        /* mov 0(%rip), %dreg */
        *(std_u32_t *) (code + 2) = (std_u32_t) (std_u64_t) target - 7;
    } else if ((code[0] == 0xff) && (code[1] == 0x15)) {
        /* call *<OFFSET>(%rip) */
        *(std_u32_t *) (code + 2) = ((std_u32_t) (std_u64_t) target) - 7;
    } else
        x86_patch(code, target);
}

#endif


#endif
