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


#ifndef KINETEK_CODES_H
#define KINETEK_CODES_H

#include <stdint.h>

// This class is for configuring Kinetek information (can id, data).
// For details about how Kinetek can frame data is structured, see 
// "CAN Message Keycode Runtime Ram Usage and ICON Log 2019 05 17 1444.xlsx"
// at this confluence page: https://braincorporation.atlassian.net/wiki/spaces/FCE/pages/749240682/Kinetek+FW+OTA+Roadmap

// Kinetek namespace
struct KinetekCodes
{
// ==================================================== CAN ID SECTION ===========================================================

//--------------------------------- iap section ---------------------------------
/*
   NOTE about IAP:
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
    {   // State 1 ids                          // State 2 ids
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

//------------------------------ standard section -----------------------------

    // standard can ids used outside of iap
    enum std_can_id
    {
        KINETEK_COMMAND_ID = 0x001,
        EEPROM_LINE_WRITE_A_ID = 0x009,
        EEPROM_ACCESS_MESSAGE_ID = 0x00A,
        EEPROM_LINE_WRITE_B_ID = 0x00E,
        HEART_BEAT_ID = 0x080,
        KINETEK_RESPONSE_ID = 0x081,
        EEPROM_LINE_READ_RESPONSE_A_ID = 0x08B,
        EEPROM_LINE_READ_RESPONSE_B_ID = 0x08E,
        EEPROM_LINE_WRITE_RESPONSE_ID = 0x091,
        ESTOP_ID = 0xAC1DC0DE
    };

    // Kinetek responses for iap and normal requests
    enum kinetek_response
    {
        NONE = -1,

        // iap responses
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

        // general responses
        EEPROM_ACCESS_READ_RESPONSE,
        EEPROM_ACCESS_WRITE_RESPONSE,
        HEART_BEAT
    };

// ================================================= DATA SECTION =======================================================================

// iap length constants
#define KT_ADDRESS_LEN 4    // address data is 4 bytes
#define KT_CS_LEN 4         // checksum data is 4 bytes
#define KT_DATA_SIZE_LEN 4  // file data size is 4 bytes

/*
    The kinetek data frames are represented as static member variables of the KinetekCodes struct
    so that they are only created once in memory and are shared by all instances of this struct.
*/

    static uint8_t enable_kinetek_data[2];
    static uint8_t disable_kinetek_data[2];
    static uint8_t enter_iap_mode_selective_data[5];
    static uint8_t fw_version_request_data[8];
    static uint8_t enter_iap_mode_forced_data[8];
    static uint8_t send_bytes_data[8];
    static uint8_t start_address_data[8];
    static uint8_t total_checksum_data[8];
    static uint8_t data_size_data[8];
    static uint8_t end_of_file_data[8];
    static uint8_t calculate_total_checksum_data[8];
    static uint8_t page_checksum_data[8];
    static uint8_t in_iap_mode_data[5];
    static uint8_t kt_calculated_page_checksum_data[5];
    static uint8_t fw_version_response_data[8];
    static uint8_t send_bytes_response_data[8];
    static uint8_t start_address_response_data[8];
    static uint8_t total_checksum_response_data[8];
    static uint8_t data_size_response_data[8];
    static uint8_t end_of_file_response_data[8];
    static uint8_t calculate_total_checksum_response_data[8];
    static uint8_t page_checksum_response_data[8];
    static uint8_t ACK_32_bytes_data[8];
    static uint8_t heart_beat_data[8];
    static uint8_t enter_iap_mode_selective_response_data[5];
    static uint8_t eeprom_access_read_request_data[8];
    static uint8_t eeprom_access_write_request_data[8];
    static uint8_t eeprom_access_line_write_data[16];


// ============================================================ HELPER FUNCTION SECTION =============================================================================
    // checks if two data arrays are equivalent
    bool array_compare(uint8_t* expected, uint8_t num_bytes_expected, uint8_t* actual, uint8_t num_bytes_actual);

    // determines response based on can_id and data bytes
    kinetek_response get_response_type(uint32_t id, uint8_t* data_array, uint8_t arr_size);
};


#endif