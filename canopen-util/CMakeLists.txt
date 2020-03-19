cmake_minimum_required(VERSION 2.8.3)



set(CANOPEN_SRC modules/CANopenNode) 
set(STACK_SRC $(CANOPEN_SRC)/stack)
set(STACKDRV_SRC $(CANOPEN_SRC)/stack/socketCAN)
set(APPL_SRC .)


include_directories(    $(CANOPEN_SRC)
                        $(STACK_SRC)
                        $(STACKDRV_SRC)
                        $(APPL_SRC))

set(SOURCES     $(STACKDRV_SRC)/CO_driver.c 
                $(STACKDRV_SRC)/eeprom.c    
                $(STACK_SRC)/crc16-ccitt.c  
                $(STACK_SRC)/CO_SDO.c       
                $(STACK_SRC)/CO_Emergency.c 
                $(STACK_SRC)/CO_NMT_Heartbeat.c
                $(STACK_SRC)/CO_SYNC.c      
                $(STACK_SRC)/CO_TIME.c      
                $(STACK_SRC)/CO_PDO.c       
                $(STACK_SRC)/CO_HBconsumer.c
                $(STACK_SRC)/CO_SDOmaster.c 
                $(STACK_SRC)/CO_LSSmaster.c 
                $(STACK_SRC)/CO_LSSslave.c  
                $(STACK_SRC)/CO_trace.c     
                $(CANOPEN_SRC)/CANopen.c    
                $(APPL_SRC)/CO_OD.c         
                $(APPL_SRC)/main.c)
                https://www.youtube.com/watch?v=vc6vs-l5dkc


