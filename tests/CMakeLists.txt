cmake_minimum_required(VERSION 2.8)
project(samples_rga_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER /opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

include_directories(
        .
        rockchip_rga)

add_library(librockchip_rga
        rockchip_rga/android_rga.cpp
        rockchip_rga/rockchip_rga.cpp)

add_executable(test_rga_copy
        test_rga_copy.c)

