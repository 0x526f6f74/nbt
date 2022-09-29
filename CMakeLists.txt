cmake_minimum_required(VERSION 3.18)

project(nbt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(nbt nbt.cc nbt.h)
