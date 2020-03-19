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
#include "CO_OD_storage.h"
#include "CO_Linux_tasks.h"
#include "CO_time.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
 
pthread_mutex_t             CO_CAN_VALID_mtx = PTHREAD_MUTEX_INITIALIZER;


/* Global variable increments each millisecond. */
volatile uint16_t           CO_timer1ms = 0U;

/* Signal handler */
volatile sig_atomic_t CO_endProgram = 0;
static void sigHandler(int sig) {
    CO_endProgram = 1;
}

/**
 * User-defined CAN base structure, passed as argument to CO_init.
 */
struct CANbase {
    uintptr_t baseAddress;  /**< Base address of the CAN module */
};

void canopen_init(int argc, char *argv[])
{
 
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
  
    /* Catch signals SIGINT and SIGTERM */
    if(signal(SIGINT, sigHandler) == SIG_ERR)
        CO_errExit("Program init - SIGINIT handler creation failed");
    if(signal(SIGTERM, sigHandler) == SIG_ERR)
        CO_errExit("Program init - SIGTERM handler creation failed");
 

    while(reset != CO_RESET_APP){
/* CANopen communication reset - initialize CANopen objects *******************/
        CO_ReturnError_t err;
        uint16_t timer1msPrevious;

        /* disable CAN and CAN interrupts */
        struct CANbase canBase = {
            .baseAddress = 0u,  /* CAN module address */
        };


        int can_fd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
        struct ifreq ifr;
        strcpy(ifr.ifr_name, "can0" );
        ioctl(can_fd, SIOCGIFINDEX, &ifr);

        printf("Found can0 interface as %d\r\n", ifr.ifr_ifindex);

        close(can_fd);

        canBase.baseAddress = ifr.ifr_ifindex;

        /* initialize CANopen */
        err = CO_init(&canBase, 0x49/* NodeID */, 250 /* bit rate */);  
        if(err != CO_ERROR_NO){
            CO_errorReport(CO->em, CO_EM_MEMORY_ALLOCATION_ERROR, CO_EMC_SOFTWARE_INTERNAL, err);  
            while(1);
        }


        /* Configure Timer interrupt function for execution every 1 millisecond */


        /* Configure CAN transmit and receive interrupt */


        /* start CAN */
        CO_CANsetNormalMode(CO->CANmodule[0]);

        reset = CO_RESET_NOT;
        timer1msPrevious = CO_timer1ms;

        while(reset == CO_RESET_NOT){
/* loop for normal program execution ******************************************/
            uint16_t timer1msCopy, timer1msDiff;

            timer1msCopy = CO_timer1ms;
            timer1msDiff = timer1msCopy - timer1msPrevious;
            timer1msPrevious = timer1msCopy;


            /* CANopen process */
            reset = CO_process(CO, timer1msDiff, NULL);

            /* Process EEPROM */
        }
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
