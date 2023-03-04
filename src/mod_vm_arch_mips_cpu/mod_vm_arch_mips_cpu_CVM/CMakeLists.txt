add_library(mod_vm_arch_mips_cpu_CVM SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mod_vm_arch_mips_cpu_CVM.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mod_vm_arch_mips_cpu_CVM_bundle.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/stub/stub_shell_mod_vm_arch_mips_cpu_CVM.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/cvm_arch_mips_cpu.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mips64_jit.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/sbox.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/vp_timer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/vp_clock.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/x86_trans.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/host_alarm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/dump_insn.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/cpu_mts.c)

target_include_directories(mod_vm_arch_mips_cpu_CVM PRIVATE
        ${SUB_PUBLIC_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/private/include)

target_link_libraries(mod_vm_arch_mips_cpu_CVM PRIVATE pthread ${LINK_PROFILER} )

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/private/script/mod_vm_arch_mips_cpu_CVM_test.nl
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/script/)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/public/desc.json
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_arch_mips_cpu_CVM/)

INSTALL(TARGETS mod_vm_arch_mips_cpu_CVM
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_arch_mips_cpu_CVM/)
