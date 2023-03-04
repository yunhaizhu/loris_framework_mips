/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    cvm_memory_32.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#include "cvm_memory_32.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_device.h"
#include "mod_vm_device_manager.h"
#include "mod_vm_memory.h"

extern mod_vm_device_manager_t *p_global_dev_manger;
extern mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;



/**
 * mips_mts32_map
 * @brief   
 * @param   op_type
 * @param   map
 * @param   entry
 * @param   alt_entry
 * @return  mts32_entry_t *
 */
mts32_entry_t *mips_mts32_map(IN std_uint_t op_type, const mts_map_t *map, mts32_entry_t *entry, mts32_entry_t *alt_entry)
{
    mod_vm_device_t *dev = NULL;
    std_u32_t offset;

    mod_vm_device_manager_find(p_global_dev_manger, map->paddr, -1, &dev);

    if (dev == NULL) {
        return NULL;
    }

    if (!dev->info.host_addr || (dev->info.flags & VDEVICE_FLAG_NO_MTS_MMAP)) {
        offset = map->paddr - dev->info.phys_addr;

        alt_entry->gvpa = map->vaddr;
        alt_entry->gppa = map->paddr;
        alt_entry->hpa = (dev->info.id << MTS_DEVID_SHIFT) + offset;
        alt_entry->flags = MTS_FLAG_DEV;
        alt_entry->mapped = map->mapped;
        return alt_entry;
    }

    entry->gvpa = map->vaddr;
    entry->gppa = map->paddr;
    entry->hpa = dev->info.host_addr + (map->paddr - dev->info.phys_addr);
    entry->flags = 0;
    entry->asid = map->asid;
    entry->g_bit = map->g_bit;
    entry->dirty_bit = map->dirty;
    entry->mapped = map->mapped;
    return entry;
}


/**
 * mips_mts32_slow_lookup
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @param   op_code
 * @param   op_size
 * @param   op_type
 * @param   data
 * @param   alt_entry
 * @return  static mts32_entry_t *
 */
static mts32_entry_t *mips_mts32_slow_lookup(IN mod_vm_memory_t *p_m, std_u32_t vaddr, std_u32_t op_code, std_u32_t op_size, std_u32_t op_type, std_u32_t *data, mts32_entry_t *alt_entry)
{
    mod_vm_memory_imp_t *p_imp_m = (mod_vm_memory_imp_t *) p_m;
    std_u32_t hash_bucket;
    std_u32_t zone;
    mts32_entry_t *entry;
    mts_map_t map;

REDO:
    map.tlb_index = -1;
    zone = (vaddr >> 29) & 0x7;

    hash_bucket = MTS32_HASH(vaddr);
    entry = &p_imp_m->mts_u.mts32_cache[hash_bucket];

    p_imp_m->mts_misses++;
    switch (zone) {
        case 0x04: /* kseg0 */
            map.vaddr = vaddr & MIPS_MIN_PAGE_MASK;
            map.paddr = map.vaddr - (std_u32_t) 0xFFFFFFFF80000000ULL;
            map.mapped = FALSE;
            map.asid = 0;
            map.g_bit = 1;
            map.dirty = 0;
            if (!(entry = mips_mts32_map(op_type, &map, entry, alt_entry))){
                goto ERR_UNDEF;
            }

            return entry;

        case 0x05: /* kseg1 */
            map.vaddr = vaddr & MIPS_MIN_PAGE_MASK;
            map.paddr = map.vaddr - (std_u32_t) 0xFFFFFFFFA0000000ULL;
            map.mapped = FALSE;
            map.asid = 0;
            map.g_bit = 1;
            map.dirty = 0;
            if (!(entry = mips_mts32_map(op_type, &map, entry, alt_entry))){
                goto ERR_UNDEF;
            }

            return entry;

        case 0x00 ... 0x03: /* kuseg */
        case 0x06: /* ksseg */
        case 0x07: /* kseg3 */
            /* trigger TLB exception if no matching entry found */
            if (!mod_vm_arch_mips_cp0_tlb_lookup(p_global_vm_arch_mips_cp0, vaddr, &map)){
                goto ERR_TLB;
            }
            /////
            map.valid = TRUE;
            map.dirty = TRUE;
            ////
            if ((map.valid & 0x1) != 0x1){
                goto ERR_TLB;
            }

            if ((MTS_WRITE == op_type) && ((map.dirty & 0x1) != 0x1)){
                goto ERR_MOD;
            }

            map.mapped = TRUE;
            if (!(entry = mips_mts32_map(op_type, &map, entry, alt_entry))){
                goto ERR_UNDEF;
            }

            return entry;
        default:
            break;
    }

ERR_MOD:
    mod_vm_arch_mips_cp0_access_special(p_global_vm_arch_mips_cp0, vaddr, MTS_ACC_M, op_code, op_type, op_size, data);
    return NULL;

ERR_UNDEF:
    mod_vm_arch_mips_cp0_access_special(p_global_vm_arch_mips_cp0, vaddr, MTS_ACC_U, op_code, op_type, op_size, data);
    return NULL;

ERR_TLB:
    mod_vm_arch_mips_cp0_access_special(p_global_vm_arch_mips_cp0, vaddr, MTS_ACC_T, op_code, op_type, op_size, data);
    goto REDO;
}


/**
 * dev_access_fast
 * @brief   
 * @param   vaddr
 * @param   offset
 * @param   op_size
 * @param   op_type
 * @param   data
 * @param   has_set_value
 * @return  static forced_inline std_void_t *
 */
static forced_inline std_void_t *dev_access_fast(std_u32_t vaddr, std_u32_t offset, std_int_t dev_id, std_uint_t op_size, std_uint_t op_type, std_u32_t *data, std_u8_t *has_set_value)
{
    mod_vm_device_t *dev = NULL;
    vm_device_access_t access_argv;

    mod_vm_device_manager_find(p_global_dev_manger, vaddr, dev_id, &dev);

    if (unlikely(!dev)) {
        STD_LOG(DEBUG, "dev_access_fast null  handler (dev_id=%u,offset=0x%x)\n", vaddr, offset);
        return NULL;
    }

    access_argv.offset = offset;
    access_argv.op_size = op_size;
    access_argv.op_type = op_type;
    access_argv.data = data;
    access_argv.has_set_value = has_set_value;

    return mod_vm_device_access(dev, &access_argv);
}

/* Initialize the MTS subsystem for the specified CPU */
std_int_t mips_mts32_init(IN mod_vm_memory_t *p_m)
{
    mod_vm_memory_imp_t *p_imp_m = (mod_vm_memory_imp_t *) p_m;
    size_t len;

    /* Initialize the cache entries to 0 (empty) */
    len = MTS32_HASH_SIZE * sizeof(mts32_entry_t);
    if (!(p_imp_m->mts_u.mts32_cache = malloc(len))) {
        return -1;
    }

    memset(p_imp_m->mts_u.mts32_cache, 0xFF, len);
    p_imp_m->mts_lookups = 0;
    p_imp_m->mts_misses = 0;
    return 0;
}


/**
 * mips_mts32_check_tlbcache
 * @brief   
 * @param   vaddr
 * @param   op_type
 * @param   entry
 * @return  static forced_inline std_int_t
 */
static forced_inline std_int_t mips_mts32_check_tlbcache(std_u32_t vaddr, std_uint_t op_type, const mts32_entry_t *entry)
{
    std_u32_t asid;

    if (unlikely((vaddr & MIPS_MIN_PAGE_MASK) != entry->gvpa)) {
        return 0;
    }

    if (entry->mapped == TRUE) {
        if ((op_type == MTS_WRITE) && (!entry->dirty_bit)) {
            return 0;
        }
        asid = p_global_vm_arch_mips_cp0->reg[MIPS_CP0_TLB_HI] & MIPS_TLB_ASID_MASK;
        if ((!entry->g_bit) && (asid != entry->asid)) {
            return 0;
        }
    }

    return 1;
}

/**
 * cvm_memory_init
 * @brief
 * @param   p_m
 * @return  std_int_t
 */
std_int_t cvm_memory_init(IN mod_vm_memory_t *p_m)
{
    mips_mts32_init(p_m);
    return 0;
}

/**
 * cvm_memory_32_lookup
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @return  std_void_t *
 */
std_void_t *cvm_memory_32_lookup(IN mod_vm_memory_t *p_m, std_u32_t vaddr)
{
    std_u32_t data;
    std_u8_t has_set_value = FALSE;

    return cvm_memory_32_access(p_m, vaddr, MIPS_MEMOP_LOOKUP, MTS_WORD, MTS_READ, &data, &has_set_value);
}


/* MTS32 access */
STD_CALL fastcall std_void_t *cvm_memory_32_access(IN mod_vm_memory_t *p_m, std_u32_t vaddr, std_uint_t op_code, std_uint_t op_size, std_uint_t op_type, std_u32_t *data, std_u8_t *has_set_value)
{
    mod_vm_memory_imp_t *p_imp_m = (mod_vm_memory_imp_t *) p_m;
    const mts32_entry_t *entry;
    mts32_entry_t alt_entry;
    std_u32_t hash_bucket;
    std_u64_t haddr;

    hash_bucket = MTS32_HASH(vaddr);
    entry = &p_imp_m->mts_u.mts32_cache[hash_bucket];

    p_imp_m->mts_lookups++;

    if (unlikely(mips_mts32_check_tlbcache(vaddr, op_type, entry) == 0)) {
        entry = mips_mts32_slow_lookup(p_m, vaddr, op_code, op_size, op_type,
                                       data, &alt_entry);
        if (!entry)
            return NULL;
        if (entry->flags & MTS_FLAG_DEV) {
            std_int_t dev_id;
            dev_id = (entry->hpa & MTS_DEVID_MASK) >> MTS_DEVID_SHIFT;
            haddr = entry->hpa & MTS_DEVOFF_MASK;
            haddr += vaddr - entry->gvpa;

            return (dev_access_fast(entry->gppa, (std_u32_t)haddr, dev_id, op_size, op_type, data, has_set_value));
        }
    }

    /* Raw memory access */
    haddr = entry->hpa + (vaddr & MIPS_MIN_PAGE_IMASK);
    return (std_void_t *) haddr;
}
