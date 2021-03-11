cmake_minimum_required(VERSION 3.17)
project(memo)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS " ../cs106lib.a ")

#file(GLOB SOURCE_FILES *.h *.c *.cpp)

add_executable(memo memo-parser.cpp tokenscanner.cpp strlib.cpp)
