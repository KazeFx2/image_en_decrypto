cmake_minimum_required(VERSION 3.20)

project(CList VERSION 0.1 LANGUAGES C)

add_library(${PROJECT_NAME} c_list.c)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        CSetmem)

if (C_COMPONENT_C_TEST)
    add_subdirectory(test)
endif ()