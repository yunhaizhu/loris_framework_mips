/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    dump_insn.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
//
// Created by yun on 2/14/22.
//

#ifndef LORIS_VM_DUMP_INSN_H
#define LORIS_VM_DUMP_INSN_H

#include "cvm_arch_mips_cpu.h"
#include "mips64_jit.h"

/**
 * mips64_dump_insn_with_buffer
 * @brief   
 * @param   pc
 * @param   insn
 */
void mips64_dump_insn_with_buffer(std_u64_t pc, mips_insn_t insn);

#endif//LORIS_VM_DUMP_INSN_H
