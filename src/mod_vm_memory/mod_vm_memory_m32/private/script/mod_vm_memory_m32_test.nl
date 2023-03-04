def mod_vm_memory_initiate(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_memory_initiate", root, json_args)
}


def mod_vm_memory_lookup(var root, var vaddr, var ret)
{
    var name_value_hash<> = <"vaddr":vaddr>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_memory_lookup", root, json_args)
}


def mod_vm_memory_access(var root, var vaddr, var op_code, var op_size, var op_type, var data, var exc, var has_set_value, var is_fromgdb, var ret)
{
    var name_value_hash<> = <"vaddr":vaddr,"op_code":op_code,"op_size":op_size,"op_type":op_type,"data":data,"exc":exc,"has_set_value":has_set_value,"is_fromgdb":is_fromgdb>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_memory_access", root, json_args)
}


def mod_vm_memory_map(var root, var vaddr, var ret)
{
    var name_value_hash<> = <"vaddr":vaddr>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_memory_map", root, json_args)
}


def mod_vm_memory_unmap(var root, var vaddr, var ret)
{
    var name_value_hash<> = <"vaddr":vaddr>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_memory_unmap", root, json_args)
}


def run_initiate(var root, var run_state)
{
    var ret_initiate
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_memory_initiate(root,ret_initiate)
}

def run_lookup(var root, var run_state)
{
    var vaddr
    var ret_lookup
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_memory_lookup(root,vaddr,ret_lookup)
}

def run_access(var root, var run_state)
{
    var vaddr
    var op_code
    var op_size
    var op_type
    var data
    var exc
    var has_set_value
    var is_fromgdb
    var ret_access
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_memory_access(root,vaddr,op_code,op_size,op_type,data,exc,has_set_value,is_fromgdb,ret_access)
}

def run_map(var root, var run_state)
{
    var vaddr
    var ret_map
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_memory_map(root,vaddr,ret_map)
}

def run_unmap(var root, var run_state)
{
    var vaddr
    var ret_unmap
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_memory_unmap(root,vaddr,ret_unmap)
}


def main()
{
    var mod_vm_memory_test
    var iid = "0x09b130a1, 0x40eb, 0x2e02, 0x99, 0x64, 0x55, 0xfd, 0xfb, 0x41, 0x35, 0x9b"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_memory_test = create_instance(iid, args)
    print("mod_vm_memory_test:", mod_vm_memory_test)

    run_initiate(mod_vm_memory_test, run_state)

    run_lookup(mod_vm_memory_test, run_state)

    run_access(mod_vm_memory_test, run_state)

    run_map(mod_vm_memory_test, run_state)

    run_unmap(mod_vm_memory_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 5

        if ( k == 0) {
            run_initiate(mod_vm_memory_test, run_state)
        }

        if ( k == 1) {
            run_lookup(mod_vm_memory_test, run_state)
        }

        if ( k == 2) {
            run_access(mod_vm_memory_test, run_state)
        }

        if ( k == 3) {
            run_map(mod_vm_memory_test, run_state)
        }

        if ( k == 4) {
            run_unmap(mod_vm_memory_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_memory_test)
}
