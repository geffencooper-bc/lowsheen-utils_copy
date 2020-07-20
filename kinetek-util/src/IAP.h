// a class to facilitate the IAP process

#ifndef IAP_H
#define IAP_H

#include "SocketCanHelper.h"
#include "HexUtility.h"

enum status_code
{
    PACKET_SENT_SUCCESS = 0,
    PACKET_SENT_FAIL,
    PAGE_CHECKSUM_FAIL,
    END_OF_FILE,
    PACKET_RESENT_FAIL,
    TOTAL_CHECKSUM_FAIL,
    NO_HEART_BEAT
};

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
    status_code upload_hex_file();


    private:
    int data_size_bytes;
    bool in_iap_mode;
    uint8_t last_line_data_size;

    int page_count;
    int packet_count;
    uint8_t current_packet[32];
    int num_bytes_uploaded;

    uint8_t start_address[4];
    uint8_t data_size[4];
    uint8_t total_checksum[4];
    

    SocketCanHelper* sc;
    HexUtility* ut;

    friend void resp_call_back(void* msg, const CO_CANrxMsg_t* can_msg);
    status_code send_hex_packet(bool is_retry=false);
};

#endif