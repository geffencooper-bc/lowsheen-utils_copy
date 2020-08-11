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

// a class is the  IAP tool and is used to update the Kinetek's firmware

#ifndef IAP_H
#define IAP_H

#include "SocketCanHelper.h"
#include "HexUtility.h"
#include "KinetekUtilityCodes.h"

#define PACKET_SIZE 32  // number of bytes in a packet (4 can frames)
#define PAGE_SIZE 32    // number of packets in a page (128 can frames)

class IAP
{
   public:
    // init member variables, IAP needs access to the Kinetek Utility can data and socket can helper
    IAP(SocketCanHelper* sc, KU::CanDataList* ku_data);

    // deallocate memory
    ~IAP();

    // initialize hex_utility object and reads in needed hex file info
    void load_hex_file(string file_path);

    // prints out hex file info
    void print();

    // shows upload status in terminal
    void progress_bar(int current, int total, int bar_length = 20);

    // step one of IAP process, uses forced mode by default
    KU::StatusCode put_in_iap_mode(bool forced_mode);

    // step two of IAP process, send hex file data size, checksum, start address etc
    KU::StatusCode send_init_frames();

    // step three of IAP process, sends actual hex data
    KU::StatusCode upload_hex_file();

    // clear the member variables
    void clear();

   private:
    // hex file data
    int hex_data_size;  // number of data bytes in hex file
    int start_address;
    int total_checksum;
    int last_packet_size;

    // IAP variables for monitoring hex file upload
    uint32_t packet_count;  // 4 CAN frames = 1 packet (32 bytes)
    uint32_t page_count;    // 32 packets = 1 page (1024 bytes)
    uint32_t num_bytes_uploaded;
    uint32_t curr_page_cs;  // page checksum = sum of all bytes in page
    bool in_iap_mode;
    uint8_t current_packet[32];  // store the last packet in case need to resend it

    SocketCanHelper* sc;  // helps with sending and receiving can messages
    HexUtility* ut;       // helps with reading hex file
    KU::CanDataList* ku_data;

    // the call back may need access to private member variables
    friend void IAP_resp_call_back(void* msg, const CO_CANrxMsg_t* can_msg);

    // sends the next 32 bytes of hex data, called by upload_hex_file
    KU::StatusCode send_hex_packet(bool is_retry = false);

    // determines which IAP state have entered into and if need to adjust can ids
    // wait_time specifies timeout value, Note: iap state is independent of iap mode (forced vs selective)
    KU::StatusCode check_iap_state(int wait_time);

    uint8_t set_7th;
    uint8_t iap_can_id_mask;
    uint32_t iap_state;
};

#endif