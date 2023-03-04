/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch_mips_cpu_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_arch_mips_cpu_CVM.h"
#include "cvm_arch_mips_cpu.h"
#include "mips64_jit.h"
#include "host_alarm.h"
#include "cpu_mts.h"

mod_vm_arch_mips_cpu_t *p_global_vm_arch_mips_cpu;
mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;
mod_vm_memory_t *p_global_vm_memory;


/**
 * mod_vm_arch_mips_cpu_CVM_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_mips_cpu_CVM_init(IN mod_vm_arch_mips_cpu_t *p_m, IN const std_char_t *arg,
                                                IN std_int_t arg_len)
{
    mod_vm_arch_mips_cpu_imp_t *p_imp_m = (mod_vm_arch_mips_cpu_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";
    mod_iid_t mod_vm_arch_mips_cp0_iid = MOD_VM_ARCH_MIPS_CP0_IID;
    mod_iid_t mod_vm_memory_iid = MOD_VM_MEMORY_IID;
    cvm_arch_mips_cpu_t *cpu = NULL;

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_fetch");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_fetch, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_exec");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_exec, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_set_pc");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_set_pc, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_set_llbit");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_set_llbit, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_is_in_bdslot");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_is_in_bdslot, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_break_idle");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cpu_CVM_break_idle, p_imp_m);


    mod_query_instance(&mod_vm_arch_mips_cp0_iid, (std_void_t **) &p_global_vm_arch_mips_cp0, (mod_ownership_t *) p_m);
    mod_query_instance(&mod_vm_memory_iid, (std_void_t **) &p_global_vm_memory, (mod_ownership_t *) p_m);
    p_global_vm_arch_mips_cpu = p_m;

    cpu = &p_imp_m->cpu;

    /* Load Operations */
    cpu->mem_op_fn[MIPS_MEMOP_LB]  = (mips_memop_fn)mips_mts32_lb;
    cpu->mem_op_fn[MIPS_MEMOP_LBU] = (mips_memop_fn)mips_mts32_lbu;
    cpu->mem_op_fn[MIPS_MEMOP_LH]  = (mips_memop_fn)mips_mts32_lh;
    cpu->mem_op_fn[MIPS_MEMOP_LHU] = (mips_memop_fn)mips_mts32_lhu;
    cpu->mem_op_fn[MIPS_MEMOP_LW]  = (mips_memop_fn)mips_mts32_fast_lw;
    cpu->mem_op_fn[MIPS_MEMOP_LWU] = (mips_memop_fn)mips_mts32_lwu;
    cpu->mem_op_fn[MIPS_MEMOP_LD]  = (mips_memop_fn)mips_mts32_ld;
    cpu->mem_op_fn[MIPS_MEMOP_LDL] = (mips_memop_fn)mips_mts32_ldl;
    cpu->mem_op_fn[MIPS_MEMOP_LDR] = (mips_memop_fn)mips_mts32_ldr;

    /* Store Operations */
    cpu->mem_op_fn[MIPS_MEMOP_SB] = (mips_memop_fn)mips_mts32_sb;
    cpu->mem_op_fn[MIPS_MEMOP_SH] = (mips_memop_fn)mips_mts32_sh;
    cpu->mem_op_fn[MIPS_MEMOP_SW] = (mips_memop_fn) mips_mts32_fast_sw;
    cpu->mem_op_fn[MIPS_MEMOP_SD] = (mips_memop_fn)mips_mts32_sd;

    /* Load Left/Right operations */
    cpu->mem_op_fn[MIPS_MEMOP_LWL] = (mips_memop_fn)mips_mts32_lwl;
    cpu->mem_op_fn[MIPS_MEMOP_LWR] = (mips_memop_fn)mips_mts32_lwr;
    cpu->mem_op_fn[MIPS_MEMOP_LDL] = (mips_memop_fn)mips_mts32_ldl;
    cpu->mem_op_fn[MIPS_MEMOP_LDR] = (mips_memop_fn)mips_mts32_ldr;

    /* Store Left/Right operations */
    cpu->mem_op_fn[MIPS_MEMOP_SWL] = (mips_memop_fn)mips_mts32_swl;
    cpu->mem_op_fn[MIPS_MEMOP_SWR] = (mips_memop_fn)mips_mts32_swr;
    cpu->mem_op_fn[MIPS_MEMOP_SDL] = (mips_memop_fn)mips_mts32_sdl;
    cpu->mem_op_fn[MIPS_MEMOP_SDR] = (mips_memop_fn)mips_mts32_sdr;

    /* LL/SC - Load Linked / Store Conditional */
    cpu->mem_op_fn[MIPS_MEMOP_LL] = (mips_memop_fn)mips_mts32_ll;
    cpu->mem_op_fn[MIPS_MEMOP_SC] = (mips_memop_fn)mips_mts32_sc;

    cpu->mem_op_fn[MIPS_MEMOP_CACHE] = (mips_memop_fn)mips_mts32_cache;

    cpu->idle_max = 2000;
    cpu->idle_sleep_time = 30000;
    pthread_mutex_init(&cpu->idle_mutex, NULL);
    pthread_cond_init(&cpu->idle_cond, NULL);

    cpu->fast_memop = FALSE;
    cpu->addr_mode = 32;

    init_timers();
    mips64_jit_init(cpu);
    host_alarm_init(p_m);
    mips64_init_host_alarm();

    cpu->mts_u.mts32_cache = p_global_vm_memory->mts_u.mts32_cache;
    cpu->p_cp0_reg = p_global_vm_arch_mips_cp0->reg;
    return STD_RV_SUC;
}

/**
 * mod_vm_arch_mips_cpu_CVM_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_mips_cpu_CVM_cleanup(mod_vm_arch_mips_cpu_t *p_m)
{
    mod_vm_arch_mips_cpu_imp_t *p_imp_m = (mod_vm_arch_mips_cpu_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_reset");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_fetch");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_exec");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_set_pc");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_set_llbit");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_is_in_bdslot");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cpu_break_idle");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * mod_vm_arch_mips_cpu_CVM_reset
 * @brief   
 * @param   p_m
 * @param   entry
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cpu_CVM_reset(IN mod_vm_arch_mips_cpu_t *p_m, IN std_uint_t entry)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);
    cpu->pc = entry;
}

/**
 * mod_vm_arch_mips_cpu_CVM_fetch
 * @brief   
 * @param   p_m
 * @param   insn
 * @return  STD_CALL             std_uint_t
 */
STD_CALL std_uint_t mod_vm_arch_mips_cpu_CVM_fetch(IN mod_vm_arch_mips_cpu_t *p_m, IN std_uint_t *insn)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);
    int ret;
    cpu->gpr[0] = 0;

    ret = mips64_exec_fetch(cpu, (std_uint_t)cpu->pc, insn);

    return ret;
}

/**
 * mod_vm_arch_mips_cpu_CVM_exec
 * @brief   
 * @param   p_m
 * @param   insn
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cpu_CVM_exec(IN mod_vm_arch_mips_cpu_t *p_m, IN std_uint_t insn)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);
    std_32_t res;

    res = mips64_exec_single_instruction(cpu, insn);

    if (likely(!res)){
        cpu->pc += sizeof(std_u32_t);
    }
}

/**
 * mod_vm_arch_mips_cpu_CVM_set_pc
 * @brief   
 * @param   p_m
 * @param   pc
 * @return  STD_CALL             std_uint_t
 */
STD_CALL std_uint_t mod_vm_arch_mips_cpu_CVM_set_pc(IN mod_vm_arch_mips_cpu_t *p_m, IN std_uint_t pc)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);

    if (pc != 0){
        cpu->pc = pc;
    }

    return (std_uint_t)cpu->pc;
}

/**
 * mod_vm_arch_mips_cpu_CVM_set_llbit
 * @brief   
 * @param   p_m
 * @param   llbit
 * @return  STD_CALL             std_uint_t
 */
STD_CALL std_uint_t mod_vm_arch_mips_cpu_CVM_set_llbit(IN mod_vm_arch_mips_cpu_t *p_m, IN std_uint_t llbit)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);

    if (llbit != 0){
        cpu->ll_bit = llbit;
    }

    return cpu->ll_bit;
}

/**
 * mod_vm_arch_mips_cpu_CVM_is_in_bdslot
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_uint_t
 */
STD_CALL std_uint_t mod_vm_arch_mips_cpu_CVM_is_in_bdslot(IN mod_vm_arch_mips_cpu_t *p_m)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);

    return cpu->is_in_bdslot;
}

/**
 * mod_vm_arch_mips_cpu_CVM_break_idle
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cpu_CVM_break_idle(IN mod_vm_arch_mips_cpu_t *p_m)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);

    cpu_idle_break_wait(cpu);
}


/**
 * single_fetch_and_exec
 * @brief   
 * @param   cpu
 * @return  STD_CALL std_void_t
 */
STD_CALL std_void_t single_fetch_and_exec(cvm_arch_mips_cpu_t *cpu)
{
    int ret;
    std_u32_t insn;

    cpu->gpr[0] = 0;

    ret = mips64_exec_fetch(cpu, (std_uint_t)cpu->pc, &insn);

    if (unlikely(ret == 1)){
        //exception happened when fetching insn
        return;
    }
    std_32_t res;

    res = mips64_exec_single_instruction(cpu, insn);

    if (likely(!res)){
        cpu->pc += sizeof(std_u32_t);
    }
}

/**
 * mod_vm_arch_mips_cpu_CVM_run
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cpu_CVM_run(IN mod_vm_arch_mips_cpu_t *p_m)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *)p_m;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);
    std_bool_t jit_run = TRUE;

    if (unlikely(cpu->state != CPU_STATE_RUNNING)){
        goto LOOP_CHECK;
    }

    if (unlikely((cpu->pause_request) & CPU_INTERRUPT_EXIT)){
        cpu->state = CPU_STATE_PAUSING;
        goto LOOP_CHECK;
    }

#ifdef _USE_JIT_
        jit_fetch_and_exec(cpu);
#else
        single_fetch_and_exec(cpu);
#endif

LOOP_CHECK:
    cpu->cpu_thread_running = TRUE;
    while (cpu->cpu_thread_running)
    {
        switch (cpu->state)
        {
            case CPU_STATE_RUNNING:
                cpu->state = CPU_STATE_RUNNING;
                return;

            case CPU_STATE_HALTED:
                cpu->cpu_thread_running = FALSE;
                break;

            case CPU_STATE_RESTARTING:
                cpu->state = CPU_STATE_RESTARTING;
                /*Just waiting for cpu restart. */
                break;

            case CPU_STATE_PAUSING:
                mips64_main_loop_wait(cpu, 0);
                cpu->state = CPU_STATE_RUNNING;
                cpu->pause_request &= ~CPU_INTERRUPT_EXIT;
                /*start cpu again */
                return;

            default:
                break;
        }
    }
}

struct mod_vm_arch_mips_cpu_ops_st mod_vm_arch_mips_cpu_CVM_ops = {
        mod_vm_arch_mips_cpu_CVM_init,
        mod_vm_arch_mips_cpu_CVM_cleanup,

        /***func_ops***/
        mod_vm_arch_mips_cpu_CVM_reset,
        mod_vm_arch_mips_cpu_CVM_fetch,
        mod_vm_arch_mips_cpu_CVM_exec,
        mod_vm_arch_mips_cpu_CVM_set_pc,
        mod_vm_arch_mips_cpu_CVM_set_llbit,
        mod_vm_arch_mips_cpu_CVM_is_in_bdslot,
        mod_vm_arch_mips_cpu_CVM_break_idle,
        mod_vm_arch_mips_cpu_CVM_run,
};

/**
 * mod_vm_arch_mips_cpu_CVM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_mips_cpu_CVM_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_arch_mips_cpu_imp_t *p_m = NULL;

    p_m = (mod_vm_arch_mips_cpu_imp_t *) CALLOC(1, sizeof(mod_vm_arch_mips_cpu_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_arch_mips_cpu_CVM_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
