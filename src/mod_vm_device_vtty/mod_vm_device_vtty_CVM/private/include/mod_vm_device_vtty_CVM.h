/**
* Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
*
* see COPYRIGHT file.
*/

/**
 * @file    mod_vm_device_vtty_CVM.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_VTTY_CVM_H
#define MOD_VM_DEVICE_VTTY_CVM_H

#include "mod_shell.h"
#include "mod_vm_device_vtty.h"

typedef struct mod_vm_device_vtty_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_vtty_ops_st *p_ops;
    mod_shell_t *p_mod_shell;
} mod_vm_device_vtty_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_vtty_CVM_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_initiate(IN std_void_t *p_handle,
                                                                       IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_vtty_CVM_create
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_create(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_vtty_CVM_get_char
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_get_char(IN std_void_t *p_handle,
                                                                       IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_vtty_CVM_put_char
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_put_char(IN std_void_t *p_handle,
                                                                       IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_vtty_CVM_put_buffer
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_put_buffer(IN std_void_t *p_handle,
                                                                         IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_vtty_CVM_is_char_avail
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_is_char_avail(IN std_void_t *p_handle,
                                                                            IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_vtty_CVM_is_full
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_is_full(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_vtty_CVM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_vtty_CVM_create_instance(INOUT std_void_t **pp_handle);

#endif
