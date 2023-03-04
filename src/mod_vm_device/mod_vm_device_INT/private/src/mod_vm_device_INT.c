/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_INT.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_INT.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_memory.h"

mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;
static std_u32_t intctrl_table[INTCTRL_INDEX_MAX];

/**
 * mod_vm_device_INT_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_INT_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_INT_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_INT_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_INT_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_INT_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_INT_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_INT_cleanup(mod_vm_device_t *p_m)
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
 * mod_vm_device_INT_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_INT_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;
    mod_vm_device_imp_t *p_int_info = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_vm_arch_mips_cp0_iid = MOD_VM_ARCH_MIPS_CP0_IID;

    mod_query_instance(&mod_vm_arch_mips_cp0_iid, (std_void_t **) &p_global_vm_arch_mips_cp0, (mod_ownership_t *) p_m);

    memcpy(p_init, arg, sizeof(vm_device_info_t));

    p_init->flags = VDEVICE_FLAG_NO_MTS_MMAP;

    p_int_info->int_data.int_ptr = (std_u8_t *) &intctrl_table[0];
    p_int_info->int_data.int_size = p_init->phys_len;

    mod_vm_device_reset(p_m);
}

/**
 * mod_vm_device_INT_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_INT_reset(IN mod_vm_device_t *p_m)
{
    return;
}

/**
 * mod_vm_device_INT_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_INT_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    mod_vm_device_imp_t *p_int_info = (mod_vm_device_imp_t *) p_m;
    struct int_data *d = &(p_int_info->int_data);

    if (arg->offset >= d->int_size) {
        *(arg->data) = 0;
        return NULL;
    }

    switch (arg->offset) {
        case IRQ_ENABLE_REG:
            if (MTS_WRITE == arg->op_type) {
                intctrl_table[IRQ_ENABLE_REG / 4] |= (*(arg->data) & 0x3ff);
                *(arg->has_set_value) = TRUE;
                return NULL;
            }
            break;

        case IRQ_DISABLE_REG:
            if (MTS_WRITE == arg->op_type) {
                intctrl_table[IRQ_ENABLE_REG / 4] &= (~(*(arg->data) & 0x3ff));
                *(arg->has_set_value) = TRUE;
                return NULL;
            }
            break;
        default:
            break;
    }

    return ((void *) (d->int_ptr + arg->offset));
}

/*ADM5120 IRQ*/
/*
Mapping adm irq to mips irq.

So why we need a mapping of interrupts?

                                       IN ADM5120,there are 10 interrupts
                        0	 Timer
                        1	 Uart 0
                        2	 Uart 1
                        3	 USB Host
                        4	 External I/O 0
                        5	 External I/O 1
                        6	 PCI 0
                        7	 PCI 1
                        8	 PCI 2
                        9	 Switch

                                ADM5120 will triger INTERRUPT 2 and 3 to MIPS.
                        INT_M(0X14) register control the interrupt releation of ADM5120 and iqr/firq.

                                  */

/**
 * adm_irq2mips_irq
 * @brief   
 * @param   irq
 * @return  std_int_t
 */
std_int_t adm_irq2mips_irq(std_uint_t irq)
{
    std_u32_t int_bit_mask = 0;

    int_bit_mask = 1 << irq;
    if ((intctrl_table[IRQ_MODE_REG / 4] & int_bit_mask) == int_bit_mask) {
        return ADM5120_MIPSINT_FIQ;
    } else {
        return ADM5120_MIPSINT_IRQ;
    }
}

/**
 * whether_irq_enable
 * @brief   
 * @param   irq
 * @return  int
 */
int whether_irq_enable(std_uint_t irq)
{
    std_u32_t int_bit_mask = 0;

    int_bit_mask = 1 << irq;
    if ((intctrl_table[IRQ_ENABLE_REG / 4] & int_bit_mask) == int_bit_mask) {
        return TRUE;
    } else {
        return FALSE;
    }
}
/**
 * adm5120_clear_irq
 * @brief   
 * @param   irq
 */
void adm5120_clear_irq(std_uint_t irq)
{
    assert(irq<=INT_LVL_MAX);
    int mips_irq_no;
    std_u32_t int_bit_mask = 0;

    int_bit_mask = 1 << irq;

    /*clear raw status */
    intctrl_table[IRQ_RAW_STATUS_REG / 4] &= ~int_bit_mask;

    if (irq != 9) {
        mips_irq_no = adm_irq2mips_irq(irq);
    } else {
        mips_irq_no = 2;
    }

    if (ADM5120_MIPSINT_FIQ == mips_irq_no) {
        intctrl_table[FIQ_STATUS_REG / 4] &= ~int_bit_mask;
    } else {
        intctrl_table[IRQ_STATUS_REG / 4] &= ~int_bit_mask;
    }
    irq = mips_irq_no;

    mod_vm_arch_mips_cp0_irq_op(p_global_vm_arch_mips_cp0, OP_IRQ_CLEAR, irq);
}


/*We must map adm irq to mips irq before setting irq*/
void adm5120_set_irq(std_uint_t irq)
{
    assert(irq<=INT_LVL_MAX);

    int mips_irq_no;
    std_u32_t int_bit_mask = 0;

    int_bit_mask = 1 << irq;

    /*set raw status */
    intctrl_table[IRQ_RAW_STATUS_REG / 4] |= int_bit_mask;

    /*check whether irq is enabled*/
    if (whether_irq_enable(irq) == FALSE)
        return;

    if (irq != 9) {
        mips_irq_no = adm_irq2mips_irq(irq);
    } else {
        mips_irq_no = 2;
    }

    if (ADM5120_MIPSINT_FIQ == mips_irq_no) {
        intctrl_table[FIQ_STATUS_REG / 4] |= int_bit_mask;
    } else {
        intctrl_table[IRQ_STATUS_REG / 4] |= int_bit_mask;
    }

    irq = mips_irq_no;

    mod_vm_arch_mips_cp0_irq_op(p_global_vm_arch_mips_cp0, OP_IRQ_SET, irq);
    mod_vm_arch_mips_cp0_irq_op(p_global_vm_arch_mips_cp0, OP_IRQ_UPDATE, irq);
}


/*END ADM5120 IRQ*/

/**
 * mod_vm_device_INT_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_INT_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    command_arg_t const *p_command = (command_arg_t *) arg;

    switch (p_command->command) {
        case SET_IRQ:
            adm5120_set_irq(p_command->irq);
            break;

        case CLR_IRQ:
            adm5120_clear_irq(p_command->irq);
            break;

        default:
            break;
    }
}

struct mod_vm_device_ops_st mod_vm_device_INT_ops = {
        mod_vm_device_INT_init,
        mod_vm_device_INT_cleanup,

        /***func_ops***/
        mod_vm_device_INT_initiate,
        mod_vm_device_INT_reset,
        mod_vm_device_INT_access,
        mod_vm_device_INT_command,

};

/**
 * mod_vm_device_INT_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_INT_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_INT_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
