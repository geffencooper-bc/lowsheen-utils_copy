cmake_minimum_required(VERSION 2.8.3)

set(APP_NAME "machine-util")

set(CMAKE_BUILD_TYPE Debug)

# default directory paths
set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MANIFEST_SRC ${APP_SRC}/../../manifest-util/src)

include_directories(    
                        /usr/include/libusb-1.0/
                        ${APPL_SRC}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_BINARY_DIR}
                        ${MANIFEST_SRC}
)

set(SOURCES     
                ${APP_SRC}/main.cpp
                ${APP_SRC}/lowsheen_headers.cpp
                ${APP_SRC}/lowsheen_interface.cpp
                ${APP_SRC}/usb_interface.cpp
                ${MANIFEST_SRC}/manifest.cpp
)

add_executable(${APP_NAME} ${SOURCES})

find_package(ZLIB)

target_link_libraries(${APP_NAME} usb-1.0 tinyxml2 ZLIB::ZLIB)

install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
