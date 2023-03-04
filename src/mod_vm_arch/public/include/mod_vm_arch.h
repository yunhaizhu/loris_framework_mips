/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_ARCH_H
#define MOD_VM_ARCH_H

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
#define ARCH_RUNNING 1
#define ARCH_STOP    2
#define ARCH_SUSPEND 3
/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/

typedef struct mod_vm_arch_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_arch_ops_st *p_ops;
} mod_vm_arch_t;

struct mod_vm_arch_ops_st {
    std_int_t (*init)(IN mod_vm_arch_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_arch_t *m);

    /***func_define***/
    std_void_t (*initiate)(IN mod_vm_arch_t *m, IN std_uint_t entry);
    std_void_t (*start)(IN mod_vm_arch_t *m);
    std_void_t (*stop)(IN mod_vm_arch_t *m);
    std_void_t (*suspend)(IN mod_vm_arch_t *m);
    std_void_t (*resume)(IN mod_vm_arch_t *m);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_ARCH_IID MOD_GUID(0x1828c092, 0xceeb, 0x9713, 0x22, 0x25, 0xe8, 0xf0, 0xbf, 0xca, 0x1c, 0x6c)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_arch_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_arch_t *) (m), arg, arg_len))
#define mod_vm_arch_cleanup(m) ((m)->p_ops->cleanup((mod_vm_arch_t *) (m)))

/***interface_define***/
#define mod_vm_arch_initiate(m, entry) ((m)->p_ops->initiate((mod_vm_arch_t *) (m), entry))
#define mod_vm_arch_start(m) ((m)->p_ops->start((mod_vm_arch_t *) (m)))
#define mod_vm_arch_stop(m) ((m)->p_ops->stop((mod_vm_arch_t *) (m)))
#define mod_vm_arch_suspend(m) ((m)->p_ops->suspend((mod_vm_arch_t *) (m)))
#define mod_vm_arch_resume(m) ((m)->p_ops->resume((mod_vm_arch_t *) (m)))

#endif
