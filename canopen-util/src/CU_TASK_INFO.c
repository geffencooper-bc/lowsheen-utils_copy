

#include "CU.h"
#include <stddef.h>
#include <stdio.h>

typedef struct
{
    const char *objectName;
    uint16_t sdoIndex;
    uint8_t sdoSubIndex;
}CU_TASK_INFO_Entries;

static const CU_TASK_INFO_Entries scan_entries[] = 
{
    {"Device Type", 0x1000, 0x00},
    {"Error Register", 0x1001, 0x00},
    {"Identity", 0x1018, 0x00},
    {"Program Software Identification", 0x1F56, 0x00},
    {"Flash Status Identification", 0x1F57, 0x00},
};

static const size_t scan_entries_size = sizeof(scan_entries) / sizeof(CU_TASK_INFO_Entries);

uint32_t scan_entries_data[sizeof(scan_entries) / sizeof(CU_TASK_INFO_Entries)];


void CU_TASK_INFO_prepare(CU_TaskDetails *cmd)
{
    cmd->step = 0;      // track request or wait for response
    cmd->index = 0;     // track entries being read
}

CU_TASK_STATUS CU_TASK_INFO_update(CU_TaskDetails *cmd, uint32_t time_diff_1ms)
{
    CU_TASK_STATUS status;
    uint32_t data_read_len;
    uint32_t sdo_abort_code;

    if(cmd->index >= scan_entries_size)
    {
        return CU_TASK_STATUS_DONE;
    }

    // alternate between requesting a new SDO upload and waiting for results
    if(cmd->step == 0)
    {
        cmd->sdoIndex = scan_entries[cmd->index].sdoIndex;
        cmd->sdoSubIndex = scan_entries[cmd->index].sdoSubIndex;

        CU_TASK_SDO_upload(cmd, (uint8_t *)&scan_entries_data[cmd->index], sizeof(uint32_t));
        cmd->step = 1;
        return CU_TASK_STATUS_CONTINUE;
    }
    else
    {
        status = CU_TASK_SDO_uploadUpdate(cmd, time_diff_1ms, &data_read_len, &sdo_abort_code);

        switch(status)
        {
            case CU_TASK_STATUS_CONTINUE:
                return CU_TASK_STATUS_CONTINUE;
            case CU_TASK_STATUS_DONE:

                printf("%s: %04X\r\n", scan_entries[cmd->index].objectName, scan_entries_data[cmd->index]);
            
                cmd->index += 1;
                cmd->step = 0;

                return CU_TASK_STATUS_CONTINUE;
            case CU_TASK_STATUS_TIMEOUT:
                printf("Timeout Error attempting to read info entries\r\n");
                return status;
            case CU_TASK_STATUS_ABORT:
                printf("Abort Error attempting to read info entries. Abort Code: %08X\r\n", sdo_abort_code);
                return status;
            case CU_TASK_STATUS_ERROR:
            default:
                printf("Unknown Error attempting to read info entries\r\n");
                return status;

        }

    }
  
    return CU_TASK_STATUS_DONE;
}
