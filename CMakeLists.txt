
cmake_minimum_required(VERSION 2.8.3)

# Project Properties

project(lowsheen-utils)

set(LOWSHEEN_UTILS_PATH /usr/local/bin)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Werror")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g")

set(BUILD_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_BINARY_DIR ${BUILD_OUTPUT_PATH})

# Extract GIT information

# call sh directly as execute_process is not a shell and pipes wont't work
# get tags associated with the HEAD, get the first one, remove new lines
# execute_process is split due to limitations of how it works
execute_process(
    COMMAND sh "-c" "git tag --points-at HEAD | head -n 1 | tr -d \"\r\n\""
    OUTPUT_VARIABLE TAG_NAME
)
execute_process (
    COMMAND sh "-c" "git rev-parse HEAD | tr -d \"\r\n\""
    OUTPUT_VARIABLE TAG_COMMIT_HASH
)

execute_process(
    COMMAND sh "-c" "git log -1 --format=%ai ${TAG_NAME} | tr -d \"\r\n\""
    OUTPUT_VARIABLE TAG_DATE
)

execute_process(
    COMMAND sh "-c" "git rev-parse --abbrev-ref HEAD | tr -d \"\r\n\""
    OUTPUT_VARIABLE BRANCH_NAME
)

if (NOT TAG_NAME)
    set(TAG_NAME "1.0.0")
    message(STATUS "There is no TAG setup, defaulting version to 1.0.0")
else()
    
endif()

message(STATUS "TAG_NAME: " ${TAG_NAME})
message(STATUS "TAG_COMMIT_HASH: " ${TAG_COMMIT_HASH})
message(STATUS "TAG_DATE: " ${TAG_DATE})

# version header with definitions
configure_file(${PROJECT_SOURCE_DIR}/version.h.cmake ${BUILD_OUTPUT_PATH}/version.h)
 
# Project Applications
add_subdirectory(vcis-util)
add_subdirectory(kinetek-util)

# Debian Package Definition (using CPack)
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lowsheen") #required
SET(CPACK_PACKAGE_VERSION ${TAG_NAME})

INCLUDE(CPack)

# place script to automatically bring up interfaces using /etc/network/interfaces.d
set(LOWSHEEN_UTILS_IFC_SCRIPT "./scripts/${CMAKE_PROJECT_NAME}.socketcan")
install(PROGRAMS ${LOWSHEEN_UTILS_IFC_SCRIPT}
    DESTINATION "/etc/network/interfaces.d/"
)


