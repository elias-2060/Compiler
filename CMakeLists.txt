cmake_minimum_required(VERSION 3.22.3)
project(CompilerMB)

set(CMAKE_CXX_STANDARD 14)

add_executable(CompilerMB
        Lexer.cpp main.cpp lrParser.cpp)
