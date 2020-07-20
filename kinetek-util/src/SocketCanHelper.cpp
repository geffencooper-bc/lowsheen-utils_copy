#include "SocketCanHelper.h"
#define PRINT_DEBUG
SocketCanHelper::SocketCanHelper()
{
    new_value = new itimerspec;
    now = new timespec;
    new_value->it_value.tv_sec = now->tv_sec + 0;
    new_value->it_value.tv_nsec = now->tv_nsec + 2000000;
    new_value->it_interval.tv_sec = 0;
    new_value->it_interval.tv_nsec = 2000000;
    timer_fd = timerfd_create(CLOCK_REALTIME, 0);
    int err = timerfd_settime(timer_fd, 0, new_value, NULL);
    int err2 = clock_gettime(CLOCK_REALTIME, now);
    printf("Errors: %i, %i, %i\n", timer_fd, err, err2);
}

SocketCanHelper::~SocketCanHelper()
{
    CO_CANmodule_disable(cm);
    delete [] tx_buff_arr;
    delete [] rx_buff_arr;
    delete can_msg;
    delete cm;
    delete new_value;
    delete now;
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
    //printf("init tx buffer\n");
    #endif
    CO_CANtx_t* tx1 = CO_CANtxBufferInit(cm, 0, can_id, 0, data_size, false);

    #ifdef PRINT_DEBUG
    printf("Sending Message-->");
    #endif
    memcpy(tx1->data, data, data_size);
	int err1 = CO_CANsend(cm, tx1);
    #ifdef PRINT_DEBUG
	printf("Error: %i\t", err1);
    printf("Id: %02X\t", can_id);
    for(uint8_t i = 0; i < data_size; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
    #endif
}

CO_CANrxMsg_t * SocketCanHelper::get_frame(uint32_t can_id, void* obj, void (*call_back)(void *object, const CO_CANrxMsg_t *message))
{
    timerfd_settime(timer_fd, 0, new_value, NULL);
    #ifdef PRINT_DEBUG
    printf("Getting Message-->");
    #endif
    int err = CO_CANrxBufferInit(cm, 0, can_id, 0x7FF, 0, obj, call_back);
    #ifdef PRINT_DEBUG
    printf("Error: %i\t", err);
    #endif
    CO_CANrxWait(cm, timer_fd, can_msg); 
    printf("Id: %02X\t", can_msg->ident);
    for(uint8_t i = 0; i < can_msg->DLC; i++)
    {
        printf("%02X ", can_msg->data[i]);
    }
    printf("\n");
    return can_msg;
}