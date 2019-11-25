cmake_minimum_required(VERSION 3.15)

project(variti-config)

set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(include)

add_library(
  variti-config
  src/util/config.cpp)
