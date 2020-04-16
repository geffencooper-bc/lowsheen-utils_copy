
cmake_minimum_required(VERSION 2.8.3)

project(lowsheen-utils)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )

add_subdirectory(vcis-util)
