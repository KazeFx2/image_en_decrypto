cmake_minimum_required(VERSION 3.20)

project(C_Lib VERSION 0.1 LANGUAGES C)

option(C_COMPONENT_C_TEST "build C test" OFF)
if (CoreTest)
    set(C_COMPONENT_C_TEST ON CACHE BOOL "build C test" FORCE)
else ()
    set(C_COMPONENT_C_TEST OFF CACHE BOOL "build C test" FORCE)
endif ()

add_subdirectory(setmem)

add_subdirectory(list)

add_subdirectory(rbtree)

add_subdirectory(c_parser)
