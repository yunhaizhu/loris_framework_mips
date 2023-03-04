/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    stub_shell_mod_vm_device_NORFLASH4M.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */


    /**
     * @file    stub_shell_mod_vm_device_NORFLASH4M.c
     * @brief   implement functions
     * @version 1.0
     * @author  Yunhai Zhu
     * @date    2022-01-05
     *
     */

#include "std_common.h"
#include "mod_vm_device.h"
#include "tiny-json.h"
#include "json-maker.h"

/**
 * shell_stub_mod_vm_device_NORFLASH4M_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_initiate(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_t *mod_vm_device = (mod_vm_device_t *) p_handle;
    /****server_stub_args_def****/

    vm_device_info_t *arg = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "arg");

	arg = (vm_device_info_t *) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_initiate(mod_vm_device, arg);

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
 * shell_stub_mod_vm_device_NORFLASH4M_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_reset(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_t *mod_vm_device = (mod_vm_device_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	mod_vm_device_reset(mod_vm_device);

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
 * shell_stub_mod_vm_device_NORFLASH4M_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_access(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_t *mod_vm_device = (mod_vm_device_t *) p_handle;
    /****server_stub_args_def****/

	std_void_t *ret = NULL;

	vm_device_access_t *arg = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "arg");

	arg = (vm_device_access_t *) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_device_access(mod_vm_device, arg);

	dest = json_objOpen(response_string, NULL);
	dest = json_objOpen(dest, "response");
	dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
	dest = json_str(dest, "ret_type", "ADDRESS_OP");
	dest = json_verylong(dest, "ret", (intptr_t) ret);

	dest = json_objClose(dest);
	dest = json_objClose(dest);
	dest = json_objClose(dest);
	json_end(dest);

	response = strdup(response_string);

	return response;
}

/**
 * shell_stub_mod_vm_device_NORFLASH4M_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_NORFLASH4M_command(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_t *mod_vm_device = (mod_vm_device_t *) p_handle;
    /****server_stub_args_def****/

	std_void_t *arg = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "arg");

	arg = (std_void_t *) json_getUInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_command(mod_vm_device, arg);

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
