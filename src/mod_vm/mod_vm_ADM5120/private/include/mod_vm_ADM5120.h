/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_ADM5120.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_ADM5120_H
#define MOD_VM_ADM5120_H

#include "mod_shell.h"
#include "mod_vm.h"
#include "vm.h"

#define ADM5120_PCI_BASE 0x11400000
#define ADM5120_MPMC_BASE 0x11000000
#define ADM5120_INTC_BASE 0x12200000
#define ADM5120_UART0_BASE 0x12600000
#define ADM5120_SWCTRL_BASE 0x12000000

#define PCI_INDEX_MAX 0X80000//0X200000/4
#define MPMC_INDEX_MAX 0x9f  //0x027C/4
#define INTCTRL_INDEX_MAX 0xa//0x028/4
#define SW_INDEX_MAX 0x45    //0x0114/4

/*FOR CS8900*/
#define CS8900_IO_BASE 0x12300000
#define CS8900_SIZE 0x10
#define CS8900_DEFAULT_IRQ 107
#define CS8900_GPIO_GROUP 1

#define JZ4740_GPIO_BASE 0x10010000
#define JZ4740_GPIO_SIZE 0x388

typedef struct vm_adm5120_info_s {
    //com_vm_cpu_t    *com_vm_cpu;
    //com_vm_device_t *com_vm_device[32];
    //com_vm_memory_t *com_vm_memory;
} vm_adm5120_info_t;
typedef struct mod_vm_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_ops_st *p_ops;
    mod_shell_t *p_mod_shell;

    vm_system_info_t vm_sys;
    vm_adm5120_info_t adm5120_info;
    std_char_t configure_file[BUF_SIZE_128];
} mod_vm_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_ADM5120_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_ADM5120_start
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_start(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_ADM5120_stop
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_stop(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_ADM5120_suspend
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_suspend(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_ADM5120_resume
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_resume(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_ADM5120_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_ADM5120_create_instance(INOUT std_void_t **pp_handle);

#endif
