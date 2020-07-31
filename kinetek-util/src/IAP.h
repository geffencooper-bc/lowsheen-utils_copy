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

// a class to facilitate the IAP process

#ifndef IAP_H
#define IAP_H

#include "SocketCanHelper.h"
#include "HexUtility.h"

// these codes are used to determine if to move on or not in the iap process
enum status_code
{
    INIT_CAN_FAIL = 0,
    INIT_CAN_SUCCESS,
    IAP_MODE_FAIL,
    IAP_MODE_SUCCESS,
    FW_VERSION_REQUEST_FAIL,
    SEND_BYTES_FAIL,
    SEND_START_ADDRESS_FAIL,
    SEND_CHECKSUM_FAIL,
    SEND_DATA_SIZE_FAIL,
    INIT_PACKET_SUCCESS,
    PACKET_SENT_SUCCESS,
    PACKET_SENT_FAIL,
    PAGE_CHECKSUM_FAIL,
    END_OF_FILE_CODE,
    PACKET_RESENT_FAIL,
    TOTAL_CHECKSUM_FAIL,
    NO_HEART_BEAT,
    END_OF_FILE_FAIL,
    UPLOAD_COMPLETE
};

#define PACKET_SIZE 32 // number of bytes in a packet (4 can frames)
#define PAGE_SIZE 32   // number of packets in a page (128 can frames)

class IAP
{
    public:

    // init member variables
    IAP();

    // deallocate memory
    ~IAP();

    // initialize hex_utility object and read in needed hex file info
    void load_hex_file(string file_path);

    // prints out hex file info
    void print();

    // shows upload status in terminal
    void progress_bar(int current, int total, int bar_length = 20);

    // sets up socket can helper object, channel name is usually "can0"
    status_code init_can(const char* channel_name);

    // step one of IAP process, uses selective mode by default
    status_code put_in_iap_mode(bool forced_mode=false);

    // step two of IAP process, send hex file data size, checksum, start address etc
    status_code send_init_frames();

    // step three of IAP process, sends actual hex data
    status_code upload_hex_file();


    private:

    // hex file data
    int data_size_bytes;     // number of data bytes in hex file
    int start_address;
    int total_checksum;
    int last_packet_size;

    // IAP variables for monitoring hex file upload
    uint32_t packet_count;          // 4 CAN frames = 1 packet (32 bytes)
    uint32_t page_count;            // 32 packets = 1 page (1024 bytes)
    uint32_t num_bytes_uploaded;    
    uint32_t curr_page_cs;          // page checksum = sum of all bytes in page
    bool in_iap_mode;                
    uint8_t current_packet[32];     // store the last packet in case need to resend it
    
    SocketCanHelper* sc;            // helps with sending and receiving can messages
    HexUtility* ut;                 // helps with reading hex file

    friend void resp_call_back(void* msg, const CO_CANrxMsg_t* can_msg);  // the call back may need access to private member variables
    status_code send_hex_packet(bool is_retry=false);                     // sends the next 32 bytes of hex data, called by upload_hex_file

    // determines which IAP state have entered into and if need to adjust can ids
    // wait_time specifies timeout value, Note: iap state is independent of iap mode (forced vs selective)
    status_code check_iap_state(int wait_time);

    uint8_t set_7th;
    uint8_t iap_can_id_mask;
    uint32_t iap_state;
};

#endif