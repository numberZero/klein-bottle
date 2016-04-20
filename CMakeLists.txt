cmake_minimum_required(VERSION 2.6)
project(klein)

add_executable(klein main.cpp)

install(TARGETS klein RUNTIME DESTINATION bin)
