/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    cvm_arch_mips_cp0.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#ifndef CVM_ARCH_MIPS_CP0_H
#define CVM_ARCH_MIPS_CP0_H

#include "std_common.h"

/* MTS operation */
#define MTS_READ 0
#define MTS_WRITE 1

/* Memory access flags */
#define MTS_ACC_OK 0x00000000 /* Access OK */
#define MTS_ACC_AE 0x00000002 /* Address Error */
#define MTS_ACC_T 0x00000004  /* TLB Exception */
#define MTS_ACC_U 0x00000006  /* Unexistent */
#define MTS_ACC_M 0x00000008  /* TLB MODE */

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

/* Minimum page size: 4 Kb */
#define MIPS_MIN_PAGE_SHIFT 12
#define MIPS_MIN_PAGE_SIZE (1 << MIPS_MIN_PAGE_SHIFT)
#define MIPS_MIN_PAGE_IMASK (MIPS_MIN_PAGE_SIZE - 1)
#define MIPS_MIN_PAGE_MASK 0xfffffffffffff000ULL


/* Number of registers in CP0 */
#define MIPS64_CP0_REG_NR 32

/*8 configure register in cp0. sel:0-7*/
#define MIPS64_CP0_CONFIG_REG_NR 8

#define MIPS64_TLB_MAX_ENTRIES 64

/*
 * MIPS INTERRUPT
 */

#define MIPS_TIMER_INTERRUPT 7

/*
 * Coprocessor 0 (System Coprocessor) Register definitions
 */
#define MIPS_CP0_INDEX 0     /* TLB Index           */
#define MIPS_CP0_RANDOM 1    /* TLB Random          */
#define MIPS_CP0_TLB_LO_0 2  /* TLB Entry Lo0       */
#define MIPS_CP0_TLB_LO_1 3  /* TLB Entry Lo1       */
#define MIPS_CP0_CONTEXT 4   /* Kernel PTE pointer  */
#define MIPS_CP0_PAGEMASK 5  /* TLB Page Mask       */
#define MIPS_CP0_WIRED 6     /* TLB Wired           */
#define MIPS_CP0_INFO 7      /* Info (RM7000)       */
#define MIPS_CP0_BADVADDR 8  /* Bad Virtual Address */
#define MIPS_CP0_COUNT 9     /* Count               */
#define MIPS_CP0_TLB_HI 10   /* TLB Entry Hi        */
#define MIPS_CP0_COMPARE 11  /* Timer Compare       */
#define MIPS_CP0_STATUS 12   /* Status              */
#define MIPS_CP0_CAUSE 13    /* Cause               */
#define MIPS_CP0_EPC 14      /* Exception PC        */
#define MIPS_CP0_PRID 15     /* Proc Rev ID         */
#define MIPS_CP0_CONFIG 16   /* Configuration       */
#define MIPS_CP0_LLADDR 17   /* Load/Link address   */
#define MIPS_CP0_WATCHLO 18  /* Low Watch address   */
#define MIPS_CP0_WATCHHI 19  /* High Watch address  */
#define MIPS_CP0_XCONTEXT 20 /* Extended context    */
#define MIPS_CP0_ECC 26      /* ECC and parity      */
#define MIPS_CP0_CACHERR 27  /* Cache Err/Status    */
#define MIPS_CP0_TAGLO 28    /* Cache Tag Lo        */
#define MIPS_CP0_TAGHI 29    /* Cache Tag Hi        */
#define MIPS_CP0_ERR_EPC 30  /* Error exception PC  */


/*
 * CP0 Status Register
 */
#define MIPS_CP0_STATUS_CU0 0x10000000
#define MIPS_CP0_STATUS_CU1 0x20000000
#define MIPS_CP0_STATUS_BEV 0x00400000
#define MIPS_CP0_STATUS_TS 0x00200000
#define MIPS_CP0_STATUS_SR 0x00100000
#define MIPS_CP0_STATUS_CH 0x00040000
#define MIPS_CP0_STATUS_CE 0x00020000
#define MIPS_CP0_STATUS_DE 0x00010000
#define MIPS_CP0_STATUS_RP 0x08000000
#define MIPS_CP0_STATUS_FR 0x04000000
#define MIPS_CP0_STATUS_RE 0x02000000
#define MIPS_CP0_STATUS_KX 0x00000080
#define MIPS_CP0_STATUS_SX 0x00000040
#define MIPS_CP0_STATUS_UX 0x00000020
#define MIPS_CP0_STATUS_KSU 0x00000018
#define MIPS_CP0_STATUS_ERL 0x00000004
#define MIPS_CP0_STATUS_EXL 0x00000002
#define MIPS_CP0_STATUS_IE 0x00000001
#define MIPS_CP0_STATUS_IMASK7 0x00008000
#define MIPS_CP0_STATUS_IMASK6 0x00004000
#define MIPS_CP0_STATUS_IMASK5 0x00002000
#define MIPS_CP0_STATUS_IMASK4 0x00001000
#define MIPS_CP0_STATUS_IMASK3 0x00000800
#define MIPS_CP0_STATUS_IMASK2 0x00000400
#define MIPS_CP0_STATUS_IMASK1 0x00000200
#define MIPS_CP0_STATUS_IMASK0 0x00000100

#define MIPS_CP0_STATUS_DS_MASK 0x00770000
#define MIPS_CP0_STATUS_CU_MASK 0xF0000000
#define MIPS_CP0_STATUS_IMASK 0x0000FF00

/* Addressing mode: Kernel, Supervisor and User */
#define MIPS_CP0_STATUS_KSU_SHIFT 0x03
#define MIPS_CP0_STATUS_KSU_MASK 0x03

#define MIPS_CP0_STATUS_KM 0x00
#define MIPS_CP0_STATUS_SM 0x01
#define MIPS_CP0_STATUS_UM 0x10

/*
 * CP0 Cause register
 */

#define MIPS_CP0_CAUSE_BD_SLOT 0x80000000

#define MIPS_CP0_CAUSE_MASK 0x0000007C
#define MIPS_CP0_CAUSE_CEMASK 0x30000000
#define MIPS_CP0_CAUSE_IMASK 0x0000FF00
#define MIPS_CP0_CAUSE_IV 0x00800000
#define MIPS_CP0_CAUSE_SHIFT 2
#define MIPS_CP0_CAUSE_CESHIFT 28
#define MIPS_CP0_CAUSE_ISHIFT 8
#define MIPS_CP0_CAUSE_EXC_MASK 0x0000007C


#define MIPS_CP0_CAUSE_INTERRUPT 0
#define MIPS_CP0_CAUSE_TLB_MOD 1
#define MIPS_CP0_CAUSE_TLB_LOAD 2
#define MIPS_CP0_CAUSE_TLB_SAVE 3
#define MIPS_CP0_CAUSE_ADDR_LOAD 4 /* ADEL */
#define MIPS_CP0_CAUSE_ADDR_SAVE 5 /* ADES */
#define MIPS_CP0_CAUSE_BUS_INSTR 6
#define MIPS_CP0_CAUSE_BUS_DATA 7
#define MIPS_CP0_CAUSE_SYSCALL 8
#define MIPS_CP0_CAUSE_BP 9
#define MIPS_CP0_CAUSE_ILLOP 10
#define MIPS_CP0_CAUSE_CP_UNUSABLE 11
#define MIPS_CP0_CAUSE_OVFLW 12
#define MIPS_CP0_CAUSE_TRAP 13
#define MIPS_CP0_CAUSE_VC_INSTR 14 /* Virtual Coherency */
#define MIPS_CP0_CAUSE_FPE 15
#define MIPS_CP0_CAUSE_WATCH 23
#define MIPS_CP0_CAUSE_VC_DATA 31 /* Virtual Coherency */

#define MIPS_CP0_CAUSE_IBIT7 0x00008000
#define MIPS_CP0_CAUSE_IBIT6 0x00004000
#define MIPS_CP0_CAUSE_IBIT5 0x00002000
#define MIPS_CP0_CAUSE_IBIT4 0x00001000
#define MIPS_CP0_CAUSE_IBIT3 0x00000800
#define MIPS_CP0_CAUSE_IBIT2 0x00000400
#define MIPS_CP0_CAUSE_IBIT1 0x00000200
#define MIPS_CP0_CAUSE_IBIT0 0x00000100

/* cp0 context */
#define MIPS_CP0_CONTEXT_PTEBASE_MASK 0xff800000
#define MIPS_CP0_CONTEXT_BADVPN2_MASK 0x0007ffff0

/* TLB masks and shifts */
#define MIPS_TLB_PAGE_MASK 0x01ffe000
#define MIPS_TLB_PAGE_SHIFT 13
#define MIPS_TLB_VPN2_MASK 0xffffe000
#define MIPS_TLB_VPN2_MASK_32 0xffffe000
#define MIPS_TLB_VPN2_MASK_64 0xc00000ffffffe000ULL
#define MIPS_TLB_PFN_MASK 0x3fffffc0
#define MIPS_TLB_ASID_MASK 0x000000ff /* "asid" in EntryHi */
#define MIPS_TLB_G_MASK 0x00001000    /* "Global" in EntryHi */
#define MIPS_TLB_V_MASK 0x2           /* "Valid" in EntryLo */
#define MIPS_TLB_D_MASK 0x4           /* "Dirty" in EntryLo */
#define MIPS_TLB_C_MASK 0x38          /* Page Coherency Attribute */
#define MIPS_TLB_C_SHIFT 3
#define MIPS_TLB_V_SHIT 1
#define MIPS_TLB_D_SHIT 2

#define MIPS_CP0_LO_G_MASK 0x00000001    /* "Global" in Lo0/1 reg */
#define MIPS_CP0_HI_SAFE_MASK 0xffffe0ff /* Safety mask for Hi reg */
#define MIPS_CP0_LO_SAFE_MASK 0x7fffffff /* Safety mask for Lo reg */

#define ADM5120_CONFIG0 0x80000082
#define ADM5120_CONFIG1 0x3E613080 /*CACHE (128SET*32 BYTES*2 WAY)= 8K */

#define ADM5120_ADDR_BUS_MASK 0xffffffff /*32bit phy address */
#define ADM5120_ROM_PC 0xbfc00000UL
#define ADM5120_PRID 0x0001800b
#define ADM5120_DEFAULT_TLB_ENTRYNO 16

#define ADM_FREQ 175000000 /*175MHZ */

#define MIPS_MEMOP_LOOKUP 0

/* TLB entry definition */
typedef struct {
    std_u32_t mask;
    std_u32_t hi;
    std_u32_t lo0;
    std_u32_t lo1;
} tlb_entry_t;


/* System Coprocessor (CP0) definition */
typedef struct {
    std_u32_t irq_pending;
    std_u32_t irq_cause;

    std_u32_t reg[MIPS64_CP0_REG_NR];

    /*because configure has sel 0-7, seperate it to reg */
    std_u32_t config_reg[MIPS64_CP0_CONFIG_REG_NR];
    std_u8_t config_usable; /*if configure register sel N is useable, set the bit in config_usable to 1 */

    tlb_entry_t tlb[MIPS64_TLB_MAX_ENTRIES];

    /* Number of TLB entries */
    std_uint_t tlb_entries;

    std_u32_t addr_mode;
    std_u32_t addr_bus_mask;


} cvm_arch_mips_cp0_t;


/**
 * mips64_cp0_reset
 * @brief   
 * @param   cp0
 * @return  std_int_t fastcall
 */
std_int_t fastcall mips64_cp0_reset(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_trigger_exception
 * @brief   
 * @param   cp0
 * @param   exc_code
 * @param   bd_slot
 * @return  std_void_t
 */
std_void_t mips64_trigger_exception(cvm_arch_mips_cp0_t *cp0, std_uint_t exc_code, std_int_t bd_slot);

/* Get value of random register */
static inline std_uint_t mips64_cp0_get_random_reg(IN const cvm_arch_mips_cp0_t *cp0)
{
    std_int_t random_value;
    random_value = (std_int_t) ((double) (cp0->tlb_entries) * rand() / (RAND_MAX + 1.0));

    return random_value;
}

std_u32_t mips64_cp0_get_vpn2_mask(const cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_update_irq_flag
 * @brief   
 * @param   cp0
 * @return  std_int_t fastcall
 */
std_int_t fastcall mips64_update_irq_flag(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_set_irq
 * @brief   
 * @param   cp0
 * @param   irq
 * @return  std_void_t
 */
std_void_t mips64_set_irq(cvm_arch_mips_cp0_t *cp0, std_u8_t irq);
/**
 * mips64_clear_irq
 * @brief   
 * @param   cp0
 * @param   irq
 * @return  std_void_t
 */
std_void_t mips64_clear_irq(cvm_arch_mips_cp0_t *cp0, std_u8_t irq);

/**
 * mips64_cp0_set_reg
 * @brief   
 * @param   cp0
 * @param   cp0_reg
 * @param   val
 * @return  std_void_t
 */
std_void_t mips64_cp0_set_reg(cvm_arch_mips_cp0_t *cp0, std_uint_t cp0_reg, std_u32_t val);

/**
 * mips64_cp0_get_reg_fast
 * @brief   
 * @param   cp0
 * @param   cp0_reg
 * @param   sel
 * @return  std_u32_t
 */
std_u32_t mips64_cp0_get_reg_fast(const cvm_arch_mips_cp0_t *cp0, std_uint_t cp0_reg, std_uint_t sel);

/**
 * mips_access_special
 * @brief   
 * @param   cp0
 * @param   vaddr
 * @param   mask
 * @param   op_code
 * @param   op_type
 * @param   op_size
 * @param   data
 * @return  std_void_t
 */
std_void_t mips_access_special(cvm_arch_mips_cp0_t *cp0, std_u32_t vaddr, std_u32_t mask, std_uint_t op_code, std_uint_t op_type, std_uint_t op_size, std_u32_t *data);

/**
 * mips64_is_pending
 * @brief   
 * @param   cp0
 * @return  std_uint_t
 */
std_uint_t mips64_is_pending(const cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_timer
 * @brief   
 * @param   cp0
 * @return  std_void_t fastcall
 */
std_void_t fastcall mips64_cp0_timer(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_eret
 * @brief   
 * @param   cp0
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips64_cp0_eret(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_soft_fpu
 * @brief   
 * @param   cp0
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips64_cp0_soft_fpu(cvm_arch_mips_cp0_t *cp0);


#endif
