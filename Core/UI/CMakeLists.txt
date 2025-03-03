cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

project(CoreUI)

find_package(SDL2)

include_directories(
        ${SDL2_INCLUDE_DIRS}
)

set(SRC_DIRS
        src
)
set(SRC_FILES)
foreach (SRC_DIR ${SRC_DIRS})
    file(GLOB_RECURSE SRC_FILE ${SRC_DIR}/*.c ${SRC_DIR}/*.cpp)
    list(APPEND SRC_FILES ${SRC_FILE})
endforeach ()

add_library(
        ${PROJECT_NAME}
        STATIC
        ${SRC_FILES}
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC CoreMutex
)
