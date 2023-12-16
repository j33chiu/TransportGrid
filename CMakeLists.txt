# version 3.11 or later of CMake needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.16)

project(TransportGrid
    VERSION "0.1"
    DESCRIPTION "Transport Grid Project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

add_subdirectory("TransportGrid")