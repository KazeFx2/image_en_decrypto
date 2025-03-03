cmake_minimum_required(VERSION 3.20)

project(ImageEn_DecryptoCore)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

#include(CheckLanguage)
#check_language(CUDA)
#if(CMAKE_CUDA_COMPILER)
#    enable_language(CUDA)
#
#    # set cuda arch
#    set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75")
#
#    set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75" CACHE STRING "" FORCE)
#
#    set(UseCuda ON)
#
#    add_definitions(-D__USE_CUDA)
#else()
#    message(STATUS "No CUDA support")
#endif()

#message(STATUS "OPENCV: INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS}")

# include dirs
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        include
)

set(SOURCE_DIRS
        "src"
)

# add sources
set(SOURCES
        include/private/Cuda.cuh)
foreach (S_DIR ${SOURCE_DIRS})
    if (${UseCuda})
        file(GLOB_RECURSE S_FILES "${S_DIR}/*.c" "${S_DIR}/*.cpp" "${S_DIR}/*.cu")
    else ()
        file(GLOB_RECURSE S_FILES "${S_DIR}/*.c" "${S_DIR}/*.cpp")
    endif ()
    foreach (S_FILE ${S_FILES})
        list(APPEND SOURCES ${S_FILE})
    endforeach ()
endforeach ()

add_library(${PROJECT_NAME} STATIC ${SOURCES}
)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        include
)

file(WRITE include/private/vars.h
        "#pragma once
static char homePath[] = \"${CMAKE_HOME_DIRECTORY}\";
")

add_subdirectory(SubModules)

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        CoreThreadPool
)

#target_include_directories(${PROJECT_NAME} PUBLIC
#        include
#)

# UI library
add_subdirectory(UI)

if (CoreTest)
    add_subdirectory(test)
endif ()

