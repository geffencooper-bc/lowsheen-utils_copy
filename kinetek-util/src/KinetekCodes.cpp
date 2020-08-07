#include "KinetekCodes.h"

// =========================================== Initialize static data arrays =====================================

// 0xFF = don't care bytes that are filled in at runtime

// ------------------------------------------------- IAP data -----------------------------------------

    //                                           IAP REQUEST data 
    // ID = 0x001
    uint8_t KinetekCodes::enter_iap_mode_selective_data[5] = {0x1D, 0x03, 0x27, 0x00, 0x00};

    // ID = 0x005/0x045
    uint8_t KinetekCodes::fw_version_request_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // ID = 0x008/0x048
    uint8_t KinetekCodes::force_enter_iap_mode_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t KinetekCodes::send_bytes_data[8] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
    uint8_t KinetekCodes::start_address_data[8] = {0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x00, 0x00};
    uint8_t KinetekCodes::total_checksum_data[8] = {0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x9B, 0x00, 0x00};
    uint8_t KinetekCodes::hex_data_size_data[8] = {0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x9C, 0x00, 0x00};
    uint8_t KinetekCodes::end_of_file_data[8] = {0x05, 0xFF, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
    uint8_t KinetekCodes::calculate_total_checksum_data[8] = {0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x9D, 0x00, 0x00};
    uint8_t KinetekCodes::page_checksum_data[8] = {0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x9E, 0xFF, 0x00};

    //                                           IAP RESPONSE data 
    // ID = 0x060/0x080
    uint8_t KinetekCodes::in_iap_mode_data[5] = {0x80, 0x00, 0x00, 0x00, 0x00};
    uint8_t KinetekCodes::kt_calculated_page_checksum_data[5] = {0x84, 0xFF, 0xFF, 0xFF, 0xFF};

    // ID = 0x067/0x087
    uint8_t KinetekCodes::fw_version_response_data[8] = {0xFF, 0xFF, 0x5E, 0xFF, 0xFF, 0x00, 0x00, 0x00};

    // ID = 0x069/0x089
    uint8_t KinetekCodes::send_bytes_response_data[8] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
    uint8_t KinetekCodes::start_address_response_data[8] = {0x02, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t KinetekCodes::total_checksum_response_data[8] = {0x03, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t KinetekCodes::data_size_response_data[8] = {0x04, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t KinetekCodes::end_of_file_response_data[8] = {0x05, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
    uint8_t KinetekCodes::calculate_total_checksum_response_data[8] = {0x06, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t KinetekCodes::page_checksum_response_data[8] = {0x07, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
    uint8_t KinetekCodes::ACK_32_bytes_data[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

    // ID = 0x81
    uint8_t KinetekCodes::enter_iap_mode_selective_response_data[5] = {0x1D, 0x03, 0x27, 0x00, 0x00};


// ---------------------------------------------------- STU data ------------------------------------------------

    // ID = 0x00A
    uint8_t KinetekCodes::eeprom_access_read_request_data[8] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};
    uint8_t KinetekCodes::eeprom_access_write_request_data[8] = {0x02, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};

    // ID = 0x009 and 0x00E
    uint8_t KinetekCodes::eeprom_access_line_write_data[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


// --------------------------------------------------Standard data ------------------------------------------------

    // ID = 0xAC1DCODE
    uint8_t KinetekCodes::enable_kinetek_data[2] = {0x02, 0x02};
    uint8_t KinetekCodes::disable_kinetek_data[2] = {0x02, 0x01};
    uint8_t KinetekCodes::reset_xt_can_data[2] = {0x02, 0x00};

    // ID = 0x80
    uint8_t KinetekCodes::heart_beat_data[8] = {0x1D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


// ============================================= Helper Functions =========================================================

// checks if two data arrays are equivalent
bool KinetekCodes::array_compare(uint8_t* expected, uint8_t num_bytes_expected, uint8_t* actual, uint8_t num_bytes_actual)
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
KinetekCodes::kinetek_response KinetekCodes::get_response_type(KU_can_id can_id, uint8_t* data_array, uint8_t arr_size)
{
    // IAP RESPONSE
    if (can_id == 0x069 || can_id == 0x089) // can be in State 1 or State 2
    {
        if (array_compare(ACK_32_bytes_data, sizeof(ACK_32_bytes_data), data_array, arr_size))
        {
            return ACK_32_BYTES;
        }
        else if (array_compare(page_checksum_response_data, sizeof(page_checksum_response_data), data_array,
                                arr_size))
        {
            return CALCULATE_PAGE_CHECKSUM_RESPONSE;
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
            return HEX_DATA_SIZE_RESPONSE;
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
    }
    // IAP STATUS RESPONSE
    else if (can_id == KINETEK_STATUS_1_ID || can_id == KINETEK_STATUS_2_ID)  // 0x60, 0x080
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
    // FW VERSION RESPONSE
    else if (can_id == 0x067 || can_id == 0x087)  // 0x067, 0x087
    {
        if (data_array[2] == 0x5E)
            return FW_VERSION_RESPONSE;
    }
    // HEART BEAT
    if (can_id == HEART_BEAT_ID && data_array[0] == heart_beat_data[0])  // 0x080
    {
        return HEART_BEAT;
    }
    // STANDARD RESPONSE 
    else if (can_id == KINETEK_RESPONSE_ID)  // 0x081
    {
        if (array_compare(enter_iap_mode_selective_response_data, sizeof(enter_iap_mode_selective_response_data),
                            data_array, arr_size))
        {
            return ENTER_IAP_MODE_SELECTIVE_RESPONSE;
        }
    }
    // READ EEPROM LINE
    else if(can_id == EEPROM_LINE_READ_A_RESPONSE_ID || can_id == EEPROM_LINE_READ_B_RESPONSE_ID)
    {
        return EEPROM_ACCESS_READ_RESPONSE;
    }
    // WRITE EEPROM LINE
    else if(can_id == EEPROM_LINE_WRITE_RESPONSE_ID)
    {
        if(array_compare(eeprom_access_write_request_data, sizeof(eeprom_access_write_request_data), data_array, arr_size))
        {
            return EEPROM_ACCESS_WRITE_RESPONSE;
        }
    }
    // READ SINGLE STU PARAM
    else if(can_id == SINGLE_STU_PARAM_READ_A_RESPONSE_ID || can_id == SINGLE_STU_PARAM_READ_B_RESPONSE_ID)
    {
        return SINGLE_STU_PARAM_READ_RESPONSE;
    }
    // WRITE SINGLE STU PARAM
    else if(can_id == SINGLE_STU_PARAM_WRITE_RESPONSE_ID)
    {
        return SINGLE_STU_PARAM_WRITE_RESPONSE;
    }
    else
    {
        return NONE;
    }
    return NONE;
}