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
    printf("Sending Message-->");
    #endif
    memcpy(tx1->data, data, data_size);
	int err1 = CO_CANsend(cm, tx1);
    #ifdef PRINT_DEBUG
	printf("Error: %i\n", err1);
    #endif
}

CO_CANrxMsg_t * SocketCanHelper::get_frame(uint32_t can_id, void* obj, void (*call_back)(void *object, const CO_CANrxMsg_t *message))
{
    #ifdef PRINT_DEBUG
    printf("Getting Message-->");
    #endif
    int err = CO_CANrxBufferInit(cm, 0, can_id, 0x7FF, 0, obj, call_back);
    #ifdef PRINT_DEBUG
    printf("Error: %i\t", err);
    #endif
    CO_CANrxWait(cm, -1, can_msg); 
    printf("Id: %02X\t", can_msg->ident);
    for(uint8_t i = 0; i < can_msg->DLC; i++)
    {
        printf("%02X ", can_msg->data[i]);
    }
    printf("\n");
    return can_msg;
}