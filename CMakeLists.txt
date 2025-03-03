cmake_minimum_required(VERSION 3.20)

project(ImageEn_Decrypto)

set(CMAKE_CXX_STANDARD 17)

# Core test module
option(CoreTest "" OFF)
option(UseCuda "" OFF)
set(CoreTest ON CACHE BOOL "" FORCE)
set(UseCuda ON CACHE BOOL "" FORCE)

include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)

    # set cuda arch
    set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75")

    set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75" CACHE STRING "" FORCE)

    set(UseCuda ON)

    add_definitions(-D__USE_CUDA)
else()
    message(STATUS "No CUDA support")
endif()

file(GLOB_RECURSE T_CMAKE_INCLUDE_FILES "cmake/*.cmake")
foreach (FILE ${T_CMAKE_INCLUDE_FILES})
    include(${FILE})
endforeach ()

# Nothing(TMP_1 TMP_2 "A" "B" "C")
# message(STATUS "PARENT: ${TMP_1}")

add_subdirectory(Core)
add_subdirectory(UI)
