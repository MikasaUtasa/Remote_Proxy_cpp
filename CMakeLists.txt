cmake_minimum_required(VERSION 3.22)
project(Remote_Proxy)

set(CMAKE_CXX_STANDARD 20)

add_executable(Remote_Proxy main.cpp Proxy.cpp Proxy.h)
