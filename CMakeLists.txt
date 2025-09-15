# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ./components/bsp_extra
    )

project(p4minitach C CXX)

### This went into main/ before, moved it here so I could have idf_component.yml in root instead of main/
idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
if (LVGL_LIB)
    target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE)
endif()
###

add_compile_options(-Wno-format)
