

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

 void CU_TASK_PROGRAM_prepare(CU_TaskDetails *cmd)
 {
    FILE *fp;
    size_t file_size;
    cmd->step = PROGRAM_STEP_REQ_SW_VERSION;
    cmd->index = 0;

    /* load program */
    fp = fopen(cmd->programFilename, "r");

    /* determine size */
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);

    if(file_size > CU_TASK_PROGRAM_MAX_FILE_SIZE)
    {
        printf("file size exceeds expected amount. Read: %uz bytes\r\n", (unsigned int)file_size);
        cmd->step = PROGRAM_STEP_OPERATION_ERROR;
        fclose(fp);
        return;
    }
    else
    {
        cmd->programDataLen = (uint32_t)file_size;
        printf("writing %s (%uz bytes)\r\n", (char *)cmd->programFilename, (unsigned int)file_size);
    }

    rewind(fp);

    cmd->programData = malloc(file_size);

    if(cmd->programData == NULL)
    {
        printf("Unable to allocate memory for writing file.\r\n");
        fclose(fp);
        return;
    }

    fgets((char *)cmd->programData, (int)file_size, fp);
 
    fclose(fp);

    printf("Begin PROGRAM_STEP_REQ_SW_VERSION\r\n");
 }

CU_TASK_STATUS CU_TASK_PROGRAM_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CU_TASK_STATUS status = CU_TASK_STATUS_ERROR;
    static uint32_t sdo_value;
    static uint32_t sdo_len;
    uint32_t abort_code;
 
    switch(cmd->step)
    {
        case PROGRAM_STEP_REQ_SW_VERSION:

            status = CU_TASK_SDO_uploadTask(cmd, time_diff_1ms, 0x49, 0x1F56, 0x00, (uint8_t *)&sdo_value, sizeof(sdo_value), &sdo_len, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                /* verify correct software version */
                cmd->step = PROGRAM_STEP_STOP_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_STOP_PROGRAM\r\n");
            }
            break;

        case PROGRAM_STEP_STOP_PROGRAM:

            sdo_value = 0;

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, 0x49, 0x1F51, 0x01, (uint8_t *)&sdo_value, sizeof(sdo_value), &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_CLEAR_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_CLEAR_PROGRAM\r\n");
            }
            break;

        case PROGRAM_STEP_CLEAR_PROGRAM:

            sdo_value = 3;

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, 0x49, 0x1F51, 0x01,  (uint8_t *)&sdo_value, sizeof(sdo_value), &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_DOWNLOAD_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_DOWNLOAD_PROGRAM\r\n");
            }
            break;

        case PROGRAM_STEP_DOWNLOAD_PROGRAM:

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, 0x49, 0x1F50, 0x01, cmd->programData, cmd->programDataLen, &abort_code);

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_FLASH_SUCCESSFUL;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_FLASH_SUCCESSFUL\r\n");
            }
            break;

        case PROGRAM_STEP_FLASH_SUCCESSFUL:

            status = CU_TASK_SDO_uploadTask(cmd, time_diff_1ms, 0x49, 0x1F57, 0x01,  (uint8_t *)&sdo_value, sizeof(sdo_value),  &sdo_len, &abort_code);    

            if(status == CU_TASK_STATUS_DONE)
            {
                cmd->step = PROGRAM_STEP_START_PROGRAM;
                status = CU_TASK_STATUS_CONTINUE;
                printf("Begin PROGRAM_STEP_START_PROGRAM\r\n");
            }
            break;

        case PROGRAM_STEP_START_PROGRAM:

            sdo_value = 1;

            status = CU_TASK_SDO_downloadTask(cmd, time_diff_1ms, 0x49, 0x1F51, 0x01, (uint8_t *)&sdo_value, sizeof(sdo_value), &abort_code);    

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
