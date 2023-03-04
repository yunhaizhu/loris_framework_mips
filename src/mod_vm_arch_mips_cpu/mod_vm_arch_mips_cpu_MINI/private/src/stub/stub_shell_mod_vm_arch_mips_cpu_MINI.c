/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    stub_shell_mod_vm_arch_mips_cpu_MINI.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-03-01
 *
 */

#include "std_common.h"
#include "mod_vm_arch_mips_cpu.h"
#include "tiny-json.h"
#include "json-maker.h"

/**
 * shell_stub_mod_vm_arch_mips_cpu_MINI_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_reset(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t entry = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "entry");

	entry = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_arch_mips_cpu_reset(mod_vm_arch_mips_cpu, entry);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_fetch
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_fetch(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t ret = 0;

	std_uint_t *insn = NULL;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "insn");

	insn = (std_uint_t *) json_getUInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cpu_fetch(mod_vm_arch_mips_cpu, insn);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_exec
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_exec(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t insn = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "insn");

	insn = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	mod_vm_arch_mips_cpu_exec(mod_vm_arch_mips_cpu, insn);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_set_pc
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_set_pc(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t ret = 0;

	std_uint_t pc = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "pc");

	pc = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cpu_set_pc(mod_vm_arch_mips_cpu, pc);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_set_llbit
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_set_llbit(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t ret = 0;

	std_uint_t llbit = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

	json_t const *param_obj = NULL;
	param_obj = json_getProperty(json, "llbit");

	llbit = (std_uint_t) json_getInteger(param_obj);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cpu_set_llbit(mod_vm_arch_mips_cpu, llbit);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_is_in_bdslot
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_is_in_bdslot(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

	std_uint_t ret = 0;

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	ret = mod_vm_arch_mips_cpu_is_in_bdslot(mod_vm_arch_mips_cpu);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_break_idle
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_break_idle(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	mod_vm_arch_mips_cpu_break_idle(mod_vm_arch_mips_cpu);

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
 * shell_stub_mod_vm_arch_mips_cpu_MINI_run
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  STD_CALL std_char_t *
 */
STD_CALL std_char_t *shell_stub_mod_vm_arch_mips_cpu_MINI_run(IN std_void_t * p_handle, IN std_char_t * params)
{
	json_t const *json;
	std_char_t response_string[128];
	std_char_t *response;
	std_char_t *dest;
	mod_vm_arch_mips_cpu_t *mod_vm_arch_mips_cpu = (mod_vm_arch_mips_cpu_t *) p_handle;
    /****server_stub_args_def****/

    /*****server_stub_args_body*****/
	json_t mem[32];
	json = json_create(params, mem, sizeof mem / sizeof *mem);
	STD_ASSERT_RV(json != NULL, NULL);

    /******server_stub_call*****/
	mod_vm_arch_mips_cpu_run(mod_vm_arch_mips_cpu);

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
