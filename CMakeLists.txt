cmake_minimum_required(VERSION 3.28)
project(CompilerDesign C)

set(CMAKE_C_STANDARD 11)

add_executable(CompilerDesign
        main.c
        parser.c
        parser.h
        semantic.c
        semantic.h
        lexer.c
        lexer.h
        common.h
)