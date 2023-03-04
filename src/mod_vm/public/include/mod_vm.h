/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_H
#define MOD_VM_H

#include "mod.h"
#include "mod_ownership.h"
#include "std_common.h"

/*****************************************************
 *                                                   *
 *                      Define                       *
 *                                                   *
 ****************************************************/

/***struct_define***/

/***macro_define***/

/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/

typedef struct mod_vm_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_ops_st *p_ops;
} mod_vm_t;

struct mod_vm_ops_st {
    std_int_t (*init)(IN mod_vm_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_t *m);

    /***func_define***/
    std_void_t (*initiate)(IN mod_vm_t *m, IN std_char_t *conf_name);
    std_void_t (*start)(IN mod_vm_t *m);
    std_void_t (*stop)(IN mod_vm_t *m);
    std_void_t (*suspend)(IN mod_vm_t *m);
    std_void_t (*resume)(IN mod_vm_t *m);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_IID MOD_GUID(0x6adc222a, 0x1fd0, 0x0a72, 0xd4, 0xa2, 0x50, 0x25, 0x18, 0x1b, 0x6e, 0x7f)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_t *) (m), arg, arg_len))
#define mod_vm_cleanup(m) ((m)->p_ops->cleanup((mod_vm_t *) (m)))

/***interface_define***/
#define mod_vm_initiate(m, conf_name) ((m)->p_ops->initiate((mod_vm_t *) (m), conf_name))
#define mod_vm_start(m) ((m)->p_ops->start((mod_vm_t *) (m)))
#define mod_vm_stop(m) ((m)->p_ops->stop((mod_vm_t *) (m)))
#define mod_vm_suspend(m) ((m)->p_ops->suspend((mod_vm_t *) (m)))
#define mod_vm_resume(m) ((m)->p_ops->resume((mod_vm_t *) (m)))

#endif
