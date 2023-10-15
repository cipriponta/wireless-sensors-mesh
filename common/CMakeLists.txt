cmake_minimum_required(VERSION 3.20)

project(common)

set(
    HEADER_FILES
    socket_helpers.h
)

set(
    SOURCE_FILES
    socket_helpers.c
)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})
