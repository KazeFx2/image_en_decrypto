cmake_minimum_required(VERSION 3.30)

project(ImageEn_DecryptoCore)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

message(STATUS "OPENCV: INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS}")

# include dirs
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        include
)

set(SOURCE_DIRS
        "src"
)

# add sources
set(SOURCES)
foreach (S_DIR ${SOURCE_DIRS})
    file(GLOB_RECURSE S_FILES "${S_DIR}/*.c" "${S_DIR}/*.cpp")
    foreach (S_FILE ${S_FILES})
        list(APPEND SOURCES ${S_FILE})
    endforeach ()
endforeach ()

add_library(${PROJECT_NAME} STATIC ${SOURCES}
)

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

