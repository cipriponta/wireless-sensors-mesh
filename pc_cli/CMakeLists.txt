cmake_minimum_required(VERSION 3.20)

project(pc-cli)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES 
    src/main.c
    ../common/socket_defs.h
)

add_executable(${PROJECT_NAME} ${SOURCES})