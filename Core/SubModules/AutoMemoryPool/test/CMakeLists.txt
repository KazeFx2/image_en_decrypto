cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

project(AutoMemPoolTest)

add_executable(
        ${PROJECT_NAME}
        main.cpp
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        CoreAutoMemoryPool
        CList
#        CoreSemaphore
#        CoreBitmap
)
