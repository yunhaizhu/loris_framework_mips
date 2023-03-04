add_library(mod_vm_ADM5120 SHARED
        private/src/mod_vm_ADM5120.c
        private/src/mod_vm_ADM5120_bundle.c
        private/src/stub/stub_shell_mod_vm_ADM5120.c
        )

target_include_directories(mod_vm_ADM5120 PUBLIC
        ${SUB_PUBLIC_INCLUDE_DIRS}
        private/include)

target_link_libraries(mod_vm_ADM5120 PUBLIC elf rt pthread confuse m ${LINK_PROFILER})

INSTALL(FILES private/conf/adm5120.conf
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/)

INSTALL(DIRECTORY private/script/
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/script/)

INSTALL(FILES public/desc.json
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_ADM5120/)

INSTALL(TARGETS mod_vm_ADM5120
        DESTINATION ${${PROJECT_CUSTOM_NAME}_SOURCE_DIR}/deploy/${CMAKE_BUILD_TYPE}/bundle/mod_vm_ADM5120/)
