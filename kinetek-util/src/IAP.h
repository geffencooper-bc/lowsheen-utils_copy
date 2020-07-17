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
    void init_can(string channel_name);
    bool put_in_iap_mode(bool force_mode=false);
    bool check_if_in_iap_mode();
    void send_init_packets();
    bool upload_hex_file();


    private:
    int data_size_bytes;
    bool in_iap_mode;
    uint8_t last_line_data_size;

    uint8_t start_address[4];
    uint8_t data_size[4];
    uint8_t total_checksum[4];
    

    SocketCanHelper* sc;
    HexUtility* ut;
};

#endif