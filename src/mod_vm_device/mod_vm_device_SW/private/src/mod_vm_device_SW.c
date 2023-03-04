/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_SW.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_SW.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_memory.h"
mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;


std_int_t timeout;
std_u32_t time_reload;
std_u32_t sw_table[SW_INDEX_MAX];


/**
 * mod_vm_device_SW_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_SW_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_SW_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_SW_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_SW_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_SW_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_SW_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_SW_cleanup(mod_vm_device_t *p_m)
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
 * mod_vm_device_SW_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_SW_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;
    mod_vm_device_imp_t *p_sw_info = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_vm_arch_mips_cp0_iid = MOD_VM_ARCH_MIPS_CP0_IID;

    mod_query_instance(&mod_vm_arch_mips_cp0_iid, (std_void_t **) &p_global_vm_arch_mips_cp0, (mod_ownership_t *) p_m);

    memcpy(p_init, arg, sizeof(vm_device_info_t));

    p_init->flags = VDEVICE_FLAG_NO_MTS_MMAP;

    p_sw_info->sw_data.sw_ptr = (std_u8_t *) &sw_table[0];
    p_sw_info->sw_data.sw_size = p_init->phys_len;

    mod_vm_device_reset(p_m);
}

/**
 * mod_vm_device_SW_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_SW_reset(IN mod_vm_device_t *p_m)
{
    /*RESET SW*/
    sw_table[CODE_REG / 4] = 0x34085120;
    sw_table[Timer_int_REG / 4] = 0x10000;
    sw_table[Timer_REG / 4] = 0xffff;

    sw_table[0x7] = 0x50423;//32M
}

/**
 * mod_vm_device_SW_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_SW_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    mod_vm_device_imp_t *p_sw_info = (mod_vm_device_imp_t *) p_m;
    struct sw_data *d = &(p_sw_info->sw_data);

    if (arg->offset >= d->sw_size) {
        *(arg->data) = 0;
        return NULL;
    }

    switch (arg->offset) {
        case Timer_int_REG:
            if (arg->op_type == MTS_WRITE) {
                if (*(arg->data) & SW_TIMER_INT) {
                    timeout = 0;
                }
            } else if (arg->op_type == MTS_READ) {
                *(arg->data) = sw_table[Timer_int_REG / 4] & (~SW_TIMER_INT);
                *(arg->data) |= timeout;
                *(arg->has_set_value) = TRUE;
                return NULL;
            } else {
                assert(0);
            }
            break;
        case Timer_REG:
            if (arg->op_type == MTS_WRITE) {
                time_reload = *(arg->data) & SW_TIMER_MASK;
            }
            break;
        default:
            break;
    }
    return ((void *) (d->sw_ptr + arg->offset));
}

#define INT_LVL_TIMER 0 /* Timer */

/**
 * virtual_adm5120_timer
 * @brief   
 * @return  void forced_inline
 */
void forced_inline virtual_adm5120_timer()
{
    std_u32_t tim;

    mod_vm_arch_mips_cp0_timer(p_global_vm_arch_mips_cp0);

    if (sw_table[Timer_REG / 4] & SW_TIMER_EN) {
        tim = sw_table[Timer_REG / 4] & SW_TIMER_MASK;
        if (tim == 0) {
            tim = time_reload;
            timeout = 1;
        } else {
            tim -= 0x2000;         /*1ms=2000*640ns.but 2000 is too slow. I set it to 0x2000 */
        }
        if ((std_32_t) tim < 0x2000){
            tim = 0;
        }
        sw_table[Timer_REG / 4] &= ~SW_TIMER_MASK;
        sw_table[Timer_REG / 4] += tim;

    }
}
/**
 * mod_vm_device_SW_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_SW_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    virtual_adm5120_timer();
}

struct mod_vm_device_ops_st mod_vm_device_SW_ops = {
        mod_vm_device_SW_init,
        mod_vm_device_SW_cleanup,

        /***func_ops***/
        mod_vm_device_SW_initiate,
        mod_vm_device_SW_reset,
        mod_vm_device_SW_access,
        mod_vm_device_SW_command,

};

/**
 * mod_vm_device_SW_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_SW_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_SW_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
