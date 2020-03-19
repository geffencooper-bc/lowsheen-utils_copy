cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_BUILD_TYPE Debug)

set(CANOPEN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../modules/CANopenNode) 
set(CANOPEND_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../modules/CANopenSocket/canopend/src) 
set(STACK_SRC ${CANOPEN_SRC}/stack)
set(STACKDRV_SRC ${CANOPEN_SRC}/stack/socketCAN)
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
                ${STACKDRV_SRC}/CO_OD_storage.c     
                ${STACKDRV_SRC}/CO_Linux_tasks.c    
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
                ${CANOPEND_SRC}/CO_master.c         
                ${STACK_SRC}/CO_TIME.c         
                ${CANOPEN_SRC}/CANopen.c                          
                ${OBJ_DICT_SRC}/CO_OD.c             
                ${APP_SRC}/main.c)

add_executable(canopen-util ${SOURCES})
