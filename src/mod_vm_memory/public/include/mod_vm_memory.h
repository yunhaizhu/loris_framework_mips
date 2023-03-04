/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    mod_vm_memory.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */


/**
     * @file    mod_vm_memory.h
     * @brief   define structure & functions
     * @version 1.0
     * @author  Yunhai Zhu
     * @date    2021-12-29
     *
     */
#ifndef MOD_VM_MEMORY_H
#define MOD_VM_MEMORY_H

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
/* Minimum page size: 4 Kb */
#define MIPS_MIN_PAGE_SHIFT 12
#define MIPS_MIN_PAGE_SIZE (1 << MIPS_MIN_PAGE_SHIFT)
#define MIPS_MIN_PAGE_IMASK (MIPS_MIN_PAGE_SIZE - 1)
#define MIPS_MIN_PAGE_MASK 0xfffffffffffff000ULL

/* Hash table size for MTS32 (default: [shift:15,bits:15]) */
#define MTS32_HASH_SHIFT 12
#define MTS32_HASH_BITS 14
#define MTS32_HASH_SIZE (1 << MTS32_HASH_BITS)
#define MTS32_HASH_MASK (MTS32_HASH_SIZE - 1)

/* MTS32 hash on virtual addresses */
#define MTS32_HASH(vaddr) (((vaddr) >> MTS32_HASH_SHIFT) & MTS32_HASH_MASK)

/* Virtual TLB entry (32-bit MMU) */
struct mts32_entry {
    std_u32_t gvpa; /* Guest Virtual Page Address */
    std_u32_t gppa; /* Guest Physical Page Address */
    std_u64_t hpa;  /* Host Page Address */
    std_u32_t asid;
    std_u8_t g_bit;
    std_u8_t dirty_bit;
    std_u8_t mapped;
    std_u32_t flags; /* Flags */
} __attribute__((aligned(16)));
typedef struct mts32_entry mts32_entry_t;

/* MTS operation */
#define MTS_READ 0
#define MTS_WRITE 1

#define MTS_BYTE 1
#define MTS_HALF_WORD 2
#define MTS_WORD 4

/* MTS entry flags */
#define MTS_FLAG_DEV 0x000000001  /* Virtual device used */
#define MTS_FLAG_COW 0x000000002  /* Copy-On-Write */
#define MTS_FLAG_EXEC 0x000000004 /* Exec page */
/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/

typedef struct mod_vm_memory_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_memory_ops_st *p_ops;

    /* MTS32/MTS64 caches */
    union {
        struct mts32_entry *mts32_cache;
    } mts_u;
} mod_vm_memory_t;

struct mod_vm_memory_ops_st {
    std_int_t (*init)(IN mod_vm_memory_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_memory_t *m);

    /***func_define***/
    std_int_t (*initiate)(IN mod_vm_memory_t *m);
    std_void_t *(*lookup)(IN mod_vm_memory_t *m, IN std_uint_t vaddr);
    std_void_t *(*access)(IN mod_vm_memory_t *m, IN std_uint_t vaddr, IN std_uint_t op_code, IN std_uint_t op_size,
                          IN std_uint_t op_type, IN std_uint_t *data,
                          IN std_uchar_t *has_set_value);
    std_void_t (*map)(IN mod_vm_memory_t *m, IN std_uint_t vaddr);
    std_void_t (*unmap)(IN mod_vm_memory_t *m, IN std_uint_t vaddr);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_MEMORY_IID MOD_GUID(0x09b130a1, 0x40eb, 0x2e02, 0x99, 0x64, 0x55, 0xfd, 0xfb, 0x41, 0x35, 0x9b)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_memory_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_memory_t *) (m), arg, arg_len))
#define mod_vm_memory_cleanup(m) ((m)->p_ops->cleanup((mod_vm_memory_t *) (m)))

/***interface_define***/
#define mod_vm_memory_initiate(m) ((m)->p_ops->initiate((mod_vm_memory_t *) (m)))
#define mod_vm_memory_lookup(m, vaddr) ((m)->p_ops->lookup((mod_vm_memory_t *) (m), vaddr))
#define mod_vm_memory_access(m, vaddr, op_code, op_size, op_type, data, has_set_value) ((m)->p_ops->access((mod_vm_memory_t *) (m), vaddr, op_code, op_size, op_type, data, has_set_value))
#define mod_vm_memory_map(m, vaddr) ((m)->p_ops->map((mod_vm_memory_t *) (m), vaddr))
#define mod_vm_memory_unmap(m, vaddr) ((m)->p_ops->unmap((mod_vm_memory_t *) (m), vaddr))

#endif
