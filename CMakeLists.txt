cmake_minimum_required(VERSION 3.10)

project(mcscript_vm VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

include_directories(include)

file(GLOB_RECURSE SOURCES src/*.c)

add_executable(mcscript_vm ${SOURCES})
