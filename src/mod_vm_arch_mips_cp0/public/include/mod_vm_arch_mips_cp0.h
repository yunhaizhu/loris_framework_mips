/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch_mips_cp0.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_ARCH_MIPS_CP0_H
#define MOD_VM_ARCH_MIPS_CP0_H

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

#define MIPS_CP0_TLB_HI      10 /* TLB Entry Hi        */
#define MIPS_TLB_ASID_MASK     0x000000ff       /* "asid" in EntryHi */

#define OP_TLBP 1
#define OP_TLBR 2
#define OP_TLBWI 3
#define OP_TLBWR 4

/* Memory access flags */
#define MTS_ACC_OK 0x00000000 /* Access OK */
#define MTS_ACC_AE 0x00000002 /* Address Error */
#define MTS_ACC_T 0x00000004  /* TLB Exception */
#define MTS_ACC_U 0x00000006  /* Unexistent */
#define MTS_ACC_M 0x00000008  /* TLB MODE */

#define OP_IRQ_UPDATE 1
#define OP_IRQ_CLEAR 2
#define OP_IRQ_SET 3
#define OP_IRQ_IS_PENDING 4
#define MIPS64_CP0_REG_NR   32
/****************************************************
 *                                                  *
 *                     MOD Define                   *
 *                                                  *
 ***************************************************/


typedef struct mod_vm_arch_mips_cp0_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_arch_mips_cp0_ops_st *p_ops;

    std_u32_t irq_pending;
    std_u32_t irq_cause;

    std_u32_t reg[MIPS64_CP0_REG_NR];
} mod_vm_arch_mips_cp0_t;

struct mod_vm_arch_mips_cp0_ops_st {
    std_int_t (*init)(IN mod_vm_arch_mips_cp0_t *m, IN const std_char_t *arg, IN std_int_t arg_len);
    std_int_t (*cleanup)(IN mod_vm_arch_mips_cp0_t *m);

    /***func_define***/
    std_void_t (*reset)(IN mod_vm_arch_mips_cp0_t *m);
    std_void_t (*trigger_exception)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t type, IN std_uint_t cause);
    std_void_t (*tlb_op)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t op_type, IN std_uint_t insn);
    std_int_t (*tlb_lookup)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t vaddr, IN std_void_t *res);
    std_void_t (*mtc_op)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t cp0_reg, IN std_uint_t val);
    std_uint_t (*mfc_op)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t cp0_reg, IN std_uint_t sel);
    std_void_t (*access_special)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t vaddr, IN std_uint_t mask,
                                 IN std_uint_t op_code, IN std_uint_t op_type, IN std_uint_t op_size,
                                 IN std_uint_t *data);
    std_int_t (*irq_op)(IN mod_vm_arch_mips_cp0_t *m, IN std_uint_t type, IN std_uint_t irq);
    std_int_t (*timer)(IN mod_vm_arch_mips_cp0_t *m);
    std_int_t (*eret)(IN mod_vm_arch_mips_cp0_t *m);
    std_int_t (*soft_fpu)(IN mod_vm_arch_mips_cp0_t *m);
};

/***************************************************
 *                                                 *
 *                     Global Variable             *
 *                                                 *
 **************************************************/

#define MOD_VM_ARCH_MIPS_CP0_IID MOD_GUID(0x6c3e2338, 0x725f, 0x5e49, 0x69, 0x36, 0x94, 0x05, 0x6e, 0xe7, 0x51, 0xf1)

/***************************************************
 *                                                 *
 *                     Interface Function          *
 *                                                 *
 **************************************************/

#define mod_vm_arch_mips_cp0_init(m, arg, arg_len) ((m)->p_ops->init((mod_vm_arch_mips_cp0_t *) (m), arg, arg_len))
#define mod_vm_arch_mips_cp0_cleanup(m) ((m)->p_ops->cleanup((mod_vm_arch_mips_cp0_t *) (m)))

/***interface_define***/
#define mod_vm_arch_mips_cp0_reset(m) ((m)->p_ops->reset((mod_vm_arch_mips_cp0_t *) (m)))
#define mod_vm_arch_mips_cp0_trigger_exception(m, type, cause) ((m)->p_ops->trigger_exception((mod_vm_arch_mips_cp0_t *) (m), type, cause))
#define mod_vm_arch_mips_cp0_tlb_op(m, op_type, insn) ((m)->p_ops->tlb_op((mod_vm_arch_mips_cp0_t *) (m), op_type, insn))
#define mod_vm_arch_mips_cp0_tlb_lookup(m, vaddr, res) ((m)->p_ops->tlb_lookup((mod_vm_arch_mips_cp0_t *) (m), vaddr, res))
#define mod_vm_arch_mips_cp0_mtc_op(m, cp0_reg, val) ((m)->p_ops->mtc_op((mod_vm_arch_mips_cp0_t *) (m), cp0_reg, val))
#define mod_vm_arch_mips_cp0_mfc_op(m, cp0_reg, sel) ((m)->p_ops->mfc_op((mod_vm_arch_mips_cp0_t *) (m), cp0_reg, sel))
#define mod_vm_arch_mips_cp0_access_special(m, vaddr, mask, op_code, op_type, op_size, data) ((m)->p_ops->access_special((mod_vm_arch_mips_cp0_t *) (m), vaddr, mask, op_code, op_type, op_size, data))
#define mod_vm_arch_mips_cp0_irq_op(m, type, irq) ((m)->p_ops->irq_op((mod_vm_arch_mips_cp0_t *) (m), type, irq))
#define mod_vm_arch_mips_cp0_timer(m) ((m)->p_ops->timer((mod_vm_arch_mips_cp0_t *) (m)))
#define mod_vm_arch_mips_cp0_eret(m) ((m)->p_ops->eret((mod_vm_arch_mips_cp0_t *) (m)))
#define mod_vm_arch_mips_cp0_soft_fpu(m) ((m)->p_ops->soft_fpu((mod_vm_arch_mips_cp0_t *) (m)))

#endif
