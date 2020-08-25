cmake_minimum_required(VERSION 2.8.3)

set(APP_NAME "machine-util")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CURRENT_BINARY_DIR ${BUILD_OUTPUT_PATH})

# default directory paths
set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(    /usr/include/libusb-1.0/
                        ${APPL_SRC}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_BINARY_DIR}
)

set(SOURCES     ${APP_SRC}/main.c
)

add_executable(${APP_NAME} ${SOURCES})

target_compile_options(${APP_NAME} PUBLIC "-pthread")

target_link_libraries(${APP_NAME} pthread usb-1.0)

install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
