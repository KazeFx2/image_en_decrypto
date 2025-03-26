cmake_minimum_required(VERSION 3.20)

project(CoreMutex)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_DIRS src)

set(SOURCES)
foreach (S_DIR ${SOURCE_DIRS})
    file(GLOB_RECURSE S_FS ${S_DIR}/*.c ${S_DIR}/*.cpp)
    string(COMPARE EQUAL ${S_FS} size.cpp VAR)
    if (NOT ${VAR})
        list(APPEND SOURCES ${S_FS})
    endif ()
endforeach ()

BuildExportSize(${PROJECT_NAME} size.cpp ${SOURCES})
