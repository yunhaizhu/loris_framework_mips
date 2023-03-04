add_library(mod_vm_memory_MINI SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mod_vm_memory_MINI.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/mod_vm_memory_MINI_bundle.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/stub/stub_shell_mod_vm_memory_MINI.c
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/cvm_memory_32.c
        )

target_include_directories(mod_vm_memory_MINI PRIVATE
        ${SUB_PUBLIC_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/private/include)

target_link_libraries(mod_vm_memory_MINI PRIVATE pthread ${LINK_PROFILER})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/private/script/mod_vm_memory_MINI_test.nl
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/script/ )

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/public/desc.json
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_memory_MINI/ )

INSTALL(TARGETS mod_vm_memory_MINI
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_memory_MINI/ )
