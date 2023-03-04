/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_arch_mips_cp0_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_arch_mips_cp0_CVM.h"
#include "cvm_arch_mips_cp0.h"
#include "mod_vm_arch_mips_cpu.h"
#include "cvm_cp0_tlb.h"

mod_vm_arch_mips_cpu_t *pg_vm_arch_mips_cpu;
/**
 * mod_vm_arch_mips_cp0_CVM_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_mips_cp0_CVM_init(IN mod_vm_arch_mips_cp0_t *p_m, IN const std_char_t *arg,
                                                IN std_int_t arg_len)
{
    mod_vm_arch_mips_cp0_imp_t *p_imp_m = (mod_vm_arch_mips_cp0_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    mod_iid_t mod_vm_arch_mips_cpu_iid = MOD_VM_ARCH_MIPS_CPU_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_trigger_exception");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_trigger_exception, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_tlb_op");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_tlb_op, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_tlb_lookup");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_tlb_lookup, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_mtc_op");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_mtc_op, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_mfc_op");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_mfc_op, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_access_special");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_access_special, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_irq_op");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_irq_op, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_timer");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_timer, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_eret");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_eret, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_soft_fpu");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_arch_mips_cp0_CVM_soft_fpu, p_imp_m);


    mod_query_instance(&mod_vm_arch_mips_cpu_iid, (std_void_t **) &pg_vm_arch_mips_cpu, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_arch_mips_cp0_CVM_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_mips_cp0_CVM_cleanup(mod_vm_arch_mips_cp0_t *p_m)
{
    mod_vm_arch_mips_cp0_imp_t *p_imp_m = (mod_vm_arch_mips_cp0_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_reset");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_trigger_exception");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_tlb_op");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_tlb_lookup");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_mtc_op");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_mfc_op");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_access_special");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_irq_op");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_timer");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_eret");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_arch_mips_cp0_soft_fpu");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/

/**
 * mod_vm_arch_mips_cp0_CVM_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cp0_CVM_reset(IN mod_vm_arch_mips_cp0_t *p_m)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips64_cp0_reset(cp0);
}

/**
 * mod_vm_arch_mips_cp0_CVM_trigger_exception
 * @brief   
 * @param   p_m
 * @param   type
 * @param   cause
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cp0_CVM_trigger_exception(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t type,
                                                               IN std_uint_t cause)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips64_trigger_exception(cp0, type, cause);
}

/**
 * mod_vm_arch_mips_cp0_CVM_tlb_op
 * @brief   
 * @param   p_m
 * @param   op_type
 * @param   insn
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cp0_CVM_tlb_op(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t op_type,
                                                    IN std_uint_t insn)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    switch (op_type){
        case OP_TLBP:
            mips64_cp0_exec_tlbp(cp0);
            break;

        case OP_TLBR:
            mips64_cp0_exec_tlbr(cp0);
            break;

        case OP_TLBWI:
            mips64_cp0_exec_tlbwi(cp0);
            break;

        case OP_TLBWR:
            mips64_cp0_exec_tlbwr(cp0);
            break;

        default:
            break;
    }
}

/**
 * mod_vm_arch_mips_cp0_CVM_tlb_lookup
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @param   res
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_arch_mips_cp0_CVM_tlb_lookup(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t vaddr,
                                                       IN std_void_t *res)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    return mips64_cp0_tlb_lookup(cp0, vaddr, (mts_map_t *)res);
}

/**
 * mod_vm_arch_mips_cp0_CVM_mtc_op
 * @brief   
 * @param   p_m
 * @param   cp0_reg
 * @param   val
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cp0_CVM_mtc_op(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t cp0_reg,
                                                    IN std_uint_t val)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips64_cp0_set_reg(cp0, cp0_reg, val);
}

/**
 * mod_vm_arch_mips_cp0_CVM_mfc_op
 * @brief   
 * @param   p_m
 * @param   cp0_reg
 * @param   sel
 * @return  STD_CALL             std_uint_t
 */
STD_CALL std_uint_t mod_vm_arch_mips_cp0_CVM_mfc_op(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t cp0_reg,
                                                    IN std_uint_t sel)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    return mips64_cp0_get_reg_fast(cp0, cp0_reg, sel);
}

/**
 * mod_vm_arch_mips_cp0_CVM_access_special
 * @brief   
 * @param   p_m
 * @param   vaddr
 * @param   mask
 * @param   op_code
 * @param   op_type
 * @param   op_size
 * @param   data
 * @param   exc
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_arch_mips_cp0_CVM_access_special(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t vaddr,
                                                            IN std_uint_t mask, IN std_uint_t op_code,
                                                            IN std_uint_t op_type, IN std_uint_t op_size,
                                                            IN std_uint_t *data)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips_access_special(cp0,
                        vaddr,
                        mask,
                        op_code,
                        op_type,
                        op_size,
                        data);
}

/**
 * mod_vm_arch_mips_cp0_CVM_irq_op
 * @brief   
 * @param   p_m
 * @param   type
 * @param   irq
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_arch_mips_cp0_CVM_irq_op(IN mod_vm_arch_mips_cp0_t *p_m, IN std_uint_t type,
                                                   IN std_uint_t irq)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;
    std_int_t ret = 0;

    switch (type){
        case OP_IRQ_UPDATE:
            ret = mips64_update_irq_flag(cp0);
            break;

        case OP_IRQ_CLEAR:
            mips64_clear_irq(cp0, (std_u8_t)irq);
            break;

        case OP_IRQ_SET:
            mips64_set_irq(cp0, (std_u8_t)irq);
            break;
        case OP_IRQ_IS_PENDING:
            ret = mips64_is_pending(cp0);
            break;
        default:
            break;
    }
    return ret;
}

/**
 * mod_vm_arch_mips_cp0_CVM_timer
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_arch_mips_cp0_CVM_timer(IN mod_vm_arch_mips_cp0_t *p_m)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips64_cp0_timer(cp0);

    return 0;
}

/**
 * mod_vm_arch_mips_cp0_CVM_eret
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_arch_mips_cp0_CVM_eret(IN mod_vm_arch_mips_cp0_t *p_m)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips64_cp0_eret(cp0);

    return 0;
}

/**
 * mod_vm_arch_mips_cp0_CVM_soft_fpu
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_int_t
 */
STD_CALL std_int_t mod_vm_arch_mips_cp0_CVM_soft_fpu(IN mod_vm_arch_mips_cp0_t *p_m)
{
    mod_vm_arch_mips_cp0_imp_t *p_vm_arch_mips_cp0_imp = (mod_vm_arch_mips_cp0_imp_t *)p_m;
    cvm_arch_mips_cp0_t *cp0 = &p_vm_arch_mips_cp0_imp->cp0;

    mips64_cp0_soft_fpu(cp0);
    return 0;
}

struct mod_vm_arch_mips_cp0_ops_st mod_vm_arch_mips_cp0_CVM_ops = {
        mod_vm_arch_mips_cp0_CVM_init,
        mod_vm_arch_mips_cp0_CVM_cleanup,

        /***func_ops***/
        mod_vm_arch_mips_cp0_CVM_reset,
        mod_vm_arch_mips_cp0_CVM_trigger_exception,
        mod_vm_arch_mips_cp0_CVM_tlb_op,
        mod_vm_arch_mips_cp0_CVM_tlb_lookup,
        mod_vm_arch_mips_cp0_CVM_mtc_op,
        mod_vm_arch_mips_cp0_CVM_mfc_op,
        mod_vm_arch_mips_cp0_CVM_access_special,
        mod_vm_arch_mips_cp0_CVM_irq_op,
        mod_vm_arch_mips_cp0_CVM_timer,
        mod_vm_arch_mips_cp0_CVM_eret,
        mod_vm_arch_mips_cp0_CVM_soft_fpu,

};

/**
 * mod_vm_arch_mips_cp0_CVM_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_arch_mips_cp0_CVM_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_arch_mips_cp0_imp_t *p_m = NULL;

    p_m = (mod_vm_arch_mips_cp0_imp_t *) CALLOC(1, sizeof(mod_vm_arch_mips_cp0_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_arch_mips_cp0_CVM_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
