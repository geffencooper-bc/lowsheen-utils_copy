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
#include <stdio.h>
#include <stdlib.h>

#define CU_TASK_PROGRAM_MAX_FILE_SIZE (4 * 1000 * 1024)     // arbitrary limit

typedef enum
{
    PROGRAM_STEP_STOP,                  // switch all nodes into the stopped state
    PROGRAM_STEP_STOP_WAIT,
    PROGRAM_STEP_PRE_OPERATIONAL,       // switch desired node into the pre-operational state
    PROGRAM_STEP_PRE_OPERATIONAL_WAIT,
    PROGRAM_STEP_REQ_SW_VERSION,        // 1F56 REQUEST_PROGRAM_SOFTWARE_IDENTIFICATION
    PROGRAM_STEP_STOP_PROGRAM,          // 1F51 = 0 PROGRAM CONTROL (STOP)
    PROGRAM_STEP_CLEAR_PROGRAM,         // 1F51 = 3 PROGRAM CONTROL (CLEAR)
    PROGRAM_STEP_DOWNLOAD_PROGRAM,      // 1F50 = binary, PROGRAM DATA
    PROGRAM_STEP_FLASH_SUCCESSFUL,      // 1F57 FLASH SOFTWARE IDENTIFICATION
    PROGRAM_STEP_START_PROGRAM,         // 1F51 = 1 PROGRAM CONTROL (START)
    PROGRAM_STEP_OPERATIONAL,           // switch desired node into operational state
    PROGRAM_STEP_OPERATIONAL_WAIT,
    PROGRAM_STEP_OPERATION_ERROR,
}PROGRAM_STEP;

#define PROGRAM_STEP_DELAY_MS (500)

 void CU_TASK_PROGRAM_prepare(CU_TaskDetails *cmd)
 {
    FILE *fp;
    size_t file_size;
    cmd->step = PROGRAM_STEP_REQ_SW_VERSION;
    cmd->index = 0;

    /* load program */
    fp = fopen(cmd->programFilename, "r");

    if(fp == NULL)
    {
        printf("Unable to open file %s\r\n", cmd->programFilename);
        exit(-1);
    }

    /* determine size */
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);

    if(file_size > CU_TASK_PROGRAM_MAX_FILE_SIZE)
    {
        printf("file size exceeds expected amount. Read: %d KB (%u bytes)\r\n", (unsigned int)file_size / 1024, (unsigned int)file_size);
        cmd->step = PROGRAM_STEP_OPERATION_ERROR;
        fclose(fp);
        return;
    }
    else
    {
        cmd->programDataLen = (uint32_t)file_size;
        printf("writing %s - %d KB (%u bytes)\r\n", (char *)cmd->programFilename, (unsigned int)file_size / 1024, (unsigned int)file_size); 
    }

    rewind(fp);

    cmd->programData = malloc(file_size);

    if(cmd->programData == NULL)
    {
        printf("Unable to allocate memory for writing file.\r\n");
        fclose(fp);
        return;
    }

    fread((char *)cmd->programData, 1, (int)file_size, fp);
 
    fclose(fp);

    printf("Begin PROGRAM_STEP_REQ_SW_VERSION\r\n");
 }

CU_TASK_STATUS CU_TASK_PROGRAM_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CU_TASK_STATUS status = CU_TASK_STATUS_ERROR;
    static uint32_t sdo_value;
    static uint32_t sdo_len;
    uint32_t abort_code;

    if(!(CU_TASK_isTimeout(cmd)))
    {
        return CU_TASK_STATUS_CONTINUE;
    }
 
    switch(cmd->step)
    {
        case PROGRAM_STEP_REQ_SW_VERSION:

            status = CU_TASK_SDO_uploadTask(cmd, time_diff_1ms, cmd->nodeID, 0x1F56, cmd->programID, (uint8_t *)&sdo_value, sizeof(sdo_value), &sdo_len, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                /* verify correct software version */
                cmd->step = PROGRAM_STEP_STOP_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_STOP_PROGRAM\r\n");
                CU_TASK_setTimeout(cmd, .3f);
            }
            break;

        case PROGRAM_STEP_STOP_PROGRAM:

            sdo_value = 0;

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, cmd->nodeID, 0x1F51, cmd->programID, (uint8_t *)&sdo_value, 0x02, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_CLEAR_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_CLEAR_PROGRAM\r\n");
                CU_TASK_setTimeout(cmd, .5f);
            }
            break;

        case PROGRAM_STEP_CLEAR_PROGRAM:

            sdo_value = 3;

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, cmd->nodeID, 0x1F51, cmd->programID,  (uint8_t *)&sdo_value, 0x02, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_DOWNLOAD_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_DOWNLOAD_PROGRAM\r\n");
                CU_TASK_setTimeout(cmd, 0.5f);
            }
            break;

        case PROGRAM_STEP_DOWNLOAD_PROGRAM:

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, cmd->nodeID, 0x1F50, cmd->programID, cmd->programData, cmd->programDataLen, &abort_code);

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_FLASH_SUCCESSFUL;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_FLASH_SUCCESSFUL\r\n");
                CU_TASK_setTimeout(cmd, 2.0f);
            }
            break;

        case PROGRAM_STEP_FLASH_SUCCESSFUL:

            status = CU_TASK_SDO_uploadTask(cmd, time_diff_1ms, cmd->nodeID, 0x1F57, cmd->programID,  (uint8_t *)&sdo_value, sizeof(sdo_value),  &sdo_len, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_START_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_START_PROGRAM\r\n");
                CU_TASK_setTimeout(cmd, .5f);
            }
            break;

        case PROGRAM_STEP_START_PROGRAM:

            sdo_value = 1;

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, cmd->nodeID, 0x1F51, cmd->programID, (uint8_t *)&sdo_value, 0x02, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_CLEAR_PROGRAM;
                status = CU_TASK_STATUS_DONE;
            }
            break;

        default:
            break;
    }

    if(status != CU_TASK_STATUS_CONTINUE)
    {
        if(status == CU_TASK_STATUS_DONE)
        {
            printf("Successfully Flashed Firmware\r\n");
        }
        else
        {
            printf("Failed to Flash Firmware. Abort Code: %08X\r\n", abort_code);
        }
        // either done or error, destroy allocated memory
        if(cmd->programData != NULL)
        {
            free(cmd->programData);
            cmd->programData = NULL;
            cmd->programDataLen = 0;
        }
    }

    return status;
}
