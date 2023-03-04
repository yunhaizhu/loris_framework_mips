/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    mod_vm_arch_mips_cpu_MINI.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-03-01
 *
 */
#ifndef MOD_VM_ARCH_MIPS_CPU_MINI_H
#define MOD_VM_ARCH_MIPS_CPU_MINI_H

#include "mod_shell.h"
#include "mod_vm_arch_mips_cpu.h"
#include "cvm_arch_mips_cpu.h"

typedef struct mod_vm_arch_mips_cpu_imp_st {
	mod_ownership_t ownership;
	std_u64_t unique_id;
	struct mod_vm_arch_mips_cpu_ops_st *p_ops;

    cvm_arch_mips_cpu_t cpu;

	mod_shell_t *p_mod_shell;
} mod_vm_arch_mips_cpu_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_reset(IN std_void_t * p_handle,
								       IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_fetch
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_fetch(IN std_void_t * p_handle,
								       IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_exec
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_exec(IN std_void_t * p_handle, IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_set_pc
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_set_pc(IN std_void_t * p_handle,
									IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_set_llbit
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_set_llbit(IN std_void_t * p_handle,
									   IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_is_in_bdslot
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_is_in_bdslot(IN std_void_t * p_handle,
									      IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_break_idle
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_break_idle(IN std_void_t * p_handle,
									    IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_run
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_run(IN std_void_t * p_handle, IN std_char_t * params);

/****rpc_service_interface*****/

/**
 * mod_vm_arch_mips_cpu_MINI_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_arch_mips_cpu_MINI_create_instance(INOUT std_void_t ** pp_handle);

#endif
