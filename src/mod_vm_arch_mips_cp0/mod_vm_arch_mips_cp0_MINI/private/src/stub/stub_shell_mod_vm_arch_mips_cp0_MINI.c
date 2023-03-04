/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    stub_shell_mod_vm_arch_mips_cp0_MINI.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-03-01
 *
 */

#include "std_common.h"
#include "mod_vm_arch_mips_cp0.h"
#include "tiny-json.h"
#include "json-maker.h"

/**
 * shell_stub_mod_vm_arch_mips_cp0_MINI_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_reset(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	mod_vm_arch_mips_cp0_reset(mod_vm_arch_mips_cp0);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_trigger_exception
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_trigger_exception(IN std_void_t * p_handle,
									    IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t type = 0;

	std_uint_t cause = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "type");

	type = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "cause");

	cause = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_arch_mips_cp0_trigger_exception(mod_vm_arch_mips_cp0, type, cause);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_op(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t op_type = 0;

	std_uint_t insn = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "op_type");

	op_type = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "insn");

	insn = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_arch_mips_cp0_tlb_op(mod_vm_arch_mips_cp0, op_type, insn);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_lookup
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_tlb_lookup(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

	std_uint_t vaddr = 0;

	std_void_t *res = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vaddr");

	vaddr = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "res");

	res = (std_void_t *) json_getUInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cp0_tlb_lookup(mod_vm_arch_mips_cp0, vaddr, res);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_mtc_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_mtc_op(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t cp0_reg = 0;

	std_uint_t val = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "cp0_reg");

	cp0_reg = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "val");

	val = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_arch_mips_cp0_mtc_op(mod_vm_arch_mips_cp0, cp0_reg, val);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_mfc_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_mfc_op(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t ret = 0;

	std_uint_t cp0_reg = 0;

	std_uint_t sel = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "cp0_reg");

	cp0_reg = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "sel");

	sel = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cp0_mfc_op(mod_vm_arch_mips_cp0, cp0_reg, sel);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_access_special
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_access_special(IN std_void_t * p_handle,
									 IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t vaddr = 0;

	std_uint_t mask = 0;

	std_uint_t op_code = 0;

	std_uint_t op_type = 0;

	std_uint_t op_size = 0;

	std_uint_t *data = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "vaddr");

	vaddr = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "mask");

	mask = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "op_code");

	op_code = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "op_type");

	op_type = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "op_size");

	op_size = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "data");

	data = (std_uint_t *) json_getUInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_arch_mips_cp0_access_special(mod_vm_arch_mips_cp0, vaddr, mask, op_code, op_type, op_size, data);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_irq_op
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_irq_op(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

	std_uint_t type = 0;

	std_uint_t irq = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "type");

	type = (std_uint_t) json_getInteger(param_obj);

	param_obj = json_getProperty(json, "irq");

	irq = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cp0_irq_op(mod_vm_arch_mips_cp0, type, irq);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_timer
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_timer(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cp0_timer(mod_vm_arch_mips_cp0);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_eret
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_eret(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cp0_eret(mod_vm_arch_mips_cp0);

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
 * shell_stub_mod_vm_arch_mips_cp0_MINI_soft_fpu
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cp0_MINI_soft_fpu(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cp0_t *mod_vm_arch_mips_cp0 = (mod_vm_arch_mips_cp0_t *) p_handle;
    /****server_stub_args_def****/

	std_int_t ret = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cp0_soft_fpu(mod_vm_arch_mips_cp0);

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
