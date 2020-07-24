// a class to abstract CO_driver and make it easy to send/receive messages

#ifndef SOCKET_CAN_HELPER_H
#define SOCKET_CAN_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "CO_driver.h"
#include "CO_driver_target.h"
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

class SocketCanHelper
{
    public:
    // sets up timer for receiving messages
    SocketCanHelper();

    ~SocketCanHelper();

    // initializes CO_driver objects and connects to an interface, ex: "can0"
    int init_socketcan(const char* interface_name);
    
    int send_frame(uint32_t can_id, uint8_t* byte_array, uint8_t arr_size);

    // receiving a message requires an identifier object and a callback function which gets the identifier, wait_time is in ms
    CO_CANrxMsg_t * get_frame(uint32_t can_id, void* obj, void (*pFunct)(void *object, const CO_CANrxMsg_t *message), int wait_time=5);

    private:
    // objects rerquired to use CO_driver
    CO_CANmodule_t* cm;
    CO_CANtx_t* tx_buff_arr;
    CO_CANrx_t* rx_buff_arr;
    CO_CANrxMsg_t* can_msg;

    // timer variables
    itimerspec* time_out;
    int timer_fd;
};

#endif

