/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    cvm_arch_mips_cpu.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#include "cvm_arch_mips_cpu.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_memory.h"
#include "cpu_mts.h"

extern mod_vm_memory_t *p_global_vm_memory;
extern mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;

/* MIPS general purpose registers names */
char *mips64_gpr_reg_names[MIPS64_GPR_NR] = {
        "zr",
        "at",
        "v0",
        "v1",
        "a0",
        "a1",
        "a2",
        "a3",
        "t0",
        "t1",
        "t2",
        "t3",
        "t4",
        "t5",
        "t6",
        "t7",
        "s0",
        "s1",
        "s2",
        "s3",
        "s4",
        "s5",
        "s6",
        "s7",
        "t8",
        "t9",
        "k0",
        "k1",
        "gp",
        "sp",
        "fp",
        "ra",
};

static inline std_int_t mips64_exec_bdslot(IN cvm_arch_mips_cpu_t *cpu);

static std_int_t forced_inline mips64_exec_memop2(struct cvm_arch_mips_cpu *cpu,
                                                  std_int_t memop,
                                                  std_u32_t base,
                                                  std_int_t offset,
                                                  std_uint_t dst_reg,
                                                  std_int_t keep_ll_bit)
{
    std_u64_t vaddr = cpu->gpr[base] + sign_extend(offset, 16);
    mips_memop_fn fn;

    if (!keep_ll_bit) {
        cpu->ll_bit = 0;
    }
    fn = cpu->mem_op_fn[memop];
    return fn(cpu, (std_u32_t)vaddr, dst_reg);
}


/* ADD */
static fastcall std_int_t mips64_exec_ADD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_u64_t res;

    res = (std_u32_t) cpu->gpr[rs] + (std_u32_t) cpu->gpr[rt];
    cpu->gpr[rd] = sign_extend(res, 32);

    return 0;
}

/* ADDI */
static fastcall std_int_t mips64_exec_ADDI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);
    std_u32_t res;
    std_u32_t val = (std_u32_t)sign_extend(imm, 16);

    res = (std_u32_t) cpu->gpr[rs] + val;
    cpu->gpr[rt] = sign_extend(res, 32);
    return 0;
}

/* ADDIU */
static fastcall std_int_t mips64_exec_ADDIU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);
    std_u32_t res;
    std_u32_t val = (std_u32_t)sign_extend(imm, 16);

    res = (std_u32_t) cpu->gpr[rs] + val;
    cpu->gpr[rt] = sign_extend(res, 32);

    return 0;
}

/* ADDU */
static fastcall std_int_t mips64_exec_ADDU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_u32_t res;

    res = (std_u32_t) cpu->gpr[rs] + (std_u32_t) cpu->gpr[rt];

    cpu->gpr[rd] = sign_extend(res, 32);

    return 0;
}

/* AND */
static fastcall std_int_t mips64_exec_AND(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);

    cpu->gpr[rd] = cpu->gpr[rs] & cpu->gpr[rt];

    return 0;
}

/* ANDI */
static fastcall std_int_t mips64_exec_ANDI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);

    cpu->gpr[rt] = cpu->gpr[rs] & imm;

    return 0;
}

/* B (Branch, virtual instruction) */
STD_CALL static fastcall std_int_t mips64_exec_B(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* set the new pc in cpu structure */
        cpu->pc = new_pc;
    }

    return 1;
}

/* BAL (Branch And Link, virtual instruction) */
STD_CALL static fastcall std_int_t mips64_exec_BAL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    cpu->gpr[MIPS_GPR_RA] = cpu->pc + 8;

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* set the new pc in cpu structure */
        cpu->pc = new_pc;
    }
    return 1;
}

/* BEQ (Branch On Equal) */
static fastcall std_int_t mips64_exec_BEQ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] == gpr[rt] */
    res = (cpu->gpr[rs] == cpu->gpr[rt]);

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    /**/
    if (likely(!ins_res)) {
        if (res) {
            cpu->pc = new_pc;
        } else {
            cpu->pc += 8;
        }
    }

    return 1;
}

/* BEQL (Branch On Equal Likely) */
static fastcall std_int_t mips64_exec_BEQL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] == gpr[rt] */
    res = (cpu->gpr[rs] == cpu->gpr[rt]);

    /* take the branch if the test result is true */
    if (res) {
        ins_res = mips64_exec_bdslot(cpu);

        if (likely(!ins_res)) {
            cpu->pc = new_pc;
        }
    } else {
        cpu->pc += 8;
    }

    return 1;
}


/* BEQZ (Branch On Equal Zero) - Virtual Instruction */
STD_CALL static fastcall std_int_t mips64_exec_BEQZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] == 0 */
    res = (cpu->gpr[rs] == 0);

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    /**/
    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res) {
            cpu->pc = new_pc;
        } else {
            cpu->pc += 8;
        }
    }

    return 1;
}


/* BNEZ (Branch On Not Equal Zero) - Virtual Instruction */
STD_CALL static fastcall std_int_t mips64_exec_BNEZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] != 0 */
    res = (cpu->gpr[rs] != 0);

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res) {
            cpu->pc = new_pc;
        } else {
            cpu->pc += 8;
        }
    }

    return 1;
}


/* BGEZ (Branch On Greater or Equal Than Zero) */
static fastcall std_int_t mips64_exec_BGEZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] >= 0 */
    res = ((std_32_t) cpu->gpr[rs] >= 0);

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res) {
            cpu->pc = new_pc;
        } else {
            cpu->pc += 8;
        }
    }

    return 1;
}


/* BGEZAL (Branch On Greater or Equal Than Zero And Link) */
static fastcall std_int_t mips64_exec_BGEZAL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    cpu->gpr[MIPS_GPR_RA] = cpu->pc + 8;

    /* take the branch if gpr[rs] >= 0 */
    res = ((std_32_t) cpu->gpr[rs] >= 0);

    /* exec the instruction in the delay slot */
    ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res) {
            cpu->pc = new_pc;
        } else {
            cpu->pc += 8;
        }
    }

    return 1;
}

/* BGEZALL (Branch On Greater or Equal Than Zero And Link Likely) */
static fastcall std_int_t mips64_exec_BGEZALL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;
    std_32_t ins_res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    cpu->gpr[MIPS_GPR_RA] = cpu->pc + 8;

    /* take the branch if gpr[rs] >= 0 */
    res = ((std_32_t) cpu->gpr[rs] >= 0);

    /* take the branch if the test result is true */
    if (res) {
        /*CHANGED by original!!*/
        ins_res = mips64_exec_bdslot(cpu);
        if (likely(!ins_res)) {
            cpu->pc = new_pc;
        }
    } else {
        cpu->pc += 8;
    }

    return 1;
}


/* BGEZL (Branch On Greater or Equal Than Zero Likely) */
static fastcall std_int_t mips64_exec_BGEZL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] >= 0 */
    res = ((std_64_t) cpu->gpr[rs] >= 0);

    /* take the branch if the test result is true */
    if (res) {
        /*NEED changed*/
        mips64_exec_bdslot(cpu);
        cpu->pc = new_pc;
    } else {
        cpu->pc += 8;
    }

    return 1;
}

/*************NEED CHECK BEGIN**********************************************/

/* BGTZ (Branch On Greater Than Zero) */
static fastcall std_int_t mips64_exec_BGTZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] > 0 */
    res = ((std_64_t) cpu->gpr[rs] > 0);

    /* exec the instruction in the delay slot */
    std_32_t ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res)
            cpu->pc = new_pc;
        else
            cpu->pc += 8;
    }

    return 1;
}

/* BGTZL (Branch On Greater Than Zero Likely) */
static fastcall std_int_t mips64_exec_BGTZL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] > 0 */
    res = ((std_64_t) cpu->gpr[rs] > 0);

    /* take the branch if the test result is true */
    if (res) {
        mips64_exec_bdslot(cpu);
        cpu->pc = new_pc;
    } else
        cpu->pc += 8;

    return 1;
}

/* BLEZ (Branch On Less or Equal Than Zero) */
static fastcall std_int_t mips64_exec_BLEZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] <= 0 */
    res = ((std_64_t) cpu->gpr[rs] <= 0);

    /* exec the instruction in the delay slot */
    std_32_t ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res)
            cpu->pc = new_pc;
        else
            cpu->pc += 8;
    }

    return 1;
}

/* BLEZL (Branch On Less or Equal Than Zero Likely) */
static fastcall std_int_t mips64_exec_BLEZL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] <= 0 */
    res = ((std_64_t) cpu->gpr[rs] <= 0);

    /* take the branch if the test result is true */
    if (res) {
        mips64_exec_bdslot(cpu);
        cpu->pc = new_pc;
    } else
        cpu->pc += 8;

    return 1;
}

/* BLTZ (Branch On Less Than Zero) */
static fastcall std_int_t mips64_exec_BLTZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] < 0 */
    res = ((std_64_t) cpu->gpr[rs] < 0);

    /* exec the instruction in the delay slot */
    std_32_t ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res))

    {
        /* take the branch if the test result is true */
        if (res)
            cpu->pc = new_pc;
        else
            cpu->pc += 8;
    }

    return 1;
}

/* BLTZAL (Branch On Less Than Zero And Link) */
static fastcall std_int_t mips64_exec_BLTZAL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    cpu->gpr[MIPS_GPR_RA] = cpu->pc + 8;

    /* take the branch if gpr[rs] < 0 */
    res = ((std_64_t) cpu->gpr[rs] < 0);

    /* exec the instruction in the delay slot */
    std_32_t ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res)
            cpu->pc = new_pc;
        else
            cpu->pc += 8;
    }

    return 1;
}

/* BLTZALL (Branch On Less Than Zero And Link Likely) */
static fastcall std_int_t mips64_exec_BLTZALL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* set the return address (instruction after the delay slot) */
    cpu->gpr[MIPS_GPR_RA] = cpu->pc + 8;

    /* take the branch if gpr[rs] < 0 */
    res = ((std_64_t) cpu->gpr[rs] < 0);

    /* take the branch if the test result is true */
    if (res) {
        mips64_exec_bdslot(cpu);
        cpu->pc = new_pc;
    } else
        cpu->pc += 8;

    return 1;
}

/* BLTZL (Branch On Less Than Zero Likely) */
static fastcall std_int_t mips64_exec_BLTZL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] < 0 */
    res = ((std_64_t) cpu->gpr[rs] < 0);

    /* take the branch if the test result is true */
    if (res) {
        mips64_exec_bdslot(cpu);
        cpu->pc = new_pc;
    } else
        cpu->pc += 8;

    return 1;
}

/* BNE (Branch On Not Equal) */
static fastcall std_int_t mips64_exec_BNE(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);


    /* take the branch if gpr[rs] != gpr[rt] */
    res = (cpu->gpr[rs] != cpu->gpr[rt]);

    /* exec the instruction in the delay slot */
    std_32_t ins_res = mips64_exec_bdslot(cpu);

    if (likely(!ins_res)) {
        /* take the branch if the test result is true */
        if (res)
            cpu->pc = new_pc;
        else
            cpu->pc += 8;
    }

    return 1;
}

/* BNEL (Branch On Not Equal Likely) */
static fastcall std_int_t mips64_exec_BNEL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);
    std_u64_t new_pc;
    std_32_t res;

    /* compute the new pc */
    new_pc = (cpu->pc + 4) + sign_extend(offset << 2, 18);

    /* take the branch if gpr[rs] != gpr[rt] */
    res = (cpu->gpr[rs] != cpu->gpr[rt]);


    /* take the branch if the test result is true */
    if (res) {


        mips64_exec_bdslot(cpu);
        cpu->pc = new_pc;
    } else
        cpu->pc += 8;

    return 1;
}


std_void_t fastcall mips64_exec_break(cvm_arch_mips_cpu_t *cpu, u_int code);
/* BREAK */
fastcall std_int_t mips64_exec_BREAK(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_uint_t code = bits(insn, 6, 25);

    mips64_exec_break(cpu, code);
    return 1;
}

/* CACHE */
static fastcall std_int_t mips64_exec_CACHE(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t op = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_CACHE, base, offset, op, FALSE));
}

/* DIV */
static fastcall std_int_t mips64_exec_DIV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);

    cpu->lo = (std_32_t) cpu->gpr[rs] / (std_32_t) cpu->gpr[rt];
    cpu->hi = (std_32_t) cpu->gpr[rs] % (std_32_t) cpu->gpr[rt];

    cpu->lo = sign_extend(cpu->lo, 32);
    cpu->hi = sign_extend(cpu->hi, 32);
    return 0;
}

/* DIVU */
static fastcall std_int_t mips64_exec_DIVU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);

    if (cpu->gpr[rt] == 0) {
        return 0;
    }

    cpu->lo = (std_u32_t) cpu->gpr[rs] / (std_u32_t) cpu->gpr[rt];
    cpu->hi = (std_u32_t) cpu->gpr[rs] % (std_u32_t) cpu->gpr[rt];

    cpu->lo = sign_extend(cpu->lo, 32);
    cpu->hi = sign_extend(cpu->hi, 32);
    return 0;
}


/* ERET */
fastcall std_int_t mips64_exec_ERET(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_eret(p_global_vm_arch_mips_cp0);
    return 1;
}

/* J */
static fastcall std_int_t mips64_exec_J(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_uint_t instr_index = bits(insn, 0, 25);
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = cpu->pc & ~((1 << 28) - 1);
    new_pc |= instr_index << 2;

    /* exec the instruction in the delay slot */
    std_32_t ins_res = mips64_exec_bdslot(cpu);
    if (likely(!ins_res))
        cpu->pc = new_pc;
    return 1;
}

/* JAL */
static fastcall std_int_t mips64_exec_JAL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_uint_t instr_index = bits(insn, 0, 25);
    std_u64_t new_pc;

    /* compute the new pc */
    new_pc = cpu->pc & ~((1 << 28) - 1);
    new_pc |= instr_index << 2;

    /* set the return address (instruction after the delay slot) */
    cpu->gpr[MIPS_GPR_RA] = cpu->pc + 8;

    std_32_t ins_res = mips64_exec_bdslot(cpu);
    if (likely(!ins_res))
        cpu->pc = new_pc;

    return 1;
}

/* JALR */
static fastcall std_int_t mips64_exec_JALR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rd = bits(insn, 11, 15);
    std_u64_t new_pc;

    /* set the return pc (instruction after the delay slot) in GPR[rd] */
    cpu->gpr[rd] = cpu->pc + 8;

    /* get the new pc */
    new_pc = cpu->gpr[rs];

    std_32_t ins_res = mips64_exec_bdslot(cpu);
    if (likely(!ins_res))
        cpu->pc = new_pc;
    return 1;
}

/* JR */
static fastcall std_int_t mips64_exec_JR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_u64_t new_pc;

    /* get the new pc */
    new_pc = cpu->gpr[rs];

    std_32_t ins_res = mips64_exec_bdslot(cpu);
    if (likely(!ins_res))
        cpu->pc = new_pc;
    return 1;
}

#if 1


/* LB (Load Byte) */
static fastcall std_int_t mips64_exec_LB(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LB, base, offset, rt, TRUE));
}

/* LBU (Load Byte Unsigned) */
static fastcall std_int_t mips64_exec_LBU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LBU, base, offset, rt, TRUE));
}

/* LD (Load Double-Word) */
static fastcall std_int_t mips64_exec_LD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LD, base, offset, rt, TRUE));
}

/* LDL (Load Double-Word Left) */
static fastcall std_int_t mips64_exec_LDL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LDL, base, offset, rt, TRUE));
}

/* LDR (Load Double-Word Right) */
static fastcall std_int_t mips64_exec_LDR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LDR, base, offset, rt, TRUE));
}

/* LH (Load Half-Word) */
static fastcall std_int_t mips64_exec_LH(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LH, base, offset, rt, TRUE));
}

/* LHU (Load Half-Word Unsigned) */
static fastcall std_int_t mips64_exec_LHU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LHU, base, offset, rt, TRUE));
}

/* LI (virtual) */
STD_CALL static fastcall std_int_t mips64_exec_LI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);

    cpu->gpr[rt] = sign_extend(imm, 16);
    return 0;
}

/* LL (Load Linked) */
static fastcall std_int_t mips64_exec_LL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LL, base, offset, rt, TRUE));
}

/* LUI */
static fastcall std_int_t mips64_exec_LUI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);

    cpu->gpr[rt] = sign_extend(imm, 16) << 16;
    return 0;
}

/* LW (Load Word) */
static fastcall std_int_t mips64_exec_LW(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LW, base, offset, rt, TRUE));
}

/* LWL (Load Word Left) */
static fastcall std_int_t mips64_exec_LWL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LWL, base, offset, rt, TRUE));
}

/* LWR (Load Word Right) */
static fastcall std_int_t mips64_exec_LWR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LWR, base, offset, rt, TRUE));
}

/* LWU (Load Word Unsigned) */
static fastcall std_int_t mips64_exec_LWU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_LWU, base, offset, rt, TRUE));
}

#endif

/**
 * mips64_exec_MAD
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall std_int_t
 */
static fastcall std_int_t mips64_exec_MAD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_64_t val;
    std_64_t temp;

    val = cpu->gpr[rs];
    val *= cpu->gpr[rt];

    temp = cpu->hi;
    temp = temp << 32;
    temp += cpu->lo;
    val += temp;

    cpu->lo = sign_extend(val, 32);
    cpu->hi = sign_extend(val >> 32, 32);
    return 0;
}

STD_CALL static fastcall std_int_t mips64_exec_MADU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_64_t val;
    std_64_t temp;

    val = cpu->gpr[rs];
    val *= cpu->gpr[rt];

    temp = cpu->hi;
    temp = temp << 32;
    temp += cpu->lo;
    val += temp;

    cpu->lo = sign_extend(val, 32);
    cpu->hi = sign_extend(val >> 32, 32);
    return 0;
}


/* MFC0 */
fastcall std_int_t mips64_exec_MFC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t sel = bits(insn, 0, 2);
    //mfc rt,rd

    cpu->gpr[rt] = sign_extend(mod_vm_arch_mips_cp0_mfc_op(p_global_vm_arch_mips_cp0, rd, sel), 32);

    return 0;
}


/* MFHI */
static fastcall std_int_t mips64_exec_MFHI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rd = bits(insn, 11, 15);

    if (rd) cpu->gpr[rd] = cpu->hi;
    return 0;
}

/* MFLO */
static fastcall std_int_t mips64_exec_MFLO(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rd = bits(insn, 11, 15);

    if (rd) cpu->gpr[rd] = cpu->lo;
    return 0;
}

/* MOVE (virtual instruction, real: ADDU) */
STD_CALL static fastcall std_int_t mips64_exec_MOVE(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rd = bits(insn, 11, 15);

    cpu->gpr[rd] = sign_extend(cpu->gpr[rs], 32);
    return 0;
}

/**
 * mips64_exec_MOVEN
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall std_int_t
 */
static fastcall std_int_t mips64_exec_MOVEN(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t rt = bits(insn, 16, 20);

    if ((cpu->gpr[rt]) != 0)
        cpu->gpr[rd] = sign_extend(cpu->gpr[rs], 32);
    return 0;
}

/**
 * mips64_exec_MOVEZ
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall std_int_t
 */
static fastcall std_int_t mips64_exec_MOVEZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t rt = bits(insn, 16, 20);


    if ((cpu->gpr[rt]) == 0)
        cpu->gpr[rd] = sign_extend(cpu->gpr[rs], 32);
    return 0;
}


/* MTC0 */
fastcall std_int_t mips64_exec_MTC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);

    mod_vm_arch_mips_cp0_mtc_op(p_global_vm_arch_mips_cp0, rd, cpu->gpr[rt] & 0xffffffff);

    return 0;
}


/* MTHI */
static fastcall std_int_t mips64_exec_MTHI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);

    cpu->hi = cpu->gpr[rs];
    return 0;
}

/* MTLO */
static fastcall std_int_t mips64_exec_MTLO(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);

    cpu->lo = cpu->gpr[rs];
    return 0;
}

/* MUL */
static fastcall std_int_t mips64_exec_MUL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t val;

    /* note: after this instruction, HI/LO regs are undefined */
    val = (std_32_t) cpu->gpr[rs] * (std_32_t) cpu->gpr[rt];
    cpu->gpr[rd] = sign_extend(val, 32);
    return 0;
}

/* MULT */
static fastcall std_int_t mips64_exec_MULT(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_64_t val;

    val = (std_64_t) (std_32_t) cpu->gpr[rs];
    val *= (std_64_t) (std_32_t) cpu->gpr[rt];

    cpu->lo = sign_extend(val, 32);
    cpu->hi = sign_extend(val >> 32, 32);
    return 0;
}

/* MULTU */
static fastcall std_int_t mips64_exec_MULTU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_u64_t val;//must be u64 bit. not std_u32_t !!!

    val = (std_u64_t) (std_u32_t) cpu->gpr[rs];
    val *= (std_u64_t) (std_u32_t) cpu->gpr[rt];
    cpu->lo = sign_extend(val, 32);
    cpu->hi = sign_extend(val >> 32, 32);
    return 0;
}

/* NOP */
STD_CALL static fastcall std_int_t mips64_exec_NOP(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    return 0;
}

/* NOR */
static fastcall std_int_t mips64_exec_NOR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);

    cpu->gpr[rd] = ~(cpu->gpr[rs] | cpu->gpr[rt]);
    return 0;
}

/* OR */
static fastcall std_int_t mips64_exec_OR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);

    cpu->gpr[rd] = cpu->gpr[rs] | cpu->gpr[rt];
    return 0;
}

/* ORI */
static fastcall std_int_t mips64_exec_ORI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);

    cpu->gpr[rt] = cpu->gpr[rs] | imm;
    return 0;
}

/* PREF */
static fastcall std_int_t mips64_exec_PREF(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    return 0;
}

/* PREFI */
STD_CALL static fastcall std_int_t mips64_exec_PREFI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    return 0;
}

#if 1
/* SB (Store Byte) */
static fastcall std_int_t mips64_exec_SB(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SB, base, offset, rt, FALSE));
}

/* SC (Store Conditional) */
static fastcall std_int_t mips64_exec_SC(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SC, base, offset, rt, TRUE));
}

/* SD (Store Double-Word) */
static fastcall std_int_t mips64_exec_SD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SD, base, offset, rt, FALSE));
}

/* SDL (Store Double-Word Left) */
static fastcall std_int_t mips64_exec_SDL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SDL, base, offset, rt, FALSE));
}

/* SDR (Store Double-Word Right) */
static fastcall std_int_t mips64_exec_SDR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SDR, base, offset, rt, FALSE));
}

/* SH (Store Half-Word) */
static fastcall std_int_t mips64_exec_SH(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SH, base, offset, rt, FALSE));
}
#endif

/* SLL */
static fastcall std_int_t mips64_exec_SLL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t sa = bits(insn, 6, 10);
    std_u32_t res;

    res = (std_u32_t) cpu->gpr[rt] << sa;
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SLLV */
static fastcall std_int_t mips64_exec_SLLV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_u32_t res;

    res = (std_u32_t) cpu->gpr[rt] << (cpu->gpr[rs] & 0x1f);
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SLT */
static fastcall std_int_t mips64_exec_SLT(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);

    if ((std_64_t) cpu->gpr[rs] < (std_64_t) cpu->gpr[rt])
        cpu->gpr[rd] = 1;
    else
        cpu->gpr[rd] = 0;

    return 0;
}

/* SLTI */
static fastcall std_int_t mips64_exec_SLTI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);
    std_64_t val = sign_extend(imm, 16);

    if ((std_64_t) cpu->gpr[rs] < val) {
        cpu->gpr[rt] = 1;
    } else {
        cpu->gpr[rt] = 0;
    }
    return 0;
}

/* SLTIU */
static fastcall std_int_t mips64_exec_SLTIU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);

    if (cpu->gpr[rs] < val) {
        cpu->gpr[rt] = 1;
    } else {
        cpu->gpr[rt] = 0;
    }
    return 0;
}

/* SLTU */
static fastcall std_int_t mips64_exec_SLTU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);


    if (cpu->gpr[rs] < cpu->gpr[rt])
        cpu->gpr[rd] = 1;
    else
        cpu->gpr[rd] = 0;

    return 0;
}

/* SRA */
static fastcall std_int_t mips64_exec_SRA(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t sa = bits(insn, 6, 10);
    std_32_t res;

    res = (std_32_t) cpu->gpr[rt] >> sa;
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SRAV */
static fastcall std_int_t mips64_exec_SRAV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t res;

    res = (std_32_t) cpu->gpr[rt] >> (cpu->gpr[rs] & 0x1f);
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SRL */
static fastcall std_int_t mips64_exec_SRL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_32_t sa = bits(insn, 6, 10);
    std_u32_t res;

    res = (std_u32_t) cpu->gpr[rt] >> sa;
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SRLV */
static fastcall std_int_t mips64_exec_SRLV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_u32_t res;

    res = (std_u32_t) cpu->gpr[rt] >> (cpu->gpr[rs] & 0x1f);
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SUB */
static fastcall std_int_t mips64_exec_SUB(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_u32_t res;


    res = (std_u32_t) cpu->gpr[rs] - (std_u32_t) cpu->gpr[rt];
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

/* SUBU */
static fastcall std_int_t mips64_exec_SUBU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);
    std_u32_t res;

    res = (std_u32_t) cpu->gpr[rs] - (std_u32_t) cpu->gpr[rt];
    cpu->gpr[rd] = sign_extend(res, 32);
    return 0;
}

#if 1
/* SW (Store Word) */
static fastcall std_int_t mips64_exec_SW(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SW, base, offset, rt, FALSE));
}

/* SWL (Store Word Left) */
static fastcall std_int_t mips64_exec_SWL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SWL, base, offset, rt, FALSE));
}

/* SWR (Store Word Right) */
static fastcall std_int_t mips64_exec_SWR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t base = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t offset = bits(insn, 0, 15);

    return (mips64_exec_memop2(cpu, MIPS_MEMOP_SWR, base, offset, rt, FALSE));
}

#endif

/* SYNC */
static fastcall std_int_t mips64_exec_SYNC(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    return 0;
}


/* Trigger a Trap Exception */
std_void_t fastcall mips64_trigger_trap_exception(IN cvm_arch_mips_cpu_t *cpu)
{

    printf("MIPS64: TRAP exception, CPU=%p\n", cpu);

    mod_vm_arch_mips_cp0_trigger_exception(p_global_vm_arch_mips_cp0, MIPS_CP0_CAUSE_TRAP, 0);
}


/* Execute SYSCALL instruction */
std_void_t fastcall mips64_exec_syscall(IN cvm_arch_mips_cpu_t *cpu)
{
#if 0
   printf("MIPS: SYSCALL at PC=0x%"  "x (RA=0x%"  "x)\n"
          "   a0=0x%"  "x, a1=0x%"  "x, a2=0x%"  "x, a3=0x%"  "x\n",
          cpu->pc, cpu->gpr[MIPS_GPR_RA],
          cpu->gpr[MIPS_GPR_A0], cpu->gpr[MIPS_GPR_A1], cpu->gpr[MIPS_GPR_A2], cpu->gpr[MIPS_GPR_A3]);
#endif

    if (cpu->is_in_bdslot == 0)
        mod_vm_arch_mips_cp0_trigger_exception(p_global_vm_arch_mips_cp0, MIPS_CP0_CAUSE_SYSCALL, 0);
    else
        mod_vm_arch_mips_cp0_trigger_exception(p_global_vm_arch_mips_cp0, MIPS_CP0_CAUSE_SYSCALL, 1);
}


/* SYSCALL */
static fastcall std_int_t mips64_exec_SYSCALL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mips64_exec_syscall(cpu);
    return 1;
}


/* TEQ (Trap if Equal) */
static fastcall std_int_t mips64_exec_TEQ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);

    if (unlikely(cpu->gpr[rs] == cpu->gpr[rt])) {
        mips64_trigger_trap_exception(cpu);
        return 1;
    }

    return 0;
}

/* TEQI (Trap if Equal Immediate) */
static fastcall std_int_t mips64_exec_TEQI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t imm = bits(insn, 0, 15);
    std_u64_t val = sign_extend(imm, 16);

    if (unlikely(cpu->gpr[rs] == val)) {
        mips64_trigger_trap_exception(cpu);
        return 1;
    }

    return 0;
}

/* TLBP */
fastcall std_int_t mips64_exec_TLBP(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_tlb_op(p_global_vm_arch_mips_cp0, OP_TLBP, insn);
    return 0;
}

/* TLBR */
fastcall std_int_t mips64_exec_TLBR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_tlb_op(p_global_vm_arch_mips_cp0, OP_TLBR, insn);
    return 0;
}

/* TLBWI */
fastcall std_int_t mips64_exec_TLBWI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_tlb_op(p_global_vm_arch_mips_cp0, OP_TLBWI, insn);
    return 0;
}

/* TLBWR */
fastcall std_int_t mips64_exec_TLBWR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_tlb_op(p_global_vm_arch_mips_cp0, OP_TLBWR, insn);
    return 0;
}


/* Virtual idle loop */

/* Get current time in number of usec since epoch */
static inline std_u64_t m_gettime_usec(void)
{
    struct timeval tvp;

    gettimeofday(&tvp, NULL);
    return (((std_u64_t) tvp.tv_sec * 1000000) + (std_u64_t) tvp.tv_usec);
}

/**
 * cpu_idle_loop
 * @brief   
 * @param   cpu
 */
std_void_t cpu_idle_loop(IN cvm_arch_mips_cpu_t *cpu)
{
    struct timespec t_spc;
    std_u64_t expire;

    expire = m_gettime_usec() + cpu->idle_sleep_time;
    t_spc.tv_sec = expire / 1000000;
    t_spc.tv_nsec = (expire % 1000000) * 1000;

#if 0
    pthread_mutex_lock(&cpu->idle_mutex);

    pthread_cond_timedwait(&cpu->idle_cond, &cpu->idle_mutex, &t_spc);
    pthread_mutex_unlock(&cpu->idle_mutex);
#else
    std_int_t s;
    while ((s = sem_timedwait(&cpu->idle_sem, &t_spc)) == -1 && errno == EINTR){
        continue;
    }
#endif

}

/* Break idle wait state */
std_void_t cpu_idle_break_wait(IN cvm_arch_mips_cpu_t *cpu)
{
#if 0
    pthread_cond_signal(&cpu->idle_cond);
#else
    sem_post(&cpu->idle_sem);
#endif

    cpu->idle_count = 0;
}


/* wait */
fastcall std_int_t mips64_exec_WAIT(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    if (++cpu->idle_count == cpu->idle_max) {
        cpu_idle_loop(cpu);
        cpu->idle_count = 0;
    }

    return 0;
}

/* XOR */
static fastcall std_int_t mips64_exec_XOR(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t rd = bits(insn, 11, 15);

    cpu->gpr[rd] = cpu->gpr[rs] ^ cpu->gpr[rt];
    return 0;
}

/* XORI */
static fastcall std_int_t mips64_exec_XORI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_32_t rs = bits(insn, 21, 25);
    std_32_t rt = bits(insn, 16, 20);
    std_32_t imm = bits(insn, 0, 15);

    cpu->gpr[rt] = cpu->gpr[rs] ^ imm;
    return 0;
}

/* Unknown opcode */
STD_CALL static fastcall std_int_t mips64_exec_unknown(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    printf("MIPS64: unknown opcode 0x%8.8x at pc = 0x%"
           "x\n",
           insn, cpu->pc);
    exit(-1);

}


/*ALL FPU INSTRUCTION*/
STD_CALL static fastcall std_int_t mips64_exec_SOFTFPU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_soft_fpu(p_global_vm_arch_mips_cp0);
    return 1;
}


/*************NEED CHECK LATER*********************************************/


#if 0

struct mips64_insn_exec_tag {
	std_char_t *name;
	fastcall std_int_t (*exec)(cvm_arch_mips_cpu_t *, std_u32_t);
	std_u32_t mask,value;
	std_32_t delay_slot;
	std_32_t instr_type;
	std_u64_t count;
};

/* MIPS instruction array */
static struct mips64_insn_exec_tag mips64_exec_tags[] = {
		{ "li"     , mips64_exec_LI      , 0xffe00000 , 0x24000000, 1, 16 },
		{ "move"   , mips64_exec_MOVE    , 0xfc1f07ff , 0x00000021, 1, 15 },
		{ "b"      , mips64_exec_B       , 0xffff0000 , 0x10000000, 0, 10 },
		{ "bal"    , mips64_exec_BAL     , 0xffff0000 , 0x04110000, 0, 10 },
		{ "beqz"   , mips64_exec_BEQZ    , 0xfc1f0000 , 0x10000000, 0, 9 },
		{ "bnez"   , mips64_exec_BNEZ    , 0xfc1f0000 , 0x14000000, 0, 9 },
		{ "add"    , mips64_exec_ADD     , 0xfc0007ff , 0x00000020, 1, 3 },
		{ "addi"   , mips64_exec_ADDI    , 0xfc000000 , 0x20000000, 1, 6 },
		{ "addiu"  , mips64_exec_ADDIU   , 0xfc000000 , 0x24000000, 1, 6 },
		{ "addu"   , mips64_exec_ADDU    , 0xfc0007ff , 0x00000021, 1, 3 },
		{ "and"    , mips64_exec_AND     , 0xfc0007ff , 0x00000024, 1, 3 },
		{ "andi"   , mips64_exec_ANDI    , 0xfc000000 , 0x30000000, 1, 5 },
		{ "beq"    , mips64_exec_BEQ     , 0xfc000000 , 0x10000000, 0, 8 },
		{ "beql"   , mips64_exec_BEQL    , 0xfc000000 , 0x50000000, 0, 8 },
		{ "bgez"   , mips64_exec_BGEZ    , 0xfc1f0000 , 0x04010000, 0, 9 },
		{ "bgezal" , mips64_exec_BGEZAL  , 0xfc1f0000 , 0x04110000, 0, 9 },
		{ "bgezall", mips64_exec_BGEZALL , 0xfc1f0000 , 0x04130000, 0, 9 },
		{ "bgezl"  , mips64_exec_BGEZL   , 0xfc1f0000 , 0x04030000, 0, 9 },
		{ "bgtz"   , mips64_exec_BGTZ    , 0xfc1f0000 , 0x1c000000, 0, 9 },
		{ "bgtzl"  , mips64_exec_BGTZL   , 0xfc1f0000 , 0x5c000000, 0, 9 },
		{ "blez"   , mips64_exec_BLEZ    , 0xfc1f0000 , 0x18000000, 0, 9 },
		{ "blezl"  , mips64_exec_BLEZL   , 0xfc1f0000 , 0x58000000, 0, 9 },
		{ "bltz"   , mips64_exec_BLTZ    , 0xfc1f0000 , 0x04000000, 0, 9 },
		{ "bltzal" , mips64_exec_BLTZAL  , 0xfc1f0000 , 0x04100000, 0, 9 },
		{ "bltzall", mips64_exec_BLTZALL , 0xfc1f0000 , 0x04120000, 0, 9 },
		{ "bltzl"  , mips64_exec_BLTZL   , 0xfc1f0000 , 0x04020000, 0, 9 },
		{ "bne"    , mips64_exec_BNE     , 0xfc000000 , 0x14000000, 0, 8 },
		{ "bnel"   , mips64_exec_BNEL    , 0xfc000000 , 0x54000000, 0, 8 },
		{ "break"  , mips64_exec_BREAK   , 0xfc00003f , 0x0000000d, 1, 0 },
		{ "cache"  , mips64_exec_CACHE   , 0xfc000000 , 0xbc000000, 1, 2 },
		{ "div"    , mips64_exec_DIV     , 0xfc00003f , 0x0000001a, 1, 17 },
		{ "divu"   , mips64_exec_DIVU    , 0xfc00003f , 0x0000001b, 1, 17 },
		{ "eret"   , mips64_exec_ERET    , 0xffffffff , 0x42000018, 0, 1 },
		{ "j"      , mips64_exec_J       , 0xfc000000 , 0x08000000, 0, 11 },
		{ "jal"    , mips64_exec_JAL     , 0xfc000000 , 0x0c000000, 0, 11 },
		{ "jalr"   , mips64_exec_JALR    , 0xfc1f003f , 0x00000009, 0, 15 },
		{ "jr"     , mips64_exec_JR      , 0xfc1ff83f , 0x00000008, 0, 13 },
		{ "lb"     , mips64_exec_LB      , 0xfc000000 , 0x80000000, 1, 2 },
		{ "lbu"    , mips64_exec_LBU     , 0xfc000000 , 0x90000000, 1, 2 },
		{ "ld"     , mips64_exec_LD      , 0xfc000000 , 0xdc000000, 1, 2 },
		{ "ldl"    , mips64_exec_LDL     , 0xfc000000 , 0x68000000, 1, 2 },
		{ "ldr"    , mips64_exec_LDR     , 0xfc000000 , 0x6c000000, 1, 2 },
		{ "lh"     , mips64_exec_LH      , 0xfc000000 , 0x84000000, 1, 2 },
		{ "lhu"    , mips64_exec_LHU     , 0xfc000000 , 0x94000000, 1, 2 },
		{ "ll"     , mips64_exec_LL      , 0xfc000000 , 0xc0000000, 1, 2 },
		{ "lui"    , mips64_exec_LUI     , 0xffe00000 , 0x3c000000, 1, 16 },
		{ "lw"     , mips64_exec_LW      , 0xfc000000 , 0x8c000000, 1, 2 },
		{ "lwl"    , mips64_exec_LWL     , 0xfc000000 , 0x88000000, 1, 2 },
		{ "lwr"    , mips64_exec_LWR     , 0xfc000000 , 0x98000000, 1, 2 },
		{ "lwu"    , mips64_exec_LWU     , 0xfc000000 , 0x9c000000, 1, 2 },
		{ "mad"    , mips64_exec_MAD     , 0xfc00ffff , 0x70000000, 1, 18 }, 
		{ "madu"   , mips64_exec_MADU    , 0xfc00ffff , 0x70000001, 1, 18 }, 
		{ "mfc0"   , mips64_exec_MFC0    , 0xffe007f8 , 0x40000000, 1, 18 },
		{ "mfhi"   , mips64_exec_MFHI    , 0xffff07ff , 0x00000010, 1, 14 },
		{ "mflo"   , mips64_exec_MFLO    , 0xffff07ff , 0x00000012, 1, 14 },
		{ "move"   , mips64_exec_MOVE    , 0xfc1f07ff , 0x00000021, 1, 15 },
		{ "moven"  , mips64_exec_MOVEN   , 0xfc0007ff , 0x0000000b, 1, 15 },
		{ "movez"  , mips64_exec_MOVEZ   , 0xfc0007ff , 0x0000000a, 1, 15 },
		{ "mtc0"   , mips64_exec_MTC0    , 0xffe007ff , 0x40800000, 1, 18 },
		{ "mthi"   , mips64_exec_MTHI    , 0xfc1fffff , 0x00000011, 1, 13 },
		{ "mtlo"   , mips64_exec_MTLO    , 0xfc1fffff , 0x00000013, 1, 13 },
		{ "mul"    , mips64_exec_MUL     , 0xfc0007ff , 0x70000002, 1, 4 },
		{ "mult"   , mips64_exec_MULT    , 0xfc00ffff , 0x00000018, 1, 17 },
		{ "multu"  , mips64_exec_MULTU   , 0xfc00ffff , 0x00000019, 1, 17 },
		{ "nop"    , mips64_exec_NOP     , 0xffffffff , 0x00000000, 1, 1 },
		{ "nor"    , mips64_exec_NOR     , 0xfc0007ff , 0x00000027, 1, 3 },
		{ "or"     , mips64_exec_OR      , 0xfc0007ff , 0x00000025, 1, 3 },
		{ "ori"    , mips64_exec_ORI     , 0xfc000000 , 0x34000000, 1, 5 },
		{ "pref"   , mips64_exec_PREF    , 0xfc000000 , 0xcc000000, 1, 0 },
		{ "prefi"  , mips64_exec_PREFI   , 0xfc0007ff , 0x4c00000f, 1, 0 },
		{ "sb"     , mips64_exec_SB      , 0xfc000000 , 0xa0000000, 1, 2 },
		{ "sc"     , mips64_exec_SC      , 0xfc000000 , 0xe0000000, 1, 2 },
		{ "sd"     , mips64_exec_SD      , 0xfc000000 , 0xfc000000, 1, 2 },
		{ "sdl"    , mips64_exec_SDL     , 0xfc000000 , 0xb0000000, 1, 2 },
		{ "sdr"    , mips64_exec_SDR     , 0xfc000000 , 0xb4000000, 1, 2 },
		{ "sh"     , mips64_exec_SH      , 0xfc000000 , 0xa4000000, 1, 2 },
		{ "sll"    , mips64_exec_SLL     , 0xffe0003f , 0x00000000, 1, 7 },
		{ "sllv"   , mips64_exec_SLLV    , 0xfc0007ff , 0x00000004, 1, 4 },
		{ "slt"    , mips64_exec_SLT     , 0xfc0007ff , 0x0000002a, 1, 3 },
		{ "slti"   , mips64_exec_SLTI    , 0xfc000000 , 0x28000000, 1, 5 },
		{ "sltiu"  , mips64_exec_SLTIU   , 0xfc000000 , 0x2c000000, 1, 5 },
		{ "sltu"   , mips64_exec_SLTU    , 0xfc0007ff , 0x0000002b, 1, 3 },
		{ "sra"    , mips64_exec_SRA     , 0xffe0003f , 0x00000003, 1, 7 },
		{ "srav"   , mips64_exec_SRAV    , 0xfc0007ff , 0x00000007, 1, 4 },
		{ "srl"    , mips64_exec_SRL     , 0xffe0003f , 0x00000002, 1, 7 },
		{ "srlv"   , mips64_exec_SRLV    , 0xfc0007ff , 0x00000006, 1, 4 },
		{ "sub"    , mips64_exec_SUB     , 0xfc0007ff , 0x00000022, 1, 3 },
		{ "subu"   , mips64_exec_SUBU    , 0xfc0007ff , 0x00000023, 1, 3 },
		{ "sw"     , mips64_exec_SW      , 0xfc000000 , 0xac000000, 1, 2 },
		{ "swl"    , mips64_exec_SWL     , 0xfc000000 , 0xa8000000, 1, 2 },
		{ "swr"    , mips64_exec_SWR     , 0xfc000000 , 0xb8000000, 1, 2 },
		{ "sync"   , mips64_exec_SYNC    , 0xfffff83f , 0x0000000f, 1, 1 },
		{ "syscall", mips64_exec_SYSCALL , 0xfc00003f , 0x0000000c, 1, 1 },
		{ "teq"    , mips64_exec_TEQ     , 0xfc00003f , 0x00000034, 1, 17 },
		{ "teqi"   , mips64_exec_TEQI    , 0xfc1f0000 , 0x040c0000, 1, 20 },
		{ "tlbp"   , mips64_exec_TLBP    , 0xffffffff , 0x42000008, 1, 1 },
		{ "tlbr"   , mips64_exec_TLBR    , 0xffffffff , 0x42000001, 1, 1 },
		{ "tlbwi"  , mips64_exec_TLBWI   , 0xffffffff , 0x42000002, 1, 1 },
		{ "tlbwr"  , mips64_exec_TLBWR   , 0xffffffff , 0x42000006, 1, 1 },
		{ "wait"   , mips64_exec_WAIT    , 0xfc00003f , 0x40000020, 1, 3 },
		{ "xor"    , mips64_exec_XOR     , 0xfc0007ff , 0x00000026, 1, 3 },
		{ "xori"   , mips64_exec_XORI    , 0xfc000000 , 0x38000000, 1, 5 },

		{ "fpu"   , mips64_exec_SOFTFPU  , 0xfc000000 , 0x44000000, 0, 1 },
		{ "ldc1"   , mips64_exec_SOFTFPU , 0xfc000000 , 0xd4000000, 1, 3 },
		{ "l.s"   , mips64_exec_SOFTFPU  , 0xfc000000 , 0xc4000000, 1, 3 },
		{ "swc1"   , mips64_exec_SOFTFPU , 0xfc000000 , 0xe4000000, 1, 3 },
		{ "fpu"   , mips64_exec_SOFTFPU  , 0xfc000000 , 0xf4000000, 1, 3 },

		{ "unknown", mips64_exec_unknown , 0x00000000 , 0x00000000, 1, 0 },
		{ NULL     , NULL                , 0x00000000 , 0x00000000, 1, 0 },
};
#endif


/* Fetch an instruction */


/**
 * mips64_exec_bdslot
 * @brief   
 * @param   cpu
 * @return  static inline std_int_t
 */
static inline std_int_t mips64_exec_bdslot(IN cvm_arch_mips_cpu_t *cpu)
{
    std_u32_t insn;
    std_int_t res = 0;

    cpu->is_in_bdslot = 1;

    /* Fetch the instruction in delay slot */
    res = mips64_exec_fetch(cpu, (std_u32_t)cpu->pc + 4, &insn);
    if (res == 1) {
        /*exception when fetching instruction*/
        cpu->is_in_bdslot = 0;
        return 1;
    }
    cpu->is_in_bdslot = 1;

    /* Execute the instruction */
    res = mips64_exec_single_instruction(cpu, insn);

    cpu->is_in_bdslot = 0;

    return res;
}


/*************NEW EXEC FUNC*********************************************/

struct mips64_op_desc {
    std_char_t *opname;
    fastcall std_int_t (*func)(cvm_arch_mips_cpu_t *, std_u32_t);
    std_u16_t num;
};

/**
 * mips64_exec_COP1
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  fastcall int
 */
fastcall std_int_t mips64_exec_COP1(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_soft_fpu(p_global_vm_arch_mips_cp0);
    return 1;
}

/**
 * mips64_exec_COP2
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_COP2(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_COP1X
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_COP1X(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DADDI
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DADDI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DADDIU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DADDIU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_LWC1
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_LWC1(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_LWC2
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_LWC2(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_LLD
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_LLD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_LDC1
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_LDC1(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    exit(0);
}

/**
 * mips64_exec_LDC2
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_LDC2(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    exit(0);
}

/**
 * mips64_exec_SWC1
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_SWC1(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_SWC2
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_SWC2(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}


/**
 * mips64_exec_SCD
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_SCD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_SDC1
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_SDC1(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    mod_vm_arch_mips_cp0_soft_fpu(p_global_vm_arch_mips_cp0);
    return 1;
}

/**
 * mips64_exec_SDC2
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_SDC2(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_MOVC
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_MOVC(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}


/**
 * mips64_exec_DSLLV
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSLLV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSRLV
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSRLV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSRAV
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSRAV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DMULT
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DMULT(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DMULTU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DMULTU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DDIV
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DDIV(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DDIVU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DDIVU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DADD
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DADD(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DADDU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DADDU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSUB
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSUB(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSUBU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSUBU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TGE
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TGE(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TGEU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TGEU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}


/**
 * mips64_exec_TLT
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TLT(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TLTU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TLTU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TNE
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TNE(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSLL
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSLL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSRL
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSRL(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSRA
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSRA(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSLL32
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSLL32(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}


/**
 * mips64_exec_DSRL32
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSRL32(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DSRA32
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DSRA32(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TGEI
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TGEI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TGEIU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TGEIU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TLTIU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TLTIU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_TNEI
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TNEI(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DMFC0
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DMFC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}


/**
 * mips64_exec_CFC0
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_CFC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_DMTC0
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_DMTC0(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_MADDU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_MADDU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_MSUB
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_MSUB(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_MSUBU
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_MSUBU(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}

/**
 * mips64_exec_CLZ
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_CLZ(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{

    exit(0);
}


/*************END NEW EXEC FUNC *********************************************/


static fastcall std_int_t mips64_exec_SPEC_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
static fastcall std_int_t mips64_exec_BCOND_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
static fastcall std_int_t mips64_exec_COP0_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
static fastcall std_int_t mips64_exec_MAD_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);
static fastcall std_int_t mips64_exec_TLB_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn);

static struct mips64_op_desc mips_exec_opcodes[] = {
        {"SPEC", mips64_exec_SPEC_, 0x00},
        {"BCOND", mips64_exec_BCOND_, 0x01},
        {"j", mips64_exec_J, 0x02},
        {"jal", mips64_exec_JAL, 0x03},
        {"beq", mips64_exec_BEQ, 0x04},
        {"bne", mips64_exec_BNE, 0x05},
        {"blez", mips64_exec_BLEZ, 0x06},
        {"bgtz", mips64_exec_BGTZ, 0x07},
        {"addi", mips64_exec_ADDI, 0x08},
        {"addiu", mips64_exec_ADDIU, 0x09},
        {"slti", mips64_exec_SLTI, 0x0A},
        {"sltiu", mips64_exec_SLTIU, 0x0B},
        {"andi", mips64_exec_ANDI, 0x0C},
        {"ori", mips64_exec_ORI, 0x0D},
        {"xori", mips64_exec_XORI, 0x0E},
        {"lui", mips64_exec_LUI, 0x0F},
        {"cop0", mips64_exec_COP0_, 0x10},
        {"cop1", mips64_exec_COP1, 0x11},
        {"cop2", mips64_exec_COP2, 0x12},
        {"cop1x", mips64_exec_COP1X, 0x13},
        {"beql", mips64_exec_BEQL, 0x14},
        {"bnel", mips64_exec_BNEL, 0x15},
        {"blezl", mips64_exec_BLEZL, 0x16},
        {"bgtzl", mips64_exec_BGTZL, 0x17},
        {"daddi", mips64_exec_DADDI, 0x18},
        {"daddiu", mips64_exec_DADDIU, 0x19},
        {"ldl", mips64_exec_LDL, 0x1A},
        {"ldr", mips64_exec_LDR, 0x1B},
        {"MAD", mips64_exec_MAD_, 0x1C},
        {"undef", NULL, 0x1D},
        {"undef", NULL, 0x1E},
        {"undef", NULL, 0x1F},
        {"lb", mips64_exec_LB, 0x20},
        {"lh", mips64_exec_LH, 0x21},
        {"lwl", mips64_exec_LWL, 0x22},
        {"lw", mips64_exec_LW, 0x23},
        {"lbu", mips64_exec_LBU, 0x24},
        {"lhu", mips64_exec_LHU, 0x25},
        {"lwr", mips64_exec_LWR, 0x26},
        {"lwu", mips64_exec_LWU, 0x27},
        {"sb", mips64_exec_SB, 0x28},
        {"sh", mips64_exec_SH, 0x29},
        {"swl", mips64_exec_SWL, 0x2A},
        {"sw", mips64_exec_SW, 0x2B},
        {"sdl", mips64_exec_SDL, 0x2C},
        {"sdr", mips64_exec_SDR, 0x2D},
        {"swr", mips64_exec_SWR, 0x2E},
        {"cache", mips64_exec_CACHE, 0x2F},
        {"ll", mips64_exec_LL, 0x30},
        {"lwc1", mips64_exec_LWC1, 0x31},
        {"lwc2", mips64_exec_LWC2, 0x32},
        {"pref", mips64_exec_PREF, 0x33},
        {"lld", mips64_exec_LLD, 0x34},
        {"ldc1", mips64_exec_LDC1, 0x35},
        {"ldc2", mips64_exec_LDC2, 0x36},
        {"ld", mips64_exec_LD, 0x37},
        {"sc", mips64_exec_SC, 0x38},
        {"swc1", mips64_exec_SWC1, 0x39},
        {"swc2", mips64_exec_SWC2, 0x3A},
        {"undef", NULL, 0x3B},
        {"scd", mips64_exec_SCD, 0x3C},
        {"sdc1", mips64_exec_SDC1, 0x3D},
        {"sdc2", mips64_exec_SDC2, 0x3E},
        {"sd", mips64_exec_SD, 0x3F},
};


/* Based on the func field of spec opcode */
static struct mips64_op_desc mips_spec_opcodes[] = {
        {"sll", mips64_exec_SLL, 0x00},
        {"movc", mips64_exec_MOVC, 0x01},
        {"srl", mips64_exec_SRL, 0x02},
        {"sra", mips64_exec_SRA, 0x03},
        {"sllv", mips64_exec_SLLV, 0x04},
        {"unknownSpec", NULL, 0x05},
        {"srlv", mips64_exec_SRLV, 0x06},
        {"srav", mips64_exec_SRAV, 0x07},
        {"jr", mips64_exec_JR, 0x08},
        {"jalr", mips64_exec_JALR, 0x09},
        {"movz", mips64_exec_MOVEZ, 0x0A},
        {"movn", mips64_exec_MOVEN, 0x0B},
        {"syscall", mips64_exec_SYSCALL, 0x0C},
        {"break", mips64_exec_BREAK, 0x0D},
        {"spim", NULL, 0x0E},
        {"sync", mips64_exec_SYNC, 0x0F},
        {"mfhi", mips64_exec_MFHI, 0x10},
        {"mthi", mips64_exec_MTHI, 0x11},
        {"mflo", mips64_exec_MFLO, 0x12},
        {"mtlo", mips64_exec_MTLO, 0x13},
        {"dsllv", mips64_exec_DSLLV, 0x14},
        {"unknownSpec", NULL, 0x15},
        {"dsrlv", mips64_exec_DSRLV, 0x16},
        {"dsrav", mips64_exec_DSRAV, 0x17},
        {"mult", mips64_exec_MULT, 0x18},
        {"multu", mips64_exec_MULTU, 0x19},
        {"div", mips64_exec_DIV, 0x1A},
        {"divu", mips64_exec_DIVU, 0x1B},
        {"dmult", mips64_exec_DMULT, 0x1C},
        {"dmultu", mips64_exec_DMULTU, 0x1D},
        {"ddiv", mips64_exec_DDIV, 0x1E},
        {"ddivu", mips64_exec_DDIVU, 0x1F},
        {"add", mips64_exec_ADD, 0x20},
        {"addu", mips64_exec_ADDU, 0x21},
        {"sub", mips64_exec_SUB, 0x22},
        {"subu", mips64_exec_SUBU, 0x23},
        {"and", mips64_exec_AND, 0x24},
        {"or", mips64_exec_OR, 0x25},
        {"xor", mips64_exec_XOR, 0x26},
        {"nor", mips64_exec_NOR, 0x27},
        {"unknownSpec", NULL, 0x28},
        {"unknownSpec", NULL, 0x29},
        {"slt", mips64_exec_SLT, 0x2A},
        {"sltu", mips64_exec_SLTU, 0x2B},
        {"dadd", mips64_exec_DADD, 0x2C},
        {"daddu", mips64_exec_DADDU, 0x2D},
        {"dsub", mips64_exec_DSUB, 0x2E},
        {"dsubu", mips64_exec_DSUBU, 0x2F},
        {"tge", mips64_exec_TGE, 0x30},
        {"tgeu", mips64_exec_TGEU, 0x31},
        {"tlt", mips64_exec_TLT, 0x32},
        {"tltu", mips64_exec_TLTU, 0x33},
        {"teq", mips64_exec_TEQ, 0x34},
        {"unknownSpec", NULL, 0x35},
        {"tne", mips64_exec_TNE, 0x36},
        {"unknownSpec", NULL, 0x37},
        {"dsll", mips64_exec_DSLL, 0x38},
        {"unknownSpec", NULL, 0x39},
        {"dsrl", mips64_exec_DSRL, 0x3A},
        {"dsra", mips64_exec_DSRA, 0x3B},
        {"dsll32", mips64_exec_DSLL32, 0x3C},
        {"unknownSpec", NULL, 0x3D},
        {"dsrl32", mips64_exec_DSRL32, 0x3E},
        {"dsra32", mips64_exec_DSRA32, 0x3F}};


/* Based on the rt field of bcond opcodes */
static struct mips64_op_desc mips_bcond_opcodes[] = {
        {"bltz", mips64_exec_BLTZ, 0x00},
        {"bgez", mips64_exec_BGEZ, 0x01},
        {"bltzl", mips64_exec_BLTZL, 0x02},
        {"bgezl", mips64_exec_BGEZL, 0x03},
        {"spimi", NULL, 0x04},
        {"unknownBcond", NULL, 0x05},
        {"unknownBcond", NULL, 0x06},
        {"unknownBcond", NULL, 0x07},
        {"tgei", mips64_exec_TGEI, 0x08},
        {"tgeiu", mips64_exec_TGEIU, 0x09},
        {"tlti", mips64_exec_TLTIU, 0x0A},
        {"tltiu", mips64_exec_TLTIU, 0x0B},
        {"teqi", mips64_exec_TEQI, 0x0C},
        {"unknownBcond", NULL, 0x0D},
        {"tnei", mips64_exec_TNEI, 0x0E},
        {"unknownBcond", NULL, 0x0F},
        {"bltzal", mips64_exec_BLTZAL, 0x10},
        {"bgezal", mips64_exec_BGEZAL, 0x11},
        {"bltzall", mips64_exec_BLTZALL, 0x12},
        {"bgezall", mips64_exec_BGEZALL, 0x13},
        {"unknownBcond", NULL, 0x14},
        {"unknownBcond", NULL, 0x15},
        {"unknownBcond", NULL, 0x16},
        {"unknownBcond", NULL, 0x17},
        {"unknownBcond", NULL, 0x18},
        {"unknownBcond", NULL, 0x19},
        {"unknownBcond", NULL, 0x1A},
        {"unknownBcond", NULL, 0x1B},
        {"unknownBcond", NULL, 0x1C},
        {"unknownBcond", NULL, 0x1D},
        {"unknownBcond", NULL, 0x1E},
        {"unknownBcond", NULL, 0x1F}};


static struct mips64_op_desc mips_cop0_opcodes[] = {
        {"mfc0", mips64_exec_MFC0, 0x0},
        {"dmfc0", mips64_exec_DMFC0, 0x1},
        {"cfc0", mips64_exec_CFC0, 0x2},
        {"unknowncop0", NULL, 0x3},
        {"mtc0", mips64_exec_MTC0, 0x4},
        {"dmtc0", mips64_exec_DMTC0, 0x5},
        {"unknowncop0", NULL, 0x6},
        {"unknowncop0", NULL, 0x7},
        {"unknowncop0", NULL, 0x8},
        {"unknowncop0", NULL, 0x9},
        {"unknowncop0", NULL, 0xa},
        {"unknowncop0", NULL, 0xb},
        {"unknowncop0", NULL, 0xc},
        {"unknowncop0", NULL, 0xd},
        {"unknowncop0", NULL, 0xe},
        {"unknowncop0", NULL, 0xf},
        {"tlb", mips64_exec_TLB_, 0x10},
        {"unknowncop0", NULL, 0x11},
        {"unknowncop0", NULL, 0x12},
        {"unknowncop0", NULL, 0x13},
        {"unknowncop0", NULL, 0x14},
        {"unknowncop0", NULL, 0x15},
        {"unknowncop0", NULL, 0x16},
        {"unknowncop0", NULL, 0x17},
        {"unknowncop0", NULL, 0x18},
        {"unknowncop0", NULL, 0x19},
        {"unknowncop0", NULL, 0x1a},
        {"unknowncop0", NULL, 0x1b},
        {"unknowncop0", NULL, 0x1c},
        {"unknowncop0", NULL, 0x1d},
        {"unknowncop0", NULL, 0x1e},
        {"unknowncop0", NULL, 0x1f},


};


static struct mips64_op_desc mips_mad_opcodes[] = {
        {"mad", mips64_exec_MAD, 0x0},
        {"maddu", mips64_exec_MADDU, 0x1},
        {"mul", mips64_exec_MUL, 0x2},
        {"NULL", NULL, 0x3},
        {"msub", mips64_exec_MSUB, 0x4},
        {"msubu", mips64_exec_MSUBU, 0x5},
        {"NULL", NULL, 0x6},
        {"NULL", NULL, 0x7},
        {"NULL", NULL, 0x8},
        {"NULL", NULL, 0x9},
        {"NULL", NULL, 0xa},
        {"NULL", NULL, 0xb},
        {"NULL", NULL, 0xc},
        {"NULL", NULL, 0xd},
        {"NULL", NULL, 0xe},
        {"NULL", NULL, 0xf},
        {"NULL", NULL, 0x10},
        {"NULL", NULL, 0x11},
        {"NULL", NULL, 0x12},
        {"NULL", NULL, 0x13},
        {"NULL", NULL, 0x14},
        {"NULL", NULL, 0x15},
        {"NULL", NULL, 0x16},
        {"NULL", NULL, 0x17},
        {"NULL", NULL, 0x18},
        {"NULL", NULL, 0x19},
        {"NULL", NULL, 0x1a},
        {"NULL", NULL, 0x1b},
        {"NULL", NULL, 0x1c},
        {"NULL", NULL, 0x1d},
        {"NULL", NULL, 0x1e},
        {"NULL", NULL, 0x1f},
        {"clz", mips64_exec_CLZ, 0x20},
        {"NULL", NULL, 0x21},
        {"NULL", NULL, 0x22},
        {"NULL", NULL, 0x23},
        {"NULL", NULL, 0x24},
        {"NULL", NULL, 0x25},
        {"NULL", NULL, 0x26},
        {"NULL", NULL, 0x27},
        {"NULL", NULL, 0x28},
        {"NULL", NULL, 0x29},
        {"NULL", NULL, 0x2a},
        {"NULL", NULL, 0x2b},
        {"NULL", NULL, 0x2c},
        {"NULL", NULL, 0x2d},
        {"NULL", NULL, 0x2e},
        {"NULL", NULL, 0x2f},
        {"NULL", NULL, 0x30},
        {"NULL", NULL, 0x31},
        {"NULL", NULL, 0x32},
        {"NULL", NULL, 0x33},
        {"NULL", NULL, 0x34},
        {"NULL", NULL, 0x35},
        {"NULL", NULL, 0x36},
        {"NULL", NULL, 0x37},
        {"NULL", NULL, 0x38},
        {"NULL", NULL, 0x39},
        {"NULL", NULL, 0x3a},
        {"NULL", NULL, 0x3b},
        {"NULL", NULL, 0x3c},
        {"NULL", NULL, 0x3d},
        {"NULL", NULL, 0x3e},
        {"NULL", NULL, 0x3f},

};


static struct mips64_op_desc mips_tlb_opcodes[] = {
        {"NULL", NULL, 0x0},
        {"tlbr", mips64_exec_TLBR, 0x1},
        {"tlbwi", mips64_exec_TLBWI, 0x2},
        {"NULL", NULL, 0x3},
        {"NULL", NULL, 0x4},
        {"NULL", NULL, 0x5},
        {"tlbwi", mips64_exec_TLBWI, 0x6},
        {"NULL", NULL, 0x7},
        {"tlbp", mips64_exec_TLBP, 0x8},
        {"NULL", NULL, 0x9},
        {"NULL", NULL, 0xa},
        {"NULL", NULL, 0xb},
        {"NULL", NULL, 0xc},
        {"NULL", NULL, 0xd},
        {"NULL", NULL, 0xe},
        {"NULL", NULL, 0xf},
        {"NULL", NULL, 0x10},
        {"NULL", NULL, 0x11},
        {"NULL", NULL, 0x12},
        {"NULL", NULL, 0x13},
        {"NULL", NULL, 0x14},
        {"NULL", NULL, 0x15},
        {"NULL", NULL, 0x16},
        {"NULL", NULL, 0x17},
        {"eret", mips64_exec_ERET, 0x18},
        {"NULL", NULL, 0x19},
        {"NULL", NULL, 0x1a},
        {"NULL", NULL, 0x1b},
        {"NULL", NULL, 0x1c},
        {"NULL", NULL, 0x1d},
        {"NULL", NULL, 0x1e},
        {"NULL", NULL, 0x1f},
        {"wait", mips64_exec_WAIT, 0x20},
        {"NULL", NULL, 0x21},
        {"NULL", NULL, 0x22},
        {"NULL", NULL, 0x23},
        {"NULL", NULL, 0x24},
        {"NULL", NULL, 0x25},
        {"NULL", NULL, 0x26},
        {"NULL", NULL, 0x27},
        {"NULL", NULL, 0x28},
        {"NULL", NULL, 0x29},
        {"NULL", NULL, 0x2a},
        {"NULL", NULL, 0x2b},
        {"NULL", NULL, 0x2c},
        {"NULL", NULL, 0x2d},
        {"NULL", NULL, 0x2e},
        {"NULL", NULL, 0x2f},
        {"NULL", NULL, 0x30},
        {"NULL", NULL, 0x31},
        {"NULL", NULL, 0x32},
        {"NULL", NULL, 0x33},
        {"NULL", NULL, 0x34},
        {"NULL", NULL, 0x35},
        {"NULL", NULL, 0x36},
        {"NULL", NULL, 0x37},
        {"NULL", NULL, 0x38},
        {"NULL", NULL, 0x39},
        {"NULL", NULL, 0x3a},
        {"NULL", NULL, 0x3b},
        {"NULL", NULL, 0x3c},
        {"NULL", NULL, 0x3d},
        {"NULL", NULL, 0x3e},
        {"NULL", NULL, 0x3f},

};

/**
 * mips64_exec_SPEC_
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_SPEC_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_u16_t special_func = (std_u16_t)bits(insn, 0, 5);

    return mips_spec_opcodes[special_func].func(cpu, insn);
}

/**
 * mips64_exec_BCOND_
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_BCOND_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_u16_t special_func = (std_u16_t)bits(insn, 16, 20);

    return mips_bcond_opcodes[special_func].func(cpu, insn);
}

/**
 * mips64_exec_COP0_
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_COP0_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_u16_t special_func = (std_u16_t)bits(insn, 21, 25);

    return mips_cop0_opcodes[special_func].func(cpu, insn);
}

/**
 * mips64_exec_MAD_
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_MAD_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_u16_t index = (std_u16_t)bits(insn, 0, 5);

    return mips_mad_opcodes[index].func(cpu, insn);
}

/**
 * mips64_exec_TLB_
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  static fastcall int
 */
static fastcall std_int_t mips64_exec_TLB_(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
    std_u16_t func = (std_u16_t)bits(insn, 0, 5);

    return mips_tlb_opcodes[func].func(cpu, insn);
}


#if 0
/* Execute a single instruction */
forced_inline std_int_t mips64_exec_single_instruction(cvm_arch_mips_cpu_t *cpu, std_u32_t instruction)
{
  std_int_t i = 0;
  
  for (i = 0; i < sizeof(mips64_exec_tags)/sizeof(struct mips64_insn_exec_tag); i++){
	  if ((instruction & mips64_exec_tags[i].mask) == mips64_exec_tags[i].value){
		  return mips64_exec_tags[i].exec(cpu, instruction);
	  }
  }
  
  return 0;
}

#else


/**
 * mips64_exec_single_instruction
 * @brief   
 * @param   cpu
 * @param   insn
 * @return  int
 */
int mips64_exec_single_instruction(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t insn)
{
#define MAJOR_OP(_inst) (((std_uint_t) _inst >> 26) & 0x3f)
    register std_u8_t op;

    op = MAJOR_OP(insn);
    return mips_exec_opcodes[op].func(cpu, insn);
}

#endif
