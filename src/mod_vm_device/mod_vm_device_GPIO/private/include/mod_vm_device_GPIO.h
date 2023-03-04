/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_GPIO.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_GPIO_H
#define MOD_VM_DEVICE_GPIO_H

#include "mod_shell.h"
#include "mod_vm_device.h"

/*---------------------GPIO----------------------------------*/
#define JZ4740_GPIO_BASE       0x10010000
#define JZ4740_GPIO_SIZE       0x388
//n = 0,1,2,3
#define GPIO_PXPIN(n)	( (0x00 + (n)*0x100))   /* PIN Level Register */
#define GPIO_PXDAT(n)	((0x10 + (n)*0x100))    /* Port Data Register */
#define GPIO_PXDATS(n)	( (0x14 + (n)*0x100))   /* Port Data Set Register */
#define GPIO_PXDATC(n)	( (0x18 + (n)*0x100))   /* Port Data Clear Register */
#define GPIO_PXIM(n)	( (0x20 + (n)*0x100))   /* Interrupt Mask Register */
#define GPIO_PXIMS(n)	( (0x24 + (n)*0x100))   /* Interrupt Mask Set Reg */
#define GPIO_PXIMC(n)	( (0x28 + (n)*0x100))   /* Interrupt Mask Clear Reg */
#define GPIO_PXPE(n)	((0x30 + (n)*0x100))    /* Pull Enable Register */
#define GPIO_PXPES(n)	( (0x34 + (n)*0x100))   /* Pull Enable Set Reg. */
#define GPIO_PXPEC(n)	( (0x38 + (n)*0x100))   /* Pull Enable Clear Reg. */
#define GPIO_PXFUN(n)	( (0x40 + (n)*0x100))   /* Function Register */
#define GPIO_PXFUNS(n)	( (0x44 + (n)*0x100))   /* Function Set Register */
#define GPIO_PXFUNC(n)	( (0x48 + (n)*0x100))   /* Function Clear Register */
#define GPIO_PXSEL(n)	( (0x50 + (n)*0x100))   /* Select Register */
#define GPIO_PXSELS(n)	( (0x54 + (n)*0x100))   /* Select Set Register */
#define GPIO_PXSELC(n)	( (0x58 + (n)*0x100))   /* Select Clear Register */
#define GPIO_PXDIR(n)	( (0x60 + (n)*0x100))   /* Direction Register */
#define GPIO_PXDIRS(n)	( (0x64 + (n)*0x100))   /* Direction Set Register */
#define GPIO_PXDIRC(n)	( (0x68 + (n)*0x100))   /* Direction Clear Register */
#define GPIO_PXTRG(n)	( (0x70 + (n)*0x100))   /* Trigger Register */
#define GPIO_PXTRGS(n)	( (0x74 + (n)*0x100))   /* Trigger Set Register */
#define GPIO_PXTRGC(n)	( (0x78 + (n)*0x100))   /* Trigger Set Register */
#define GPIO_PXFLG(n)	( (0x80 + (n)*0x100))   /* Port Flag Register */
/* According to datasheet, it is 0x14. I think it shoud be 0x84*/
#define GPIO_PXFLGC(n)	( (0x84 + (n)*0x100))   /* Port Flag clear Register */

#define JZ4740_GPIO_INDEX_MAX  0xe2     /*0x388/4 */

/* GPIO is in 4 groups. 32 per group*/
/*

48-79     0
80-111    1
112-143   2
144-175   3

#define IRQ_GPIO3	25
#define IRQ_GPIO2	26
#define IRQ_GPIO1	27
#define IRQ_GPIO0	28
#define IRQ_GPIO_0	48   48 to 175 for GPIO pin 0 to 127
*/

#define IRQ_GPIO_0	48      /* 48 to 175 for GPIO pin 0 to 127 */

typedef struct jz4740_gpio_data{
    std_u8_t *jz4740_gpio_ptr;
    std_u32_t jz4740_gpio_size;
}jz4740_gpio_data_t;

typedef struct mod_vm_device_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;

    vm_device_info_t info;

    mod_shell_t *p_mod_shell;
    jz4740_gpio_data_t gpio_data;
} mod_vm_device_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_GPIO_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_GPIO_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_GPIO_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_GPIO_reset(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_GPIO_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_GPIO_access(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_GPIO_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_GPIO_command(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_GPIO_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_GPIO_create_instance(INOUT std_void_t **pp_handle);

#endif
