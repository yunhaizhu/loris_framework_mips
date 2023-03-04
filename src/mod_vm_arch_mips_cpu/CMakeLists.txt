MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(instance_dirlist "")
    FOREACH (child ${children})
        IF (IS_DIRECTORY ${curdir}/${child})
            IF (NOT ${child} STREQUAL "public")
                LIST(APPEND instance_dirlist ${child})
            ENDIF ()
        ENDIF ()
    ENDFOREACH ()
    SET(${result} ${instance_dirlist})
ENDMACRO()

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "SUBDIRS: ${SUBDIRS}")

FOREACH (subdir ${SUBDIRS})
    ADD_SUBDIRECTORY(${subdir})
ENDFOREACH ()

