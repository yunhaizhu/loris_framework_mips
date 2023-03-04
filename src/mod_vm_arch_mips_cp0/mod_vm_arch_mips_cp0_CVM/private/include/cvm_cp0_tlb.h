#ifndef LORIS_VM_CVM_CP0_TLB_H
#define LORIS_VM_CVM_CP0_TLB_H

#include "cvm_arch_mips_cp0.h"
#include "mod_vm_arch_mips_cpu.h"

/**
 * mips64_cp0_exec_tlbp
 * @brief
 * @param   cp0
 * @return  std_void_t fastcall
 */
std_void_t fastcall mips64_cp0_exec_tlbp(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_exec_tlbr
 * @brief
 * @param   cp0
 * @return  std_void_t fastcall
 */
std_void_t fastcall mips64_cp0_exec_tlbr(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_exec_tlbwi
 * @brief
 * @param   cp0
 * @return  std_void_t fastcall
 */
std_void_t fastcall mips64_cp0_exec_tlbwi(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_exec_tlbwr
 * @brief
 * @param   cp0
 * @return  std_void_t fastcall
 */
std_void_t fastcall mips64_cp0_exec_tlbwr(cvm_arch_mips_cp0_t *cp0);

/**
 * mips64_cp0_tlb_lookup
 * @brief
 * @param   cp0
 * @param   vaddr
 * @param   res
 * @return  std_int_t
 */
std_int_t mips64_cp0_tlb_lookup(const cvm_arch_mips_cp0_t *cp0, std_u32_t vaddr, mts_map_t *res);

#endif//LORIS_VM_CVM_CP0_TLB_H
