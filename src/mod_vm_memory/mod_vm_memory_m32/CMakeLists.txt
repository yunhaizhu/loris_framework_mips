add_library(mod_vm_memory_m32 SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mod_vm_memory_m32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mod_vm_memory_m32_bundle.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/stub/stub_shell_mod_vm_memory_m32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/cvm_memory_32.c
        )

target_include_directories(mod_vm_memory_m32 PRIVATE
        ${SUB_PUBLIC_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/private/include)

target_link_libraries(mod_vm_memory_m32 PRIVATE pthread ${LINK_PROFILER})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/private/script/mod_vm_memory_m32_test.nl
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/script/)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/public/desc.json
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_memory_m32/)

INSTALL(TARGETS mod_vm_memory_m32
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_memory_m32/)
