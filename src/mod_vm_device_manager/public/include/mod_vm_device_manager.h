/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_manager.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_MANAGER_H
#define MOD_VM_DEVICE_MANAGER_H

#include "mod.h"
#include "mod_ownership.h"
#include "mod_vm_device.h"
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

typedef struct mod_vm_device_manager_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_manager_ops_st *p_ops;
} mod_vm_device_manager_t;

struct mod_vm_device_manager_ops_st {
    std_int_t (*init)(IN mod_vm_device_manager_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_device_manager_t *m);

    /***func_define***/
	std_void_t(*add) (IN mod_vm_device_manager_t * m, IN mod_vm_device_t * p_dev, IN std_uint_t base,
			  IN std_uint_t len);
	std_void_t(*del) (IN mod_vm_device_manager_t * m, IN mod_vm_device_t * p_dev);
	std_void_t(*find) (IN mod_vm_device_manager_t * m, IN std_uint_t addr, IN std_int_t dev_id, IN mod_vm_device_t ** pp_dev);

};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_DEVICE_MANAGER_IID MOD_GUID(0x080129f1, 0x0dc5, 0xb86b, 0xe1, 0x44, 0xc1, 0x76, 0xcc, 0x5b, 0x70, 0x06)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_device_manager_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_device_manager_t *) (m), arg, arg_len))
#define mod_vm_device_manager_cleanup(m) ((m)->p_ops->cleanup((mod_vm_device_manager_t *) (m)))

/***interface_define***/
#define mod_vm_device_manager_add(m, p_dev, base, len) ((m)->p_ops->add((mod_vm_device_manager_t *)(m), p_dev, base, len))
#define mod_vm_device_manager_del(m, p_dev) ((m)->p_ops->del((mod_vm_device_manager_t *)(m), p_dev))
#define mod_vm_device_manager_find(m, addr, dev_id, pp_dev) ((m)->p_ops->find((mod_vm_device_manager_t *)(m), addr, dev_id, pp_dev))

#endif
