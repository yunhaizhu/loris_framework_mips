/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_SW.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_SW_H
#define MOD_VM_DEVICE_SW_H

#include "mod_shell.h"
#include "mod_vm_device.h"

#define SW_INDEX_MAX			0x45    //0x0114/4

/* Timer_int_REG */
#define SW_TIMER_INT_DISABLE		0x10000
#define SW_TIMER_INT			0x1

/* Timer_REG */
#define SW_TIMER_EN			0x10000
#define SW_TIMER_MASK			0xffff
#define SW_TIMER_10MS_TICKS		0x3D09
#define SW_TIMER_1MS_TICKS		0x61A
#define SW_TIMER_100US_TICKS		0x9D

#define CODE_REG			0x0000

// Timer Control
#define Timer_int_REG			0x00F0
#define Timer_REG			0x00F4

typedef struct sw_data {
    std_u8_t *sw_ptr;
    std_u32_t sw_size;
}sw_data_t;

typedef struct mod_vm_device_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;
    vm_device_info_t info;

    mod_shell_t *p_mod_shell;
    std_u32_t sw_table[SW_INDEX_MAX];
	sw_data_t sw_data;
} mod_vm_device_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_SW_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_SW_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_SW_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_SW_reset(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_SW_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_SW_access(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_SW_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_SW_command(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_SW_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_SW_create_instance(INOUT std_void_t **pp_handle);

#endif
