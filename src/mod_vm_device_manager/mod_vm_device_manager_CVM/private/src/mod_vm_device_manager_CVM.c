/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_manager_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_manager_CVM.h"

/**
 * mod_vm_device_manager_CVM_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_manager_CVM_init(IN mod_vm_device_manager_t *p_m, IN const std_char_t *arg,
                                                 IN std_int_t arg_len)
{
    mod_vm_device_manager_imp_t *p_imp_m = (mod_vm_device_manager_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_manager_add");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_manager_CVM_add, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_manager_del");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_manager_CVM_del, p_imp_m);

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_manager_find");
	mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
			   shell_stub_mod_vm_device_manager_CVM_find, p_imp_m);

    std_int_t offset = std_lock_free_list_head_entry_offset(vm_device_manger_CVM_list_t, list);
    std_lock_free_list_init(&p_imp_m->device_list_head, offset, STD_BOOL_TRUE, STD_BOOL_TRUE);

	return STD_RV_SUC;
}

/**
 * mod_vm_device_manager_CVM_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_manager_CVM_cleanup(mod_vm_device_manager_t *p_m)
{
    mod_vm_device_manager_imp_t *p_imp_m = (mod_vm_device_manager_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_manager_add");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_manager_del");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_manager_find");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    std_lock_free_list_cleanup(&p_imp_m->device_list_head);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * mod_vm_device_manager_CVM_add
 * @brief   
 * @param   p_m
 * @param   p_dev
 * @param   base
 * @param   len
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_manager_CVM_add(IN mod_vm_device_manager_t *p_m, IN mod_vm_device_t *p_dev,
                                                  IN std_uint_t base, IN std_uint_t len)
{
    mod_vm_device_manager_imp_t *p_vm_device_manger_CVM_stat = (mod_vm_device_manager_imp_t *)p_m;
    vm_device_manger_CVM_list_t *p_node = NULL;
    std_int_t i;

    p_node = (vm_device_manger_CVM_list_t *)CALLOC(1, sizeof(vm_device_manger_CVM_list_t));

    p_node->p_dev = p_dev;
    p_node->base = base;
    p_node->len = len;

    std_lock_free_list_add(&p_vm_device_manger_CVM_stat->device_list_head, &p_node->list, (std_u64_t)p_dev);

    for (i = 0; i < DEVICE_MAX; i++){
        if (!p_vm_device_manger_CVM_stat->dev_array[i]){
            break;
        }
    }
    p_vm_device_manger_CVM_stat->dev_array[i] = p_dev;
    p_dev->info.id = i;
}

/**
 * mod_vm_device_manager_CVM_del
 * @brief   
 * @param   p_m
 * @param   p_dev
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_manager_CVM_del(IN mod_vm_device_manager_t *p_m, IN mod_vm_device_t *p_dev)
{
    mod_vm_device_manager_imp_t *p_vm_device_manger_CVM_stat = (mod_vm_device_manager_imp_t *)p_m;

    std_lock_free_list_del(&p_vm_device_manger_CVM_stat->device_list_head, (std_u64_t)p_dev);

    for (std_int_t i  = 0; i < DEVICE_MAX; i++){
        if (p_vm_device_manger_CVM_stat->dev_array[i] == p_dev){
            p_vm_device_manger_CVM_stat->dev_array[i] = NULL;
            break;
        }
    }
}

/**
 * mod_vm_device_manager_CVM_find
 * @brief
 * @param   p_m
 * @param   addr
 * @param   pp_dev
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_manager_CVM_find(IN mod_vm_device_manager_t * p_m, IN std_uint_t addr, IN std_int_t dev_id,
                                                   IN mod_vm_device_t ** pp_dev)
{
    mod_vm_device_manager_imp_t *p_vm_device_manger_CVM_stat = (mod_vm_device_manager_imp_t *)p_m;
    vm_device_manger_CVM_list_t *p_node = NULL;

    if (dev_id >= 0 && p_vm_device_manger_CVM_stat->dev_array[dev_id]){
        *pp_dev = p_vm_device_manger_CVM_stat->dev_array[dev_id];
        return;
    }

    for(std_lock_free_list_head_t *p_list = p_vm_device_manger_CVM_stat->device_list_head.next; p_list != NULL;){
        p_node = std_lock_free_list_head_entry(p_list, vm_device_manger_CVM_list_t, list);
        p_list = p_list->next;

        if (p_node->base <= addr && p_node->base + p_node->len >= addr){
            *pp_dev = p_node->p_dev;
            return;
        }
    }
    *pp_dev = NULL;
}

struct mod_vm_device_manager_ops_st mod_vm_device_manager_CVM_ops = {
        mod_vm_device_manager_CVM_init,
        mod_vm_device_manager_CVM_cleanup,

    /***func_ops***/
	mod_vm_device_manager_CVM_add,
	mod_vm_device_manager_CVM_del,
	mod_vm_device_manager_CVM_find,

};

/**
 * mod_vm_device_manager_CVM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_manager_CVM_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_manager_imp_t *p_m = NULL;

    p_m = (mod_vm_device_manager_imp_t *) CALLOC(1, sizeof(mod_vm_device_manager_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_manager_CVM_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
