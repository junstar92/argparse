cmake_minimum_required(VERSION 3.11)
project(argparse
    VERSION 1.0.0
    LANGUAGES CXX
)

option(ARGPARSE_BUILD_SAMPLE "" ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (ARGPARSE_BUILD_SAMPLE)
    add_subdirectory(samples "_samples")
endif()