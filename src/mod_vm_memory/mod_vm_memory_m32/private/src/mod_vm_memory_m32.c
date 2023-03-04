/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    mod_vm_memory_m32.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */

#include "mod_vm_memory_m32.h"
#include "cvm_memory_32.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_device_manager.h"

mod_vm_device_manager_t *p_global_dev_manger;
mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;

/**
 * mod_vm_memory_m32_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_memory_m32_init(IN mod_vm_memory_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_memory_imp_t *p_imp_m = (mod_vm_memory_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_memory_m32_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_lookup");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_memory_m32_lookup, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_memory_m32_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_map");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_memory_m32_map, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_unmap");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_memory_m32_unmap, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_memory_m32_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_memory_m32_cleanup(mod_vm_memory_t *p_m)
{
    mod_vm_memory_imp_t *p_imp_m = (mod_vm_memory_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_initiate");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_lookup");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_access");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_map");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_memory_unmap");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * mod_vm_memory_m32_initiate
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_memory_m32_initiate(IN mod_vm_memory_t *p_m)
{
    mod_iid_t mod_vm_device_manager_iid = MOD_VM_DEVICE_MANAGER_IID;
    mod_iid_t mod_vm_arch_mips_cp0_iid = MOD_VM_ARCH_MIPS_CP0_IID;

    mod_query_instance(&mod_vm_device_manager_iid, (std_void_t **) &p_global_dev_manger, (mod_ownership_t *) p_m);
    mod_query_instance(&mod_vm_arch_mips_cp0_iid, (std_void_t **) &p_global_vm_arch_mips_cp0, (mod_ownership_t *) p_m);

    return cvm_memory_init(p_m);
}

/**
 * mod_vm_memory_m32_lookup
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_memory_m32_lookup(IN mod_vm_memory_t *p_m, IN std_uint_t vaddr)
{
    return cvm_memory_32_lookup(p_m, vaddr);
}

/**
 * mod_vm_memory_m32_access
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @param   op_code
 * @param   op_size
 * @param   op_type
 * @param   data
 * @param   has_set_value
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_memory_m32_access(IN mod_vm_memory_t *p_m, IN std_uint_t vaddr, IN std_uint_t op_code,
                                              IN std_uint_t op_size, IN std_uint_t op_type, IN std_uint_t *data,
                                              IN std_uchar_t *has_set_value)
{
    return cvm_memory_32_access(p_m,
                                vaddr,
                                op_code,
                                op_size,
                                op_type,
                                data,
                                has_set_value);
}

/**
 * mod_vm_memory_m32_map
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_memory_m32_map(IN mod_vm_memory_t *p_m, IN std_uint_t vaddr)
{
    return;
}

/**
 * mod_vm_memory_m32_unmap
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_memory_m32_unmap(IN mod_vm_memory_t *p_m, IN std_uint_t vaddr)
{
    return;
}

struct mod_vm_memory_ops_st mod_vm_memory_m32_ops = {
        mod_vm_memory_m32_init,
        mod_vm_memory_m32_cleanup,

        /***func_ops***/
        mod_vm_memory_m32_initiate,
        mod_vm_memory_m32_lookup,
        mod_vm_memory_m32_access,
        mod_vm_memory_m32_map,
        mod_vm_memory_m32_unmap,

};

/**
 * mod_vm_memory_m32_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_memory_m32_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_memory_imp_t *p_m = NULL;

    p_m = (mod_vm_memory_imp_t *) CALLOC(1, sizeof(mod_vm_memory_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_memory_m32_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
