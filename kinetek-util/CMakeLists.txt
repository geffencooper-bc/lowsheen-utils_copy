cmake_minimum_required(VERSION 2.8.3)

set(APP_NAME "kinetek-util")

set(CMAKE_BUILD_TYPE Debug)

# default directory paths
set(CANOPEN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../modules/CANopenNode) 
set(STACKDRV_SRC ${CANOPEN_SRC}/stack/neuberger-socketCAN)
set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)


include_directories(   
                        ${STACKDRV_SRC}
                        ${CANOPEN_SRC}
                        ${APPL_SRC}
                        ${CMAKE_CURRENT_BINARY_DIR}
)

set(SOURCES     ${STACKDRV_SRC}/CO_driver.c        
                ${STACKDRV_SRC}/CO_notify_pipe.c 
                ${APP_SRC}/KinetekUtility.cpp                      
                ${APP_SRC}/main.cpp
                ${APP_SRC}/SocketCanHelper.cpp
                ${APP_SRC}/HexUtility.cpp
                ${APP_SRC}/IAP.cpp
                ${APP_SRC}/KinetekUtilityCodes.cpp
                ${APP_SRC}/STUparam.cpp
                ${APP_SRC}/LiveData.cpp
)

add_executable(${APP_NAME} ${SOURCES})

target_compile_options(${APP_NAME} PUBLIC "-pthread")

target_link_libraries(${APP_NAME} pthread)

install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
