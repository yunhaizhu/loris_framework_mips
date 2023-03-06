## MIPS virtual machine 
MIPS virtual machine (JIT) build with loris_framework.

## link file into loris_framework
sh init.sh

## compile and run
1. cmake --build cmake-build-debug
2. cmake --install cmake-build-debug
3. cd loris_framework/deploy/Debug
4. ./loris_framework
5. type script("init_adm5120.nl") in loris_framework's console. 
6. type script("mod_vm_ADM5120_test.nl") in loris_framework's console.
7. enjoy the JIT MIPS 

## COPYRIGHT
GNU General Public License, version 3


