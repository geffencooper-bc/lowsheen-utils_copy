#include "SocketCanHelper.h"

#define PRINT_LOG

SocketCanHelper::SocketCanHelper()
{
    new_value = new itimerspec;
    now = new timespec;
    new_value->it_value.tv_sec = now->tv_sec + 0;
    new_value->it_value.tv_nsec = now->tv_nsec + 5000000;
    new_value->it_interval.tv_sec = 0;
    new_value->it_interval.tv_nsec = 5000000;
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
    // CO_driver oriented around can_module object which facilitates Tx/Rx can messages
    cm = new CO_CANmodule_t;
    can_msg = new CO_CANrxMsg_t;

    // lists of transmit and receive buffers-->store callback function and void* object
    tx_buff_arr = new CO_CANtx_t[1];
    rx_buff_arr = new CO_CANrx_t[1];
    
    // interface init (can0), will not work unless initialize using ip link setup command
    unsigned int if_index = if_nametoindex(interface_name);
    if(if_index < 0)
    {
        #ifdef PRINT_LOG
        printf("If Index Error\n");
        #endif
        return if_index;
    }

    uintptr_t can_interface = if_index;

    #ifdef PRINT_LOG
    printf("if index: %i\n", can_interface);
    #endif

    // can module object init
    int err =  CO_CANmodule_init(cm, (void*)if_index, rx_buff_arr, 1, tx_buff_arr, 1, 250);
    #ifdef PRINT_LOG
    printf("Init CO_CANmodule. Error: %i     Interface Count: %i\n", err, cm->CANinterfaceCount);
    #endif

    // sets up receive filters
    CO_CANsetNormalMode(cm);
}

int SocketCanHelper::send_frame(uint32_t can_id, uint8_t* data, uint8_t data_size)
{
    CO_CANtx_t* tx1 = CO_CANtxBufferInit(cm, 0, can_id, 0, data_size, false);

    #ifdef PRINT_LOG
    printf("Sending Message-->");
    #endif

    // copy the message data into the transmit buffer
    memcpy(tx1->data, data, data_size);

    // send the message
	int err = CO_CANsend(cm, tx1);

    while(err < 0)
    {
        #ifdef PRINT_LOG
	    printf("Transmit Error: %i\t", err);
        #endif
        return err;
    }
    
    #ifdef PRINT_LOG
    printf("Id: %02X\t", can_id);
    for(uint8_t i = 0; i < data_size; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
    #endif
}

CO_CANrxMsg_t * SocketCanHelper::get_frame(uint32_t can_id, void* obj, void (*call_back)(void *object, const CO_CANrxMsg_t *message), int wait_time)
{
    memset(can_msg, 0, sizeof(can_msg));
    // setup desired time_out, 5ms by default
    new_value->it_value.tv_sec = wait_time/1000;
    new_value->it_value.tv_nsec = (wait_time%1000)*1000000;

    // reset the timer for the receive message
    timerfd_settime(timer_fd, 0, new_value, NULL);

    #ifdef PRINT_LOG
    printf("Getting Message-->");
    #endif

    int err = CO_CANrxBufferInit(cm, 0, can_id, 0x7FF, 0, obj, call_back);

    if(err < 0)
    {
        #ifdef PRINT_LOG
        printf("Receive Error: %i\t", err);
        #endif
        return NULL;
    }
    
    // waits until receive specified can_id until timer ends, blocking function
    CO_CANrxWait(cm, timer_fd, can_msg); 

    #ifdef PRINT_LOG
    printf("Id: %02X\t", can_msg->ident);
    for(uint8_t i = 0; i < can_msg->DLC; i++)
    {
        printf("%02X ", can_msg->data[i]);
    }
    printf("\n");
    #endif

    return can_msg;
}