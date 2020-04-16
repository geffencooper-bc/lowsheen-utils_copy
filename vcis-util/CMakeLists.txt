cmake_minimum_required(VERSION 2.8.3)

set(APP_NAME "vcis-util")

set(CMAKE_BUILD_TYPE Debug)

# default directory paths
set(CANOPEN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../modules/CANopenNode) 
set(STACK_SRC ${CANOPEN_SRC}/stack)
set(STACKDRV_SRC ${CANOPEN_SRC}/stack/neuberger-socketCAN)
set(OBJ_DICT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/dictionary)
set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)


include_directories(   
                        ${CANOPEND_SRC}
                        ${STACKDRV_SRC}
                        ${STACK_SRC}
                        ${CANOPEN_SRC}
                        ${OBJ_DICT_SRC}
                        ${APPL_SRC})

set(SOURCES     ${STACKDRV_SRC}/CO_driver.c         
                ${STACKDRV_SRC}/CO_Linux_threads.c    
                ${STACKDRV_SRC}/CO_notify_pipe.c    
                ${STACK_SRC}/crc16-ccitt.c          
                ${STACK_SRC}/CO_SDO.c               
                ${STACK_SRC}/CO_Emergency.c         
                ${STACK_SRC}/CO_NMT_Heartbeat.c     
                ${STACK_SRC}/CO_SYNC.c              
                ${STACK_SRC}/CO_PDO.c               
                ${STACK_SRC}/CO_HBconsumer.c        
                ${STACK_SRC}/CO_SDOmaster.c         
                ${STACK_SRC}/CO_LSSmaster.c     		
                ${STACK_SRC}/CO_trace.c            
                ${STACK_SRC}/CO_TIME.c         
                ${CANOPEN_SRC}/CANopen.c                          
                ${OBJ_DICT_SRC}/CO_OD.c            
                ${APP_SRC}/CU_TASK.c 
                ${APP_SRC}/CU_TASK_INFO.c 
                ${APP_SRC}/CU_TASK_PROGRAM.c 
                ${APP_SRC}/CU_TASK_RESET.c 
                ${APP_SRC}/CU_TASK_SDO.c
                ${APP_SRC}/CU_COMMAND.c
                ${APP_SRC}/main.c)

# remove stopping compilation due to warnings caused by the auto-generated file OC_OD.c
set_source_files_properties(${OBJ_DICT_SRC}/CO_OD.c PROPERTIES COMPILE_FLAGS -w)

add_executable(${APP_NAME} ${SOURCES})

target_compile_options(${APP_NAME} PUBLIC "-pthread")

target_link_libraries(${APP_NAME} pthread)

install(TARGETS ${APP_NAME}
    COMPONENT linapp
    RUNTIME DESTINATION ${LOWSHEEN_UTILS_PATH}
    LIBRARY DESTINATION ${LOWSHEEN_UTILS_PATH}
    DESTINATION ${LOWSHEEN_UTILS_PATH}
)
