cmake_minimum_required(VERSION 3.20)

project(ImageEn_DecryptoCore)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

# include dirs
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        include
)

# ffmepg
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavcodec libavformat libavutil libswresample)

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
        PkgConfig::FFMPEG
)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()
#target_include_directories(${PROJECT_NAME} PUBLIC
#        include
#)

if (CoreTest)
    add_subdirectory(test)
endif ()

