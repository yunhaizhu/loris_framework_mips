def mod_vm_device_initiate(var root, var arg, var ret)
{
    var name_value_hash<> = <"arg":arg>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_initiate", root, json_args)
}


def mod_vm_device_reset(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_reset", root, json_args)
}


def mod_vm_device_access(var root, var arg, var ret)
{
    var name_value_hash<> = <"arg":arg>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_access", root, json_args)
}


def mod_vm_device_command(var root, var arg, var ret)
{
    var name_value_hash<> = <"arg":arg>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_command", root, json_args)
}


def run_initiate(var root, var run_state)
{
    var arg
    var ret_initiate
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_initiate(root,arg,ret_initiate)
}

def run_reset(var root, var run_state)
{
    var ret_reset
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_reset(root,ret_reset)
}

def run_access(var root, var run_state)
{
    var arg
    var ret_access
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_access(root,arg,ret_access)
}

def run_command(var root, var run_state)
{
    var arg
    var ret_command
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_command(root,arg,ret_command)
}


def main()
{
    var mod_vm_device_test
    var iid = "0xaeee373f, 0xf721, 0xf79d, 0x0a, 0xa3, 0xd3, 0x5f, 0xdc, 0xcd, 0xd9, 0x10"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_device_test = create_instance(iid, args)
    print("mod_vm_device_test:", mod_vm_device_test)

    run_initiate(mod_vm_device_test, run_state)

    run_reset(mod_vm_device_test, run_state)

    run_access(mod_vm_device_test, run_state)

    run_command(mod_vm_device_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 4

        if ( k == 0) {
            run_initiate(mod_vm_device_test, run_state)
        }

        if ( k == 1) {
            run_reset(mod_vm_device_test, run_state)
        }

        if ( k == 2) {
            run_access(mod_vm_device_test, run_state)
        }

        if ( k == 3) {
            run_command(mod_vm_device_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_device_test)
}
