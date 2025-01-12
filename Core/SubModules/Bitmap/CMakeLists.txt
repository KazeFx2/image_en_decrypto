cmake_minimum_required(VERSION 3.30)

project(CoreBitmap)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_DIRS src)

set(SOURCES)
foreach (S_DIR ${SOURCE_DIRS})
    file(GLOB_RECURSE S_FS ${S_DIR}/*.c ${S_DIR}/*.cpp)
    list(APPEND SOURCES ${S_FS})
endforeach ()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        include
)

if (${CoreTest})
    add_subdirectory(test)
endif ()
