cmake_minimum_required(VERSION 3.30)

project(ImageEn_Decrypto_Test)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Core Test Configured!")

add_executable(${PROJECT_NAME} main.cpp
        vars.h)

file(WRITE vars.h
        "#pragma once
static char homePath[] = \"${CMAKE_HOME_DIRECTORY}\";
")

target_link_libraries(${PROJECT_NAME} PRIVATE ImageEn_DecryptoCore)
