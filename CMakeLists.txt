
cmake_minimum_required(VERSION 2.8.3)

# Project Properties

project(lowsheen-utils)

set(LOWSHEEN_UTILS_PATH /usr/local/bin)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Werror")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g")

# Project Applications
add_subdirectory(vcis-util)
add_subdirectory(kinetek-util)

# Debian Package Definition (using CPack)
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lowsheen") #required
INCLUDE(CPack)

# place script to automatically bring up interfaces using /etc/network/interfaces.d
set(LOWSHEEN_UTILS_IFC_SCRIPT "./scripts/${CMAKE_PROJECT_NAME}.socketcan")
install(PROGRAMS ${LOWSHEEN_UTILS_IFC_SCRIPT}
    DESTINATION "/etc/network/interfaces.d/"
)
