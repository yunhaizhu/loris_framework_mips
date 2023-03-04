add_library(mod_vm_MINI SHARED
        private/src/mod_vm_MINI.c
        private/src/mod_vm_MINI_bundle.c
        private/src/stub/stub_shell_mod_vm_MINI.c
        )

target_include_directories(mod_vm_MINI PRIVATE
        ${SUB_PUBLIC_INCLUDE_DIRS}
        private/include)

target_link_libraries(mod_vm_MINI PRIVATE elf rt pthread m ${LINK_PROFILER})

INSTALL(FILES private/script/mod_vm_MINI_test.nl
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/script/ )

INSTALL(FILES public/desc.json
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_MINI/ )

INSTALL(TARGETS mod_vm_MINI
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_MINI/ )
