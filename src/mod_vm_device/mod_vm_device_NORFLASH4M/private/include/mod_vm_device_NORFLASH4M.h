/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_NORFLASH4M.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_NORFLASH4M_H
#define MOD_VM_DEVICE_NORFLASH4M_H

#include "mod_shell.h"
#include "mod_vm_device.h"

/*FLASH DEVICE NOR 4M */
#define ROM_INIT_STATE              0
#define BPTR(d,offset) (((char *)d->flash_ptr) + offset)

/* flash private data */
typedef struct flash_data
{
    std_u8_t *flash_ptr;
    std_u32_t flash_size;
    std_u8_t *flash_file_name;
    std_u32_t state;
    std_int_t flash_fd;
} flash_data_t;

typedef struct mod_vm_device_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;
    vm_device_info_t info;

    mod_shell_t *p_mod_shell;
    flash_data_t flash;
} mod_vm_device_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_NORFLASH4M_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_initiate(IN std_void_t *p_handle,
                                                                         IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_NORFLASH4M_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_reset(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_NORFLASH4M_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_access(IN std_void_t *p_handle,
                                                                       IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_NORFLASH4M_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_command(IN std_void_t *p_handle,
                                                                        IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_NORFLASH4M_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_NORFLASH4M_create_instance(INOUT std_void_t **pp_handle);

#endif
