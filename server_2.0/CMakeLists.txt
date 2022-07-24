cmake_minimum_required(VERSION 3.21)
project(server_2_0)

set(CMAKE_CXX_STANDARD 11)

add_executable(server_2_0 main.cpp ./protocol/myprotocol.pb.cc sql/sqlConnectionPool.cpp threadpool/thread_pool.cpp timer/timer.cpp utils/utils.cpp utils/utils.h)
target_link_libraries(server_2_0 libprotobuf.a)
