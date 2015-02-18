cmake_minimum_required (VERSION 2.8.11)
project(truncatednsd)

option(USE_SANDBOX "Build sandbox support" OFF)

set(CMAKE_C_FLAGS "-Wall")

if(USE_SANDBOX)
  add_definitions(-DUSE_SANDBOX)
endif()

add_executable(truncatednsd
  src/truncatednsd.c src/config.c src/sandbox.c
  src/log.c
  )
add_executable(runinetd test/runinetd.c)
