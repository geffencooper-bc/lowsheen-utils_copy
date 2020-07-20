// a class to abstract CO_driver and make it easy to send/receive messages

#ifndef SOCKET_CAN_HELPER_H
#define SOCKET_CAN_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "CO_driver.h"
#include "CO_driver_target.h"
#include <string>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
using std::string;
using std::to_string;

class SocketCanHelper
{
    public:
    SocketCanHelper();

    ~SocketCanHelper();

    int init_socketcan(const char* interface_name); //"can0"
    
    int send_frame(uint32_t can_id, uint8_t* data, uint8_t data_size);

    // the object is going to be the iap object which will have information about current state in iap process etc
    CO_CANrxMsg_t * get_frame(uint32_t can_id, void* obj, void (*pFunct)(void *object, const CO_CANrxMsg_t *message));

    void print_frame();

    private:
    CO_CANmodule_t* cm;
    CO_CANtx_t* tx_buff_arr;
    CO_CANrx_t* rx_buff_arr;
    CO_CANrxMsg_t* can_msg;

    // make into pointers later
    itimerspec* new_value;
    int timer_fd;
    timespec* now;
};

#endif

