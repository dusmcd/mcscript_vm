cmake_minimum_required(VERSION 3.10)

project(test VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include_directories(
  include
  ../include
)

file(GLOB_RECURSE SOURCES 
  src/*.c
  ../src/*.c
)

# we don't want the main function for the interpreter
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/../src/main.c")

add_executable(test ${SOURCES})
