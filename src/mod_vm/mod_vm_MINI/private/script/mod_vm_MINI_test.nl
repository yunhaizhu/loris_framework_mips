load_lib "shell_lib"

def mod_vm_initiate(var root, var conf_name, var ret)
{
    var name_value_hash<> = <"conf_name":conf_name>
    var json_args

    make_json(name_value_hash, json_args)
    print("json_args:", json_args)
    run("mod_vm_initiate", root, json_args, ret)
}


def mod_vm_start(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    make_json(name_value_hash, json_args)

    run("mod_vm_start", root, json_args, ret)
}


def mod_vm_stop(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    make_json(name_value_hash, json_args)

    run("mod_vm_stop", root, json_args, ret)
}


def mod_vm_suspend(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    make_json(name_value_hash, json_args)

    run("mod_vm_suspend", root, json_args, ret)
}


def mod_vm_resume(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    make_json(name_value_hash, json_args)

    run("mod_vm_resume", root, json_args, ret)
}


def run_initiate(var root, var run_state)
{
    var conf_name = "adm5120.conf"
    var ret_initiate
    var keys_tuple = run_state.find_item("keys_tuple")
    var hash_key_value = run_state.find_item("hash_key_value")

    mod_vm_initiate(root,conf_name,ret_initiate)
}

def run_start(var root, var run_state)
{
    var ret_start
    var keys_tuple = run_state.find_item("keys_tuple")
    var hash_key_value = run_state.find_item("hash_key_value")

    mod_vm_start(root,ret_start)
}

def run_stop(var root, var run_state)
{
    var ret_stop
    var keys_tuple = run_state.find_item("keys_tuple")
    var hash_key_value = run_state.find_item("hash_key_value")

    mod_vm_stop(root,ret_stop)
}

def run_suspend(var root, var run_state)
{
    var ret_suspend
    var keys_tuple = run_state.find_item("keys_tuple")
    var hash_key_value = run_state.find_item("hash_key_value")

    mod_vm_suspend(root,ret_suspend)
}

def run_resume(var root, var run_state)
{
    var ret_resume
    var keys_tuple = run_state.find_item("keys_tuple")
    var hash_key_value = run_state.find_item("hash_key_value")

    mod_vm_resume(root,ret_resume)
}

def main()
{
    var mod_vm_test
    var iid = "0x6adc222a, 0x1fd0, 0x0a72, 0xd4, 0xa2, 0x50, 0x25, 0x18, 0x1b, 0x6e, 0x7f"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    create_instance(iid, args, mod_vm_test)
    print("mod_vm_test:", mod_vm_test)

    run_initiate(mod_vm_test, run_state)

    run_start(mod_vm_test, run_state)

    delete_instance(iid, mod_vm_test)
}
