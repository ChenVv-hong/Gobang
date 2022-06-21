cmake_minimum_required(VERSION 3.21)
project(server)

set(CMAKE_CXX_STANDARD 14)


add_executable(server main.cpp thread_pool.h thread_pool.cpp player.cpp player.h room.cpp room.h)
