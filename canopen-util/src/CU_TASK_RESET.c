

#include "CU.h"
#include "CANopen.h"
#include <stdio.h>

 void CU_TASK_RESET_prepare(CU_TaskDetails *cmd)
 {


 }

CU_TASK_STATUS CU_TASK_RESET_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    return CU_TASK_STATUS_DONE;
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
