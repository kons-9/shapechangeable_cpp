# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# if target is esp32c3, use esp32c3/
if (CONFIG_IDF_TARGET STREQUAL "esp32c3")
    FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/esp32c3/*.*)
else()
#native
    FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/native/*.*)
endif()

idf_component_register(SRCS ${app_sources})
