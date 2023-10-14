cmake_minimum_required(VERSION 3.20)

project(rpi-gw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES 
    src/main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})