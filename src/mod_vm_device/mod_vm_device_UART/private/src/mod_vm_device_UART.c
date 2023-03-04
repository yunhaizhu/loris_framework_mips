/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_UART.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_UART.h"
#include "mod_vm_arch_mips_cpu.h"
#include "mod_vm_memory.h"

mod_vm_device_t *p_global_device_INT;
mod_vm_device_vtty_t *p_global_device_VTTY;
mod_vm_arch_mips_cpu_t *p_global_vm_arch_mips_cpu;

std_u32_t uart_table[2][UART_INDEX_MAX];
/**
 * mod_vm_device_UART_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_UART_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_UART_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_UART_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_UART_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_UART_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_UART_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_UART_cleanup(mod_vm_device_t *p_m)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * uart_set_interrupt
 * @brief   
 * @param   channel
 */
void uart_set_interrupt(int channel)
{
    command_arg_t cmd;

    if (channel == 0){
        cmd.command = SET_IRQ;
        cmd.irq = INT_LVL_UART0;

        mod_vm_device_command(p_global_device_INT, &cmd);
    } else if (channel == 1) {
        cmd.command = SET_IRQ;
        cmd.irq = INT_LVL_UART1;

        mod_vm_device_command(p_global_device_INT, &cmd);
    }
}

/**
 * uart_clear_interrupt
 * @brief   
 * @param   channel
 */
void uart_clear_interrupt(int channel)
{
    command_arg_t cmd;
    if (channel == 0) {
        cmd.command = CLR_IRQ;
        cmd.irq = INT_LVL_UART0;

        mod_vm_device_command(p_global_device_INT, &cmd);

    } else if (channel == 1) {
        cmd.command = CLR_IRQ;
        cmd.irq = INT_LVL_UART1;

        mod_vm_device_command(p_global_device_INT, &cmd);

    }
}



/* Console port input */
static void tty_con0_input(vtty_tt * vtty)
{
    uart_table[0][UART_FR_REG / 4] &= ~UART_RX_FIFO_EMPTY;

    if (mod_vm_device_vtty_is_full(p_global_device_VTTY, vtty))
        uart_table[0][UART_FR_REG / 4] |= UART_RX_FIFO_FULL;
    if ((uart_table[0][UART_CR_REG / 4] & UART_RX_INT_EN) && (uart_table[0][UART_CR_REG / 4] & UART_PORT_EN)) {
        uart_table[0][UART_ICR_REG / 4] |= UART_RX_INT;
        uart_set_interrupt(0);

        mod_vm_arch_mips_cpu_break_idle(p_global_vm_arch_mips_cpu);
    }
}


/* Console port input */
__attribute__((unused)) static void tty_con1_input(vtty_tt * vtty)
{
    uart_table[1][UART_FR_REG / 4] &= ~UART_RX_FIFO_EMPTY;

    if (mod_vm_device_vtty_is_full(p_global_device_VTTY, vtty))
        uart_table[1][UART_FR_REG / 4] |= UART_RX_FIFO_FULL;
    if ((uart_table[1][UART_CR_REG / 4] & UART_RX_INT_EN) && (uart_table[1][UART_CR_REG / 4] & UART_PORT_EN)) {
        uart_set_interrupt(1);
        uart_table[1][UART_ICR_REG / 4] |= UART_RX_INT;
    }
}
/* VTTY connection types */
enum
{
    VTTY_TYPE_NONE = 0,
    VTTY_TYPE_TERM,
    VTTY_TYPE_TCP,
    VTTY_TYPE_SERIAL,
};

/**
 * mod_vm_device_UART_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_UART_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;
    mod_vm_device_imp_t *p_uart_info = (mod_vm_device_imp_t *)p_m;
    mod_iid_t mod_vm_arch_mips_cpu_iid = MOD_VM_ARCH_MIPS_CPU_IID;
    mod_iid_t mod_vm_device_vtty_iid = MOD_VM_DEVICE_VTTY_IID;
    mod_iid_t mod_vm_device_int_iid = MOD_VM_DEVICE_INT_IID;

    mod_query_instance(&mod_vm_arch_mips_cpu_iid, (std_void_t **) &p_global_vm_arch_mips_cpu, (mod_ownership_t *) p_m);
    mod_query_instance(&mod_vm_device_vtty_iid, (std_void_t **) &p_global_device_VTTY, (mod_ownership_t *) p_m);
    mod_query_instance(&mod_vm_device_int_iid, (std_void_t **) &p_global_device_INT, (mod_ownership_t *) p_m);

    memcpy(p_init, arg, sizeof(vm_device_info_t));

    p_init->flags = VDEVICE_FLAG_NO_MTS_MMAP;

    p_uart_info->uart_data.uart_ptr = (std_u8_t *) (&uart_table[0]);
    p_uart_info->uart_data.uart_size = p_init->phys_len;
    p_uart_info->uart_data.vtty[0] = mod_vm_device_vtty_create(p_global_device_VTTY, "Console port", VTTY_TYPE_TERM, 0, NULL, tty_con0_input);

    mod_vm_device_reset(p_m);
}

/**
 * mod_vm_device_UART_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_UART_reset(IN mod_vm_device_t *p_m)
{
    /*RESET UART*/
    uart_table[0][UART_FR_REG / 4] = 0x90;
    uart_table[0][UART_RSR_REG / 4] = 0;
}

/**
 * mod_vm_device_UART_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_UART_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    mod_vm_device_imp_t *p_uart_info = (mod_vm_device_imp_t *)p_m;
    struct uart_data *d = &(p_uart_info->uart_data);
    std_u32_t offset = arg->offset;
    std_uint_t op_type = arg->op_type;
    std_u32_t * data = arg->data;
    std_u8_t * has_set_value  = arg->has_set_value;
    std_u8_t channel = 0;

    if (offset >= d->uart_size) {
        *data = 0;
        return NULL;
    }

    switch (offset) {
        case UART_DR_REG:
            if (!(uart_table[channel][UART_CR_REG / 4] & UART_PORT_EN)) {
                //uart port is disabled.
                if (op_type == MTS_READ) {
                    *data = vmtoh32(0xffffffff);
                }
                *has_set_value = TRUE;
                return NULL;
            }

            if (op_type == MTS_READ) {
                if (mod_vm_device_vtty_is_char_avail(p_global_device_VTTY, d->vtty[channel])) {
                    *data = mod_vm_device_vtty_get_char(p_global_device_VTTY, d->vtty[channel]);
                    uart_table[channel][UART_RSR_REG / 4] = 0;
                }
                else{
                    *data = vmtoh32(0xffffffff);
                }
                if (mod_vm_device_vtty_is_char_avail(p_global_device_VTTY, d->vtty[channel])) {
                    uart_table[channel][UART_FR_REG / 4] &= ~UART_RX_FIFO_EMPTY;
                    uart_table[channel][UART_FR_REG / 4] |= UART_RX_FIFO_FULL;
                    if ((uart_table[channel][UART_CR_REG / 4] & UART_RX_INT_EN)
                        && (uart_table[channel][UART_CR_REG / 4] & UART_PORT_EN)) {
                        uart_table[channel][UART_ICR_REG / 4] |= UART_RX_INT;
                        uart_set_interrupt(channel);
                    }

                } else {
                    uart_table[channel][UART_FR_REG / 4] |= UART_RX_FIFO_EMPTY;
                    uart_table[channel][UART_FR_REG / 4] &= ~UART_RX_FIFO_FULL;
                    if ((uart_table[channel][UART_CR_REG / 4] & UART_RX_INT_EN)
                        && (uart_table[channel][UART_CR_REG / 4] & UART_PORT_EN)) {
                        uart_table[channel][UART_ICR_REG / 4] &= ~UART_RX_INT;
                        uart_clear_interrupt(channel);
                    }

                }

                *has_set_value = TRUE;

            } else if (op_type == MTS_WRITE) {
                mod_vm_device_vtty_put_char(p_global_device_VTTY, d->vtty[channel], (char)*data);
                *has_set_value = TRUE;
            } else {
                assert(0);
            }
            return NULL;

        case UART_RSR_REG:
            if (op_type == MTS_WRITE) {
                uart_table[channel][UART_RSR_REG / 4] = 0;
                *has_set_value = TRUE;
            }
            break;
        case UART_CR_REG:
            if (op_type == MTS_WRITE) {
                //enable UART
                if ((*data) & UART_PORT_EN) {
                    if (*data & UART_TX_INT_EN) {
                        //START TX
                        uart_table[channel][UART_ICR_REG / 4] |= UART_TX_INT;
                        uart_set_interrupt(channel);
                    } else {
                        //TX interrupt dissabled
                        uart_table[channel][UART_ICR_REG / 4] &= ~UART_TX_INT;
                        uart_clear_interrupt(channel);
                    }
                    if (*data & UART_RX_INT_EN) {
                        if (mod_vm_device_vtty_is_char_avail(p_global_device_VTTY, d->vtty[channel])) {
                            //set RX interrupt
                            uart_table[channel][UART_ICR_REG / 4] |= UART_RX_INT;
                            uart_set_interrupt(channel);
                        }

                    } else {
                        //disable RX interrupt
                        uart_table[channel][UART_ICR_REG / 4] &= ~UART_RX_INT;
                        uart_clear_interrupt(channel);
                    }
                } else {
                    //disable UART
                    //clear rx and tx interrupt
                    uart_table[channel][UART_ICR_REG / 4] &= ~UART_TX_INT;
                    uart_clear_interrupt(channel);
                    uart_table[channel][UART_ICR_REG / 4] &= ~UART_RX_INT;
                    uart_clear_interrupt(channel);
                }
            }

            break;

        default:
            break;

    }

    return ((void *) (d->uart_ptr + offset));
}

/**
 * mod_vm_device_UART_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_UART_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    return;
}

struct mod_vm_device_ops_st mod_vm_device_UART_ops = {
        mod_vm_device_UART_init,
        mod_vm_device_UART_cleanup,

        /***func_ops***/
        mod_vm_device_UART_initiate,
        mod_vm_device_UART_reset,
        mod_vm_device_UART_access,
        mod_vm_device_UART_command,

};

/**
 * mod_vm_device_UART_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_UART_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_UART_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
