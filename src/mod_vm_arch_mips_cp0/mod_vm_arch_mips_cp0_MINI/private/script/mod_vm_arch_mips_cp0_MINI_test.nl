def mod_vm_arch_mips_cp0_reset(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_reset", root, json_args)
}


def mod_vm_arch_mips_cp0_trigger_exception(var root, var type, var cause, var ret)
{
    var name_value_hash<> = <"type":type,"cause":cause>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_trigger_exception", root, json_args)
}


def mod_vm_arch_mips_cp0_tlb_op(var root, var op_type, var insn, var ret)
{
    var name_value_hash<> = <"op_type":op_type,"insn":insn>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_tlb_op", root, json_args)
}


def mod_vm_arch_mips_cp0_tlb_lookup(var root, var vaddr, var res, var ret)
{
    var name_value_hash<> = <"vaddr":vaddr,"res":res>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_tlb_lookup", root, json_args)
}


def mod_vm_arch_mips_cp0_mtc_op(var root, var cp0_reg, var val, var ret)
{
    var name_value_hash<> = <"cp0_reg":cp0_reg,"val":val>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_mtc_op", root, json_args)
}


def mod_vm_arch_mips_cp0_mfc_op(var root, var cp0_reg, var sel, var ret)
{
    var name_value_hash<> = <"cp0_reg":cp0_reg,"sel":sel>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_mfc_op", root, json_args)
}


def mod_vm_arch_mips_cp0_access_special(var root, var vaddr, var mask, var op_code, var op_type, var op_size, var data, var ret)
{
    var name_value_hash<> = <"vaddr":vaddr,"mask":mask,"op_code":op_code,"op_type":op_type,"op_size":op_size,"data":data>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_access_special", root, json_args)
}


def mod_vm_arch_mips_cp0_irq_op(var root, var type, var irq, var ret)
{
    var name_value_hash<> = <"type":type,"irq":irq>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_irq_op", root, json_args)
}


def mod_vm_arch_mips_cp0_timer(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_timer", root, json_args)
}


def mod_vm_arch_mips_cp0_eret(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_eret", root, json_args)
}


def mod_vm_arch_mips_cp0_soft_fpu(var root, var ret)
{
    var name_value_hash<> = <>
    var json_args

    json_args = make_json(name_value_hash)

    ret = run("mod_vm_arch_mips_cp0_soft_fpu", root, json_args)
}


def run_reset(var root, var run_state)
{
    var ret_reset
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_reset(root,ret_reset)
}

def run_trigger_exception(var root, var run_state)
{
    var type
    var cause
    var ret_trigger_exception
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_trigger_exception(root,type,cause,ret_trigger_exception)
}

def run_tlb_op(var root, var run_state)
{
    var op_type
    var insn
    var ret_tlb_op
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_tlb_op(root,op_type,insn,ret_tlb_op)
}

def run_tlb_lookup(var root, var run_state)
{
    var vaddr
    var res
    var ret_tlb_lookup
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_tlb_lookup(root,vaddr,res,ret_tlb_lookup)
}

def run_mtc_op(var root, var run_state)
{
    var cp0_reg
    var val
    var ret_mtc_op
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_mtc_op(root,cp0_reg,val,ret_mtc_op)
}

def run_mfc_op(var root, var run_state)
{
    var cp0_reg
    var sel
    var ret_mfc_op
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_mfc_op(root,cp0_reg,sel,ret_mfc_op)
}

def run_access_special(var root, var run_state)
{
    var vaddr
    var mask
    var op_code
    var op_type
    var op_size
    var data
    var ret_access_special
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_access_special(root,vaddr,mask,op_code,op_type,op_size,data,ret_access_special)
}

def run_irq_op(var root, var run_state)
{
    var type
    var irq
    var ret_irq_op
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_irq_op(root,type,irq,ret_irq_op)
}

def run_timer(var root, var run_state)
{
    var ret_timer
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_timer(root,ret_timer)
}

def run_eret(var root, var run_state)
{
    var ret_eret
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_eret(root,ret_eret)
}

def run_soft_fpu(var root, var run_state)
{
    var ret_soft_fpu
    var keys_tuple = run_state.find("keys_tuple")
    var hash_key_value = run_state.find("hash_key_value")

    mod_vm_arch_mips_cp0_soft_fpu(root,ret_soft_fpu)
}


def main()
{
    var mod_vm_arch_mips_cp0_test
    var iid = "0x6c3e2338, 0x725f, 0x5e49, 0x69, 0x36, 0x94, 0x05, 0x6e, 0xe7, 0x51, 0xf1"
    var args = "{}"
    var keys_tuple{} = {}
    var hash_key_value<> = <>
	var run_state<> = <"keys_tuple": keys_tuple, "hash_key_value":hash_key_value>
    var i
    var run_max = 1000

    debug("ERR")

    mod_vm_arch_mips_cp0_test = create_instance(iid, args)
    print("mod_vm_arch_mips_cp0_test:", mod_vm_arch_mips_cp0_test)

    run_reset(mod_vm_arch_mips_cp0_test, run_state)

    run_trigger_exception(mod_vm_arch_mips_cp0_test, run_state)

    run_tlb_op(mod_vm_arch_mips_cp0_test, run_state)

    run_tlb_lookup(mod_vm_arch_mips_cp0_test, run_state)

    run_mtc_op(mod_vm_arch_mips_cp0_test, run_state)

    run_mfc_op(mod_vm_arch_mips_cp0_test, run_state)

    run_access_special(mod_vm_arch_mips_cp0_test, run_state)

    run_irq_op(mod_vm_arch_mips_cp0_test, run_state)

    run_timer(mod_vm_arch_mips_cp0_test, run_state)

    run_eret(mod_vm_arch_mips_cp0_test, run_state)

    run_soft_fpu(mod_vm_arch_mips_cp0_test, run_state)


    for (i = 0, i < run_max, i += 1) {
        var k
        var run_random

        run_random = random_number(32)
        k = run_random % 11

        if ( k == 0) {
            run_reset(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 1) {
            run_trigger_exception(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 2) {
            run_tlb_op(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 3) {
            run_tlb_lookup(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 4) {
            run_mtc_op(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 5) {
            run_mfc_op(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 6) {
            run_access_special(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 7) {
            run_irq_op(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 8) {
            run_timer(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 9) {
            run_eret(mod_vm_arch_mips_cp0_test, run_state)
        }

        if ( k == 10) {
            run_soft_fpu(mod_vm_arch_mips_cp0_test, run_state)
        }

    }

    delete_instance(iid, mod_vm_arch_mips_cp0_test)
}
