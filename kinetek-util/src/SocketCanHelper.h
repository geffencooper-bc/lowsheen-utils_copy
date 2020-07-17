// a class to abstract CO_driver and make it easy to send/receive messages

#ifndef SOCKET_CAN_HELPER_H
#define SOCKET_CAN_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include "CO_driver.h"

using std::string;
using std::cout;
using std::endl;

class SocketCanHelper
{
    public:
    SocketCanHelper();

    ~SocketCanHelper();

    int init_socketcan(const char* interface_name); //"can0"
    
    int send_frame(uint32_t can_id, uint8_t* data, uint8_t data_size);

    int get_frame(uint32_t can_id, uint8_t* data, uint8_t data_size);

    void print_frame();

    private:
    CO_CANmodule_t* cm;
    CO_CANtx_t* tx_buff_arr;
    CO_CANrx_t* rx_buff_arr;
};

#endif

