cmake_minimum_required(VERSION 3.20)

project(rpi-gw-src)

set(
    HEADER_FILES
    ble_driver.h
)

set(
    SOURCE_FILES
    main.c
    ble_driver.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../common)
target_link_libraries(${PROJECT_NAME} common)