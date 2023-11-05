cmake_minimum_required(VERSION 3.20)

project(rpi-gw-src)

set(
    SOURCE_FILES
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../common ${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/gattlib)
target_link_libraries(${PROJECT_NAME} common gattlib)