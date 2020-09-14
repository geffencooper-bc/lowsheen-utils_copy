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

# # options for testing modes
# set(IAP_WINDOW_CENTER "30" CACHE STRING "window center")

# option(COMMAND_LINE_MODE              "Use the normal kinetek-util app" ON)
# option(TEST_IAP_MODE                  "Test entering IAP mode (can test only a specific mode as another option)" OFF)
# option(IAP_SELECTIVE_MODE_TEST        "Test only selective mdoe" OFF)
# option(IAP_FORCED_MODE_TEST           "Test only forced mode" OFF)
# option(TEST_FORCED_WINDOW             "Find the 6ms forced window" OFF)
# option(FULL_IAP_TEST                  "Test the full update process" OFF)

# if(COMMAND_LINE_MODE) # default normal usage of utility
#   add_definitions(-DCOMMAND_LINE_MODE)
# elseif(TEST_IAP_MODE) # test entering iap mode, if no specific mode specified then full state machine executed
#   add_definitions(-DTEST_IAP_MODE)
#   if(IAP_SELECTIVE_MODE_TEST)
#     add_definitions(-DIAP_SELECTIVE_MODE_TEST)
#   elseif(IAP_FORCED_MODE_TEST)
#     add_definitions(-DIAP_FORCED_MODE_TEST)
#   endif()
# elseif(TEST_FORCED_WINDOW) # find the forced mode window
#   add_definitions(-DTEST_FORCED_WINDOW)
# elseif(FULL_IAP_TEST) # test the entire iap process
#   add_definitions(-DFULL_IAP_TEST)
# endif()

add_executable(${APP_NAME} ${SOURCES})

target_compile_options(${APP_NAME} PUBLIC "-pthread")

target_link_libraries(${APP_NAME} pthread)

install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
