

#include "CU.h"
#include "CANopen.h"
#include <stdio.h>

 void CU_TASK_RESET_prepare(CU_TaskDetails *cmd)
 {


 }

CU_TASK_STATUS CU_TASK_RESET_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CO_CANtx_t tx_msg;   

    tx_msg.ident = (0x3 << 29);
    tx_msg.DLC = 0x08;
    tx_msg.data[0] = 0x0D;
    tx_msg.data[1] = 0x0E;
    tx_msg.data[2] = 0x0A;
    tx_msg.data[3] = 0x0D;
    tx_msg.data[4] = 0x0B;
    tx_msg.data[5] = 0x0E;
    tx_msg.data[6] = 0x0E;
    tx_msg.data[7] = 0x0F;
    tx_msg.CANdriverState = NULL;

    printf("Resetting Interface\r\n");
     
    return (CO_CANsend(CO->CANmodule[0], &tx_msg) == CO_ERROR_NO) ? CU_TASK_STATUS_DONE : CU_TASK_STATUS_ERROR;
}

 void CU_TASK_STATE_prepare(CU_TaskDetails *cmd)
 {

 }

CU_TASK_STATUS CU_TASK_STATE_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{

    printf("Setting Node 0x%02X to State: 0x%02X\r\n", cmd->nodeID, cmd->options);
    CO_sendNMTcommand(CO, (uint8_t)cmd->options, cmd->nodeID);  // Send everyone to pre-operational

    return CU_TASK_STATUS_DONE;
}
