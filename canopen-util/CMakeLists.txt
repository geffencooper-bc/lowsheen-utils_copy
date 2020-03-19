cmake_minimum_required(VERSION 2.8.3)

set(CANOPEN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../modules/CANopenNode) 
set(CANOPEND_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../modules/CANopenSocket/canopend/src) 
set(STACK_SRC ${CANOPEN_SRC}/stack)
set(STACKDRV_SRC ${CANOPEN_SRC}/stack/socketCAN)
set(OBJ_DICT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/object_dictionary)
set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(    ${CANOPEN_SRC}
                        ${STACK_SRC}
                        ${STACKDRV_SRC}
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
                ${CANOPEN_SRC}/CANopen.c            
                ${CANOPEND_SRC}/CO_command.c        
                ${CANOPEND_SRC}/CO_comm_helpers.c   
                ${CANOPEND_SRC}/CO_master.c         
                ${CANOPEND_SRC}/CO_LSS_master.c     
                ${CANOPEND_SRC}/CO_time.c           
                ${CANOPEND_SRC}/main.c              
                ${OBJ_DICT_SRC}/CO_OD.c             
                ${APP_SRC}/main.c)

add_executable(canopen-util ${SOURCES})
