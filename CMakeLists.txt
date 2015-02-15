cmake_minimum_required (VERSION 2.8.11)
project(truncatednsd)
set(CMAKE_C_FLAGS "-Wall")
add_executable(truncatednsd src/truncatednsd.c src/config.c)
