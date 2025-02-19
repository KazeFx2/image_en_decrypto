cmake_minimum_required(VERSION 3.20)

if (NOT ${UseCuda})
    return()
endif ()

project(CoreCudaCalc LANGUAGES CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75")

set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_DIRS src)

set(SOURCES)
foreach (S_DIR ${SOURCE_DIRS})
    file(GLOB_RECURSE S_FS ${S_DIR}/*.c ${S_DIR}/*.cpp ${S_DIR}/*.cu)
    list(APPEND SOURCES ${S_FS})
endforeach ()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        include
)

if (CoreTest)
    add_subdirectory(test)
endif ()
