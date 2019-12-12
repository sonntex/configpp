cmake_minimum_required(VERSION 3.15)

project(variti-config-examples)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(../include)

add_executable(
  config-example1
  ../src/util/config.cpp
  config_example1.cpp)
target_link_libraries(
  config-example1
  config
  pthread)
