cmake_minimum_required(VERSION 3.20)

project(pc-cli-src)

set(
    SOURCE_FILES
    main.c
)

add_executable(pc-cli ${SOURCE_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../common)
target_link_libraries(${PROJECT_NAME} common)