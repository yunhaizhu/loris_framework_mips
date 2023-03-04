def mod_vm_device_vtty_initiate(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_initiate", root, json_args)
}


def mod_vm_device_vtty_create(var root, var name, var type, var tcp_port, var option, var rnf, var ret)
{
    var name_value_hash<> = <"name":name,"type":type,"tcp_port":tcp_port,"option":option,"rnf":rnf>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_create", root, json_args)
}


def mod_vm_device_vtty_get_char(var root, var vtty, var ret)
{
    var name_value_hash<> = <"vtty":vtty>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_get_char", root, json_args)
}


def mod_vm_device_vtty_put_char(var root, var vtty, var ch, var ret)
{
    var name_value_hash<> = <"vtty":vtty,"ch":ch>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_put_char", root, json_args)
}


def mod_vm_device_vtty_put_buffer(var root, var vtty, var buf, var len, var ret)
{
    var name_value_hash<> = <"vtty":vtty,"buf":buf,"len":len>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_put_buffer", root, json_args)
}


def mod_vm_device_vtty_is_char_avail(var root, var vtty, var ret)
{
    var name_value_hash<> = <"vtty":vtty>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_is_char_avail", root, json_args)
}


def mod_vm_device_vtty_is_full(var root, var vtty, var ret)
{
    var name_value_hash<> = <"vtty":vtty>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_device_vtty_is_full", root, json_args)
}


def run_initiate(var root, var run_state)
{
    var ret_initiate
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_initiate(root,ret_initiate)
}

def run_create(var root, var run_state)
{
    var name
    var type
    var tcp_port
    var option
    var rnf
    var ret_create
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_create(root,name,type,tcp_port,option,rnf,ret_create)
}

def run_get_char(var root, var run_state)
{
    var vtty
    var ret_get_char
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_get_char(root,vtty,ret_get_char)
}

def run_put_char(var root, var run_state)
{
    var vtty
    var ch
    var ret_put_char
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_put_char(root,vtty,ch,ret_put_char)
}

def run_put_buffer(var root, var run_state)
{
    var vtty
    var buf
    var len
    var ret_put_buffer
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_put_buffer(root,vtty,buf,len,ret_put_buffer)
}

def run_is_char_avail(var root, var run_state)
{
    var vtty
    var ret_is_char_avail
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_is_char_avail(root,vtty,ret_is_char_avail)
}

def run_is_full(var root, var run_state)
{
    var vtty
    var ret_is_full
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_device_vtty_is_full(root,vtty,ret_is_full)
}


def main()
{
    var mod_vm_device_vtty_test
    var iid = "0x47968a94, 0x9df9, 0x89e4, 0xf4, 0x56, 0xff, 0x4c, 0xcc, 0x38, 0xa2, 0xc0"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_device_vtty_test = create_instance(iid, args)
    print("mod_vm_device_vtty_test:", mod_vm_device_vtty_test)

    run_initiate(mod_vm_device_vtty_test, run_state)

    run_create(mod_vm_device_vtty_test, run_state)

    run_get_char(mod_vm_device_vtty_test, run_state)

    run_put_char(mod_vm_device_vtty_test, run_state)

    run_put_buffer(mod_vm_device_vtty_test, run_state)

    run_is_char_avail(mod_vm_device_vtty_test, run_state)

    run_is_full(mod_vm_device_vtty_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 7

        if ( k == 0) {
            run_initiate(mod_vm_device_vtty_test, run_state)
        }

        if ( k == 1) {
            run_create(mod_vm_device_vtty_test, run_state)
        }

        if ( k == 2) {
            run_get_char(mod_vm_device_vtty_test, run_state)
        }

        if ( k == 3) {
            run_put_char(mod_vm_device_vtty_test, run_state)
        }

        if ( k == 4) {
            run_put_buffer(mod_vm_device_vtty_test, run_state)
        }

        if ( k == 5) {
            run_is_char_avail(mod_vm_device_vtty_test, run_state)
        }

        if ( k == 6) {
            run_is_full(mod_vm_device_vtty_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_device_vtty_test)
}
