#include "SocketCanHelper.h"

SocketCanHelper::SocketCanHelper()
{

}

SocketCanHelper::~SocketCanHelper()
{
    delete [] tx_buff_arr;
    delete [] rx_buff_arr;
    delete cm;
}

int SocketCanHelper::init_socketcan(const char* interface_name)
{
    // empty objects
    cm = new CO_CANmodule_t;
    // CO_CANrxMsg_t can_msg;

    // lists of transmit and receive buffers
    tx_buff_arr = new CO_CANtx_t[1];
    rx_buff_arr = new CO_CANrx_t[1];
    
    // interface init, make sure to do ip link before this
    unsigned int if_index = if_nametoindex(interface_name); // get ifindex from name
    uintptr_t can_interface = if_index;

    cout << "if index: " << can_interface << endl;

    // can module object init
    int err =  CO_CANmodule_init(cm, (void*)if_index, rx_buff_arr, 1, tx_buff_arr, 1, 250);
    cout << "init module. Error: " << err << " Interface count: " << cm->CANinterfaceCount << " " << endl;
    CO_CANsetNormalMode(cm);
}

int SocketCanHelper::send_frame(uint32_t can_id, uint8_t* data, uint8_t data_size)
{
    //printf("init tx buffer\n");
    CO_CANtx_t* tx1 = CO_CANtxBufferInit(cm, 0, can_id, 0, data_size, false);

    printf("Sending Message\n");
    memcpy(tx1->data, data, data_size);
	int err1 = CO_CANsend(cm, tx1);
	printf("err: %i", err1);
}