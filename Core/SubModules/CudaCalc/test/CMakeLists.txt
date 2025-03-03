cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

project(CudaTest LANGUAGES CXX CUDA)

add_executable(
        ${PROJECT_NAME}
        main.cu
)

include_directories(
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        ${OpenCV_LIBS}
        CoreCudaCalc
)
