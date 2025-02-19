cmake_minimum_required(VERSION 3.20)

project(ImageEn_Decrypto_Diffusion_Test)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
        ImageEn_DecryptoCore CoreThreadPool)
