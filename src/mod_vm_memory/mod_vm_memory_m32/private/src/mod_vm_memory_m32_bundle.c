/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    mod_vm_memory_m32_bundle.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */

#include "mod.h"
#include "mod_bundle.h"
#include "mod_vm_memory_m32.h"
#include "std_common.h"

static std_uint_t bundle_service_id = 0;
static mod_iid_t mod_vm_memory_iid = MOD_VM_MEMORY_IID;

/**
 * bundle_create
 * @brief   
 * @param   bundle_id
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t bundle_create(std_uint_t bundle_id)
{
    bundle_service_id = bundle_id;

    return STD_RV_SUC;
}

/**
 * bundle_destroy
 * @brief   
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t bundle_destroy()
{
    bundle_service_id = 0;
    return STD_RV_SUC;
}

/**
 * bundle_start
 * @brief   
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_int_t
 */
STD_CALL std_int_t bundle_start(IN const std_char_t *arg, IN const std_int_t arg_len)
{
    STD_ASSERT_RV(mod_register_instance_func(&mod_vm_memory_iid, mod_vm_memory_m32_create_instance) == STD_RV_SUC,
                  STD_RV_ERR_FAIL);
    STD_ASSERT_RV(mod_bundle_IID_register(bundle_service_id, &mod_vm_memory_iid) == STD_RV_SUC, STD_RV_ERR_FAIL);

    return STD_RV_SUC;
}

/**
 * bundle_stop
 * @brief   
 * @return  STD_CALL std_void_t
 */
STD_CALL std_void_t bundle_stop()
{
    STD_ASSERT_RV(mod_unregister_instance_func(&mod_vm_memory_iid) == STD_RV_SUC, );
    STD_ASSERT_RV(mod_bundle_IID_unregister(bundle_service_id) == STD_RV_SUC, );
}
