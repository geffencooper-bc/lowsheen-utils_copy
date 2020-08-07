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

// this file is essentially a config file with all the kinetek command and response codes
// there are also a couple of functions to determine accordings response based on input frame

#ifndef KINETEK_CODES_H
#define KINETEK_CODES_H

// Kinetek namespace
namespace KT
{
/*
   The IAP state (STATUS_ID) is determined upon entering IAP mode and is independent
   of the mode (forced vs selective). There are two iap states which
   each use a different but related set of can ids. State 1 uses iap_can_id
   as defined by iap_can_id below. State 2 uses the following,
   iap_can_id | 01000000 for commands (set the 7th bit high). Since the
   4 least significant bits are the same for responses for both states, the iap_can_id
   responses will have a bit mask of 00001111 --> iap_can_id & 00001111.
*/
#define KINETEK_STATUS_1_ID 0x080  // if state 1 then commands same, responses &= 00001111
#define KINETEK_STATUS_2_ID 0x060  // if state 2 then can id |= 01000000 for commands, responses &= 00001111
    // can ids used for iap request/response
    enum iap_can_id
    {  // State 1 ids                          // State 2 ids
        // command ids
        FW_VERSION_REQUEST_ID = 0x005,     // 0x045
        IAP_REQUEST_ID = 0x008,            // 0x048
        SEND_FRAME_1_ID = 0x00F,           // 0x04F
        SEND_FRAME_2_ID = 0x010,           // 0x050
        SEND_FRAME_3_ID = 0x011,           // 0x051
        SEND_FRAME_4_ID = 0x012,           // 0x052
        RESEND_FRAME_1_ID = 0x013,         // 0x053
        RESEND_FRAME_2_ID = 0x014,         // 0x054
        RESEND_FRAME_3_ID = 0x015,         // 0x055
        RESEND_FRAME_4_ID = 0x016,         // 0x056
        FORCE_ENTER_IAP_MODE_IAP = 0x048,  // 0x048

        // response ids
        FW_VERSION_RESPONSE_ID = 0x087,  // 0x067
        IAP_RESPONSE_ID = 0x089          // 0x069
    };

    // standard can ids used outside of iap
    enum std_can_id
    {
        KINETEK_COMMAND_ID = 0x001,
        HEART_BEAT_ID = 0x080,
        KINETEK_RESPONSE_ID = 0x081,
        XT_CAN_ID = 0xAC1DC0DE
    };

    // Kinetek iap responses to iap requests
    enum iap_response
    {
        NONE = -1,
        IN_IAP_MODE = 2,
        ENTER_IAP_MODE_SELECTIVE_RESPONSE,
        SEND_BYTES_RESPONSE,
        FW_VERSION_RESPONSE,
        START_ADDRESS_RESPONSE,
        TOTAL_CHECKSUM_RESPONSE,
        DATA_SIZE_RESPONSE,
        ACK_32_BYTES,
        END_OF_HEX_FILE_RESPONSE,
        CALCULATE_TOTAL_CHECKSUM_RESPONSE,
        CALCULATE_PAGE_CHECKSUM_RESPONSE,
        KT_CALCULATED_PAGE_CHECKSUM,
        HEART_BEAT
    };

    // =======================================================================================================

    // xt_can commands

    uint8_t enable_kinetek_data[2] = {0x02, 0x02};
    uint8_t disable_kinetek_data[2] = {0x02, 0x01};
    uint8_t exit_xt_can_data[2] = {0x02, 0x00};

// =======================================================================================================

// IAP request data --> 0xFF = don't care bytes

#define KT_ADDRESS_LEN 4    // address data is 4 bytes
#define KT_CS_LEN 4         // checksum data is 4 bytes
#define KT_DATA_SIZE_LEN 4  // file data size is 4 bytes

    // ID = 0x001
    uint8_t enter_iap_mode_selective_data[5] = {0x1D, 0x03, 0x27, 0x00, 0x00};

    // ID = 0x045/0x005
    uint8_t fw_version_request_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // ID = 0x048/0x008
    uint8_t enter_iap_mode_forced_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t send_bytes_data[8] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
    uint8_t start_address_data[8] = {0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x00, 0x00};
    uint8_t total_checksum_data[8] = {0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x9B, 0x00, 0x00};
    uint8_t data_size_data[8] = {0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x9C, 0x00, 0x00};
    uint8_t end_of_file_data[8] = {0x05, 0xFF, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
    uint8_t calculate_total_checksum_data[8] = {0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x9D, 0x00, 0x00};
    uint8_t page_checksum_data[8] = {0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x9E, 0xFF, 0x00};

    // =======================================================================================================

    // IAP response data --> 0xFF = don't care bytes

    // ID = 0x060/0x080
    uint8_t in_iap_mode_data[5] = {0x80, 0x00, 0x00, 0x00, 0x00};
    uint8_t kt_calculated_page_checksum_data[5] = {0x84, 0xFF, 0xFF, 0xFF, 0xFF};

    // ID = 0x067/0x087
    uint8_t fw_version_response_data[8] = {0xFF, 0xFF, 0x5E, 0xFF, 0xFF, 0x00, 0x00, 0x00};

    // ID = 0x069/0x089
    uint8_t send_bytes_response_data[8] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
    uint8_t start_address_response_data[8] = {0x02, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t total_checksum_response_data[8] = {0x03, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t data_size_response_data[8] = {0x04, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t end_of_file_response_data[8] = {0x05, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
    uint8_t calculate_total_checksum_response_data[8] = {0x06, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t page_checksum_response_data[8] = {0x07, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
    uint8_t ACK_32_bytes_data[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

    // ID = 0x80
    uint8_t heart_beat_data[8] = {0x1D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // ID = 0x81
    uint8_t enter_iap_mode_selective_response_data[5] = {0x1D, 0x03, 0x27, 0x00, 0x00};

    // =======================================================================================================

    // checks if two data arrays are equivalent
    bool array_compare(uint8_t* expected, uint8_t num_bytes_expected, uint8_t* actual, uint8_t num_bytes_actual)
    {
        if (num_bytes_expected != num_bytes_actual)
        {
            return false;
        }
        for (uint8_t i = 0; i < num_bytes_expected; i++)
        {
            if (actual[i] != expected[i])
            {
                return false;
            }
        }
        return true;
    }

    // determines response based on can_id and data bytes
    iap_response get_response_type(uint32_t id, uint8_t* data_array, uint8_t arr_size)
    {
        // first category of responses is IAP_RESPONSE which can be either 0x69, 0x089 based on iap state
        if (id == 0x069 || id == 0x089)  
        {
            if (array_compare(ACK_32_bytes_data, sizeof(ACK_32_bytes_data), data_array, arr_size))
            {
                return ACK_32_BYTES;
            }
            else if (array_compare(send_bytes_response_data, sizeof(send_bytes_response_data), data_array, arr_size))
            {
                return SEND_BYTES_RESPONSE;
            }
            else if (array_compare(start_address_response_data, sizeof(start_address_response_data), data_array,
                                   arr_size))
            {
                return START_ADDRESS_RESPONSE;
            }
            else if (array_compare(total_checksum_response_data, sizeof(total_checksum_response_data), data_array,
                                   arr_size))
            {
                return TOTAL_CHECKSUM_RESPONSE;
            }
            else if (array_compare(data_size_response_data, sizeof(data_size_response_data), data_array, arr_size))
            {
                return DATA_SIZE_RESPONSE;
            }
            else if (array_compare(end_of_file_response_data, sizeof(end_of_file_response_data), data_array, arr_size))
            {
                return END_OF_HEX_FILE_RESPONSE;
            }
            else if (array_compare(calculate_total_checksum_response_data,
                                   sizeof(calculate_total_checksum_response_data), data_array, arr_size))
            {
                return CALCULATE_TOTAL_CHECKSUM_RESPONSE;
            }
            else if (array_compare(page_checksum_response_data, sizeof(page_checksum_response_data), data_array,
                                   arr_size))
            {
                return CALCULATE_PAGE_CHECKSUM_RESPONSE;
            }
        }
        // second category of responses is HEART_BEAT
        else if ((std_can_id)id == HEART_BEAT_ID && data_array[0] == heart_beat_data[0])  // 0x080
        {
            printf("\nheart beat\n");
            return HEART_BEAT;
        }
        // third category of responses is KINETEK STATUS
        else if (id == KINETEK_STATUS_1_ID || id == KINETEK_STATUS_2_ID)  // 0x60, 0x080
        {
            if (array_compare(in_iap_mode_data, sizeof(in_iap_mode_data), data_array, arr_size))
            {
                return IN_IAP_MODE;
            }
            else if (data_array[0] == 0x84)
            {
                return KT_CALCULATED_PAGE_CHECKSUM;
            }
        }
        // only care if the last four bit match (last hex digit)
        else if (id & 0x07)  // 0x067, 0x087
        {
                return FW_VERSION_RESPONSE;
            if (array_compare(enter_iap_mode_selective_response_data, sizeof(enter_iap_mode_selective_response_data),
                              data_array, arr_size))
            {
                return ENTER_IAP_MODE_SELECTIVE_RESPONSE;
            }
        }
        else
        {
            return NONE;
        }
    }
}

#endif