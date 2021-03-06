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

#include <stdint.h>


// canopen-util --interface can0 --node 0x49 --program_file ./curtis_trac.bin --program 1 --info --reset

typedef struct
{
    uint8_t nodeID;
    uint16_t sdoIndex;
    uint8_t sdoSubIndex;
    char *interfaceName;
    uint8_t programID;
    char *programFilename;
    uint8_t *programData;
    uint32_t programDataLen;
    uint8_t sdo_process;
    uint8_t step;
    uint8_t index;
    int32_t options;
    float timeoutSec;
}CU_TaskDetails;

typedef enum
{   
    CU_TASK_STATUS_NO_ERROR,
    CU_TASK_STATUS_PENDING,
    CU_TASK_STATUS_CONTINUE,
    CU_TASK_STATUS_DONE,
    CU_TASK_STATUS_TIMEOUT,
    CU_TASK_STATUS_ABORT,
    CU_TASK_STATUS_ERROR,
}CU_TASK_STATUS;

/* parse command line entries */
void CU_COMMAND_parse(CU_TaskDetails *cmd, int argc, char *argv[]);

/* task invocation */
void CU_TASK_addTask(const char *request_task_name, int32_t option);
CU_TASK_STATUS CU_TASK_update(CU_TaskDetails *task_details, uint32_t time_diff_1ms);
 
/* sdo transfer helpers */
CU_TASK_STATUS CU_TASK_SDO_download(CU_TaskDetails *cmd, uint8_t *data, uint32_t data_len);
CU_TASK_STATUS CU_TAKS_SDO_downloadUpdate(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint32_t *abort_code);
CU_TASK_STATUS CU_TASK_SDO_upload(CU_TaskDetails *cmd, uint8_t *data, uint32_t data_len);
CU_TASK_STATUS CU_TASK_SDO_uploadUpdate(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint32_t *data_read_len, uint32_t *abort_code);

CU_TASK_STATUS CU_TASK_SDO_downloadTask(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint8_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint32_t data_len, uint32_t *abort_code);
CU_TASK_STATUS CU_TASK_SDO_uploadTask(CU_TaskDetails *cmd, uint32_t time_diff_1ms, uint8_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint32_t data_len, uint32_t *data_read_len, uint32_t *abort_code);

/* request and display info task */
void CU_TASK_INFO_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_INFO_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* program update task */
void CU_TASK_PROGRAM_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_PROGRAM_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* request exit back to default application*/
void CU_TASK_RESET_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_RESET_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* change Node state */
void CU_TASK_STATE_prepare(CU_TaskDetails *cmd);
CU_TASK_STATUS CU_TASK_STATE_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* send test command */
CU_TASK_STATUS CU_TASK_TEST_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

/* send estop control command */
CU_TASK_STATUS CU_TASK_ESTOP_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms);

void CU_COMMAND_parseArgs(CU_TaskDetails *cmd, int argc, char *argv[]);

uint8_t CU_TASK_setTimeout(CU_TaskDetails *cmd, float time_sec);
uint8_t CU_TASK_isTimeout(CU_TaskDetails *cmd);

