/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    mod_vm_device_vtty_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */


/**
 * @file    mod_vm_device_vtty_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_vtty_CVM.h"
#include "dev_vtty.h"

/**
 * mod_vm_device_vtty_CVM_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_vtty_CVM_init(IN mod_vm_device_vtty_t *p_m, IN const std_char_t *arg,
                                              IN std_int_t arg_len)
{
    mod_vm_device_vtty_imp_t *p_imp_m = (mod_vm_device_vtty_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_create");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_create, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_get_char");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_get_char, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_put_char");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_put_char, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_put_buffer");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_put_buffer, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_is_char_avail");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_is_char_avail, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_is_full");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_vtty_CVM_is_full, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_vtty_CVM_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_vtty_CVM_cleanup(mod_vm_device_vtty_t *p_m)
{
    mod_vm_device_vtty_imp_t *p_imp_m = (mod_vm_device_vtty_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_initiate");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_create");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_get_char");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_put_char");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_put_buffer");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_is_char_avail");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_vtty_is_full");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * mod_vm_device_vtty_CVM_initiate
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_vtty_CVM_initiate(IN mod_vm_device_vtty_t *p_m)
{
    vtty_init();
}

/**
 * mod_vm_device_vtty_CVM_create
 * @brief   
 * @param   p_m
 * @param   name
 * @param   type
 * @param   tcp_port
 * @param   option
 * @param   rnf
 * @return  STD_CALL             vtty_t *
 */
STD_CALL vtty_tt *mod_vm_device_vtty_CVM_create(IN mod_vm_device_vtty_t *p_m, IN std_char_t *name, IN std_int_t type,
                                               IN std_int_t tcp_port, IN vtty_serial_option_t *option,
                                               IN read_notifier_t rnf)
{
    vtty_t *vtty = 	vtty_create(NULL, name, type, tcp_port, (vtty_serial_option_t *)option);
    vtty->read_notifier = (void (*)(struct virtual_tty *)) rnf;
    return (vtty_tt  *)vtty;
}

/**
 * mod_vm_device_vtty_CVM_get_char
 * @brief   
 * @param   p_m
 * @param   vtty
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_device_vtty_CVM_get_char(IN mod_vm_device_vtty_t *p_m, IN vtty_tt *vtty)
{
    return vtty_get_char((vtty_t *)vtty);
}

/**
 * mod_vm_device_vtty_CVM_put_char
 * @brief   
 * @param   p_m
 * @param   vtty
 * @param   ch
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_vtty_CVM_put_char(IN mod_vm_device_vtty_t *p_m, IN vtty_tt *vtty, IN std_char_t ch)
{
    return vtty_put_char((vtty_t *)vtty, ch);
}

/**
 * mod_vm_device_vtty_CVM_put_buffer
 * @brief   
 * @param   p_m
 * @param   vtty
 * @param   buf
 * @param   len
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_vtty_CVM_put_buffer(IN mod_vm_device_vtty_t *p_m, IN vtty_tt *vtty,
                                                      IN std_char_t *buf, IN std_uint_t len)
{
    return vtty_put_buffer((vtty_t *)vtty, buf, len);
}

/**
 * mod_vm_device_vtty_CVM_is_char_avail
 * @brief   
 * @param   p_m
 * @param   vtty
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_device_vtty_CVM_is_char_avail(IN mod_vm_device_vtty_t *p_m, IN vtty_tt *vtty)
{
    return vtty_is_char_avail((vtty_t *)vtty);
}

/**
 * mod_vm_device_vtty_CVM_is_full
 * @brief   
 * @param   p_m
 * @param   vtty
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_device_vtty_CVM_is_full(IN mod_vm_device_vtty_t *p_m, IN vtty_tt *vtty)
{
    return vtty_is_full((vtty_t *)vtty);
}

struct mod_vm_device_vtty_ops_st mod_vm_device_vtty_CVM_ops = {
        mod_vm_device_vtty_CVM_init,
        mod_vm_device_vtty_CVM_cleanup,

        /***func_ops***/
        mod_vm_device_vtty_CVM_initiate,
        mod_vm_device_vtty_CVM_create,
        mod_vm_device_vtty_CVM_get_char,
        mod_vm_device_vtty_CVM_put_char,
        mod_vm_device_vtty_CVM_put_buffer,
        mod_vm_device_vtty_CVM_is_char_avail,
        mod_vm_device_vtty_CVM_is_full,

};

/**
 * mod_vm_device_vtty_CVM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_vtty_CVM_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_vtty_imp_t *p_m = NULL;

    p_m = (mod_vm_device_vtty_imp_t *) CALLOC(1, sizeof(mod_vm_device_vtty_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_vtty_CVM_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
