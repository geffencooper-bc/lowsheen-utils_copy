// this file is essentially a config file with all the kinetek command and response codes
// there is also a couple of functions to determine accordings response based on inpout frame

#ifndef KINETEK_CODES_H
#define KINETEK_CODES_H

//#define SELECTIVE_MODE // for some reason when removing the switch, a new set of can_ids appeared for selective/forced mode
 #define FORCED_MODE
// request and response can ids
#ifdef FORCED_MODE

// Kinetek namespace
namespace KT
{
    enum can_id
    {
        KINETEK_COMMAND_ID =       0x001,
        FW_VERSION_REQUEST_ID  =   0x045,
        IAP_REQUEST_ID =           0x048,
        SEND_FRAME_1_ID =          0x04F,
        SEND_FRAME_2_ID =          0x050,
        SEND_FRAME_3_ID =          0x051,
        SEND_FRAME_4_ID =          0x052,
        RESEND_FRAME_1_ID =        0x053,
        RESEND_FRAME_2_ID =        0x054,
        RESEND_FRAME_3_ID =        0x055,
        RESEND_FRAME_4_ID =        0x056,
        KINETEK_STATUS_ID =        0x060,
        FW_VERSION_RESPONSE_ID =   0x067,
        IAP_RESPONSE_ID =          0x069,
        HEART_BEAT_ID =            0x080,
        KINETEK_RESPONSE_ID =      0x081
    };
    #endif

    #ifdef SELECTIVE_MODE
    enum can_id
    {
        KINETEK_COMMAND_ID =       0x001,
        FW_VERSION_REQUEST_ID =    0x005,
        IAP_REQUEST_ID =           0x008,
        SEND_FRAME_1_ID =          0x00F,
        SEND_FRAME_2_ID =          0x010,
        SEND_FRAME_3_ID =          0x011,
        SEND_FRAME_4_ID =          0x012,
        RESEND_FRAME_1_ID =        0x013,
        RESEND_FRAME_2_ID =        0x014,
        RESEND_FRAME_3_ID =        0x015,
        RESEND_FRAME_4_ID =        0x016,
        KINETEK_STATUS_ID =        0x080,
        FW_VERSION_RESPONSE_ID =   0x087,
        IAP_RESPONSE_ID =          0x089,
        HEART_BEAT_ID =            0x080,
        KINETEK_RESPONSE_ID =      0x081
    };
    #endif


    // IAP response name
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


    // IAP request data

    #define KT_ADDRESS_LEN 4
    #define KT_CS_LEN 4
    #define KT_DATA_SIZE_LEN 4

    // ID = 0x001
    uint8_t enter_iap_mode_selective_data[5]  =  {0x1D, 0x03, 0x27, 0x00, 0x00};

    // ID = 0x045
    uint8_t fw_version_request_data[8]       =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // ID = 0x048
    uint8_t enter_iap_mode_forced_data[8]     =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t send_bytes_data[8]                =  {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
    uint8_t start_address_data[8]             =  {0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x00, 0x00};
    uint8_t total_checksum_data[8]            =  {0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x9B, 0x00, 0x00};
    uint8_t data_size_data[8]                 =  {0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x9C, 0x00, 0x00};
    uint8_t end_of_file_data[8]               =  {0x05, 0xFF, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
    uint8_t calculate_total_checksum_data[8]  =  {0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x9D, 0x00, 0x00};
    uint8_t page_checksum_data[8]             =  {0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x9E, 0xFF, 0x00};


    // =======================================================================================================


    // IAP response data --> 0xFF = don't care bytes

    // ID = 0x60
    uint8_t in_iap_mode_data[5]                           =  {0x80, 0x00, 0x00, 0x00, 0x00};
    uint8_t kt_calculated_page_checksum_data[5]           =  {0x84, 0xFF, 0xFF, 0xFF, 0xFF};

    //ID = 0x067
    uint8_t fw_version_response_data[8]                   =  {0xFF, 0xFF, 0x5E, 0xFF, 0xFF, 0x00, 0x00, 0x00};

    // ID = 0x069
    uint8_t send_bytes_response_data[8]                   =  {0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
    uint8_t start_address_response_data[8]                =  {0x02, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t total_checksum_response_data[8]               =  {0x03, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t data_size_response_data[8]                    =  {0x04, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    uint8_t end_of_file_response_data[8]                  =  {0x05, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
    uint8_t calculate_total_checksum_response_data[8]     =  {0x06, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t page_checksum_response_data[8]                =  {0x07, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
    uint8_t ACK_32_bytes_data[8]                          =  {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

    // ID = 0x80
    uint8_t heart_beat_data[8]                            =  {0x1D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // ID = 0x81
    uint8_t enter_iap_mode_selective_response_data[5]     =  {0x1D, 0x03, 0x27, 0x00, 0x00};


    // =======================================================================================================


    // checks if two data arrays are equivalent
    bool array_compare(uint8_t* expected, uint8_t num_bytes_expected, uint8_t* actual, uint8_t num_bytes_actual)
    {
        if(num_bytes_expected != num_bytes_actual)
        {
            return false;
        }
        for(uint8_t i = 0; i < num_bytes_expected; i++)
        {
            if(actual[i] != expected[i])
            {
                return false;
            }
        }
        return true;
    }

    // determines response based on can_id and data bytes
    iap_response get_response_type(uint32_t id, uint8_t* data_array, uint8_t arr_size)
    {
        if((can_id)id == IAP_RESPONSE_ID) // 0x69, 0x089
        {
            if(array_compare(ACK_32_bytes_data, sizeof(ACK_32_bytes_data), data_array, arr_size))
            {
                return ACK_32_BYTES;
            }
            else if(array_compare(send_bytes_response_data, sizeof(send_bytes_response_data), data_array, arr_size))
            {
                return SEND_BYTES_RESPONSE;
            }
            else if(array_compare(start_address_response_data, sizeof(start_address_response_data), data_array, arr_size))
            {
                return START_ADDRESS_RESPONSE;
            }
            else if(array_compare(total_checksum_response_data, sizeof(total_checksum_response_data), data_array, arr_size))
            {
                return TOTAL_CHECKSUM_RESPONSE;
            }
            else if(array_compare(data_size_response_data, sizeof(data_size_response_data), data_array, arr_size))
            {
                return DATA_SIZE_RESPONSE;
            }
            else if(array_compare(end_of_file_response_data, sizeof(end_of_file_response_data), data_array, arr_size))
            {
                return END_OF_HEX_FILE_RESPONSE;
            }
            else if(array_compare(calculate_total_checksum_response_data, sizeof(calculate_total_checksum_response_data), data_array, arr_size))
            {
                return CALCULATE_TOTAL_CHECKSUM_RESPONSE;
            }
            else if(array_compare(page_checksum_response_data, sizeof(page_checksum_response_data), data_array, arr_size))
            {
                return CALCULATE_PAGE_CHECKSUM_RESPONSE;
            }
        }
        else if((can_id)id == HEART_BEAT_ID && data_array[0] == heart_beat_data[0]) // 0x080
        {
            printf("heart beat");
            return HEART_BEAT;
        }
        else if((can_id)id == KINETEK_STATUS_ID) // 0x60, 0x080
        {
            if(array_compare(in_iap_mode_data, sizeof(in_iap_mode_data), data_array, arr_size))
            {
                return IN_IAP_MODE;
            }
            else if(data_array[0] == 0x84)
            {
                return KT_CALCULATED_PAGE_CHECKSUM;
            }
        }
        else if((can_id)id == FW_VERSION_RESPONSE_ID) // 0x067, 0x087
        {
            if(data_array[2] == 0x5E)
            return FW_VERSION_RESPONSE;
        } 
        else if ((can_id)id == KINETEK_RESPONSE_ID) // 0x081
        {
            if(array_compare(enter_iap_mode_selective_response_data, sizeof(enter_iap_mode_selective_response_data), data_array, arr_size))
            {
                return ENTER_IAP_MODE_SELECTIVE_RESPONSE;
            }
        }
    }

}

#endif