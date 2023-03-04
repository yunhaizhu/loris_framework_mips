/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    vm.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#ifndef VM_H
#define VM_H

#include "std_common.h"
enum {
    VM_TYPE_ADM5120 = 0,
    VM_TYPE_BCM462,
    VM_TYPE_MINI
};

enum {
    VM_STATUS_HALTED = 0,
    VM_STATUS_SHUTDOWN,
    VM_STATUS_RUNNING,
    VM_STATUS_SUSPENDED
};


typedef struct vm_system_info_s {
    std_char_t vm_name[BUF_SIZE_128];
    std_long_t vm_type;
    std_long_t vm_status;

} vm_system_info_t;


#endif
