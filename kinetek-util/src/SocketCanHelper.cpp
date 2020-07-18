#include "SocketCanHelper.h"
#define PRINT_DEBUG
SocketCanHelper::SocketCanHelper()
{

}

SocketCanHelper::~SocketCanHelper()
{
    CO_CANmodule_disable(cm);
    delete [] tx_buff_arr;
    delete [] rx_buff_arr;
    delete can_msg;
    delete cm;
}

int SocketCanHelper::init_socketcan(const char* interface_name)
{
    // CO_driver oriented around can_module object
    cm = new CO_CANmodule_t;
    can_msg = new CO_CANrxMsg_t;

    // lists of transmit and receive buffers-->store callback function and void* object
    tx_buff_arr = new CO_CANtx_t[1];
    rx_buff_arr = new CO_CANrx_t[1];
    
    // interface init, make sure to do ip link before this
    unsigned int if_index = if_nametoindex(interface_name); // get ifindex from name
    uintptr_t can_interface = if_index;

    #ifdef PRINT_DEBUG
    printf("if index: %i\n", can_interface);
    #endif

    // can module object init
    int err =  CO_CANmodule_init(cm, (void*)if_index, rx_buff_arr, 1, tx_buff_arr, 1, 250);
    #ifdef PRINT_DEBUG
    printf("Init CO_CANmodule. Error: %i     Interface Count: %i\n", err, cm->CANinterfaceCount);
    #endif
    CO_CANsetNormalMode(cm);
}

int SocketCanHelper::send_frame(uint32_t can_id, uint8_t* data, uint8_t data_size)
{
    #ifdef PRINT_DEBUG
    printf("init tx buffer\n");
    #endif
    CO_CANtx_t* tx1 = CO_CANtxBufferInit(cm, 0, can_id, 0, data_size, false);

    #ifdef PRINT_DEBUG
    printf("Sending Message\n");
    #endif
    memcpy(tx1->data, data, data_size);
	int err1 = CO_CANsend(cm, tx1);
    #ifdef PRINT_DEBUG
	printf("err: %i\n", err1);
    #endif
}

string SocketCanHelper::decode_can_msg(const CO_CANrxMsg_t* can_msg)
{
    string can_id = to_string(can_msg->ident);
    string data = "";
    for(int i = 0; i < can_msg->DLC; i++)
    {
        data += to_string(can_msg->data[i]);
    }
    return (can_id + "|" + data);
}

string SocketCanHelper::get_frame(uint32_t can_id, void* obj, void (*call_back)(void *object, const CO_CANrxMsg_t *message))
{
    #ifdef PRINT_DEBUG
    printf("Getting Message\n");
    #endif
    int err = CO_CANrxBufferInit(cm, 0, can_id, 0x7FF, 0, obj, call_back);
    #ifdef PRINT_DEBUG
    printf("Error: %i", err);
    #endif
    CO_CANrxWait(cm, -1, can_msg); 
    string dec = decode_can_msg(can_msg);
    printf("%s\n", dec.c_str());
    return dec;
}