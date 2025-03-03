cmake_minimum_required(VERSION 3.20)

project(ImageEn_Decrypto_Test)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Core Test Configured!")

add_executable(${PROJECT_NAME} main.cpp
)

#file(WRITE vars.h
#        "#pragma once
#static char homePath[] = \"${CMAKE_HOME_DIRECTORY}\";
#")

target_link_libraries(${PROJECT_NAME}
        ImageEn_DecryptoCore CoreThreadPool)

set(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB SUB_FILES ${TARGET_DIR}/*)
foreach (SUB_FILE ${SUB_FILES})
    file(RELATIVE_PATH FILE_NAME ${TARGET_DIR} ${SUB_FILE})
    if (IS_DIRECTORY "${TARGET_DIR}/${FILE_NAME}" AND NOT ${FILE_NAME} STREQUAL CMakeFiles)
        add_subdirectory(${FILE_NAME})
    endif ()
endforeach ()
