/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_vtty.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_VTTY_H
#define MOD_VM_DEVICE_VTTY_H

#include "mod.h"
#include "mod_ownership.h"
#include "std_common.h"

/*****************************************************
 *                                                   *
 *                      Define                       *
 *                                                   *
 ****************************************************/

/***struct_define***/
typedef struct vtty_serial_option {
    std_char_t *device;
    std_int_t baudrate;
    std_int_t data_bits;
    std_int_t parity;
    std_int_t stop_bits;
    std_int_t hw_flow;
} vtty_serial_option_t;

typedef struct vtty_tt {
} vtty_tt;

typedef void (*read_notifier_t) (vtty_tt *);

/***macro_define***/

/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/

typedef struct mod_vm_device_vtty_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_vtty_ops_st *p_ops;
} mod_vm_device_vtty_t;

struct mod_vm_device_vtty_ops_st {
    std_int_t (*init)(IN mod_vm_device_vtty_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_device_vtty_t *m);

    /***func_define***/
    std_void_t (*initiate)(IN mod_vm_device_vtty_t *m);
    vtty_tt *(*create)(IN mod_vm_device_vtty_t *m, IN std_char_t *name, IN std_int_t type, IN std_int_t tcp_port,
                      IN vtty_serial_option_t *option, IN read_notifier_t rnf);
    std_int_t (*get_char)(IN mod_vm_device_vtty_t *m, IN vtty_tt *vtty);
    std_void_t (*put_char)(IN mod_vm_device_vtty_t *m, IN vtty_tt *vtty, IN std_char_t ch);
    std_void_t (*put_buffer)(IN mod_vm_device_vtty_t *m, IN vtty_tt *vtty, IN std_char_t *buf,
                             IN std_uint_t len);
    std_int_t (*is_char_avail)(IN mod_vm_device_vtty_t *m, IN vtty_tt *vtty);
    std_int_t (*is_full)(IN mod_vm_device_vtty_t *m, IN vtty_tt *vtty);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_DEVICE_VTTY_IID MOD_GUID(0x47968a94, 0x9df9, 0x89e4, 0xf4, 0x56, 0xff, 0x4c, 0xcc, 0x38, 0xa2, 0xc0)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_device_vtty_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_device_vtty_t *) (m), arg, arg_len))
#define mod_vm_device_vtty_cleanup(m) ((m)->p_ops->cleanup((mod_vm_device_vtty_t *) (m)))

/***interface_define***/
#define mod_vm_device_vtty_initiate(m) ((m)->p_ops->initiate((mod_vm_device_vtty_t *) (m)))
#define mod_vm_device_vtty_create(m, name, type, tcp_port, option, rnf) ((m)->p_ops->create((mod_vm_device_vtty_t *) (m), name, type, tcp_port, option, rnf))
#define mod_vm_device_vtty_get_char(m, vtty) ((m)->p_ops->get_char((mod_vm_device_vtty_t *) (m), vtty))
#define mod_vm_device_vtty_put_char(m, vtty, ch) ((m)->p_ops->put_char((mod_vm_device_vtty_t *) (m), vtty, ch))
#define mod_vm_device_vtty_put_buffer(m, vtty, buf, len) ((m)->p_ops->put_buffer((mod_vm_device_vtty_t *) (m), vtty, buf, len))
#define mod_vm_device_vtty_is_char_avail(m, vtty) ((m)->p_ops->is_char_avail((mod_vm_device_vtty_t *) (m), vtty))
#define mod_vm_device_vtty_is_full(m, vtty) ((m)->p_ops->is_full((mod_vm_device_vtty_t *) (m), vtty))

#endif
