def mod_vm_device_manager_add(var root, var p_dev, var base, var len, var ret)
{
    var name_value_hash<> = <"p_dev":p_dev,"base":base,"len":len>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_manager_add", root, json_args)
}


def mod_vm_device_manager_del(var root, var p_dev, var ret)
{
    var name_value_hash<> = <"p_dev":p_dev>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_manager_del", root, json_args)
}


def mod_vm_device_manager_find(var root, var addr, var pp_dev, var ret)
{
    var name_value_hash<> = <"addr":addr,"pp_dev":pp_dev>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_manager_find", root, json_args)
}


def run_add(var root, var run_state)
{
    var p_dev
    var base
    var len
    var ret_add
    var keys_tuple
    var keys_count
    var key_add = random_number(64)

    keys_tuple = run_state.find("keys_tuple")

    p_dev = key_add
    base = key_add
    len = key_add

    mod_vm_device_manager_add(root,p_dev,base,len,ret_add)
    assert(ret_add == 0, "ret_add == STD_RV_SUC")

    keys_tuple.add(key_add)
}

def run_del(var root, var run_state)
{
    var p_dev
    var ret_del
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_manager_del(root,p_dev,ret_del)
}

def run_find(var root, var run_state)
{
    var addr
    var pp_dev
    var ret_find
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_manager_find(root,addr,pp_dev,ret_find)
}


def main()
{
    var mod_vm_device_manager_test
    var iid = "0x080129f1, 0x0dc5, 0xb86b, 0xe1, 0x44, 0xc1, 0x76, 0xcc, 0x5b, 0x70, 0x06"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_device_manager_test = create_instance(iid, args)
    print("mod_vm_device_manager_test:", mod_vm_device_manager_test)

    run_add(mod_vm_device_manager_test, run_state)

    run_del(mod_vm_device_manager_test, run_state)

    run_find(mod_vm_device_manager_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 2

        if ( k == 0) {
            run_add(mod_vm_device_manager_test, run_state)
        }

        if ( k == 1) {
            run_del(mod_vm_device_manager_test, run_state)
        }

        if ( k == 2) {
            run_find(mod_vm_device_manager_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_device_manager_test)
}
