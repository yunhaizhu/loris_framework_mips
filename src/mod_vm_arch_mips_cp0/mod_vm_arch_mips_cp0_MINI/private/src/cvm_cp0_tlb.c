
#include "cvm_cp0_tlb.h"

/* Get the page size corresponding to a page mask */
static inline std_u32_t get_page_size(std_u32_t page_mask)
{
    return ((page_mask + 0x2000) >> 1);
}

/**
 * mips64_cp0_unmap_tlb_to_mts
 * @brief
 * @param   cp0
 * @param   index
 */
std_void_t mips64_cp0_unmap_tlb_to_mts(IN const cvm_arch_mips_cp0_t *cp0, IN int index)
{
#if 0
	std_u32_t v0_addr,v1_addr;
	std_u32_t page_size;
	tlb_entry_t *entry;

	entry = &cp0->tlb[index];

	page_size = get_page_size(entry->mask);
	v0_addr = entry->hi & mips64_cp0_get_vpn2_mask(cp0);
	v1_addr = v0_addr + page_size;


	if (entry->lo0 & MIPS_TLB_V_MASK)
		cpu->mts_unmap(cpu, v0_addr, page_size, MTS_ACC_T, index);

	if (entry->lo1 & MIPS_TLB_V_MASK)
		cpu->mts_unmap(cpu, v1_addr, page_size, MTS_ACC_T, index);
#endif
}

/* TLBP: Probe a TLB entry */
fastcall std_void_t mips64_cp0_exec_tlbp(cvm_arch_mips_cp0_t *cp0)
{
    std_u32_t vpn2;
    std_u32_t hi_reg;
    std_u32_t vpn2_mask;
    std_u32_t page_mask;
    std_u32_t hi_addr;
    const tlb_entry_t *entry;
    std_uint_t asid;

    vpn2_mask = mips64_cp0_get_vpn2_mask(cp0);
    hi_reg = cp0->reg[MIPS_CP0_TLB_HI];
    asid = hi_reg & MIPS_TLB_ASID_MASK;
    vpn2 = hi_reg & vpn2_mask;

    cp0->reg[MIPS_CP0_INDEX] = 0x80000000;
    for (std_int_t i = 0; i < cp0->tlb_entries; i++) {
        entry = &cp0->tlb[i];
        page_mask = ~(entry->mask + 0x1FFF);
        hi_addr = entry->hi & mips64_cp0_get_vpn2_mask(cp0);
        if (((vpn2 & page_mask) == (hi_addr & page_mask)) &&
            ((entry->hi & MIPS_TLB_G_MASK) || ((entry->hi & MIPS_TLB_ASID_MASK) == asid))) {
            cp0->reg[MIPS_CP0_INDEX] = i;
            cp0->reg[MIPS_CP0_INDEX] &= ~0x80000000ULL;
            return;
        }
    }
}


/* TLBW: Write a TLB entry */
static forced_inline std_void_t mips64_cp0_exec_tlbw(cvm_arch_mips_cp0_t *cp0, std_uint_t index)
{
    tlb_entry_t *entry;

    if (index < cp0->tlb_entries) {
        entry = &cp0->tlb[index];

        /* Unmap the old entry if it was valid */
        mips64_cp0_unmap_tlb_to_mts(cp0, (std_int_t) index);

        entry->mask = cp0->reg[MIPS_CP0_PAGEMASK];
        entry->hi = cp0->reg[MIPS_CP0_TLB_HI];
        entry->lo0 = cp0->reg[MIPS_CP0_TLB_LO_0];
        entry->lo1 = cp0->reg[MIPS_CP0_TLB_LO_1];
        /* if G bit is set in lo0 and lo1, set it in hi */
        if ((entry->lo0 & entry->lo1) & MIPS_CP0_LO_G_MASK) {
            entry->hi |= MIPS_TLB_G_MASK;
        }

        /* Clear G bit in TLB lo0 and lo1 */
        entry->lo0 &= ~MIPS_CP0_LO_G_MASK;
        entry->lo1 &= ~MIPS_CP0_LO_G_MASK;
    }
}

/* TLBWR: Write Random TLB entry */
fastcall std_void_t  mips64_cp0_exec_tlbwr(cvm_arch_mips_cp0_t *cp0)
{
    mips64_cp0_exec_tlbw(cp0, mips64_cp0_get_random_reg(cp0));
}


/* TLBWI: Write Indexed TLB entry */
fastcall std_void_t  mips64_cp0_exec_tlbwi(cvm_arch_mips_cp0_t *cp0)
{
    std_u32_t index;

    if (cp0->reg[MIPS_CP0_INDEX] & 0x80000000) {
        mips64_cp0_exec_tlbwr(cp0);
    } else {
        index = cp0->reg[MIPS_CP0_INDEX];
        mips64_cp0_exec_tlbw(cp0, index);
    }
}


/* TLBR: Read Indexed TLB entry */
fastcall std_void_t  mips64_cp0_exec_tlbr(cvm_arch_mips_cp0_t *cp0)
{
    const tlb_entry_t *entry;
    std_uint_t index;

    index = cp0->reg[MIPS_CP0_INDEX];

    if (index < cp0->tlb_entries) {
        entry = &cp0->tlb[index];

        cp0->reg[MIPS_CP0_PAGEMASK] = entry->mask;
        cp0->reg[MIPS_CP0_TLB_HI] = entry->hi;
        cp0->reg[MIPS_CP0_TLB_LO_0] = entry->lo0;
        cp0->reg[MIPS_CP0_TLB_LO_1] = entry->lo1;
        if (entry->hi & MIPS_TLB_G_MASK) {
            cp0->reg[MIPS_CP0_TLB_LO_0] |= MIPS_CP0_LO_G_MASK;
            cp0->reg[MIPS_CP0_TLB_LO_1] |= MIPS_CP0_LO_G_MASK;
            cp0->reg[MIPS_CP0_TLB_HI] &= ~MIPS_TLB_G_MASK;
        }
    }
}


/**
 * mips64_cp0_tlb_lookup
 * @brief
 * @param   cp0
 * @param   vaddr
 * @param   res
 * @return  int
 */
std_int_t mips64_cp0_tlb_lookup(const cvm_arch_mips_cp0_t *cp0, std_u32_t vaddr, mts_map_t *res)
{
    std_u32_t vpn_addr;
    std_u32_t hi_addr;
    std_u32_t page_mask;
    std_u32_t page_size;
    const tlb_entry_t *entry;
    std_uint_t asid;

    vpn_addr = vaddr & mips64_cp0_get_vpn2_mask(cp0);

    asid = cp0->reg[MIPS_CP0_TLB_HI] & MIPS_TLB_ASID_MASK;
    for (std_int_t i = 0; i < cp0->tlb_entries; i++) {
        entry = &cp0->tlb[i];
        page_mask = ~(entry->mask + 0x1FFF);
        hi_addr = entry->hi & mips64_cp0_get_vpn2_mask(cp0);

        if (((vpn_addr & page_mask) == (hi_addr & page_mask)) &&
            ((entry->hi & MIPS_TLB_G_MASK) || ((entry->hi & MIPS_TLB_ASID_MASK) == asid))) {
            page_size = get_page_size(entry->mask);
            if ((vaddr & page_size) == 0) {
                res->tlb_index = i;

                res->vaddr = vaddr & MIPS_MIN_PAGE_MASK;
                res->paddr = (entry->lo0 & MIPS_TLB_PFN_MASK) << 6;
                res->paddr += ((res->vaddr) & (page_size - 1));
                res->paddr &= cp0->addr_bus_mask;

                res->dirty = (entry->lo0 & MIPS_TLB_D_MASK) >> MIPS_TLB_D_SHIT;
                res->valid = (entry->lo0 & MIPS_TLB_V_MASK) >> MIPS_TLB_V_SHIT;
                res->asid = asid;
                res->g_bit = (std_u8_t) (entry->hi & MIPS_TLB_G_MASK);

                return TRUE;
            } else {
                res->tlb_index = i;
                res->vaddr = vaddr & MIPS_MIN_PAGE_MASK;
                res->paddr = (entry->lo1 & MIPS_TLB_PFN_MASK) << 6;
                res->paddr += ((res->vaddr) & (page_size - 1));
                res->paddr &= cp0->addr_bus_mask;

                res->dirty = (entry->lo1 & MIPS_TLB_D_MASK) >> MIPS_TLB_D_SHIT;
                res->valid = (entry->lo1 & MIPS_TLB_V_MASK) >> MIPS_TLB_V_SHIT;

                res->asid = asid;
                res->g_bit = (std_u8_t) (entry->hi & MIPS_TLB_G_MASK);
                return TRUE;
            }
        }
    }
    return FALSE;
}