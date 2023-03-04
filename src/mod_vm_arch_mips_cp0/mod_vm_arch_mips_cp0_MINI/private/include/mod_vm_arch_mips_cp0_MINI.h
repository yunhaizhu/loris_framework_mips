/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    mod_vm_arch_mips_cp0_MINI.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-03-01
 *
 */
#ifndef MOD_VM_ARCH_MIPS_CP0_MINI_H
#define MOD_VM_ARCH_MIPS_CP0_MINI_H

#include "mod_shell.h"
#include "mod_vm_arch_mips_cp0.h"
#include "cvm_arch_mips_cp0.h"

typedef struct mod_vm_arch_mips_cp0_imp_st {
	mod_ownership_t ownership;
	std_u64_t unique_id;
	struct mod_vm_arch_mips_cp0_ops_st *p_ops;

    cvm_arch_mips_cp0_t cp0;

	mod_shell_t *p_mod_shell;
} mod_vm_arch_mips_cp0_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_reset(IN std_void_t * p_handle,
								       IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_trigger_exception
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_trigger_exception(IN std_void_t * p_handle,
										   IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_op(IN std_void_t * p_handle,
									IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_lookup
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_lookup(IN std_void_t * p_handle,
									    IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_mtc_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_mtc_op(IN std_void_t * p_handle,
									IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_mfc_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_mfc_op(IN std_void_t * p_handle,
									IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_access_special
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_access_special(IN std_void_t * p_handle,
										IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_irq_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_irq_op(IN std_void_t * p_handle,
									IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_timer
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_timer(IN std_void_t * p_handle,
								       IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_eret
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_eret(IN std_void_t * p_handle, IN std_char_t * params);

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_soft_fpu
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_soft_fpu(IN std_void_t * p_handle,
									  IN std_char_t * params);

/****rpc_service_interface*****/

/**
 * mod_vm_arch_mips_cp0_MINI_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_arch_mips_cp0_MINI_create_instance(INOUT std_void_t ** pp_handle);

#endif
