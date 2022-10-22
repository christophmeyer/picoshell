cmake_minimum_required(VERSION 3.10)

project(picoshell VERSION 0.1 DESCRIPTION "A rudimentary shell")

add_library(picoshell STATIC
src/picoshell.c
src/parser.c
src/utils.c)

add_executable(psh ./src/psh.c)
target_include_directories(psh PUBLIC ./src)
target_compile_options(psh PUBLIC -O3 -Wall)
target_link_libraries(psh PUBLIC picoshell readline)

install (TARGETS psh RUNTIME DESTINATION /usr/bin)
