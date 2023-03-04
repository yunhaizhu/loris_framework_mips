/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    mod_vm_memory_m32.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */

#ifndef MOD_VM_MEMORY_M32_H
#define MOD_VM_MEMORY_M32_H

#include "mod_shell.h"

#include "cvm_memory_32.h"
#include "mod_vm_memory.h"

typedef struct mod_vm_memory_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_memory_ops_st *p_ops;

    /* MTS32/MTS64 caches */
    union {
        struct mts32_entry *mts32_cache;
    } mts_u;

    std_u64_t mts_misses;
    std_u64_t mts_lookups;

    mod_shell_t *p_mod_shell;
} mod_vm_memory_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_memory_m32_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_memory_m32_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_memory_m32_lookup
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_memory_m32_lookup(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_memory_m32_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_memory_m32_access(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_memory_m32_map
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_memory_m32_map(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_memory_m32_unmap
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_memory_m32_unmap(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_memory_m32_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_memory_m32_create_instance(INOUT std_void_t **pp_handle);

#endif
