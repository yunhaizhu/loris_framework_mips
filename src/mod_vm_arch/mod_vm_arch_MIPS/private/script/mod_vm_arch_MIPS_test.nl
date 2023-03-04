def mod_vm_arch_initiate(var root, var entry, var ret)
{
    var name_value_hash<> = <"entry":entry>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_initiate", root, json_args)
}


def mod_vm_arch_start(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_start", root, json_args)
}


def mod_vm_arch_stop(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_stop", root, json_args)
}


def mod_vm_arch_suspend(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_suspend", root, json_args)
}


def mod_vm_arch_resume(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_resume", root, json_args)
}


def run_initiate(var root, var run_state)
{
    var entry
    var ret_initiate
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_initiate(root,entry,ret_initiate)
}

def run_start(var root, var run_state)
{
    var ret_start
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_start(root,ret_start)
}

def run_stop(var root, var run_state)
{
    var ret_stop
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_stop(root,ret_stop)
}

def run_suspend(var root, var run_state)
{
    var ret_suspend
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_suspend(root,ret_suspend)
}

def run_resume(var root, var run_state)
{
    var ret_resume
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_resume(root,ret_resume)
}


def main()
{
    var mod_vm_arch_test
    var iid = "0x1828c092, 0xceeb, 0x9713, 0x22, 0x25, 0xe8, 0xf0, 0xbf, 0xca, 0x1c, 0x6c"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_arch_test = create_instance(iid, args)
    print("mod_vm_arch_test:", mod_vm_arch_test)

    run_initiate(mod_vm_arch_test, run_state)

    run_start(mod_vm_arch_test, run_state)

    run_stop(mod_vm_arch_test, run_state)

    run_suspend(mod_vm_arch_test, run_state)

    run_resume(mod_vm_arch_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 5

        if ( k == 0) {
            run_initiate(mod_vm_arch_test, run_state)
        }

        if ( k == 1) {
            run_start(mod_vm_arch_test, run_state)
        }

        if ( k == 2) {
            run_stop(mod_vm_arch_test, run_state)
        }

        if ( k == 3) {
            run_suspend(mod_vm_arch_test, run_state)
        }

        if ( k == 4) {
            run_resume(mod_vm_arch_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_arch_test)
}
