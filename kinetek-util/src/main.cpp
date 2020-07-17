// main file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include "CO_driver.h"
#include "SocketCanHelper.h"
using namespace std;

// identifier passed with the callback function
int id = 21;

// callback function passed into buffer init
void call_back(void* msg, const CO_CANrxMsg_t* can_msg)
{
    printf("in call back. Obj: %i\t Id: %i    data[0]:%i\n", (uintptr_t)msg, can_msg->ident, can_msg->data[0]);
}

int main()
{
    SocketCanHelper sc;
    sc.init_socketcan("can0");
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sc.send_frame(0x001, data, 8);
    /*====================================
    // empty objects
    CO_CANmodule_t cm;
    CO_CANrxMsg_t can_msg;

    // lists of transmit and receive buffers
    CO_CANrx_t rxArr[1];
    CO_CANtx_t txArr[1];

    // interface init, make sure to do ip link before this
    unsigned int if_index = if_nametoindex("can0"); // get ifindex from name
    uintptr_t can_interface = if_index;

    cout << "if index: " << can_interface << endl;

    // can module object init
    int err =  CO_CANmodule_init(&cm, (void*)if_index, rxArr, 1, txArr, 1, 250);
    cout << "init module. Error: " << err << " Interface count: " << cm.CANinterfaceCount << " " << endl;
    CO_CANsetNormalMode(&cm);

    // // receive
    // err = CO_CANrxBufferInit(&cm, 0, 0x080, 0x7FF, 0, (void*)id, call_back);
    // cout << "init rx buffer. Error: " << err << endl;
    // printf("error %i\n",CO_CANrxWait(&cm, -1, &can_msg)); 


    // send
    uint8_t data1[8] = {0x1D, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t data2[8] = {0x1D, 0xFF, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00};
    printf("init tx buffer\n");
    CO_CANtx_t* tx1 = CO_CANtxBufferInit(&cm, 0, 0x001, 0, 5, false);
    CO_CANtx_t* tx2 = CO_CANtxBufferInit(&cm, 0, 0x001, 0, 5, false);

    printf("Sending Message\n");
    memcpy(tx1->data, data1, 8);
    memcpy(tx2->data, data2, 8);
	int err1 = CO_CANsend(&cm, tx1);
	int err2 = CO_CANsend(&cm, tx2);
	printf("err: %i, %i\n", err1, err2);
    ===========================================*/
}

