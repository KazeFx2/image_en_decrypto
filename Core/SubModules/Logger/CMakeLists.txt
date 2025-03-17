cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

project(CoreLogger)

set(SOURCES)
foreach (S_DIR ${SOURCE_DIRS})
    file(GLOB_RECURSE S_FS ${S_DIR}/*.c ${S_DIR}/*.cpp)
    list(APPEND SOURCES ${S_FS})
endforeach ()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        CoreThreadPool
        CoreConditionVariable
        CList
)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        include
)

if (CoreTest)
    add_definitions(-D__DEBUG)
    add_subdirectory(test)
endif ()
