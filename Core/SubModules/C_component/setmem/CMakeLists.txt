cmake_minimum_required(VERSION 3.20)

project(CSetmem VERSION 0.1 LANGUAGES C)

add_library(${PROJECT_NAME} setmem.c)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)
