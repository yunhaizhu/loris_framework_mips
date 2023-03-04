/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    stub_shell_mod_vm_memory_MINI.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-03-01
 *
 */

#include "std_common.h"
#include "mod_vm_memory.h"
#include "tiny-json.h"
#include "json-maker.h"

/**
 * shell_stub_mod_vm_memory_MINI_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_memory_MINI_initiate(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_memory_t *mod_vm_memory = (mod_vm_memory_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	ret = mod_vm_memory_initiate(mod_vm_memory);

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
 * shell_stub_mod_vm_memory_MINI_lookup
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_memory_MINI_lookup(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_memory_t *mod_vm_memory = (mod_vm_memory_t *) p_handle;
    /****server_stub_args_def****/

	std_void_t *ret = NULL;

	std_uint_t vaddr = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vaddr");

	vaddr = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_memory_lookup(mod_vm_memory, vaddr);

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
 * shell_stub_mod_vm_memory_MINI_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_memory_MINI_access(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_memory_t *mod_vm_memory = (mod_vm_memory_t *) p_handle;
    /****server_stub_args_def****/

	std_void_t *ret = NULL;

	std_uint_t vaddr = 0;

	std_uint_t op_code = 0;

	std_uint_t op_size = 0;

	std_uint_t op_type = 0;

	std_uint_t *data = NULL;

	std_uchar_t *has_set_value = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vaddr");

	vaddr = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "op_code");

	op_code = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "op_size");

	op_size = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "op_type");

	op_type = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "data");

	data = (std_uint_t *) json_getUInteger(param_obj);

	param_obj = json_getProperty(json, "has_set_value");

	has_set_value = (std_uchar_t *) json_getValue(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_memory_access(mod_vm_memory, vaddr, op_code, op_size, op_type, data, has_set_value);

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
 * shell_stub_mod_vm_memory_MINI_map
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_memory_MINI_map(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_memory_t *mod_vm_memory = (mod_vm_memory_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t vaddr = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vaddr");

	vaddr = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_memory_map(mod_vm_memory, vaddr);

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
 * shell_stub_mod_vm_memory_MINI_unmap
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_memory_MINI_unmap(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_memory_t *mod_vm_memory = (mod_vm_memory_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t vaddr = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vaddr");

	vaddr = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_memory_unmap(mod_vm_memory, vaddr);

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
