cmake_minimum_required(VERSION 3.20)

project(rpi-gw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common ${CMAKE_CURRENT_SOURCE_DIR}/out/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
