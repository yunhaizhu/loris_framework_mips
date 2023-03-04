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
};

enum {
    VM_STATUS_HALTED = 0,
    VM_STATUS_SHUTDOWN,
    VM_STATUS_RUNNING,
    VM_STATUS_SUSPENDED
};

enum {
    VM_BOOT_METHOD_ELF = 0,
    VM_BOOT_METHOD_BINARY,
};

enum {
    VM_FLASH_TYPE_NOR = 0,
    VM_FLASH_TYPE_NAND,
};

typedef struct vm_system_info_s {
    std_char_t vm_name[BUF_SIZE_128];
    std_long_t vm_type;
    std_long_t vm_status;
    std_long_t vm_boot_method;
    std_long_t vm_boot_from;
    std_char_t vm_kernel_filename[BUF_SIZE_128];

    std_long_t vm_ram_size;
    std_long_t vm_rom_size;
    std_long_t vm_rom_address;

    std_long_t vm_flash_type;
    std_long_t vm_flash_size;
    std_char_t vm_flash_filename[BUF_SIZE_128];
    std_long_t vm_flash_address;

    std_long_t vm_gdb_debug;
    std_long_t vm_gdb_port;
} vm_system_info_t;


#endif
