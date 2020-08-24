//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions


// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//==================================================================

#include "SocketCanHelper.h"

// if defined then will print out can trace
#include "debug.h"
#if DEBUG_SOCKET_CAN
#else
#define DEBUG_PRINTF(...) \
        do {} while (0)
#endif

// create timer used for receive message timeouts
SocketCanHelper::SocketCanHelper()
{
    time_out = new itimerspec;
    timer_fd = timerfd_create(CLOCK_REALTIME, 0);
}

// deallocate memory
SocketCanHelper::~SocketCanHelper()
{
    CO_CANmodule_disable(can_module);
    delete[] tx_arr;
    delete[] rx_arr;
    delete can_msg;
    delete can_module;
    delete time_out;
}

// initialize CO_driver objects and connects to can interface, ex: "can0"
int SocketCanHelper::init_socketcan(const char* interface_name)
{
    // CO_driver uses can_module object to Tx/Rx can messages
    can_module = new CO_CANmodule_t;

    // stores the lastest can frame received
    can_msg = new CO_CANrxMsg_t;

    // array of transmit and receive message objects used by can_module
    tx_arr = new CO_CANtx_t[1];
    rx_arr = new CO_CANrx_t[1];

    // interface setup (typically 'can0'), must send ip link commands first
    unsigned int if_index = if_nametoindex(interface_name);
    if (if_index == 0)
    {
        printf("If Index Error\n");
        return -1;
    }

    uintptr_t can_interface = if_index;
    DEBUG_PRINTF("if index: %i\n", can_interface);

    int err = CO_CANmodule_init(can_module, (void*)if_index, rx_arr, 1, tx_arr, 1, 250);
    if (err != 0)
    {
        printf("Init CO_CANmodule. Error: %i\tInterface Count: %i\n", err, can_module->CANinterfaceCount);
    }

    // sets up Rx filters
    CO_CANsetNormalMode(can_module);
}

// send a CAN frame, pass in can id (11 bit or extended) and data as array of bytes {0x01, 0x02, ...}
int SocketCanHelper::send_frame(uint32_t can_id, uint8_t* data, uint8_t data_len)
{
    // tx message object
    CO_CANtx_t* tx1 = nullptr;

    // manual tx init because CO_CANtxBufferInit only takes 11 bit ids
    if (can_id > 0x7FF)  // largest 11 bit id
    {
        tx1 = &can_module->txArray[0];
        tx1->ident = can_id;
        tx1->DLC = data_len;
        tx1->ident = can_id;

        tx1->bufferFull = false;
        tx1->syncFlag = false;
    }
    else
    {
        tx1 = CO_CANtxBufferInit(can_module, 0, can_id, 0, data_len, false);
    }

    DEBUG_PRINTF("Sending Message-->");

    // copy the message data into the transmit buffer
    memcpy(tx1->data, data, data_len);

    // send the message
    int err = CO_CANsend(can_module, tx1);

    if (err < 0)
    {
        printf("Transmit Error: %i\t", err);
        exit(EXIT_FAILURE);
    }

    DEBUG_PRINTF("Id: %02X\t", can_id);
    for (uint8_t i = 0; i < data_len; i++)
    {
        DEBUG_PRINTF("%02X ", data[i]);
    }
    DEBUG_PRINTF("\n");
}

// wait for next can frame with the specified id (and mask)
// CO_CANrxBufferInit requires identifier object and callback function that gets called
// if a frame with specified id is received within the wait_time (in ms)
CO_CANrxMsg_t* SocketCanHelper::get_frame(uint32_t can_id,
                                          void* obj,
                                          void (*call_back)(void* obj, const CO_CANrxMsg_t* msg),
                                          int wait_time,
                                          uint32_t can_id_mask)
{
    // zero out the last receive message
    memset(can_msg, 0, sizeof(can_msg));

    // setup desired time_out given ms input, 5ms by default
    time_out->it_value.tv_sec = wait_time / 1000;
    time_out->it_value.tv_nsec = (wait_time % 1000) * 1000000;

    // reset the timer for the receive message
    int err = timerfd_settime(timer_fd, 0, time_out, NULL);

    if (err < 0)
    {
        printf("Timer Error: %i\n", err);
        exit(EXIT_FAILURE);
    }

    DEBUG_PRINTF("Getting Message-->");

    // initialize the rx message object
    err = CO_CANrxBufferInit(can_module, 0, can_id, can_id_mask, 0, obj, call_back);

    if (err < 0)
    {
        printf("Receive Error: %i\t", err);
        exit(EXIT_FAILURE);
    }

    // waits until receive specified can id or until timer ends (blocking function)
    err = CO_CANrxWait(can_module, timer_fd, can_msg);

    if (err < 0)
    {
        DEBUG_PRINTF("TIME OUT\n");
    }
    else
    {
        DEBUG_PRINTF("Id: %02X\t", can_msg->ident);
        for (uint8_t i = 0; i < can_msg->DLC; i++)
        {
            DEBUG_PRINTF("%02X ", can_msg->data[i]);
        }
        DEBUG_PRINTF("\n");
    }

    return can_msg;
}