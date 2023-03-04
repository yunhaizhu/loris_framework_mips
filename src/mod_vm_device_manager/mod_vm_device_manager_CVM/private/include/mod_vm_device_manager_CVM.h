/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_manager_CVM.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_MANAGER_CVM_H
#define MOD_VM_DEVICE_MANAGER_CVM_H

#include "mod_shell.h"
#include "mod_vm_device_manager.h"
#include "std_lock_free_list.h"

#define DEVICE_MAX 1 << 6
typedef struct vm_device_manger_CVM_list_st{
    mod_vm_device_t *p_dev;
    std_u32_t base;
    std_u32_t len;
    std_lock_free_list_head_t list;
}vm_device_manger_CVM_list_t;

typedef struct mod_vm_device_manager_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_manager_ops_st *p_ops;
    mod_shell_t *p_mod_shell;
    std_lock_free_list_head_t device_list_head;
    mod_vm_device_t *dev_array[DEVICE_MAX];
} mod_vm_device_manager_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_manager_CVM_add
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_manager_CVM_add(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_manager_CVM_del
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_manager_CVM_del(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_manager_CVM_find
 * @brief
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_manager_CVM_find(IN std_void_t * p_handle, IN std_char_t * params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_manager_CVM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_manager_CVM_create_instance(INOUT std_void_t **pp_handle);

#endif
