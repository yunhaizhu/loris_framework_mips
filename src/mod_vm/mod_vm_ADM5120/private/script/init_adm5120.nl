load_lib "shell_lib"

def init()
{
    debug("ERR")

    install("mod_vm_ADM5120")
    start(5)

    install("mod_vm_arch_MIPS")
    start(6)

    install("mod_vm_arch_mips_cp0_CVM")
    start(7)

    install("mod_vm_arch_mips_cpu_CVM")
    start(8)

    install("mod_vm_device_manager_CVM")
    start(9)

    install("mod_vm_memory_m32")
    start(10)

    install("mod_vm_device_vtty_CVM")
    start(11)

    install("mod_vm_device_ETH_CS8900")
    start(12)

    install("mod_vm_device_GPIO")
    start(13)

    install("mod_vm_device_INT")
    start(14)

    install("mod_vm_device_MPMC")
    start(15)

    install("mod_vm_device_NORFLASH4M")
    start(16)

    install("mod_vm_device_PCI")
    start(17)

    install("mod_vm_device_RAM")
    start(18)

    install("mod_vm_device_SW")
    start(19)

    install("mod_vm_device_UART")
    start(20)

    ps()
}

def main()
{
    init()
}
