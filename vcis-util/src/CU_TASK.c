
#include "CU.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>      /* printf, fopen */
#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <time.h>

typedef struct
{
    char *taskName;
    void (*initCallback)(CU_TaskDetails *cmd);
    CU_TASK_STATUS (*updateCallback)(CU_TaskDetails *cmd, uint32_t time_diff_1ms);
}CU_TASK_Entry;

// task entries list
static const CU_TASK_Entry taskEntries[] = 
{ 
    {"info", CU_TASK_INFO_prepare, CU_TASK_INFO_update},
    {"program", CU_TASK_PROGRAM_prepare, CU_TASK_PROGRAM_update},
    {"reset", CU_TASK_RESET_prepare, CU_TASK_RESET_update},
    {"state", NULL, CU_TASK_STATE_update},
    {"test", NULL, CU_TASK_TEST_update},
} ;

// size of entries
static const size_t taskEntriesSize = sizeof(taskEntries) / sizeof(CU_TASK_Entry);

// keep track of entries and options passed during command line
typedef struct
{
    CU_TASK_STATUS state;
    int32_t option;
}CU_TASK_Requested;

// use to determine which tasks to execute (should be zeroed out at runtime by the c starndard)
static CU_TASK_Requested taskEntriesRequested[sizeof(taskEntries) / sizeof(CU_TASK_Entry)];

// enable tasks to execute
void CU_TASK_addTask(const char *request_task_name, int32_t option)
{
    size_t i;

    for(i = 0; i < taskEntriesSize; i+= 1)
    {
        if(strcmp(taskEntries[i].taskName, request_task_name) == 0)
        {
            if(taskEntriesRequested[i].state != CU_TASK_STATUS_PENDING)
            {
               // printf("Adding Task: %s\r\n", request_task_name);
                taskEntriesRequested[i].state = CU_TASK_STATUS_PENDING; 
                taskEntriesRequested[i].option = option;        
            }
            return;
        }
    }

    printf("Invalid argument passed: %s\r\n", request_task_name);
    exit(-1);
}


CU_TASK_STATUS CU_TASK_update(CU_TaskDetails *task_details, uint32_t time_diff_1ms)
{
    size_t i;
    CU_TASK_STATUS status;

    // cycle through all entries, execute only one until its done, 
    // order indicates priority of execution
    for(i = 0; i < taskEntriesSize; i += 1)
    {
        // prepare task to run
        if(taskEntriesRequested[i].state == CU_TASK_STATUS_PENDING)
        {
            if(taskEntries[i].initCallback != NULL)
            {
                taskEntries[i].initCallback(task_details);
                taskEntriesRequested[i].state = CU_TASK_STATUS_CONTINUE;
            }
            return CU_TASK_STATUS_CONTINUE;
        }
        // update task until complete, or an error occurs
        else if(taskEntriesRequested[i].state == CU_TASK_STATUS_CONTINUE)
        {
            status = taskEntries[i].updateCallback(task_details, time_diff_1ms);

            // mark task as done, will move to next on the list next time the function is executed
            if(status == CU_TASK_STATUS_DONE)
            {
                taskEntriesRequested[i].state = CU_TASK_STATUS_DONE;
                return CU_TASK_STATUS_DONE;
            }
            // task is running, but not done
            else if(status == CU_TASK_STATUS_CONTINUE)
            {
                return CU_TASK_STATUS_CONTINUE;
            }
            // an error occurred,
            else
            {
                taskEntriesRequested[i].state = CU_TASK_STATUS_ERROR;
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


uint8_t CU_TASK_setTimeout(CU_TaskDetails *cmd, float time_sec)
{
    struct timespec current_time={0,0};
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    cmd->timeoutSec = (float)current_time.tv_sec + (float)current_time.tv_nsec / 1e9f + time_sec;

    return 0;
}

uint8_t CU_TASK_isTimeout(CU_TaskDetails *cmd)
{
    struct timespec current_time={0,0};
    clock_gettime(CLOCK_MONOTONIC, &current_time);

   float new_time = (float)current_time.tv_sec + (float)current_time.tv_nsec / 1e9f;

   return (new_time > cmd->timeoutSec);

}