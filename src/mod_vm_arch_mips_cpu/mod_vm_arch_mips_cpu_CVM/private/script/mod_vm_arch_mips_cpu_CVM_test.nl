def mod_vm_arch_mips_cpu_reset(var root, var entry, var ret)
{
    var name_value_hash<> = <"entry":entry>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_reset", root, json_args)
}


def mod_vm_arch_mips_cpu_fetch(var root, var insn, var ret)
{
    var name_value_hash<> = <"insn":insn>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_fetch", root, json_args)
}


def mod_vm_arch_mips_cpu_exec(var root, var insn, var ret)
{
    var name_value_hash<> = <"insn":insn>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_exec", root, json_args)
}


def mod_vm_arch_mips_cpu_set_pc(var root, var pc, var ret)
{
    var name_value_hash<> = <"pc":pc>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_set_pc", root, json_args)
}


def mod_vm_arch_mips_cpu_set_llbit(var root, var llbit, var ret)
{
    var name_value_hash<> = <"llbit":llbit>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_set_llbit", root, json_args)
}


def mod_vm_arch_mips_cpu_is_in_bdslot(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_is_in_bdslot", root, json_args)
}


def mod_vm_arch_mips_cpu_break_idle(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cpu_break_idle", root, json_args)
}


def run_reset(var root, var run_state)
{
    var entry
    var ret_reset
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_reset(root,entry,ret_reset)
}

def run_fetch(var root, var run_state)
{
    var insn
    var ret_fetch
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_fetch(root,insn,ret_fetch)
}

def run_exec(var root, var run_state)
{
    var insn
    var ret_exec
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_exec(root,insn,ret_exec)
}

def run_set_pc(var root, var run_state)
{
    var pc
    var ret_set_pc
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_set_pc(root,pc,ret_set_pc)
}

def run_set_llbit(var root, var run_state)
{
    var llbit
    var ret_set_llbit
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_set_llbit(root,llbit,ret_set_llbit)
}

def run_is_in_bdslot(var root, var run_state)
{
    var ret_is_in_bdslot
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_is_in_bdslot(root,ret_is_in_bdslot)
}

def run_break_idle(var root, var run_state)
{
    var ret_break_idle
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cpu_break_idle(root,ret_break_idle)
}


def main()
{
    var mod_vm_arch_mips_cpu_test
    var iid = "0xbaee1ebe, 0x1bb0, 0x85f4, 0xa6, 0xb8, 0xac, 0x02, 0x47, 0x56, 0x72, 0xb6"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_arch_mips_cpu_test = create_instance(iid, args)
    print("mod_vm_arch_mips_cpu_test:", mod_vm_arch_mips_cpu_test)

    run_reset(mod_vm_arch_mips_cpu_test, run_state)

    run_fetch(mod_vm_arch_mips_cpu_test, run_state)

    run_exec(mod_vm_arch_mips_cpu_test, run_state)

    run_set_pc(mod_vm_arch_mips_cpu_test, run_state)

    run_set_llbit(mod_vm_arch_mips_cpu_test, run_state)

    run_is_in_bdslot(mod_vm_arch_mips_cpu_test, run_state)

    run_break_idle(mod_vm_arch_mips_cpu_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 7

        if ( k == 0) {
            run_reset(mod_vm_arch_mips_cpu_test, run_state)
        }

        if ( k == 1) {
            run_fetch(mod_vm_arch_mips_cpu_test, run_state)
        }

        if ( k == 2) {
            run_exec(mod_vm_arch_mips_cpu_test, run_state)
        }

        if ( k == 3) {
            run_set_pc(mod_vm_arch_mips_cpu_test, run_state)
        }

        if ( k == 4) {
            run_set_llbit(mod_vm_arch_mips_cpu_test, run_state)
        }

        if ( k == 5) {
            run_is_in_bdslot(mod_vm_arch_mips_cpu_test, run_state)
        }

        if ( k == 6) {
            run_break_idle(mod_vm_arch_mips_cpu_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_arch_mips_cpu_test)
}
