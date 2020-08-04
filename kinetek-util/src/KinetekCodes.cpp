#include "KinetekCodes.h"

// ********************************** initialize static arrays ********************************

// ------------------- independent of Kinetek codes-----------------------
// commands to toggle the estop line and turn on/off the Kinetek
    uint8_t KinetekCodes::enable_kinetek_data[2] = {0x02, 0x02};
    uint8_t KinetekCodes::disable_kinetek_data[2] = {0x02, 0x01};

// ----------------------- kinetek IAP REQUEST data -------------------------
// 0xFF = don't care bytes that are filled in at runtime

    // ID = 0x001
    uint8_t KinetekCodes::enter_iap_mode_selective_data[5] = {0x1D, 0x03, 0x27, 0x00, 0x00};

    // ID = 0x045/0x005
    uint8_t KinetekCodes::fw_version_request_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // ID = 0x048/0x008
    uint8_t KinetekCodes::enter_iap_mode_forced_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t KinetekCodes::send_bytes_data[8] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
    uint8_t KinetekCodes::start_address_data[8] = {0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x00, 0x00};
    uint8_t KinetekCodes::total_checksum_data[8] = {0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x9B, 0x00, 0x00};
    uint8_t KinetekCodes::data_size_data[8] = {0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x9C, 0x00, 0x00};
    uint8_t KinetekCodes::end_of_file_data[8] = {0x05, 0xFF, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
    uint8_t KinetekCodes::calculate_total_checksum_data[8] = {0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x9D, 0x00, 0x00};
    uint8_t KinetekCodes::page_checksum_data[8] = {0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x9E, 0xFF, 0x00};

// ----------------------- kinetek IAP RESPONSE data -------------------------
// 0xFF = don't care bytes that are filled in at runtime

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

    // ID = 0x80
    uint8_t KinetekCodes::heart_beat_data[8] = {0x1D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // ID = 0x81
    uint8_t KinetekCodes::enter_iap_mode_selective_response_data[5] = {0x1D, 0x03, 0x27, 0x00, 0x00};

// ----------------------- kinetek EEPROM ACCESS (STU REQUEST) data -------------------------
// 0xFF = don't care bytes that are filled in at runtime

    uint8_t KinetekCodes::eeprom_access_read_request_data[8] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};

// ----------------------- kinetek EEPROM ACCESS (STU REQUEST) data -------------------------
// 0xFF = don't care bytes that are filled in at runtime

    uint8_t KinetekCodes::eeprom_access_read_response_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};



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

KinetekCodes::kinetek_response KinetekCodes::get_response_type(uint32_t id, uint8_t* data_array, uint8_t arr_size)
{
    //using KT::KinetekCodes;
    // ----------------------------- iap response types --------------------------
    // only care if the last four bits match (last hex digit)
    // IAP_RESPONSE section
    if ((id & 0b00001111) == 0x09)  // 0x69, 0x089
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
    // KINETEK STATUS section
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
    // FW REVISION section
    // only care if the last four bit match (last hex digit)
    else if ((id & 0b00001111) == 0x07)  // 0x067, 0x087
    {
        if (data_array[2] == 0x5E)
            return FW_VERSION_RESPONSE;
    }
    // HEART BEAT section
    if ((std_can_id)id == HEART_BEAT_ID && data_array[0] == heart_beat_data[0])  // 0x080
    {
        return HEART_BEAT;
    }
    // STANDARD RESPONSE section
    else if ((std_can_id)id == KINETEK_RESPONSE_ID)  // 0x081
    {
        if (array_compare(enter_iap_mode_selective_response_data, sizeof(enter_iap_mode_selective_response_data),
                            data_array, arr_size))
        {
            return ENTER_IAP_MODE_SELECTIVE_RESPONSE;
        }
    }
    else if((std_can_id)id == EEPROM_LINE_READ_RESPONSE_A_ID || (std_can_id)id == EEPROM_LINE_READ_RESPONSE_B_ID)
    {
        return EEPROM_ACCESS_READ_RESPONSE;
    }
    else
    {
        return NONE;
    }
}