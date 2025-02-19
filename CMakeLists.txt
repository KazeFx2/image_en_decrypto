cmake_minimum_required(VERSION 3.20)

project(ImageEn_Decrypto)

set(CMAKE_CXX_STANDARD 17)

# Core test module
option(CoreTest "" OFF)
option(UseCuda "" OFF)
set(CoreTest ON CACHE BOOL "" FORCE)
set(UseCuda ON CACHE BOOL "" FORCE)

file(GLOB_RECURSE T_CMAKE_INCLUDE_FILES "cmake/*.cmake")
foreach (FILE ${T_CMAKE_INCLUDE_FILES})
    include(${FILE})
endforeach ()

# Nothing(TMP_1 TMP_2 "A" "B" "C")
# message(STATUS "PARENT: ${TMP_1}")

add_subdirectory(Core)
add_subdirectory(UI)
