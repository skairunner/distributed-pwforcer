# SHA2 hasher, for Operating System, NYU Shanghai, Prof. Olivier Marin
# Computer the SHA2 series hash

# cmake prior to version 3.0 may have problem dealing with
# cygwin64 environment
cmake_minimum_required(VERSION 3.0)

# make sure that we use C11
set(CMAKE_C_STANDARD 11)

# set up project source files
project(SHA2 C)
set(SHA2_SOURCES
        sha2.h sha2.c)

# assemble the project to a shared library
add_library(SHA2 STATIC ${SHA2_SOURCES})
