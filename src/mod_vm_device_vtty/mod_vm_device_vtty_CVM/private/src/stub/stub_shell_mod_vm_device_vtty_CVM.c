/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    stub_shell_mod_vm_device_vtty_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-25
 *
 */


/**
 * @file    stub_shell_mod_vm_device_vtty_CVM.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-01-05
 *
 */

#include "std_common.h"
#include "mod_vm_device_vtty.h"
#include "tiny-json.h"
#include "json-maker.h"

/**
 * shell_stub_mod_vm_device_vtty_CVM_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_initiate(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	mod_vm_device_vtty_initiate(mod_vm_device_vtty);

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
 * shell_stub_mod_vm_device_vtty_CVM_create
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_create(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

	vtty_tt *ret = NULL;

	std_char_t *name = NULL;

	std_int_t type = 0;

	std_int_t tcp_port = 0;

	vtty_serial_option_t *option = NULL;

	read_notifier_t rnf = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "name");

	name = (std_char_t *) json_getValue(param_obj);

	param_obj = json_getProperty(json, "type");

	type = (std_int_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "tcp_port");

	tcp_port = (std_int_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "option");

	option = (vtty_serial_option_t *) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "rnf");

	rnf = (read_notifier_t ) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_device_vtty_create(mod_vm_device_vtty, name, type, tcp_port, option, rnf);

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
 * shell_stub_mod_vm_device_vtty_CVM_get_char
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_get_char(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

	vtty_tt *vtty = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vtty");

	vtty = (vtty_tt *) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_device_vtty_get_char(mod_vm_device_vtty, vtty);

	dest = json_objOpen(response_string, NULL);
	dest = json_objOpen(dest, "response");
	dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
	dest = json_str(dest, "ret_type", "INTEGER");
	dest = json_verylong(dest, "ret", ret);

	dest = json_objClose(dest);
	dest = json_objClose(dest);
	dest = json_objClose(dest);
	json_end(dest);

	response = strdup(response_string);

	return response;
}

/**
 * shell_stub_mod_vm_device_vtty_CVM_put_char
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_put_char(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

	vtty_tt *vtty = NULL;

	std_char_t ch = '\0';

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vtty");

	vtty = (vtty_tt *) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "ch");

	ch = (std_char_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_vtty_put_char(mod_vm_device_vtty, vtty, ch);

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
 * shell_stub_mod_vm_device_vtty_CVM_put_buffer
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_put_buffer(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

	vtty_tt *vtty = NULL;

	std_char_t *buf = NULL;

	std_uint_t len = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vtty");

	vtty = (vtty_tt *) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "buf");

	buf = (std_char_t *) json_getValue(param_obj);

	param_obj = json_getProperty(json, "len");

	len = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_device_vtty_put_buffer(mod_vm_device_vtty, vtty, buf, len);

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
 * shell_stub_mod_vm_device_vtty_CVM_is_char_avail
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_is_char_avail(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

	vtty_tt *vtty = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vtty");

	vtty = (vtty_tt *) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_device_vtty_is_char_avail(mod_vm_device_vtty, vtty);

	dest = json_objOpen(response_string, NULL);
	dest = json_objOpen(dest, "response");
	dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
	dest = json_str(dest, "ret_type", "INTEGER");
	dest = json_verylong(dest, "ret", ret);

	dest = json_objClose(dest);
	dest = json_objClose(dest);
	dest = json_objClose(dest);
	json_end(dest);

	response = strdup(response_string);

	return response;
}

/**
 * shell_stub_mod_vm_device_vtty_CVM_is_full
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_device_vtty_CVM_is_full(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_device_vtty_t *mod_vm_device_vtty = (mod_vm_device_vtty_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

	vtty_tt *vtty = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vtty");

	vtty = (vtty_tt *) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_device_vtty_is_full(mod_vm_device_vtty, vtty);

	dest = json_objOpen(response_string, NULL);
	dest = json_objOpen(dest, "response");
	dest = json_objOpen(dest, "return");

    /******server_stub_ret******/
	dest = json_str(dest, "ret_type", "INTEGER");
	dest = json_verylong(dest, "ret", ret);

	dest = json_objClose(dest);
	dest = json_objClose(dest);
	dest = json_objClose(dest);
	json_end(dest);

	response = strdup(response_string);

	return response;
}
