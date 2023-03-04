/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    stub_shell_mod_vm_ADM5120.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */


/**
     * @file    stub_shell_mod_vm_ADM5120.c
     * @brief   implement functions
     * @version 1.0
     * @author  Yunhai Zhu
     * @date    2022-01-05
     *
     */

#include "json-maker.h"
#include "mod_vm.h"
#include "std_common.h"
#include "tiny-json.h"

/**
 * shell_stub_mod_vm_ADM5120_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_initiate(IN std_void_t *p_handle, IN std_char_t *params)
{
    json_t const *json;
    std_char_t response_string[128];
    std_char_t *response;
    std_char_t *dest;
    mod_vm_t *mod_vm = (mod_vm_t *) p_handle;
    /****server_stub_args_def****/

    std_char_t *conf_name = NULL;

    /*****server_stub_args_body*****/
    json_t mem[32];
    json = json_create(params, mem, sizeof mem / sizeof *mem);
    STD_ASSERT_RV(json != NULL, NULL);

    json_t const *param_obj = NULL;
    param_obj = json_getProperty(json, "conf_name");

    conf_name = (std_char_t *) json_getValue(param_obj);

    /******server_stub_call*****/
    mod_vm_initiate(mod_vm, conf_name);

    dest = json_objOpen(response_string, NULL);
    dest = json_objOpen(dest, "response");
    dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
    dest = json_str(dest, "ret_type", "VOID");

    dest = json_objClose(dest);
    dest = json_objClose(dest);
    dest = json_objClose(dest);
    json_end(dest);

    response = strdup(response_string);

    return response;
}

/**
 * shell_stub_mod_vm_ADM5120_start
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_start(IN std_void_t *p_handle, IN std_char_t *params)
{
    json_t const *json;
    std_char_t response_string[128];
    std_char_t *response;
    std_char_t *dest;
    mod_vm_t *mod_vm = (mod_vm_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
    json_t mem[32];
    json = json_create(params, mem, sizeof mem / sizeof *mem);
    STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
    mod_vm_start(mod_vm);

    dest = json_objOpen(response_string, NULL);
    dest = json_objOpen(dest, "response");
    dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
    dest = json_str(dest, "ret_type", "VOID");

    dest = json_objClose(dest);
    dest = json_objClose(dest);
    dest = json_objClose(dest);
    json_end(dest);

    response = strdup(response_string);

    return response;
}

/**
 * shell_stub_mod_vm_ADM5120_stop
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_stop(IN std_void_t *p_handle, IN std_char_t *params)
{
    json_t const *json;
    std_char_t response_string[128];
    std_char_t *response;
    std_char_t *dest;
    mod_vm_t *mod_vm = (mod_vm_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
    json_t mem[32];
    json = json_create(params, mem, sizeof mem / sizeof *mem);
    STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
    mod_vm_stop(mod_vm);

    dest = json_objOpen(response_string, NULL);
    dest = json_objOpen(dest, "response");
    dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
    dest = json_str(dest, "ret_type", "VOID");

    dest = json_objClose(dest);
    dest = json_objClose(dest);
    dest = json_objClose(dest);
    json_end(dest);

    response = strdup(response_string);

    return response;
}

/**
 * shell_stub_mod_vm_ADM5120_suspend
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_suspend(IN std_void_t *p_handle, IN std_char_t *params)
{
    json_t const *json;
    std_char_t response_string[128];
    std_char_t *response;
    std_char_t *dest;
    mod_vm_t *mod_vm = (mod_vm_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
    json_t mem[32];
    json = json_create(params, mem, sizeof mem / sizeof *mem);
    STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
    mod_vm_suspend(mod_vm);

    dest = json_objOpen(response_string, NULL);
    dest = json_objOpen(dest, "response");
    dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
    dest = json_str(dest, "ret_type", "VOID");

    dest = json_objClose(dest);
    dest = json_objClose(dest);
    dest = json_objClose(dest);
    json_end(dest);

    response = strdup(response_string);

    return response;
}

/**
 * shell_stub_mod_vm_ADM5120_resume
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_ADM5120_resume(IN std_void_t *p_handle, IN std_char_t *params)
{
    json_t const *json;
    std_char_t response_string[128];
    std_char_t *response;
    std_char_t *dest;
    mod_vm_t *mod_vm = (mod_vm_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
    json_t mem[32];
    json = json_create(params, mem, sizeof mem / sizeof *mem);
    STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
    mod_vm_resume(mod_vm);

    dest = json_objOpen(response_string, NULL);
    dest = json_objOpen(dest, "response");
    dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
    dest = json_str(dest, "ret_type", "VOID");

    dest = json_objClose(dest);
    dest = json_objClose(dest);
    dest = json_objClose(dest);
    json_end(dest);

    response = strdup(response_string);

    return response;
}
