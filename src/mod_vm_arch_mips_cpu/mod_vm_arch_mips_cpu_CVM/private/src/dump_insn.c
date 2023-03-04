/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    dump_insn.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */

#include "dump_insn.h"

/* MIPS general purpose registers names */
extern char *mips64_gpr_reg_names[MIPS64_GPR_NR];

/* MIPS cp0 registers names */
char *mips64_cp0_reg_names[32] = {
        "index" ,
        "random",
        "entry_lo0",
        "entry_lo1",
        "context",
        "pagemask",
        "wired",
        "info",
        "badvaddr",
        "count",
        "entry_hi",
        "compare",
        "status",
        "cause",
        "epc",
        "prid",
        "config",
        "ll_addr",
        "watch_lo",
        "watch_hi",
        "xcontext",
        "cp0_r21",
        "cp0_r22",
        "cp0_r23",
        "cp0_r24",
        "cp0_r25",
        "ecc",
        "cache_err",
        "tag_lo",
        "tag_hi",
        "err_epc",
        "cp0_r31",
};

extern struct mips64_jit_desc mips_jit[];
extern struct mips64_jit_desc mips_spec_jit[];
extern struct mips64_jit_desc mips_bcond_jit[];
extern struct mips64_jit_desc mips_cop0_jit[];

/* Dump an instruction */
int mips64_dump_insn(char *buffer,size_t buf_size,size_t insn_name_size, std_u64_t pc, mips_insn_t instruction)
{
    std_char_t insn_name[64];
    std_char_t insn_format[32];
    std_char_t *name;
    std_int_t base;
    std_int_t rs;
    std_int_t rd;
    std_int_t rt;
    std_int_t sa;
    std_int_t offset;
    std_int_t imm;
    struct mips64_jit_desc *tag;
    std_u64_t new_pc;
    std_16_t special_func;
    register std_uint_t op;

    op = MAJOR_OP(instruction);

    if (op == 0){
        special_func = (std_16_t)bits(instruction, 0, 5);
        tag = &mips_spec_jit[special_func];
    }else if (op == 1){
        special_func = (std_16_t)bits(instruction, 16, 20);
        tag = &mips_bcond_jit[special_func];
    }else if (op == 16){
        special_func = (std_16_t)bits(instruction, 21, 25);
        tag = &mips_cop0_jit[special_func];
    }else {
        tag = &mips_jit[op];
    }

    if (!tag) {
        snprintf(buffer,buf_size,"%8.8x  (unknown)",instruction);
        return(-1);
    }

    if (!(name = tag->opname))
        name = "[unknown]";

    if (!insn_name_size)
        insn_name_size = 10;

    snprintf(insn_format,sizeof(insn_format),"%%-%lus",insn_name_size);
    snprintf(insn_name,sizeof(insn_name),insn_format,name);

    switch(tag->instr_type) {
        case 1:   /* instructions without operands */
            snprintf(buffer,buf_size,"%8.8x  %s",instruction,insn_name);
            break;

        case 2:   /* load/store instructions */
            base   = bits(instruction,21,25);
            rt     = bits(instruction,16,20);
            offset = (std_16_t)bits(instruction,0,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%d(%s)",
                     instruction,insn_name,mips64_gpr_reg_names[rt],
                     offset,mips64_gpr_reg_names[base]);
            break;

        case 3:   /* GPR[rd] = GPR[rs] op GPR[rt] */
            rs = bits(instruction,21,25);
            rt = bits(instruction,16,20);
            rd = bits(instruction,11,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s,%s",
                     instruction,insn_name,mips64_gpr_reg_names[rd],
                     mips64_gpr_reg_names[rs],mips64_gpr_reg_names[rt]);
            break;

        case 4:   /* GPR[rd] = GPR[rt] op GPR[rs] */
            rs = bits(instruction,21,25);
            rt = bits(instruction,16,20);
            rd = bits(instruction,11,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s,%s",
                     instruction,insn_name,mips64_gpr_reg_names[rd],
                     mips64_gpr_reg_names[rt],mips64_gpr_reg_names[rs]);
            break;

        case 5:   /* GPR[rt] = GPR[rs] op immediate (hex) */
            rs  = bits(instruction,21,25);
            rt  = bits(instruction,16,20);
            imm = bits(instruction,0,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s,0x%x",
                     instruction,insn_name,mips64_gpr_reg_names[rt],
                     mips64_gpr_reg_names[rs],imm);
            break;

        case 6:   /* GPR[rt] = GPR[rs] op immediate (dec) */
            rs  = bits(instruction,21,25);
            rt  = bits(instruction,16,20);
            imm = bits(instruction,0,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s,%d",
                     instruction,insn_name,mips64_gpr_reg_names[rt],
                     mips64_gpr_reg_names[rs],(std_16_t)imm);
            break;

        case 7:   /* GPR[rd] = GPR[rt] op sa */
            rt = bits(instruction,16,20);
            rd = bits(instruction,11,15);
            sa = bits(instruction,6,10);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s,%d",
                     instruction,insn_name,mips64_gpr_reg_names[rd],
                     mips64_gpr_reg_names[rt],sa);
            break;

        case 8:   /* Branch with: GPR[rs] / GPR[rt] / offset */
            rs = bits(instruction,21,25);
            rt = bits(instruction,16,20);
            offset = bits(instruction,0,15);
            new_pc = (pc + 4) + sign_extend(offset << 2,18);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s,0x%lx",
                     instruction,insn_name,mips64_gpr_reg_names[rs],
                     mips64_gpr_reg_names[rt],new_pc);
            break;

        case 9:   /* Branch with: GPR[rs] / offset */
            rs = bits(instruction,21,25);
            offset = bits(instruction,0,15);
            new_pc = (pc + 4) + sign_extend(offset << 2,18);
            snprintf(buffer,buf_size,"%8.8x  %s %s,0x%lx",
                     instruction,insn_name,mips64_gpr_reg_names[rs],new_pc);
            break;

        case 10:   /* Branch with: offset */
            offset = bits(instruction,0,15);
            new_pc = (pc + 4) + sign_extend(offset << 2,18);
            snprintf(buffer,buf_size,"%8.8x  %s 0x%lx",
                     instruction,insn_name,new_pc);
            break;

        case 11:   /* Jump */
            offset = bits(instruction,0,25);
            new_pc = (pc & ~((1 << 28) - 1)) | (offset << 2);
            snprintf(buffer,buf_size,"%8.8x  %s 0x%lx",
                     instruction,insn_name,new_pc);
            break;

        case 13:   /* op GPR[rs] */
            rs = bits(instruction,21,25);
            snprintf(buffer,buf_size,"%8.8x  %s %s",
                     instruction,insn_name,mips64_gpr_reg_names[rs]);
            break;

        case 14:   /* op GPR[rd] */
            rd = bits(instruction,11,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s",
                     instruction,insn_name,mips64_gpr_reg_names[rd]);
            break;

        case 15:   /* op GPR[rd], GPR[rs] */
            rs = bits(instruction,21,25);
            rd = bits(instruction,11,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s",
                     instruction,insn_name,mips64_gpr_reg_names[rd],
                     mips64_gpr_reg_names[rs]);
            break;

        case 16:   /* op GPR[rt], imm */
            rt  = bits(instruction,16,20);
            imm = bits(instruction,0,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,0x%x",
                     instruction,insn_name,mips64_gpr_reg_names[rt],imm);
            break;

        case 17:   /* op GPR[rs], GPR[rt] */
            rs = bits(instruction,21,25);
            rt = bits(instruction,16,20);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s",
                     instruction,insn_name,mips64_gpr_reg_names[rs],
                     mips64_gpr_reg_names[rt]);
            break;

        case 18:   /* op GPR[rt], CP0[rd] */
            rt = bits(instruction,16,20);
            rd = bits(instruction,11,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,%s",
                     instruction,insn_name,mips64_gpr_reg_names[rt],
                     mips64_cp0_reg_names[rd]);
            break;

        case 19:   /* op GPR[rt], $rd */
            rt = bits(instruction,16,20);
            rd = bits(instruction,11,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,$%d",
                     instruction,insn_name,mips64_gpr_reg_names[rt],rd);
            break;

        case 20:   /* op GPR[rs], imm */
            rs = bits(instruction,21,25);
            imm = bits(instruction,0,15);
            snprintf(buffer,buf_size,"%8.8x  %s %s,0x%x",
                     instruction,insn_name,mips64_gpr_reg_names[rs],imm);
            break;

        default:
            snprintf(buffer,buf_size,"%8.8x  %s (TO DEFINE - %d)",
                     instruction,insn_name,tag->instr_type);
            return(-1);
    }

    return 0;
}

/**
 * mips64_dump_insn_with_buffer
 * @brief   
 * @param   pc
 * @param   insn
 */
void mips64_dump_insn_with_buffer(std_u64_t pc, mips_insn_t insn)
{
    char buffer[80];

    mips64_dump_insn(buffer,sizeof(buffer),sizeof(mips_insn_t),pc,insn);

    STD_LOG(LOG_FILE, "pc %x %s\n", pc, buffer);
}

