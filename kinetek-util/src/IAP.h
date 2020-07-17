// a class to facilitate the IAP process

#ifndef IAP_H
#define IAP_H

#include "SocketCanHelper.h"
#include "HexUtility.h"

class IAP
{
    public:
    IAP();
    ~IAP();

    void load_hex_file(string file_path);
    void print();
    void progress_bar();
    void init_can(const char* channel_name);
    bool put_in_iap_mode(bool forced_mode=false);
    bool check_if_in_iap_mode();
    void send_init_packets();
    bool upload_hex_file();


    private:
    int data_size_bytes;
    bool in_iap_mode;
    bool resend_msg;
    uint8_t last_line_data_size;

    uint8_t start_address[4];
    uint8_t data_size[4];
    uint8_t total_checksum[4];
    

    SocketCanHelper* sc;
    HexUtility* ut;

    friend void resp_call_back(void* msg, const CO_CANrxMsg_t* can_msg);
};

struct call_back_checker
{
    IAP* iap_obj;
    iap_response expected;
};

#endif