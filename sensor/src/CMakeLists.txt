FILE(GLOB_RECURSE app_sources
    ${CMAKE_SOURCE_DIR}/src/*.*
    ${CMAKE_SOURCE_DIR}/../common/ble/*.c
    ${CMAKE_SOURCE_DIR}/../common/led/*.c
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/../common/ble
        ${CMAKE_SOURCE_DIR}/../common/led
)