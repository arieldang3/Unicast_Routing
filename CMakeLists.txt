cmake_minimum_required(VERSION 3.16)
project(test8)

set(CMAKE_CXX_STANDARD 14)

add_executable(test8 main.cpp monitor_neighbors.cpp monitor_neighbors.h LSDB.cpp LSDB.h json.hpp debug_file.h)