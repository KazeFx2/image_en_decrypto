cmake_minimum_required(VERSION 3.20)

project(BitmapTest)

add_executable(
        ${PROJECT_NAME}
        main.cpp
)

target_link_libraries(
        ${PROJECT_NAME}
        CoreBitmap
)
