/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch_mips_cpu.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_ARCH_MIPS_CPU_H
#define MOD_VM_ARCH_MIPS_CPU_H

#include <setjmp.h>
#include "mod.h"
#include "mod_ownership.h"
#include "std_common.h"

/*****************************************************
 *                                                   *
 *                      Define                       *
 *                                                   *
 ****************************************************/

/***struct_define***/

/***macro_define***/

/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/

typedef struct mod_vm_arch_mips_cpu_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_arch_mips_cpu_ops_st *p_ops;

    jmp_buf exec_loop_env;
} mod_vm_arch_mips_cpu_t;

struct mod_vm_arch_mips_cpu_ops_st {
    std_int_t (*init)(IN mod_vm_arch_mips_cpu_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_arch_mips_cpu_t *m);

    /***func_define***/
    std_void_t (*reset)(IN mod_vm_arch_mips_cpu_t *m, IN std_uint_t entry);
    std_uint_t (*fetch)(IN mod_vm_arch_mips_cpu_t *m, IN std_uint_t *insn);
    std_void_t (*exec)(IN mod_vm_arch_mips_cpu_t *m, IN std_uint_t insn);
    std_uint_t (*set_pc)(IN mod_vm_arch_mips_cpu_t *m, IN std_uint_t pc);
    std_uint_t (*set_llbit)(IN mod_vm_arch_mips_cpu_t *m, IN std_uint_t llbit);
    std_uint_t (*is_in_bdslot)(IN mod_vm_arch_mips_cpu_t *m);
    std_void_t (*break_idle)(IN mod_vm_arch_mips_cpu_t *m);

    std_void_t (*run)(IN mod_vm_arch_mips_cpu_t *m);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_ARCH_MIPS_CPU_IID MOD_GUID(0xbaee1ebe, 0x1bb0, 0x85f4, 0xa6, 0xb8, 0xac, 0x02, 0x47, 0x56, 0x72, 0xb6)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_arch_mips_cpu_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_arch_mips_cpu_t *) (m), arg, arg_len))
#define mod_vm_arch_mips_cpu_cleanup(m) ((m)->p_ops->cleanup((mod_vm_arch_mips_cpu_t *) (m)))

/***interface_define***/
#define mod_vm_arch_mips_cpu_reset(m, entry) ((m)->p_ops->reset((mod_vm_arch_mips_cpu_t *) (m), entry))
#define mod_vm_arch_mips_cpu_fetch(m, insn) ((m)->p_ops->fetch((mod_vm_arch_mips_cpu_t *) (m), insn))
#define mod_vm_arch_mips_cpu_exec(m, insn) ((m)->p_ops->exec((mod_vm_arch_mips_cpu_t *) (m), insn))
#define mod_vm_arch_mips_cpu_set_pc(m, pc) ((m)->p_ops->set_pc((mod_vm_arch_mips_cpu_t *) (m), pc))
#define mod_vm_arch_mips_cpu_set_llbit(m, llbit) ((m)->p_ops->set_llbit((mod_vm_arch_mips_cpu_t *) (m), llbit))
#define mod_vm_arch_mips_cpu_is_in_bdslot(m) ((m)->p_ops->is_in_bdslot((mod_vm_arch_mips_cpu_t *) (m)))
#define mod_vm_arch_mips_cpu_break_idle(m) ((m)->p_ops->break_idle((mod_vm_arch_mips_cpu_t *) (m)))
#define mod_vm_arch_mips_cpu_run(m) ((m)->p_ops->run((mod_vm_arch_mips_cpu_t *) (m)))

#endif
