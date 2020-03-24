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
/* Global variable increments each millisecond. */
volatile uint16_t           CO_timer1ms = 0U;
 

/**
 * User-defined CAN base structure, passed as argument to CO_init.
 */


void threadMain_callback(void *arg)
{
 
}

void * threadRx(void *arg)
{
    CANrx_threadTmr_init(1);

    while(1)
    {
        CANrx_threadTmr_process();
    }


    return NULL;
}

uint16_t idx = 1501;
uint8_t subidx = 1;

uint8_t dataTx[256];
uint32_t dataTxLen = sizeof(dataTx);

uint8_t blockTransferEnable = 1;
uint8_t block_transfer = 1;

int block_stage = 0;

int BCMnodeID = 0x48;
int VCUnodeID = 0x49;

void canopen_init(int argc, char *argv[])
{

    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;

    CU_TASK_init();
  
 
/* CANopen communication reset - initialize CANopen objects *******************/
    CO_ReturnError_t err;

    unsigned int interface_index = if_nametoindex("can0");

    uintptr_t can_interface = interface_index;

    /* initialize CANopen */
    err = CO_init((void *)can_interface, BCMnodeID /* NodeID */, 250 /* bit rate */);  
    if(err != CO_ERROR_NO){
        CO_errorReport(CO->em, CO_EM_MEMORY_ALLOCATION_ERROR, CO_EMC_SOFTWARE_INTERNAL, err);  
        while(1);
    }


    /* Configure CAN transmit and receive interrupt */
    

    /* start CAN */
    CO_CANsetNormalMode(CO->CANmodule[0]);

    reset = CO_RESET_NOT;

    threadMain_init(threadMain_callback, NULL);

    pthread_create(&CO_rx_thread_id, NULL, threadRx, NULL);

    uint16_t timer1msPrevious = 0;

    while(reset == CO_RESET_NOT)
    {
        uint16_t timer1msCopy, timer1msDiff;

        timer1msCopy = CO_timer1ms;
        timer1msDiff = timer1msCopy - timer1msPrevious;
        timer1msPrevious = timer1msCopy;

        threadMain_process(&reset);

        if(CU_TASK_update(timer1msDiff) != CU_TASK_STATUS_CONTINUE)
        {
            printf("Done.\r\n");
            return;
        }
        
        struct timespec sleepTime;

        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 1000000;
        nanosleep(&sleepTime, NULL);
        CO_timer1ms += 1;

    }



/* program exit ***************************************************************/
    /* stop threads */


    /* delete objects from memory */
    CO_delete((void*) 0/* CAN module address */);
 
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
    printf("canopen-util application\r\n");

    for(size_t i = 0; i < sizeof(dataTx); i += 1)
    {
        dataTx[i] = (uint8_t)i;
    }
    
    canopen_init(argc, argv);

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
