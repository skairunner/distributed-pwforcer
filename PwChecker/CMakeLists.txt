# Password checker, for Operating System, NYU Shanghai, Prof. Olivier Marin
# Checks if conjectured password matches

# cmake prior to version 3.0 may have problem dealing with
# cygwin64 environment
cmake_minimum_required(VERSION 3.0)

# make sure that we use C11
set(CMAKE_C_STANDARD 11)

# set up project source files
project(PwChecker C)
set(PW_CHECKER_SOURCES
        pw_checker.h pw_checker.c sha2_wrapper.h sha2_wrapper.c)

# assemble the project to a shared library
add_library(PwChecker STATIC ${PW_CHECKER_SOURCES})

# module SHA2
add_subdirectory(SHA2)
target_include_directories(PwChecker PRIVATE SHA2)
target_link_libraries(PwChecker SHA2)
