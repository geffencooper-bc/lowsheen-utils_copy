cmake_minimum_required(VERSION 2.8.3)

set(APP_NAME "manifest-util")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CURRENT_BINARY_DIR ${BUILD_OUTPUT_PATH})

# default directory paths
set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)


include_directories(   
                        ${APPL_SRC}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_BINARY_DIR}
)

set(SOURCES     
                ${APP_SRC}/main.cpp
                ${APP_SRC}/manifest.cpp
)

add_executable(${APP_NAME} ${SOURCES})

target_link_libraries(${APP_NAME} tinyxml2)

 
install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
