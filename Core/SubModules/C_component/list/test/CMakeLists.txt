cmake_minimum_required(VERSION 3.20)

project(CListTest VERSION 0.1 LANGUAGES C)

add_executable(
        ${PROJECT_NAME}
        test.c
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        CList
)
