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
if(WIN32)
    set(FFMPEG_DIR "D:/EnvPrograms/ffmpeg")

    add_library(FFmpeg::avcodec SHARED IMPORTED)
    set_target_properties(FFmpeg::avcodec PROPERTIES
            IMPORTED_LOCATION "${FFMPEG_DIR}/bin/avcodec-61.dll"
            IMPORTED_IMPLIB "${FFMPEG_DIR}/lib/avcodec.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_DIR}/include"
    )

    add_library(FFmpeg::avformat SHARED IMPORTED)
    set_target_properties(FFmpeg::avformat PROPERTIES
            IMPORTED_LOCATION "${FFMPEG_DIR}/bin/avformat-61.dll"
            IMPORTED_IMPLIB "${FFMPEG_DIR}/lib/avformat.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_DIR}/include"
    )

    add_library(FFmpeg::avutil SHARED IMPORTED)
    set_target_properties(FFmpeg::avutil PROPERTIES
            IMPORTED_LOCATION "${FFMPEG_DIR}/bin/avutil-59.dll"
            IMPORTED_IMPLIB "${FFMPEG_DIR}/lib/avutil.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_DIR}/include"
    )

    add_library(FFmpeg::swresample SHARED IMPORTED)
    set_target_properties(FFmpeg::swresample PROPERTIES
            IMPORTED_LOCATION "${FFMPEG_DIR}/bin/swresample-5.dll"
            IMPORTED_IMPLIB "${FFMPEG_DIR}/lib/swresample.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_DIR}/include"
    )

    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
    target_link_libraries(FFmpeg::FFmpeg INTERFACE
            FFmpeg::avcodec
            FFmpeg::avformat
            FFmpeg::avutil
            FFmpeg::swresample
    )
else ()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
            libavcodec libavformat libavutil libswresample)
endif ()

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

if (WIN32)
    target_link_libraries(${PROJECT_NAME}
            ${OpenCV_LIBS}
            CoreThreadPool
            CoreLogger
            CList
            FFmpeg::FFmpeg
    )
else ()
    target_link_libraries(${PROJECT_NAME}
            ${OpenCV_LIBS}
            CoreThreadPool
            CoreLogger
            CList
            PkgConfig::FFMPEG
    )
endif ()

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

