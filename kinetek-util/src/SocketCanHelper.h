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
    // create timer used for receive message timeouts
    SocketCanHelper();

    // deallocate memory, disable CO_CANmodule
    ~SocketCanHelper();

    // initialize CO_driver objects and connect to can interface, ex: "can0"
    int init_socketcan(const char* interface_name);

    // send a CAN frame, pass in can id (11 bit or extended) and data as array of bytes {0x01, 0x02, ...}
    int send_frame(uint32_t can_id, uint8_t* data, uint8_t data_len);

    // wait for next can frame with the specified 11 bit id (and mask)
    // CO_CANrxBufferInit requires identifier object and callback function that gets called
    // if a frame with specified id is received within the wait_time (in ms)
    CO_CANrxMsg_t* get_frame(uint32_t can_id,
                             void* obj,
                             void (*call_back)(void* obj, const CO_CANrxMsg_t* msg),
                             int wait_time,
                             uint16_t can_id_mask = 0x7FFU);

   private:
    // objects rerquired to use CO_driver
    CO_CANmodule_t* can_module;
    CO_CANtx_t* tx_arr;
    CO_CANrx_t* rx_arr;
    CO_CANrxMsg_t* can_msg;

    // timer variables
    itimerspec* time_out;
    int timer_fd;
};

#endif
