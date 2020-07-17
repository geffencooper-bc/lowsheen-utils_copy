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
               # ${STACKDRV_SRC}/CO_Linux_threads.c 
                ${STACKDRV_SRC}/CO_notify_pipe.c   
               # ${CANOPEN_SRC}/CANopen.c                          
                ${APP_SRC}/main.cpp
                ${APP_SRC}/SocketCanHelper.cpp
                ${APP_SRC}/HexUtility.cpp
                ${APP_SRC}/IAP.cpp
)

# attempt to generate version.h if info changed
add_custom_target(
   gen_version_headers2
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/__df_1   # depend on non-existent file to force target to build every time
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/__df_1
    COMMAND python ${PROJECT_SOURCE_DIR}/scripts/version.py
)

# remove stopping compilation due to warnings caused by the auto-generated file OC_OD.c
set_source_files_properties(${OBJ_DICT_SRC}/CO_OD.c PROPERTIES COMPILE_FLAGS -w)

add_executable(${APP_NAME} ${SOURCES})
add_dependencies(${APP_NAME} gen_version_headers2)

target_compile_options(${APP_NAME} PUBLIC "-pthread")

target_link_libraries(${APP_NAME} pthread)

install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
