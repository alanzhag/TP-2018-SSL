cmake_minimum_required(VERSION 3.12)
project(TP_2018_SSL C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address ")
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak ")
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined ")
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fPIE")

add_executable(TP_2018_SSL main.c)