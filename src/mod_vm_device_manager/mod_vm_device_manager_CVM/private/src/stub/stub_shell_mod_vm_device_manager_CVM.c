/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    stub_shell_mod_vm_device_manager_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */


    /**
     * @file    stub_shell_mod_vm_device_manager_CVM.c
     * @brief   implement functions
     * @version 1.0
     * @author  Yunhai Zhu
     * @date    2022-01-05
     *
     */

#include "std_common.h"
#include "mod_vm_device_manager.h"
#include "tiny-json.h"
#include "json-maker.h"

/**
 * shell_stub_mod_vm_device_manager_CVM_add
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_manager_CVM_add(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_manager_t *mod_vm_device_manager = (mod_vm_device_manager_t *) p_handle;
    /****server_stub_args_def****/

	mod_vm_device_t *p_dev = NULL;

	std_uint_t base = 0;

	std_uint_t len = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "p_dev");

	p_dev = (mod_vm_device_t *) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "base");

	base = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "len");

	len = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_manager_add(mod_vm_device_manager, p_dev, base, len);

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
 * shell_stub_mod_vm_device_manager_CVM_del
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_manager_CVM_del(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_manager_t *mod_vm_device_manager = (mod_vm_device_manager_t *) p_handle;
    /****server_stub_args_def****/

	mod_vm_device_t *p_dev = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "p_dev");

	p_dev = (mod_vm_device_t *) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_manager_del(mod_vm_device_manager, p_dev);

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
 * shell_stub_mod_vm_device_manager_CVM_find
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_manager_CVM_find(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_manager_t *mod_vm_device_manager = (mod_vm_device_manager_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t addr = 0;
    std_int_t dev_id;

	mod_vm_device_t **pp_dev = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "addr");

	addr = (std_uint_t) json_getInteger(param_obj);

    param_obj = json_getProperty(json, "dev_id");

    dev_id = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "pp_dev");

	pp_dev = (mod_vm_device_t **) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_manager_find(mod_vm_device_manager, addr, dev_id, pp_dev);

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
