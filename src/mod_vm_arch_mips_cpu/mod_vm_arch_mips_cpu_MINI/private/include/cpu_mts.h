//
// Created by yun on 2/27/22.
//

#ifndef LORIS_VM_CPU_MTS_H
#define LORIS_VM_CPU_MTS_H

#include "std_common.h"
#include "mod_vm_memory.h"
#include "cvm_arch_mips_cpu.h"

/**
 * mips_mts32_lb
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lb(struct cvm_arch_mips_cpu *cpu, std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_lbu
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lbu(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_lh
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lh(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_lhu
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lhu(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_lw
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lw(cvm_arch_mips_cpu_t * cpu, std_u32_t vaddr, std_uint_t reg);
/**
 * mips_mts32_lwu
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lwu(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_ld
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_ld(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sb
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sb(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sh
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sh(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sw
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sw(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sd
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sd(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_lwl
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lwl(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_lwr
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_lwr(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_ldl
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_ldl(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_ldr
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_ldr(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_swl
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_swl(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_swr
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_swr(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sdl
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sdl(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sdr
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sdr(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_ll
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_ll(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_sc
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_sc(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t reg);
/**
 * mips_mts32_cache
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   op
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_cache(cvm_arch_mips_cpu_t *cpu,std_u32_t vaddr,std_uint_t op);

/**
 * mips_mts32_fast_sw
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_fast_sw(IN cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, std_uint_t reg);
/**
 * mips_mts32_fast_lw
 * @brief
 * @param   cpu
 * @param   vaddr
 * @param   reg
 * @return  fastcall std_void_t
 */
fastcall std_void_t mips_mts32_fast_lw(cvm_arch_mips_cpu_t *cpu, IN std_u32_t vaddr, std_uint_t reg);


#endif//LORIS_VM_CPU_MTS_H
