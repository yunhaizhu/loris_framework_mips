/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    cvm_memory_32.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#ifndef CVM_MEMORY_32_H
#define CVM_MEMORY_32_H

#include "mod_vm_memory.h"
#include "mod_vm_memory_m32.h"
#include "std_common.h"

/* MTS entry flags */
#define MTS_FLAG_DEV 0x000000001  /* Virtual device used */
#define MTS_FLAG_COW 0x000000002  /* Copy-On-Write */
#define MTS_FLAG_EXEC 0x000000004 /* Exec page */

/* Device ID mask and shift, device offset mask */
#define MTS_DEVID_MASK 0xfc000000
#define MTS_DEVID_SHIFT 26
#define MTS_DEVOFF_MASK 0x03fffff0


/* MTS mapping info */
typedef struct {
    std_u32_t vaddr;
    std_u32_t paddr;
    std_u64_t len;

    std_32_t cached;
    std_u32_t tlb_index;
    std_u8_t mapped;
    std_u8_t dirty;
    std_u8_t valid;
    std_u32_t asid;
    std_u8_t g_bit;
} mts_map_t;

/* Hash table size for MTS32 (default: [shift:15,bits:15]) */
#define MTS32_HASH_SHIFT 12
#define MTS32_HASH_BITS 14
#define MTS32_HASH_SIZE (1 << MTS32_HASH_BITS)
#define MTS32_HASH_MASK (MTS32_HASH_SIZE - 1)

/* MTS32 hash on virtual addresses */
#define MTS32_HASH(vaddr) (((vaddr) >> MTS32_HASH_SHIFT) & MTS32_HASH_MASK)

#define MIPS_MEMOP_LOOKUP 0
/**
 * mips_mts32_init
 * @brief   
 * @param   p_m
 * @return  std_int_t
 */
std_int_t mips_mts32_init(IN mod_vm_memory_t *p_m);

/**
 * cvm_memory_init
 * @brief   
 * @param   p_m
 * @return  std_int_t
 */
std_int_t cvm_memory_init(IN mod_vm_memory_t *p_m);

/**
 * cvm_memory_32_lookup
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @return  std_void_t *
 */
std_void_t *cvm_memory_32_lookup(IN mod_vm_memory_t *p_m, std_u32_t vaddr);

/**
 * cvm_memory_32_access
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @param   op_code
 * @param   op_size
 * @param   op_type
 * @param   data
 * @param   has_set_value
 * @return  STD_CALL fastcall std_void_t *
 */
STD_CALL fastcall std_void_t *cvm_memory_32_access(IN mod_vm_memory_t *p_m, std_u32_t vaddr, std_uint_t op_code, std_uint_t op_size, std_uint_t op_type, std_u32_t *data, std_u8_t *has_set_value);

#endif
