/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_INT.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_INT_H
#define MOD_VM_DEVICE_INT_H

#include "mod_shell.h"
#include "mod_vm_device.h"

typedef struct int_data {
    std_u8_t *int_ptr;
    std_u32_t int_size;
} int_data_t;

#define INTCTRL_INDEX_MAX 0xa//0x028/4


/*==========================  Interrupt Controller  ==========================*/
/* registers offset */
#define IRQ_STATUS_REG 0x00     /* Read */
#define IRQ_RAW_STATUS_REG 0x04 /* Read */
#define IRQ_ENABLE_REG 0x08     /* Read/Write */
#define IRQ_DISABLE_REG 0x0C    /* Write */
#define IRQ_SOFT_REG 0x10       /* Write */

#define IRQ_MODE_REG 0x14   /* Read/Write */
#define FIQ_STATUS_REG 0x18 /* Read */

/* test registers */
#define IRQ_TESTSRC_REG 0x1c /* Read/Write */
#define IRQ_SRCSEL_REG 0x20  /* Read/Write */
#define IRQ_LEVEL_REG 0x24   /* Read/Write */

/* interrupt levels */
#define INT_LVL_TIMER 0   /* Timer */
#define INT_LVL_UART0 1   /* Uart 0 */
#define INT_LVL_UART1 2   /* Uart 1 */
#define INT_LVL_USBHOST 3 /* USB Host */
#define INT_LVL_EXTIO_0 4 /* External I/O 0 */
#define INT_LVL_EXTIO_1 5 /* External I/O 1 */
#define INT_LVL_PCI_0 6   /* PCI 0 */
#define INT_LVL_PCI_1 7   /* PCI 1 */
#define INT_LVL_PCI_2 8   /* PCI 2 */
#define INT_LVL_SWITCH 9  /* Switch */
#define INT_LVL_MAX INT_LVL_SWITCH

/* interrupts */
#define IRQ_TIMER (0x1 << INT_LVL_TIMER)
#define IRQ_UART0 (0x1 << INT_LVL_UART0)
#define IRQ_UART1 (0x1 << INT_LVL_UART1)
#define IRQ_USBHOST (0x1 << INT_LVL_USBHOST)
#define IRQ_EXTIO_0 (0x1 << INT_LVL_EXTIO_0)
#define IRQ_EXTIO_1 (0x1 << INT_LVL_EXTIO_1)
#define IRQ_PCI_INT0 (0x1 << INT_LVL_PCI_0)
#define IRQ_PCI_INT1 (0x1 << INT_LVL_PCI_1)
#define IRQ_PCI_INT2 (0x1 << INT_LVL_PCI_2)
#define IRQ_SWITCH (0x1 << INT_LVL_SWITCH)

#define IRQ_MASK 0x3ff


#define ADM5120_MIPSINT_SOFT0 0
#define ADM5120_MIPSINT_SOFT1 1
#define ADM5120_MIPSINT_IRQ 2
#define ADM5120_MIPSINT_FIQ 3
#define ADM5120_MIPSINT_REV0 4
#define ADM5120_MIPSINT_REV1 5
#define ADM5120_MIPSINT_REV2 6
#define ADM5120_MIPSINT_TIMER 7

typedef struct command_arg {
    std_u32_t command;
    std_u32_t irq;
} command_arg_t;

#define SET_IRQ 1
#define CLR_IRQ 2

typedef struct mod_vm_device_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;
    vm_device_info_t info;

    mod_shell_t *p_mod_shell;
    int_data_t int_data;
} mod_vm_device_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_INT_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_INT_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_INT_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_INT_reset(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_INT_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_INT_access(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_INT_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_INT_command(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_INT_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_INT_create_instance(INOUT std_void_t **pp_handle);

#endif
