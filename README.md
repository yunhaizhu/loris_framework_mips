## MIPS virtual machine 
MIPS virtual machine (JIT) build with loris_framework.

## link file into loris_framework
1. git submodule update --init --recursive
2. sh init.sh

## loris_framework configure 
Read loris_framework/README.md to install necessary depended files. 

## compile and run
1. cmake -S . -B ./build-debug
2. cmake --build build-debug
3. cmake --install build-debug
4. cd loris_framework/deploy/Debug
5. ./loris_framework
6. type script("init_adm5120.nl") in loris_framework's console. 
7. type script("mod_vm_ADM5120_test.nl") in loris_framework's console.
8. enjoy the JIT MIPS 

## COPYRIGHT
GNU General Public License, version 3


