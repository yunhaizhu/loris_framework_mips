/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch_MIPS.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_ARCH_MIPS_H
#define MOD_VM_ARCH_MIPS_H

#include "mod_shell.h"
#include "mod_vm_arch.h"

typedef struct mod_vm_arch_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_arch_ops_st *p_ops;
    mod_shell_t *p_mod_shell;

    std_int_t ARCH_STATUS;
} mod_vm_arch_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_arch_MIPS_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_MIPS_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_arch_MIPS_start
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_MIPS_start(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_arch_MIPS_stop
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_MIPS_stop(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_arch_MIPS_suspend
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_MIPS_suspend(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_arch_MIPS_resume
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_arch_MIPS_resume(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_arch_MIPS_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_arch_MIPS_create_instance(INOUT std_void_t **pp_handle);

#endif
