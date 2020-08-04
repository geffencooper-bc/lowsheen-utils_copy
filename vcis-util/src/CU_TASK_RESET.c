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


#include "CU.h"
#include "CANopen.h"
#include <stdio.h>

CU_TASK_STATUS CU_TASK_RESET_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CO_CANtx_t tx_msg;   
    /* 
     * 3 MSB bits for SocketCAN
     * ExtID    = 0x80000000
     * RTR      = 0x40000000
     * ERR      = 0x20000000
     * Special Message: 0xAC1DC0DE (used as signal "ACID CODE")
     */

    tx_msg.ident = 0xAC1DC0DE;  // little endian
    tx_msg.DLC = 0x02;
    tx_msg.data[0] = 0x00;  // config mode
    tx_msg.data[1] = 0x01;  // reset
    tx_msg.CANdriverState = NULL;

    printf("Resetting Interface\r\n");
     
    return (CO_CANsend(CO->CANmodule[0], &tx_msg) == CO_ERROR_NO) ? CU_TASK_STATUS_DONE : CU_TASK_STATUS_ERROR;
}

CU_TASK_STATUS CU_TASK_STATE_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{

    printf("Setting Node 0x%02X to State: 0x%02X\r\n", cmd->nodeID, cmd->options);
    CO_sendNMTcommand(CO, (uint8_t)cmd->options, cmd->nodeID);  // Send everyone to pre-operational

    return CU_TASK_STATUS_DONE;
}

CU_TASK_STATUS CU_TASK_TEST_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CO_CANtx_t tx_msg;   

    /* 
     * 3 MSB bits for SocketCAN
     * 1 = Ext Identifier 
     * 2 = RTR bit
     * 3 = ? (used as signal "ACID CODE")
     *
     * test execution:
     * typedef enum {
     *     TEST_EXCEPTION_USAGE_FAULT = 1,
     *     TEST_EXCEPTION_BUS_FAULT = 2,
     *     TEST_EXCEPTION_FPU_FAULT = 3,
     *     TEST_EXCEPTION_MEM_FAULT = 4,
     *     TEST_EXCEPTION_HARD_FAULT = 5,
     *     TEST_EXCEPTION_WWDG = 6,
     *     TEST_EXCEPTION_IWDG = 7,
     * } TEST_EXCEPTION;
     */

    tx_msg.ident = 0xAC1DC0DE;  // little endian
    tx_msg.DLC = 0x02;
    tx_msg.data[0] = 0x01;  // test mode
    tx_msg.data[1] = (uint8_t)cmd->options;  // test type
    tx_msg.CANdriverState = NULL;

    printf("Sending Test Command: 0x%02X\r\n", cmd->options);
     
    return (CO_CANsend(CO->CANmodule[0], &tx_msg) == CO_ERROR_NO) ? CU_TASK_STATUS_DONE : CU_TASK_STATUS_ERROR;
}

CU_TASK_STATUS CU_TASK_ESTOP_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CO_CANtx_t tx_msg;   
    /* 
     * 3 MSB bits for SocketCAN
     * ExtID    = 0x80000000
     * RTR      = 0x40000000
     * ERR      = 0x20000000
     * Special Message: 0xAC1DC0DE (used as signal "ACID CODE")
     */

    tx_msg.ident = 0xAC1DC0DE;  // little endian
    tx_msg.DLC = 0x02;
    tx_msg.data[0] = 0x02;  // estop control
    tx_msg.data[1] = (uint8_t)cmd->options;  // estop assert
    tx_msg.CANdriverState = NULL;

    printf("Sending %s ESTOP Command\r\n", cmd->options == 2 ? "Assert" : "De-Assert");
     
    return (CO_CANsend(CO->CANmodule[0], &tx_msg) == CO_ERROR_NO) ? CU_TASK_STATUS_DONE : CU_TASK_STATUS_ERROR;
}