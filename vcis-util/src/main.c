//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions
//==================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "CANopen.h"
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include "CO_Linux_threads.h"
#include "CU.h"

#define NSEC_PER_SEC            (1000000000)    /* The number of nanoseconds per second. */
#define NSEC_PER_MSEC           (1000000)       /* The number of nanoseconds per millisecond. */
#define TMR_TASK_INTERVAL_NS    (1000000)       /* Interval of taskTmr in nanoseconds */
#define TMR_TASK_OVERFLOW_US    (5000)          /* Overflow detect limit for taskTmr in microseconds */
#define INCREMENT_1MS(var)      (var++)         /* Increment 1ms variable in taskTmr */

pthread_mutex_t             CO_CAN_VALID_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t                   CO_rx_thread_id;
volatile uint16_t           CO_timer1ms = 0U;   /* Global variable increments each millisecond. */
CO_NMT_reset_cmd_t          reset = CO_RESET_NOT;

const int8_t BCMnodeID = 0x48;

/* CANopenNode main thread callback */
void threadMain_callback(void *arg)
{
    /* nothing to do */
}

/* receive thread */
void *threadRx(void *arg)
{
    CANrx_threadTmr_init(1);

    while(reset != CO_RESET_QUIT)
    {
        CANrx_threadTmr_process();
    }

    return NULL;
}

static void canopen_init(CU_TaskDetails *args)
{
    CO_ReturnError_t err;

    if(args->interfaceName == NULL)
    {
        printf("Missing Socket CAN Interface. Please specify can0, can1, etc.\r\n");
        exit(-1);
    }

    unsigned int interface_index = if_nametoindex(args->interfaceName);
    uintptr_t can_interface = interface_index;  // interface dependent device selection

    /* initialize CANopen */
    err = CO_init((void *)can_interface, BCMnodeID /* NodeID */, 250 /* bit rate */);  
    if(err != CO_ERROR_NO)
    {
        printf("Unable to Initialize Socket CAN Interface: %s\r\n", args->interfaceName);
        exit(-1);
    }

    /* start CAN */
    CO_CANsetNormalMode(CO->CANmodule[0]);

    threadMain_init(threadMain_callback, NULL);

    pthread_create(&CO_rx_thread_id, NULL, threadRx, NULL);
}

static void canopen_close(void)
{
    CO_delete((void*) 0/* CAN module address */);
}

int main(int argc, char *argv[])
{
    struct timespec sleepTime;
    uint16_t timer1msPrevious = 0;
    CU_TASK_STATUS status;
    CU_TaskDetails task_details;

    printf("canopen-util application\r\n");

    CU_COMMAND_parseArgs(&task_details, argc, argv);

    canopen_init(&task_details);

    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 1000000;

    while(reset != CO_RESET_QUIT)
    {
        uint16_t timer1msCopy, timer1msDiff;

        timer1msCopy = CO_timer1ms;
        timer1msDiff = timer1msCopy - timer1msPrevious;
        timer1msPrevious = timer1msCopy;

        threadMain_process(&reset);

        status = CU_TASK_update(&task_details, timer1msDiff);

        if(status != CU_TASK_STATUS_CONTINUE)
        {
            printf("Done.\r\n");
            return (status == CU_TASK_STATUS_DONE) ? 0 : -1;
        }

        nanosleep(&sleepTime, NULL);
        CO_timer1ms += 1;
    }

    canopen_close();

    return 0;
}

/* Helper functions ***********************************************************/
void CO_errExit(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/* send CANopen generic emergency message */
void CO_error(const uint32_t info) {
    CO_errorReport(CO->em, CO_EM_GENERIC_SOFTWARE_ERROR, CO_EMC_SOFTWARE_INTERNAL, info);
    fprintf(stderr, "canopend generic error: 0x%X\n", info);
}
