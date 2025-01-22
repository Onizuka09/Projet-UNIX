cmake_minimum_required(VERSION 3.10)
project(TCP_MONO VERSION 1.0 LANGUAGES C)
add_executable(tcp_serv logger.c services.c network.c tcp_ser.c)


