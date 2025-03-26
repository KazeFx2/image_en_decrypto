cmake_minimum_required(VERSION 3.20)

project(CParser VERSION 0.1 LANGUAGES C)

add_executable(${PROJECT_NAME}
        c_parser.c
        c_string_ref_list/string_ref_list.c
        split_line_module/split_line_module.c
        split_line_module/quote_match/quote_match.c
        split_line_module/quote_match/sample_match/quote_sample_match.c
        split_line_module/quote_match/line_endless_match/line_endless_match.c
        split_line_module/symbol_match/symbol_match.c
        split_line_module/symbol_match/split_symbol_match/split_symbol_match.c
        split_line_module/symbol_match/key_symbol_match/key_symbol_match.c)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../setmem
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        CList)

#if (C_COMPONENT_C_TEST)
#    add_subdirectory(test)
#endif ()
