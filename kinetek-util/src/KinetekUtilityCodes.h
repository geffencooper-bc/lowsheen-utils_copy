//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//==================================================================

#ifndef KINETEK_CODES_H
#define KINETEK_CODES_H

#include <stdint.h>

// This file holds Kinetek information (can ids, can data), helper functions,
// and status codes used by kinetek utility tools to decipher can messages.
// For details about how Kinetek can frame data is structured, see
// "CAN Message Keycode Runtime Ram Usage and ICON Log 2019 05 17 1444.xlsx"
// at this confluence page:https://braincorporation.atlassian.net/wiki/spaces/FCE/pages/865009733/Kinetek-Util

// Kinetek Utility Namespace
namespace KU
{
// =============================================== CAN ID SECTION ==========================================

    // All the CAN IDs used by kinetek-utility app, divided into sections
    enum CanId
    {
        // ----------------- IAP Section ----------------
        // IAP modes
#define SELECTIVE_MODE 0
#define FORCED_MODE 1

        //                   REQUEST IDs
        FW_VERSION_REQUEST_ID = 0x045,
        IAP_REQUEST_ID = 0x048,
        SEND_FRAME_1_ID = 0x04F,
        SEND_FRAME_2_ID = 0x050,
        SEND_FRAME_3_ID = 0x051,
        SEND_FRAME_4_ID = 0x052,
        RESEND_FRAME_1_ID = 0x053,
        RESEND_FRAME_2_ID = 0x054,
        RESEND_FRAME_3_ID = 0x055,
        RESEND_FRAME_4_ID = 0x056,
        HAND_HELD_PROGRAMMER_ID = 0x040,
        //                  RESPONSE IDs
        IAP_HEARTBEAT_ID = 0x060,
        LCD_IAP_HEARTBEAT_ID = 0x080,
        FW_VERSION_RESPONSE_ID = 0x067,
        IAP_RESPONSE_ID = 0x069,

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
        HEARTBEAT_ID = 0x080,
        KINETEK_RESPONSE_ID = 0x081,
        XT_CAN_REQUEST_ID = 0xAC1DC0DE
    };

    // Kinetek responses types for IAP, STU, and normal operation
    enum KinetekResponse
    {
        NONE = -1,

        // IAP responses
        IN_IAP_MODE = 2,
        ENTER_IAP_MODE_SELECTIVE_RESPONSE,
        FW_VERSION_RESPONSE,
        START_DOWNLOAD_RESPONSE,
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
        HEARTBEAT
    };

    // ========================================== DATA SECTION ==========================================
    /*
        The kinetek utility data frames are represented as static member variables of the KUFrameData struct
        so that they are only created once in memory and are shared by all instances of this struct.
    */
    struct CanDataList
    {
// IAP length constants
#define KT_ADDRESS_LEN 4    // address data is 4 bytes
#define KT_CS_LEN 4         // checksum data is 4 bytes
#define KT_DATA_SIZE_LEN 4  // file data size is 4 bytes

        // --------------------------- IAP data ---------------------
        //                    IAP REQUEST data
        // ID = 0x001
        static uint8_t enter_iap_mode_selective_data[5];

        // ID = 0x040
        static uint8_t use_hand_held_programmer_ids_data[8];

        // ID = 0x045
        static uint8_t fw_version_request_data[8];

        // ID = 0x048
        static uint8_t force_enter_iap_mode_data[8];
        static uint8_t start_download_data[8];
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

        // ID = 0x067
        static uint8_t fw_version_response_data[8];

        // ID = 0x069
        static uint8_t start_download_response_data[8];
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
        static uint8_t heartbeat_data[8];

        // ========================================== HELPER FUNCTION SECTION ==========================================

        // checks if two data arrays are equivalent
        bool array_compare(uint8_t* expected, uint8_t num_bytes_expected, uint8_t* actual, uint8_t num_bytes_actual);

        // determines response based on can_id and data bytes
        KinetekResponse get_response_type(uint32_t can_id, uint8_t* data_array, uint8_t arr_size);
    };

    // ========================================== OTHER CODES SECTION ==========================================

    // these codes are used to determine the status of the kinetek utility, make all negative
    enum StatusCode
    {
        // IAP status codes
        IAP_MODE_SUCCESS = -100,
        IAP_HEARTBEAT_TIMEOUT,
        FW_VERSION_REQUEST_FAIL,
        START_DOWNLOAD_FAIL,
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
        END_OF_FILE_FAIL,
        UPLOAD_COMPLETE,
        UPLOAD_ERROR,

        // STU status codes
        STU_READ_LINE_A_FAIL,
        STU_READ_LINE_B_FAIL,
        INVALID_STU_FILE,
        VALID_STU_FILE,
        STU_FILE_WRITE_FAIL,
        STU_FILE_READ_FAIL,
        STU_FILE_WRITE_SUCCESS,
        STU_FILE_READ_SUCCESS,
        STU_PARAM_READ_A_FAIL,
        STU_PARAM_READ_B_FAIL,
        STU_PARAM_WRITE_FAIL,
        STU_PARAM_READ_FAIL,
        STU_PARAM_WRITE_SUCCESS,
        STU_PARAM_READ_SUCCESS,

        // Live Data status codes
        INVALID_INI_FILE,

        // Standard status codes
        INIT_CAN_FAIL,
        INIT_CAN_SUCCESS,
        NO_HEARTBEAT_DETECTED,
        KU_INIT_ERROR,
        KINETEK_ERROR,
        HEARTBEAT_DETECTED,
        NO_ERROR
    };
};

#endif