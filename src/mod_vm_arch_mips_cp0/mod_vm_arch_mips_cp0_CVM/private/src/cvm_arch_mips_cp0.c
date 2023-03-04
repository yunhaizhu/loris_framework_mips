/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    cvm_arch_mips_cp0.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#include "cvm_arch_mips_cp0.h"
#include "mod_vm_arch_mips_cpu.h"
#include <setjmp.h>

extern mod_vm_arch_mips_cpu_t *pg_vm_arch_mips_cpu;

/* MIPS cp0 registers names */
char *mips64_cp0_reg_names[MIPS64_CP0_REG_NR] = {
        "index",
        "random",
        "entry_lo",
        "cp0_r3",
        "context",
        "cp0_r5",
        "wired",
        "cp0_r7",
        "badvaddr",
        "cp0_r9",
        "entry_hi",
        "cp0_r11",
        "status",
        "cause",
        "epc",
        "prid",
        "dreg",
        "depc",
        "cp0_r18",
        "cp0_r19",
        "cctl",
        "cp0_r21",
        "cp0_r22",
        "cp0_r23",
        "cp0_r24",
        "cp0_r25",
        "cp0_r26",
        "cp0_r27",
        "cp0_r28",
        "cp0_r29",
        "cp0_r30",
        "desave",
};

/* Get the VPN2 mask */
std_u32_t mips64_cp0_get_vpn2_mask(const cvm_arch_mips_cp0_t *cp0)
{
    if (cp0->addr_mode == 64) {
        return ((std_u32_t) MIPS_TLB_VPN2_MASK_64);
    } else {
        return MIPS_TLB_VPN2_MASK_32;
    }
}

/* Execute fpu instruction */
fastcall std_void_t mips64_cp0_soft_fpu(cvm_arch_mips_cp0_t *cp0)
{
    cp0->reg[MIPS_CP0_CAUSE] |= 0x10000000;//CE=1

    if (mod_vm_arch_mips_cpu_is_in_bdslot(pg_vm_arch_mips_cpu) == 0) {
        mips64_trigger_exception(cp0, MIPS_CP0_CAUSE_CP_UNUSABLE, 0);
    } else {
        mips64_trigger_exception(cp0, MIPS_CP0_CAUSE_CP_UNUSABLE, 1);
    }
}

fastcall std_void_t mips64_cp0_timer(cvm_arch_mips_cp0_t *cp0)
{
    /*check count and compare*/
    cp0->reg[MIPS_CP0_COUNT] += ADM_FREQ / (2 * 1000);
    if (cp0->reg[MIPS_CP0_COMPARE] != 0 && cp0->reg[MIPS_CP0_COUNT] >= cp0->reg[MIPS_CP0_COMPARE]) {
        mips64_set_irq(cp0, MIPS_TIMER_INTERRUPT);
        mips64_update_irq_flag(cp0);
    }
}

/**
 * mips64_cp0_reset
 * @brief   
 * @param   cp0
 * @return  int fastcall
 */
fastcall std_int_t mips64_cp0_reset(cvm_arch_mips_cp0_t *cp0)
{
    /*set configure register */
    cp0->config_usable = 0x3; /*only configure sel 0 and 1 is valid */
    cp0->config_reg[0] = ADM5120_CONFIG0;
    cp0->config_reg[1] = ADM5120_CONFIG1;


    /*set PC and PRID */
    cp0->reg[MIPS_CP0_PRID] = ADM5120_PRID;
    cp0->tlb_entries = ADM5120_DEFAULT_TLB_ENTRYNO;

    cp0->reg[MIPS_CP0_STATUS] = MIPS_CP0_STATUS_BEV;
    cp0->reg[MIPS_CP0_CAUSE] = 0;

    cp0->addr_bus_mask = ADM5120_ADDR_BUS_MASK;

    /* Clear the complete TLB */
    memset(&(cp0->tlb), 0, sizeof(cp0->tlb));

    return 0;
}

/* Execute ERET instruction */
fastcall std_void_t mips64_cp0_eret(cvm_arch_mips_cp0_t *cp0)
{
    if (cp0->reg[MIPS_CP0_STATUS] & MIPS_CP0_STATUS_ERL) {
        cp0->reg[MIPS_CP0_STATUS] &= ~MIPS_CP0_STATUS_ERL;
        mod_vm_arch_mips_cpu_set_pc(pg_vm_arch_mips_cpu, cp0->reg[MIPS_CP0_ERR_EPC]);
    } else {
        cp0->reg[MIPS_CP0_STATUS] &= ~MIPS_CP0_STATUS_EXL;
        mod_vm_arch_mips_cpu_set_pc(pg_vm_arch_mips_cpu, cp0->reg[MIPS_CP0_EPC]);
    }

    /* We have to clear the LLbit */
    mod_vm_arch_mips_cpu_set_llbit(pg_vm_arch_mips_cpu, 0);
}


/* MTS access with special access mask */
std_void_t mips_access_special(cvm_arch_mips_cp0_t *cp0, std_u32_t vaddr, std_u32_t mask, IN std_uint_t op_code, std_uint_t op_type, IN std_uint_t op_size, std_u32_t *data)
{
    std_u32_t vpn;
    std_u8_t exc_code = 0;

    switch (mask) {
        case MTS_ACC_U:
            if (op_type == MTS_READ) {
                *data = 0;
            }
            break;

        case MTS_ACC_T:
        case MTS_ACC_M:
        case MTS_ACC_AE:
            cp0->reg[MIPS_CP0_BADVADDR] = vaddr;

            //clear vpn of entry hi
            cp0->reg[MIPS_CP0_TLB_HI] &= ~(mips64_cp0_get_vpn2_mask(cp0));

            //set VPN of entryhi
            vpn = vaddr & mips64_cp0_get_vpn2_mask(cp0);
            cp0->reg[MIPS_CP0_TLB_HI] |= vpn;

            //set context register
            cp0->reg[MIPS_CP0_CONTEXT] &= ~MIPS_CP0_CONTEXT_BADVPN2_MASK;
            vaddr = (vaddr >> 13) << 4;
            vaddr = vaddr & MIPS_CP0_CONTEXT_BADVPN2_MASK;

            cp0->reg[MIPS_CP0_CONTEXT] |= vaddr;

            if (mask == MTS_ACC_M) {
                exc_code = MIPS_CP0_CAUSE_TLB_MOD;
            } else if (mask == MTS_ACC_T) {
                if (op_type == MTS_READ) {
                    exc_code = MIPS_CP0_CAUSE_TLB_LOAD;
                } else {
                    exc_code = MIPS_CP0_CAUSE_TLB_SAVE;
                }
            } else {
                if (op_type == MTS_READ) {
                    exc_code = MIPS_CP0_CAUSE_ADDR_LOAD;
                } else {
                    exc_code = MIPS_CP0_CAUSE_ADDR_SAVE;
                }
            }

            mips64_trigger_exception(cp0, exc_code, mod_vm_arch_mips_cpu_is_in_bdslot(pg_vm_arch_mips_cpu));
            longjmp(pg_vm_arch_mips_cpu->exec_loop_env, 1);

        default:
            break;
    }
}

/* Get a cp0 register (fast version) */
std_u32_t mips64_cp0_get_reg_fast(IN const cvm_arch_mips_cp0_t *cp0, std_uint_t cp0_reg, std_uint_t sel)
{
    switch (cp0_reg) {
        case MIPS_CP0_RANDOM:
            return mips64_cp0_get_random_reg(cp0);
        case MIPS_CP0_CONFIG:
            STD_ASSERT_MSG((1 << sel) & (cp0->config_usable), "Unimplemented configure register sel 0x%x\n", sel);
            return cp0->config_reg[sel];
        default:
            return (cp0->reg[cp0_reg]);
    }
}


/* Get a cp0 register */
std_u32_t mips64_cp0_get_reg(const cvm_arch_mips_cp0_t *cp0, std_uint_t cp0_reg)
{
    return (mips64_cp0_get_reg_fast(cp0, cp0_reg, 0));
}


/* Set a cp0 register */
std_void_t mips64_cp0_set_reg(cvm_arch_mips_cp0_t *cp0, std_uint_t cp0_reg, std_u32_t val)
{
    switch (cp0_reg) {
        case MIPS_CP0_STATUS:
        case MIPS_CP0_CAUSE:
            cp0->reg[cp0_reg] = val;
            mips64_update_irq_flag(cp0);
            break;

        case MIPS_CP0_TLB_HI:
            cp0->reg[cp0_reg] = val;
            break;

        case MIPS_CP0_TLB_LO_0:
        case MIPS_CP0_TLB_LO_1:
            cp0->reg[cp0_reg] = val;
            break;
        case MIPS_CP0_PAGEMASK:
            cp0->reg[cp0_reg] = val;
            break;
        case MIPS_CP0_INDEX:
            cp0->reg[cp0_reg] = val;
            break;
        case MIPS_CP0_RANDOM:
        case MIPS_CP0_PRID:
            /* read only registers */
            break;

        case MIPS_CP0_COMPARE:
            //Write to compare will clear timer interrupt
            mips64_clear_irq(cp0, MIPS_TIMER_INTERRUPT);
            mips64_update_irq_flag(cp0);
            cp0->reg[cp0_reg] = val;
            break;
        case MIPS_CP0_EPC:
            cp0->reg[MIPS_CP0_EPC] = val;
            break;
        case MIPS_CP0_WIRED:
            /* read only registers */
            break;

        default:
            cp0->reg[cp0_reg] = val;
    }
}

/*
IRQ
*/

/* Update the IRQ flag (inline) */
static forced_inline fastcall std_int_t mips64_update_irq_flag_fast(cvm_arch_mips_cp0_t *cp0)
{
    std_u32_t imask;
    std_u32_t sreg_mask;
    std_u32_t cause;

    cp0->irq_pending = FALSE;

    cause = cp0->reg[MIPS_CP0_CAUSE] & ~MIPS_CP0_CAUSE_IMASK;
    cp0->reg[MIPS_CP0_CAUSE] = cause | cp0->irq_cause;

    sreg_mask = MIPS_CP0_STATUS_IE | MIPS_CP0_STATUS_EXL | MIPS_CP0_STATUS_ERL;

    if ((cp0->reg[MIPS_CP0_STATUS] & sreg_mask) == MIPS_CP0_STATUS_IE) {
        imask = cp0->reg[MIPS_CP0_STATUS] & MIPS_CP0_STATUS_IMASK;
        if (unlikely(cp0->reg[MIPS_CP0_CAUSE] & imask)) {
            cp0->irq_pending = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

/* Update the IRQ flag */
fastcall std_int_t mips64_update_irq_flag(cvm_arch_mips_cp0_t *cp0)
{
    return mips64_update_irq_flag_fast(cp0);
}

/* Set an IRQ */
std_void_t mips64_set_irq(cvm_arch_mips_cp0_t *cp0, std_u8_t irq)
{
    std_u32_t m;
    m = (1 << (irq + MIPS_CP0_CAUSE_ISHIFT)) & MIPS_CP0_CAUSE_IMASK;

    cp0->irq_cause |= m;
}

/* Clear an IRQ */
std_void_t mips64_clear_irq(cvm_arch_mips_cp0_t *cp0, std_u8_t irq)
{
    std_u32_t m;

    m = (1 << (irq + MIPS_CP0_CAUSE_ISHIFT)) & MIPS_CP0_CAUSE_IMASK;
    cp0->irq_cause &= ~m;

    if (!cp0->irq_cause) {
        cp0->irq_pending = 0;
    }
}

/**
 * mips64_is_pending
 * @brief   
 * @param   cp0
 * @return  std_uint_t
 */
std_uint_t mips64_is_pending(const cvm_arch_mips_cp0_t *cp0)
{
    return cp0->irq_pending;
}

/* Generate an exception */
std_void_t mips64_trigger_exception(cvm_arch_mips_cp0_t *cp0, std_uint_t exc_code, int bd_slot)
{
    std_u64_t new_pc;
    std_u32_t cause;

    /* keep IM, set exception code and bd slot */
    cause = cp0->reg[MIPS_CP0_CAUSE];

    /* we don't set EPC if EXL is set */
    if (!(cp0->reg[MIPS_CP0_STATUS] & MIPS_CP0_STATUS_EXL)) {
        cp0->reg[MIPS_CP0_EPC] = mod_vm_arch_mips_cpu_set_pc(pg_vm_arch_mips_cpu, 0);
        /*Cause BD is not update. MIPS VOLUME  V3 P65 */
        cause &= ~MIPS_CP0_CAUSE_BD_SLOT;//clear bd
        if (bd_slot) {
            cause |= MIPS_CP0_CAUSE_BD_SLOT;
        } else {
            cause &= ~MIPS_CP0_CAUSE_BD_SLOT;
        }
    }

    cause &= ~MIPS_CP0_CAUSE_EXC_MASK;//clear exec-code
    cause |= (exc_code << 2);
    cp0->reg[MIPS_CP0_CAUSE] = cause;

    /* Set EXL bit in status register */
    /*TODO: RESET SOFT RESET AND NMI EXCEPTION */
    cp0->reg[MIPS_CP0_STATUS] |= MIPS_CP0_STATUS_EXL;

    /* clear ERL bit in status register */
    cp0->reg[MIPS_CP0_STATUS] &= ~MIPS_CP0_STATUS_ERL;

    if (cp0->reg[MIPS_CP0_STATUS] & MIPS_CP0_STATUS_BEV) {
        if ((exc_code == MIPS_CP0_CAUSE_TLB_LOAD) || (exc_code == MIPS_CP0_CAUSE_TLB_SAVE)) {
            if (cp0->reg[MIPS_CP0_STATUS] & MIPS_CP0_STATUS_EXL)
                new_pc = 0xffffffffbfc00380ULL;
            else
                new_pc = 0xffffffffbfc00200ULL;
        } else if (exc_code == MIPS_CP0_CAUSE_INTERRUPT) {
            if (cp0->reg[MIPS_CP0_CAUSE] & MIPS_CP0_CAUSE_IV)
                new_pc = 0xffffffffbfc00400ULL;
            else
                new_pc = 0xffffffffbfc00380ULL;

        } else
            new_pc = 0xffffffffbfc00380ULL;

    } else {
        if ((exc_code == MIPS_CP0_CAUSE_TLB_LOAD) || (exc_code == MIPS_CP0_CAUSE_TLB_SAVE)) {
            if (cp0->reg[MIPS_CP0_STATUS] & MIPS_CP0_STATUS_EXL)
                new_pc = 0xffffffff80000180ULL;
            else
                new_pc = 0xffffffff80000000ULL;
        } else if (exc_code == MIPS_CP0_CAUSE_INTERRUPT) {
            if (cp0->reg[MIPS_CP0_CAUSE] & MIPS_CP0_CAUSE_IV)
                new_pc = 0xffffffff80000200ULL;
            else
                new_pc = 0xffffffff80000180ULL;
        } else
            new_pc = 0xffffffff80000180ULL;
    }

    mod_vm_arch_mips_cpu_set_pc(pg_vm_arch_mips_cpu, new_pc);

    /* Clear the pending IRQ flag */
    cp0->irq_pending = 0;
}

