/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_H
#define MOD_VM_DEVICE_H

#include "mod.h"
#include "mod_ownership.h"
#include "std_common.h"

/*****************************************************
 *                                                   *
 *                      Define                       *
 *                                                   *
 ****************************************************/

/***struct_define***/
typedef struct vm_device_info_st {
    std_char_t name[BUF_SIZE_32];
    std_uint_t id;
    std_uint_t phys_addr;
    std_uint_t phys_len;
    std_u64_t host_addr;
    std_int_t flags;
} vm_device_info_t;

typedef struct vm_device_access_st {
    std_uint_t offset;
    std_uint_t op_size;
    std_uint_t op_type;
    std_uint_t *data;
    std_uchar_t *has_set_value;
} vm_device_access_t;

/***macro_define***/
/* Device Flags */
#define VDEVICE_FLAG_NO_MTS_MMAP  0x01  /* Prevent MMAPed access by MTS */
#define VDEVICE_FLAG_CACHING      0x02  /* Device does support caching */
#define VDEVICE_FLAG_REMAP        0x04  /* Physical address remapping */
#define VDEVICE_FLAG_COW          0x08  /* Copy on write device  */
/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/

typedef struct mod_vm_device_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;

    vm_device_info_t info;
} mod_vm_device_t;

struct mod_vm_device_ops_st {
    std_int_t (*init)(IN mod_vm_device_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_device_t *m);

    /***func_define***/
    std_void_t (*initiate)(IN mod_vm_device_t *m, IN vm_device_info_t *arg);
    std_void_t (*reset)(IN mod_vm_device_t *m);
    std_void_t *(*access)(IN mod_vm_device_t *m, IN vm_device_access_t *arg);
    std_void_t (*command)(IN mod_vm_device_t *m, IN std_void_t *arg);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_DEVICE_IID MOD_GUID(0xaeee373f, 0xf721, 0xf79d, 0x0a, 0xa3, 0xd3, 0x5f, 0xdc, 0xcd, 0xd9, 0x10)
#define MOD_VM_DEVICE_ETH_CS8900_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x7f, 0x31)
#define MOD_VM_DEVICE_GPIO_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x8f, 0x51)
#define MOD_VM_DEVICE_INT_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x6f, 0x0f)
#define MOD_VM_DEVICE_MPMC_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x6f, 0x0d)
#define MOD_VM_DEVICE_NORFLASH4M_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x31, 0x1f)
#define MOD_VM_DEVICE_PCI_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x6f, 0x11)
#define MOD_VM_DEVICE_RAM_IID MOD_GUID(0x5a770f8e, 0xd739, 0xb61c, 0x7f, 0xac, 0xf0, 0xa2, 0x7e, 0xb5, 0x6e, 0x10)
#define MOD_VM_DEVICE_SW_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x6f, 0x0e)
#define MOD_VM_DEVICE_UART_IID MOD_GUID(0x5a770f8f, 0xd730, 0xb61c, 0x7f, 0xac, 0xf0, 0xa3, 0x7f, 0xb6, 0x6f, 0x09)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_device_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_device_t *) (m), arg, arg_len))
#define mod_vm_device_cleanup(m) ((m)->p_ops->cleanup((mod_vm_device_t *) (m)))

/***interface_define***/
#define mod_vm_device_initiate(m, arg) ((m)->p_ops->initiate((mod_vm_device_t *) (m), arg))
#define mod_vm_device_reset(m) ((m)->p_ops->reset((mod_vm_device_t *) (m)))
#define mod_vm_device_access(m, arg) ((m)->p_ops->access((mod_vm_device_t *) (m), arg))
#define mod_vm_device_command(m, arg) ((m)->p_ops->command((mod_vm_device_t *) (m), arg))

#endif
