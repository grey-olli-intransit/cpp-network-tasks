cmake_minimum_required(VERSION 3.10)

project(dns-resolver C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_include_directories("${PROJECT_NAME}" PUBLIC /usr/src/gb/src/socket_wrapper/include/)
target_link_libraries("${PROJECT_NAME}" socket-wrapper)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

