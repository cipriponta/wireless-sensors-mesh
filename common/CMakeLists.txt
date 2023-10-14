cmake_minimum_required(VERSION 3.20)

project(common)

set(
    HEADER_FILES
    socket_defs.h
)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
