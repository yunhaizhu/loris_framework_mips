ln -s ../../files/vmlinux loris_framework/files/vmlinux

mkdir -p loris_framework/src/mod_vm
mkdir -p loris_framework/src/mod_vm_arch
mkdir -p loris_framework/src/mod_vm_arch_mips_cp0
mkdir -p loris_framework/src/mod_vm_arch_mips_cpu
mkdir -p loris_framework/src/mod_vm_device
mkdir -p loris_framework/src/mod_vm_device_manager
mkdir -p loris_framework/src/mod_vm_device_vtty
mkdir -p loris_framework/src/mod_vm_memory

sudo mount --bind src/mod_vm/ loris_framework/src/mod_vm
sudo mount --bind src/mod_vm_arch loris_framework/src/mod_vm_arch
sudo mount --bind src/mod_vm_arch_mips_cp0 loris_framework/src/mod_vm_arch_mips_cp0
sudo mount --bind src/mod_vm_arch_mips_cpu loris_framework/src/mod_vm_arch_mips_cpu
sudo mount --bind src/mod_vm_device loris_framework/src/mod_vm_device
sudo mount --bind src/mod_vm_device_manager loris_framework/src/mod_vm_device_manager
sudo mount --bind src/mod_vm_device_vtty loris_framework/src/mod_vm_device_vtty
sudo mount --bind src/mod_vm_memory loris_framework/src/mod_vm_memory

