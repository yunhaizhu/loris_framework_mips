/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch_MIPS.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_arch_MIPS.h"
#include "mod_vm_arch_mips_cpu.h"

mod_vm_arch_mips_cpu_t *p_global_vm_arch_mips_cpu;

/**
 * mod_vm_arch_MIPS_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_MIPS_init(IN mod_vm_arch_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_arch_imp_t *p_imp_m = (mod_vm_arch_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_MIPS_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_start");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_MIPS_start, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_stop");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_MIPS_stop, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_suspend");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_MIPS_suspend, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_resume");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_MIPS_resume, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_arch_MIPS_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_MIPS_cleanup(mod_vm_arch_t *p_m)
{
    mod_vm_arch_imp_t *p_imp_m = (mod_vm_arch_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_initiate");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_start");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_stop");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_suspend");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_resume");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * mod_vm_arch_MIPS_initiate
 * @brief   
 * @param   p_m
 * @param   entry
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_MIPS_initiate(IN mod_vm_arch_t *p_m, IN std_uint_t entry)
{
    mod_iid_t mod_vm_arch_mips_cpu_iid = MOD_VM_ARCH_MIPS_CPU_IID;

    mod_query_instance(&mod_vm_arch_mips_cpu_iid, (std_void_t **) &p_global_vm_arch_mips_cpu, (mod_ownership_t *) p_m);

    mod_vm_arch_mips_cpu_reset(p_global_vm_arch_mips_cpu, entry);
}


/**
 * mod_vm_arch_MIPS_start
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_MIPS_start(IN mod_vm_arch_t *p_m)
{
    mod_vm_arch_imp_t *cvm_info = (mod_vm_arch_imp_t *)p_m;

    cvm_info->ARCH_STATUS = ARCH_RUNNING;

    mod_vm_arch_mips_cpu_run(p_global_vm_arch_mips_cpu);
}

/**
 * mod_vm_arch_MIPS_stop
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_MIPS_stop(IN mod_vm_arch_t *p_m)
{
    return;
}

/**
 * mod_vm_arch_MIPS_suspend
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_MIPS_suspend(IN mod_vm_arch_t *p_m)
{
    return;
}

/**
 * mod_vm_arch_MIPS_resume
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_MIPS_resume(IN mod_vm_arch_t *p_m)
{
    return;
}

struct mod_vm_arch_ops_st mod_vm_arch_MIPS_ops = {
        mod_vm_arch_MIPS_init,
        mod_vm_arch_MIPS_cleanup,

        /***func_ops***/
        mod_vm_arch_MIPS_initiate,
        mod_vm_arch_MIPS_start,
        mod_vm_arch_MIPS_stop,
        mod_vm_arch_MIPS_suspend,
        mod_vm_arch_MIPS_resume,

};

/**
 * mod_vm_arch_MIPS_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_MIPS_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_arch_imp_t *p_m = NULL;

    p_m = (mod_vm_arch_imp_t *) CALLOC(1, sizeof(mod_vm_arch_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_arch_MIPS_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
