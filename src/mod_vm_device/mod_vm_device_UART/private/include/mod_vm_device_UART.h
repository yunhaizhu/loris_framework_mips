/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_UART.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_UART_H
#define MOD_VM_DEVICE_UART_H

#include "mod_shell.h"
#include "mod_vm_device.h"
#include "mod_vm_device_vtty.h"

/*===========================  UART Control Register
========================*/
#define UART_DR_REG								0x00
#define UART_RSR_REG							0x04
#define UART_ECR_REG							0x04
#define UART_LCR_H_REG							0x08
#define UART_LCR_M_REG							0x0c
#define UART_LCR_L_REG							0x10
#define UART_CR_REG								0x14
#define UART_FR_REG								0x18
#define UART_IIR_REG							0x1c
#define UART_ICR_REG							0x1C
#define UART_ILPR_REG							0x20

/*  rsr/ecr reg  */
#define UART_OVERRUN_ERR						0x08
#define UART_BREAK_ERR							0x04
#define UART_PARITY_ERR							0x02
#define UART_FRAMING_ERR						0x01
#define UART_RX_STATUS_MASK						0x0f
#define UART_RX_ERROR							( UART_BREAK_ERR	\
												| UART_PARITY_ERR	\
												| UART_FRAMING_ERR)

/*  lcr_h reg  */
#define UART_SEND_BREAK							0x01
#define UART_PARITY_EN							0x02
#define UART_EVEN_PARITY						0x04
#define UART_TWO_STOP_BITS						0x08
#define UART_ENABLE_FIFO						0x10

#define UART_WLEN_5BITS							0x00
#define UART_WLEN_6BITS							0x20
#define UART_WLEN_7BITS							0x40
#define UART_WLEN_8BITS							0x60
#define UART_WLEN_MASK							0x60

/*  cr reg  */
#define UART_PORT_EN							0x01
#define UART_SIREN								0x02
#define UART_SIRLP								0x04
#define UART_MODEM_STATUS_INT_EN				0x08
#define UART_RX_INT_EN							0x10
#define UART_TX_INT_EN							0x20
#define UART_RX_TIMEOUT_INT_EN					0x40
#define UART_LOOPBACK_EN						0x80

/*  fr reg  */
#define UART_CTS								0x01
#define UART_DSR								0x02
#define UART_DCD								0x04
#define UART_BUSY								0x08
#define UART_RX_FIFO_EMPTY						0x10
#define UART_TX_FIFO_FULL						0x20
#define UART_RX_FIFO_FULL						0x40
#define UART_TX_FIFO_EMPTY						0x80

/*  iir/icr reg  */
#define UART_MODEM_STATUS_INT					0x01
#define UART_RX_INT								0x02
#define UART_TX_INT								0x04
#define UART_RX_TIMEOUT_INT						0x08

#define UART_INT_MASK							0x0f

#define ADM5120_UARTCLK_FREQ					62500000



/*  uart_baudrate  */
#define UART_230400bps_DIVISOR					UART_BAUDDIV(230400)
// #define UART_115200bps_DIVISOR                                       UART_BAUDDIV(115200)
#define UART_115200bps_DIVISOR					33
// #define UART_76800bps_DIVISOR                                        UART_BAUDDIV(76800)
#define UART_76800bps_DIVISOR					50
// #define UART_57600bps_DIVISOR                                        UART_BAUDDIV(57600)
#define UART_57600bps_DIVISOR					67
//#define UART_38400bps_DIVISOR                                 UART_BAUDDIV(38400)
#define UART_38400bps_DIVISOR					102
//#define UART_19200bps_DIVISOR                                 UART_BAUDDIV(19200)
#define UART_19200bps_DIVISOR					202
//#define UART_14400bps_DIVISOR                                 UART_BAUDDIV(14400)
#define UART_14400bps_DIVISOR					270
//#define UART_9600bps_DIVISOR                                  UART_BAUDDIV(9600)
#define UART_9600bps_DIVISOR					406
//#define UART_2400bps_DIVISOR                                  UART_BAUDDIV(2400)
#define UART_2400bps_DIVISOR					1627
//#define UART_1200bps_DIVISOR                                  UART_BAUDDIV(1200)


#define UART_INDEX_MAX						0x9     //0x024/4

#define INT_LVL_UART0							1       /* Uart 0 */
#define INT_LVL_UART1							2       /* Uart 1 */

typedef struct uart_data {
    std_u8_t *uart_ptr;
    std_u32_t uart_size;
    vtty_tt *vtty[2];
}uart_data_t;

typedef struct command_arg{
    std_u32_t command;
    std_u32_t irq;
}command_arg_t;

#define SET_IRQ 1
#define CLR_IRQ 2

typedef struct mod_vm_device_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;
    vm_device_info_t info;

    mod_shell_t *p_mod_shell;
    uart_data_t uart_data;
} mod_vm_device_imp_t;

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_UART_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_UART_initiate(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_UART_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_UART_reset(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_UART_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_UART_access(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_UART_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_UART_command(IN std_void_t *p_handle, IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_UART_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_UART_create_instance(INOUT std_void_t **pp_handle);

#endif
