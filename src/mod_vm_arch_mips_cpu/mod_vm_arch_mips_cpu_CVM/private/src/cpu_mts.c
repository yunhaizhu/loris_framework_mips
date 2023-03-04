//
// Created by yun on 2/27/22.
//

#include "cpu_mts.h"

/*****************BEGIN MTS32 ACCESS ******************/

/**
 * bad_memory_access
 * @brief
 * @param   cpu
 * @param   vaddr
 */
std_void_t bad_memory_access(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr)
{
    //not implement
}

/* LB: Load Byte */
fastcall std_void_t mips_mts32_lb(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;

    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LB, 1, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (likely(has_set_value == FALSE)) {
        data = *(std_u8_t *) haddr;
    }

    cpu->gpr[reg] = sign_extend(data, 8);
}

/* LBU: Load Byte Unsigned */
fastcall std_void_t mips_mts32_lbu(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LBU, 1, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(has_set_value == FALSE)) {
        data = *(std_u8_t *) haddr;
    }

    cpu->gpr[reg] = data & 0xff;
}

/* LH: Load Half-Word */
fastcall std_void_t mips_mts32_lh(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LH, 2, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(has_set_value == FALSE)) {
        data = vmtoh16(*(std_u16_t *) haddr);
    }

    cpu->gpr[reg] = sign_extend(data, 16);
}

/* LHU: Load Half-Word Unsigned */
fastcall std_void_t mips_mts32_lhu(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LHU, 2, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(has_set_value == FALSE)) {
        data = vmtoh16(*(std_u16_t *) haddr);
    }

    cpu->gpr[reg] = data & 0xffff;
}


/* LW: Load Word */
std_void_t fastcall mips_mts32_lw(cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LW, 4, MTS_READ, &data, &has_set_value);

    if (likely(has_set_value == FALSE)) {
        data = vmtoh32(*(std_u32_t *) haddr);
    }

    cpu->gpr[reg] = sign_extend(data, 32);
}


static forced_inline std_int_t mips_mts32_check_read_tlbcache(cvm_arch_mips_cpu_t *cpu,
                                                              std_u32_t vaddr,
                                                              mts32_entry_t *entry)
{
    if (unlikely((vaddr & MIPS_MIN_PAGE_MASK) != entry->gvpa)) {
        return 0;
    }

    if ((entry->mapped == TRUE) && (!entry->g_bit) && ((cpu->p_cp0_reg[MIPS_CP0_TLB_HI] & MIPS_TLB_ASID_MASK) != entry->asid)) {
        return 0;
    }

    return 1;
}

static forced_inline std_int_t mips_mts32_check_write_tlbcache(cvm_arch_mips_cpu_t *cpu,
                                                               std_u32_t vaddr,
                                                               mts32_entry_t *entry)
{
    if (unlikely((vaddr & MIPS_MIN_PAGE_MASK) != entry->gvpa)) {
        return 0;
    }

    if (unlikely(entry->mapped == TRUE)) {
        if (!entry->dirty_bit) {
            return 0;
        }

        if ((!entry->g_bit) && ((cpu->p_cp0_reg[MIPS_CP0_TLB_HI] & MIPS_TLB_ASID_MASK) != entry->asid)) {
            return 0;
        }
    }

    return 1;
}


/**
 * mips_mts32_fast_lw
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  std_void_t fastcall
 */
std_void_t fastcall mips_mts32_fast_lw(cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    mts32_entry_t *entry;
    std_u32_t hash_bucket;
    std_u64_t haddr;

    hash_bucket = MTS32_HASH(vaddr);
    entry = &cpu->mts_u.mts32_cache[hash_bucket];

    if (likely(mips_mts32_check_read_tlbcache(cpu, vaddr, entry) == 1)) {
        /* Raw memory access */
        haddr = entry->hpa + (vaddr & MIPS_MIN_PAGE_IMASK);
        data = vmtoh32(*(std_u32_t *) haddr);

        cpu->gpr[reg] = sign_extend(data, 32);

        return;
    }

    return mips_mts32_lw(cpu, vaddr, reg);
}


/**
 * mips_mts32_fast_sw
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_fast_sw(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    mts32_entry_t *entry;
    std_u32_t hash_bucket;
    std_u64_t haddr;

    hash_bucket = MTS32_HASH(vaddr);
    entry = &cpu->mts_u.mts32_cache[hash_bucket];

    if (likely(mips_mts32_check_write_tlbcache(cpu, vaddr, entry) == 1)) {
        haddr = entry->hpa + (vaddr & MIPS_MIN_PAGE_IMASK);

        data = cpu->gpr[reg] & 0xffffffff;
        *(std_u32_t *) haddr = htovm32(data);
        return;
    }

    return mips_mts32_sw(cpu, vaddr, reg);
}

/* LWU: Load Word Unsigned */
fastcall std_void_t mips_mts32_lwu(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LWU, 4, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(haddr != NULL)) {
        data = vmtoh32(*(std_u32_t *) haddr);
    }

    cpu->gpr[reg] = data & 0xffffffff;
}

/* LD: Load Double-Word */
fastcall std_void_t mips_mts32_ld(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LD, 8, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(has_set_value == FALSE)) {
        data = vmtoh64(*(std_u64_t *) haddr);
    }

    cpu->gpr[reg] = data;
}

/* SB: Store Byte */
fastcall std_void_t mips_mts32_sb(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    data = cpu->gpr[reg] & 0xff;
    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_SB, 1, MTS_WRITE, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
        return;
    }
    if (likely(has_set_value == FALSE)) {
        *(std_u8_t *) haddr = (std_u8_t) data;
    }
}

/* SH: Store Half-Word */
fastcall std_void_t mips_mts32_sh(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    data = cpu->gpr[reg] & 0xffff;
    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_SH, 2, MTS_WRITE, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
        return;
    }

    if (likely(has_set_value == FALSE)) {
        *(std_u16_t *) haddr = htovm16((std_u16_t) data);
    }
}

/* SW: Store Word */
fastcall std_void_t mips_mts32_sw(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    data = cpu->gpr[reg] & 0xffffffff;
    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_SW, 4, MTS_WRITE, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
        return;
    }

    if (likely(has_set_value == FALSE)) {
        *(std_u32_t *) haddr = htovm32(data);
    }
}


/* SD: Store Double-Word */
fastcall std_void_t mips_mts32_sd(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    data = cpu->gpr[reg];
    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_SD, 8, MTS_WRITE, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
        return;
    }
    if (likely(has_set_value == FALSE)) {
        *(std_u64_t *) haddr = htovm64(data);
    }
}


/**
 * mips_mts32_lwl
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lwl(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_void_t *haddr = NULL;
    std_u32_t data;
    std_u32_t naddr;
    std_u32_t shift = 0;
    std_u32_t mask1 = 0;
    std_u32_t mask2 = 0;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x03;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_LW, 4, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (has_set_value == FALSE) {
        data = vmtoh32(*(std_u32_t *) haddr);

        switch (vaddr & 0x3) {
            case 0x0:
                mask1 = 0xff;
                mask2 = 0xff000000;
                shift = 24;
                break;
            case 0x1:
                mask1 = 0xffff;
                mask2 = 0xffff0000;
                shift = 16;
                break;
            case 0x2:
                mask1 = 0xffffff;
                mask2 = 0xffffff00;
                shift = 8;
                break;
            case 0x3:
                mask1 = 0xffffffff;
                mask2 = 0xffffffff;
                shift = 0;
                break;
            default:
                break;
        }

        data = (data & mask1) << shift;
        data &= mask2;
        cpu->gpr[reg] &= ~mask2;
        cpu->gpr[reg] |= data;
        cpu->gpr[reg] = sign_extend(cpu->gpr[reg], 32);
    }
}


/* LWR: Load Word Right */

/**
 * mips_mts32_lwr
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lwr(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_void_t *haddr = NULL;
    std_u32_t data;
    std_u32_t naddr;
    std_u32_t shift = 0;
    std_u32_t mask1 = 0;
    std_u32_t mask2 = 0;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x03;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_LW, 4, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (has_set_value == FALSE) {
        data = vmtoh32(*(std_u32_t *) haddr);
        switch (vaddr & 0x3) {
            case 0x3:
                mask1 = 0xff;
                mask2 = 0xff000000;
                shift = 24;
                break;
            case 0x2:
                mask1 = 0xffff;
                mask2 = 0xffff0000;
                shift = 16;
                break;
            case 0x1:
                mask1 = 0xffffff;
                mask2 = 0xffffff00;
                shift = 8;
                break;
            case 0x0:
                mask1 = 0xffffffff;
                mask2 = 0xffffffff;
                shift = 0;
                break;
            default:
                break;
        }

        data = (data & mask2) >> shift;
        data &= mask1;
        cpu->gpr[reg] &= ~mask1;
        cpu->gpr[reg] |= data;
        cpu->gpr[reg] = sign_extend(cpu->gpr[reg], 32);
    }
}


/* LDL: Load Double-Word Left */
fastcall std_void_t mips_mts32_ldl(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t r_mask;
    std_u32_t naddr;
    std_u32_t data;
    std_uint_t m_shift;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x07;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_LDL, 8, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (likely(haddr != NULL)) {
        data = (std_u32_t) (vmtoh64(*(std_u64_t *) haddr));
    }


    m_shift = (vaddr & 0x07) << 3;
    r_mask = (1ULL << m_shift) - 1;
    data <<= m_shift;

    cpu->gpr[reg] &= r_mask;
    cpu->gpr[reg] |= data;
}

/* LDR: Load Double-Word Right */
fastcall std_void_t mips_mts32_ldr(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t r_mask;
    std_u32_t naddr;
    std_u32_t data;
    std_uint_t m_shift;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x07;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_LDR, 8, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (likely(haddr != NULL))
        data = (std_u32_t) (vmtoh64(*(std_u64_t *) haddr));


    m_shift = ((vaddr & 0x07) + 1) << 3;
    r_mask = (1ULL << m_shift) - 1;
    data >>= (64 - m_shift);

    cpu->gpr[reg] &= ~r_mask;
    cpu->gpr[reg] |= data;
}


/**
 * mips_mts32_swl
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_swl(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_void_t *haddr = NULL;
    std_u32_t data;
    std_u32_t naddr;
    std_u32_t temp;
    std_u32_t mask1 = 0;
    std_u32_t mask2 = 0;
    std_u32_t shift = 0;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x03ULL;
    data = cpu->gpr[reg] & 0xffffffff;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_SW, 4, MTS_WRITE, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (has_set_value == FALSE) {
        switch (vaddr & 0x3) {
            case 0x0:
                mask1 = 0xff;
                mask2 = 0xff000000;
                shift = 24;
                break;
            case 0x1:
                mask1 = 0xffff;
                mask2 = 0xffff0000;
                shift = 16;
                break;
            case 0x2:
                mask1 = 0xffffff;
                mask2 = 0xffffff00;
                shift = 8;
                break;
            case 0x3:
                mask1 = 0xffffffff;
                mask2 = 0xffffffff;
                shift = 0;
                break;
            default:
                break;
        }

        data = (data & mask2) >> shift;
        data &= mask1;
        temp = vmtoh32(*(std_u32_t *) haddr);

        temp &= ~mask1;
        temp = temp | data;
        *(std_u32_t *) haddr = htovm32(temp);
    }
}


/**
 * mips_mts32_swr
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_swr(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_void_t *haddr = NULL;
    std_u32_t data;
    std_u32_t naddr;
    std_u32_t temp;
    std_u32_t mask1 = 0;
    std_u32_t mask2 = 0;
    std_u32_t shift = 0;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x03ULL;
    data = cpu->gpr[reg] & 0xffffffff;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_SW, 4, MTS_WRITE, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (has_set_value == FALSE) {
        switch (vaddr & 0x3) {
            case 0x3:
                mask1 = 0xff;
                mask2 = 0xff000000;
                shift = 24;
                break;
            case 0x2:
                mask1 = 0xffff;
                mask2 = 0xffff0000;
                shift = 16;
                break;
            case 0x1:
                mask1 = 0xffffff;
                mask2 = 0xffffff00;
                shift = 8;
                break;
            case 0x0:
                mask1 = 0xffffffff;
                mask2 = 0xffffffff;
                shift = 0;
                break;
            default:
                break;
        }

        data = (data & mask1) << shift;
        data &= mask2;
        temp = vmtoh32(*(std_u32_t *) haddr);

        temp &= ~mask2;
        temp = temp | data;
        *(std_u32_t *) haddr = htovm32(temp);
    }
}


/* SDL: Store Double-Word Left */
fastcall std_void_t mips_mts32_sdl(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t d_mask;
    std_u32_t naddr;
    std_u32_t data;
    std_uint_t r_shift;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x07;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_SDL, 8, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (likely(haddr != NULL))
        data = (std_u32_t) (vmtoh64(*(std_u64_t *) haddr));

    r_shift = (vaddr & 0x07) << 3;
    d_mask = 0xffffffffffffffffULL >> r_shift;

    data &= ~d_mask;
    data |= cpu->gpr[reg] >> r_shift;

    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_SDL, 8, MTS_WRITE, &data, &has_set_value);
    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(haddr != NULL)) {
        *(std_u32_t *) haddr = htovm64(data);
    }
}

/* SDR: Store Double-Word Right */
fastcall std_void_t mips_mts32_sdr(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t d_mask;
    std_u32_t naddr;
    std_u32_t data;
    std_uint_t r_shift;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    naddr = vaddr & ~0x07;
    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_SDR, 8, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }

    if (likely(haddr != NULL)) {
        data = vmtoh64(*(std_u64_t *) haddr);
    }

    r_shift = ((vaddr & 0x07) + 1) << 3;
    d_mask = 0xffffffffffffffffULL >> r_shift;

    data &= d_mask;
    data |= cpu->gpr[reg] << (64 - r_shift);

    haddr = mod_vm_memory_access(p_global_vm_memory, naddr, MIPS_MEMOP_SDR, 8, MTS_WRITE, &data, &has_set_value);
    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(haddr != NULL)) {
        *(std_u32_t *) haddr = htovm64(data);
    }
}

/* LL: Load Linked */
fastcall std_void_t mips_mts32_ll(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_LL, 4, MTS_READ, &data, &has_set_value);

    if ((haddr == NULL) && (has_set_value == FALSE)) {
        bad_memory_access(cpu, vaddr);
    }
    if (likely(haddr != NULL)) {
        data = vmtoh32(*(std_u32_t *) haddr);
    }


    cpu->gpr[reg] = sign_extend(data, 32);
    cpu->ll_bit = 1;
}

/* SC: Store Conditional */
fastcall std_void_t mips_mts32_sc(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t reg)
{
    std_u32_t data;
    std_void_t *haddr;
    std_u8_t has_set_value = FALSE;

    if (cpu->ll_bit) {
        data = cpu->gpr[reg] & 0xffffffff;
        haddr = mod_vm_memory_access(p_global_vm_memory, vaddr, MIPS_MEMOP_SC, 4, MTS_WRITE,
                                     &data, &has_set_value);

        if ((haddr == NULL) && (has_set_value == FALSE)) {
            bad_memory_access(cpu, vaddr);
        }
        if (likely(haddr != NULL)) {
            *(std_u32_t *) haddr = htovm32(data);
        }
    }


    cpu->gpr[reg] = cpu->ll_bit;
}


/**
 * mips_mts32_cache
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   op
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_cache(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, IN std_uint_t op)
{
    //not implement
}


/*****************MTS32 ACCESS******************/