cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 11)
project(yfc)

add_compile_options(-O3 -Wall -Wpedantic -g -Wno-gnu-zero-variadic-macro-arguments)

file(GLOB_RECURSE SOURCES_API src/api/*.c)
add_library(api OBJECT ${SOURCES_API})
target_include_directories(api PRIVATE src)

file(GLOB_RECURSE SOURCES_DRIVER src/driver/*.c)
add_library(driver OBJECT ${SOURCES_DRIVER})
target_include_directories(driver PRIVATE src)

file(GLOB_RECURSE SOURCES_LEXER src/lexer/*.c)
add_library(lexer OBJECT ${SOURCES_LEXER})
target_include_directories(lexer PRIVATE src)

file(GLOB_RECURSE SOURCES_PARSER src/parser/*.c)
add_library(parser OBJECT ${SOURCES_PARSER})
target_include_directories(parser PRIVATE src)

file(GLOB_RECURSE SOURCES_SEMANTICS src/semantics/*.c)
add_library(semantics OBJECT ${SOURCES_SEMANTICS})
target_include_directories(semantics PRIVATE src)

file(GLOB_RECURSE SOURCES_UTIL src/util/*.c)
add_library(util OBJECT ${SOURCES_UTIL})
target_include_directories(util PRIVATE src)

add_executable(yfc
    $<TARGET_OBJECTS:api>
    $<TARGET_OBJECTS:driver>
    $<TARGET_OBJECTS:lexer>
    $<TARGET_OBJECTS:parser>
    $<TARGET_OBJECTS:semantics>
    $<TARGET_OBJECTS:util>
)
