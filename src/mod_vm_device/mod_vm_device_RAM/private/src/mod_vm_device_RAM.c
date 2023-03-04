/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_RAM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_RAM.h"

#include <assert.h>
/**
 * m_memalign
 * @brief   
 * @param   boundary
 * @param   size
 * @return  void *
 */
void *m_memalign(size_t boundary, size_t size)
{
    void *p;

#ifdef __linux__
    if (posix_memalign((void *) &p, boundary, size))
#else
#if defined(__CYGWIN__) || defined(SUNOS)
    if (!(p = memalign(boundary, size)))
#else
    if (!(p = malloc(size)))
#endif
#endif
        return NULL;

    assert(((unsigned long) p & (boundary - 1)) == 0);
    return p;
}
/**
 * mod_vm_device_RAM_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_RAM_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_RAM_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_RAM_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_RAM_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_RAM_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_RAM_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_RAM_cleanup(mod_vm_device_t *p_m)
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
 * mod_vm_device_RAM_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_RAM_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;

    memcpy(p_init, arg, sizeof(vm_device_info_t));

    p_init->host_addr = (std_u64_t )m_memalign(4096,arg->phys_len);
}

/**
 * mod_vm_device_RAM_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_RAM_reset(IN mod_vm_device_t *p_m)
{
    vm_device_info_t *p_init = &p_m->info;

    /*RESET RAM*/
    memset((std_uchar_t *) p_init->host_addr, 0x0, p_init->phys_len);
}

/**
 * mod_vm_device_RAM_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_RAM_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    return NULL;
}

/**
 * mod_vm_device_RAM_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_RAM_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    return;
}

struct mod_vm_device_ops_st mod_vm_device_RAM_ops = {
        mod_vm_device_RAM_init,
        mod_vm_device_RAM_cleanup,

        /***func_ops***/
        mod_vm_device_RAM_initiate,
        mod_vm_device_RAM_reset,
        mod_vm_device_RAM_access,
        mod_vm_device_RAM_command,

};

/**
 * mod_vm_device_RAM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_RAM_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_RAM_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
