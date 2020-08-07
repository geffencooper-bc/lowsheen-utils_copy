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

// This struct holds Kinetek information (can ids, can data), helper functions,
// and status codes used by kinetek utility tools to decipher can messages.
// For details about how Kinetek can frame data is structured, see 
// "CAN Message Keycode Runtime Ram Usage and ICON Log 2019 05 17 1444.xlsx"
// at this confluence page:https://braincorporation.atlassian.net/wiki/spaces/FCE/pages/865009733/Kinetek-Util

// NOTE about messages:
// REQUEST: Host --> Kinetek        RESPONSE: Host <-- Kinetek
// A REQUEST message expects a RESPONSE message
// A CAN ID or DATA array without "request" in its name does not
// expect an immediate response when it is sent
struct KinetekCodes
{
// ==================================================== CAN ID SECTION ===========================================================

/*
   NOTE about IAP (firmware download mode):
   The IAP state (STATUS_ID) is determined upon entering IAP mode and is independent
   of the mode (forced vs selective). There are two IAP states which each use a
   different but related set of CAN IDs (have the same function). State 1 uses the CAN
   IDs defined in the IAP section of KU_can_id. State 2 uses the same CAN IDs with
   a slight modification for REQUESTS and RESPONSES as shown below.
   
   REQUESTS: (State 1 can id) | 01000000 (set the 7th bit high).
   RESPONSES: Since the 4 least significant bits are the same for RESPONSES for both states,
   the can id will have a bit mask of 00001111 --> (can id) & 00001111.
*/

#define KINETEK_STATUS_1_ID 0x080  // if in IAP state 1 then REQUESTS same, RESPONSES &= 00001111
#define KINETEK_STATUS_2_ID 0x060  // if in IAP state 2 then REQUESTS |= 01000000, RESPONSES &= 00001111

    // All the CAN IDs used by kinetek-utility app, divided into sections
    enum KU_can_id
    {
        // ----------------- IAP Section ----------------
        // State 1 IDs                     // State 2 IDs
        //                  REQUEST IDs
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
        FORCE_ENTER_IAP_MODE_ID = 0x048,   // 0x048
        //                RESPONSE IDs
        FW_VERSION_RESPONSE_ID = 0x087,    // 0x067
        IAP_RESPONSE_ID = 0x089,           // 0x069


        // ---------------- STU Section -----------------
        EEPROM_ACCESS_MESSAGE_ID = 0x00A,
        //                 REQUEST IDs
        SINGLE_STU_PARAM_READ_REQUEST_ID = 0x006,
        EEPROM_LINE_WRITE_A_REQUEST_ID = 0x009,
        SINGLE_STU_PARAM_WRITE_REQUEST_ID = 0x00B,
        EEPROM_LINE_WRITE_B_REQUEST_ID = 0x00E,
        //                 RESPONSE IDs
        SINGLE_STU_PARAM_READ_A_RESPONSE_ID = 0x085,
        SINGLE_STU_PARAM_READ_B_RESPONSE_ID = 0x086,
        EEPROM_LINE_READ_A_RESPONSE_ID = 0x08B,
        SINGLE_STU_PARAM_WRITE_RESPONSE_ID = 0x08C,
        EEPROM_LINE_READ_B_RESPONSE_ID = 0x08E,
        EEPROM_LINE_WRITE_RESPONSE_ID = 0x091,


        // --------- Standard Message Section ----------
        KINETEK_REQUEST_ID = 0x001,
        HEART_BEAT_ID = 0x080,
        KINETEK_RESPONSE_ID = 0x081,
        XT_CAN_REQUEST_ID = 0xAC1DC0DE
    };


// ================================================= DATA SECTION =======================================================================

// IAP length constants
#define KT_ADDRESS_LEN 4    // address data is 4 bytes
#define KT_CS_LEN 4         // checksum data is 4 bytes
#define KT_DATA_SIZE_LEN 4  // file data size is 4 bytes

/*
    The kinetek data frames are represented as static member variables of the KinetekCodes struct
    so that they are only created once in memory and are shared by all instances of this struct.
*/

// --------------------------- IAP data ---------------------
    //                    IAP REQUEST data
    // ID = 0x001
    static uint8_t enter_iap_mode_selective_data[5];
    
    // ID = 0x005/0x045
    static uint8_t fw_version_request_data[8];

    // ID = 0x008/0x048
    static uint8_t force_enter_iap_mode_data[8];
    static uint8_t send_bytes_data[8];
    static uint8_t start_address_data[8];
    static uint8_t total_checksum_data[8];
    static uint8_t hex_data_size_data[8];
    static uint8_t end_of_file_data[8];
    static uint8_t calculate_total_checksum_data[8];
    static uint8_t page_checksum_data[8];
    
    //                   IAP RESPONSE data
    // ID = 0x060/0x080
    static uint8_t in_iap_mode_data[5];
    static uint8_t kt_calculated_page_checksum_data[5];

    // ID = 0x067/0x087
    static uint8_t fw_version_response_data[8];

    // ID = 0x069/0x089
    static uint8_t send_bytes_response_data[8];
    static uint8_t start_address_response_data[8];
    static uint8_t total_checksum_response_data[8];
    static uint8_t data_size_response_data[8];
    static uint8_t end_of_file_response_data[8];
    static uint8_t calculate_total_checksum_response_data[8];
    static uint8_t page_checksum_response_data[8];
    static uint8_t ACK_32_bytes_data[8];

    // ID = 0x81
    static uint8_t enter_iap_mode_selective_response_data[5];


// -------------------- STU data ---------------------------
    // ID = 0x00A
    static uint8_t eeprom_access_read_request_data[8];
    static uint8_t eeprom_access_write_request_data[8];

    // ID = 0x009 and 0x00E
    static uint8_t eeprom_access_line_write_data[16];


// ------------------- Standard data -----------------------
    // ID = 0xAC1DCODE
    static uint8_t enable_kinetek_data[2];
    static uint8_t disable_kinetek_data[2];
    static uint8_t reset_xt_can_data[2];

    // ID = 0x080
    static uint8_t heart_beat_data[8];
   

// ============================================================ HELPER FUNCTION SECTION =============================================================================

    // Kinetek responses types for IAP, STU, and normal operation
    enum kinetek_response
    {
        NONE = -1,

        // IAP responses
        IN_IAP_MODE = 2,
        ENTER_IAP_MODE_SELECTIVE_RESPONSE,
        FW_VERSION_RESPONSE,
        SEND_BYTES_RESPONSE,
        START_ADDRESS_RESPONSE,
        TOTAL_CHECKSUM_RESPONSE,
        HEX_DATA_SIZE_RESPONSE,
        ACK_32_BYTES,
        END_OF_HEX_FILE_RESPONSE,
        CALCULATE_TOTAL_CHECKSUM_RESPONSE,
        CALCULATE_PAGE_CHECKSUM_RESPONSE,
        KT_CALCULATED_PAGE_CHECKSUM,

        // STU responses
        EEPROM_ACCESS_READ_RESPONSE,
        EEPROM_ACCESS_WRITE_RESPONSE,
        SINGLE_STU_PARAM_READ_RESPONSE,
        SINGLE_STU_PARAM_WRITE_RESPONSE,

        // Standard Responses
        HEART_BEAT
    };

    // checks if two data arrays are equivalent
    bool array_compare(uint8_t* expected, uint8_t num_bytes_expected, uint8_t* actual, uint8_t num_bytes_actual);

    // determines response based on can_id and data bytes
    kinetek_response get_response_type(KU_can_id can_id, uint8_t* data_array, uint8_t arr_size);
};


#endif