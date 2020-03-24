
#include "CU.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>      /* printf, fopen */
#include <stdlib.h>     /* exit, EXIT_FAILURE */


typedef struct
{
    char *taskName;
    void (*prepareCallback)(CU_TaskDetails *cmd);
    CU_TASK_STATUS (*prepareUpdate)(CU_TaskDetails *cmd, uint32_t time_diff_1ms);
}CU_TASK_Entry;

// task entries list
static const CU_TASK_Entry g_taskEntries[] = 
{ 
    {"info", CU_TASK_INFO_prepare, CU_TASK_INFO_update},
    {"program", CU_TASK_PROGRAM_prepare, CU_TASK_PROGRAM_update},
    {"reset", CU_TASK_RESET_prepare, CU_TASK_RESET_update},
} ;

// size of entries
static const size_t g_taskEntriesSize = sizeof(g_taskEntries) / sizeof(CU_TASK_Entry);

// keep track of entries and options passed during command line
typedef struct
{
    CU_TASK_STATUS state;
    int32_t option;
}CU_TASK_Requested;

// use to determine which tasks to execute (should be zeroed out at runtime by the c starndard)
static CU_TASK_Requested g_taskEntriesRequested[sizeof(g_taskEntries) / sizeof(CU_TASK_Entry)];

static CU_TaskDetails task_details;

void CU_TASK_init(void)
{
    task_details.nodeID = 0x49;
}

// enable tasks to execute
void CU_TASK_addTask(const char *request_task_name, int32_t option)
{
    size_t i;

    for(i = 0; i < g_taskEntriesSize; i+= 1)
    {
        if(strcmp(g_taskEntries[i].taskName, request_task_name) == 0)
        {
            g_taskEntriesRequested[i].state = CU_TASK_STATUS_READY;
            return;
        }
    }

    printf("Invalid argument passed: %s\r\n", request_task_name);
    exit(-1);
}


CU_TASK_STATUS CU_TASK_update(uint32_t time_diff_1ms)
{
    size_t i;
    CU_TASK_STATUS status;

    // cycle through all entries, execute only one until its done, 
    // order indicates priority of execution
    for(i = 0; i < g_taskEntriesSize; i += 1)
    {
        // prepare task to run
        if(g_taskEntriesRequested[i].state == CU_TASK_STATUS_READY)
        {
            g_taskEntries[i].prepareCallback(&task_details);
            g_taskEntriesRequested[i].state = CU_TASK_STATUS_CONTINUE;
            return CU_TASK_STATUS_CONTINUE;
        }
        // update task until complete, or an error occurs
        else if(g_taskEntriesRequested[i].state == CU_TASK_STATUS_CONTINUE)
        {
            status = g_taskEntries[i].prepareUpdate(&task_details, time_diff_1ms);

            // mark task as done, will move to next on the list next time the function is executed
            if(status == CU_TASK_STATUS_DONE)
            {
                g_taskEntriesRequested[i].state = CU_TASK_STATUS_DONE;
                return CU_TASK_STATUS_CONTINUE;
            }
            // task is running, but not done
            else if(status == CU_TASK_STATUS_CONTINUE)
            {
                return CU_TASK_STATUS_CONTINUE;
            }
            // an error occurred,
            else
            {
                g_taskEntriesRequested[i].state = CU_TASK_STATUS_ERROR;
;
                return CU_TASK_STATUS_ERROR;
            }
        }
        else
        {
            continue;
        }
    }

    return CU_TASK_STATUS_DONE;
}