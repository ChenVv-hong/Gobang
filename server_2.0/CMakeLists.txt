cmake_minimum_required(VERSION 3.16)
project(server_2_0)

set(CMAKE_CXX_STANDARD 11)

add_executable(server_2_0 main.cpp ./protocol/myprotocol.pb.cc sql/sqlConnectionPool.cpp threadpool/thread_pool.cpp timer/timer.cpp utils/myUtils.cpp utils/myUtils.h game/room.cpp game/room.h main.h DAO/UserDao.cpp DAO/UserDao.h)
target_link_libraries(server_2_0 libprotobuf.a)
target_link_libraries(server_2_0 mysqlclient)
target_link_libraries(server_2_0 pthread)