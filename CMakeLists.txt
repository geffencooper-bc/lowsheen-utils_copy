
cmake_minimum_required(VERSION 2.8.3)

# Project Properties

project(lowsheen-utils)

set(LOWSHEEN_UTILS_PATH /usr/local/${CMAKE_PROJECT_NAME})

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Werror")

# Project Applications
add_subdirectory(vcis-util)

# Debian Package Definition (using CPack)
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lowsheen") #required
INCLUDE(CPack)

# create a script that adds the lowsheen-utils directory to the PATH environment variable
# using /etc/profile.d
set(LOWSHEEN_UTILS_TOOLS_PATH_SCRIPT "${CMAKE_PROJECT_NAME}.sh")
file(WRITE ${LOWSHEEN_UTILS_TOOLS_PATH_SCRIPT} "export PATH=$PATH:${LOWSHEEN_UTILS_PATH}")

install(PROGRAMS ${LOWSHEEN_UTILS_TOOLS_PATH_SCRIPT}
    DESTINATION "/etc/profile.d/"
)

# create a script that automatically bring up can interface on the system
# using /etc/network/interfaces.d
set(LOWSHEEN_UTILS_IFC_SCRIPT "./scripts/${CMAKE_PROJECT_NAME}.socketcan")
install(PROGRAMS ${LOWSHEEN_UTILS_IFC_SCRIPT}
    DESTINATION "/etc/network/interfaces.d/"
)
