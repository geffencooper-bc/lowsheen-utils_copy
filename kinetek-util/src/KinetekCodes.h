// this file is essentially a config file with all the kinetek command and response codes
// there is also a couple of functions to determine accordings response based on inpout frame

#ifndef KINETEK_CODES_H
#define KINETEK_CODES_H

#define SELECTIVE_MODE // for some reason when removing the switch, a new set of can_ids appeared for selective/forced mode
// #define FORCED_MODE
// request and response can ids
#ifdef FORCED_MODE
enum kt_can_id
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
    KINETEK_MESSAGE_ID =       0x060,
    FW_VERSION_RESPONSE_ID =   0x067,
    IAP_RESPONSE_ID =          0x069,
    HEART_BEAT_ID =            0x080,
    KINETEK_RESPONSE_ID =      0x081
};
#endif

#ifdef SELECTIVE_MODE
enum kt_can_id
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
    KINETEK_MESSAGE_ID =       0x080,
    FW_VERSION_RESPONSE_ID =   0x087,
    IAP_RESPONSE_ID =          0x089,
    HEART_BEAT_ID =            0x080,
    KINETEK_RESPONSE_ID =      0x081
};
#endif


// IAP response name
enum iap_response
{
    IN_IAP_MODE = 2,
    ENTER_IAP_MODE_SELECTIVE_RESPONSE,
    RECEIVED_32_BYTES,
    SEND_BYTES_RESPONSE,
    FW_VERSION_RESPONSE,
    SEND_START_ADDRESS_RESPONSE,
    SEND_CHECKSUM_DATA_RESPONSE,
    SEND_DATA_SIZE_RESPONSE,
    END_OF_HEX_FILE_RESPONSE,
    CALCULATE_TOTAL_CHECKSUM_RESPONSE,
    CALCULATE_PAGE_CHECKSUM_RESPONSE,
    SELF_CALCULATED_PAGE_CHECKSUM,
    HEART_BEAT,
    NONE = -1
};

// =======================================================================================================


// IAP request data

// ID = 0x001
uint8_t ENTER_IAP_MODE_SELECTIVE_DATA[5]  =  {0x1D, 0x03, 0x27, 0x00, 0x00};

// ID = 0x045
uint8_t FW_VERSION_REQUEST_DATA[8]       =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// ID = 0x048
uint8_t ENTER_IAP_MODE_FORCED_DATA[8]     =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t SEND_BYTES_DATA[8]                =  {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
uint8_t SEND_START_ADDRESS_DATA[8]        =  {0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x00, 0x00};
uint8_t SEND_CHECKSUM_DATA[8]             =  {0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x9B, 0x00, 0x00};
uint8_t SEND_DATA_SIZE_DATA[8]            =  {0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x9C, 0x00, 0x00};
uint8_t SEND_END_OF_FILE_DATA[8]          =  {0x05, 0xFF, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
uint8_t TOTAL_CHECKSUM_DATA[8]            =  {0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x9D, 0x00, 0x00};
uint8_t SEND_PAGE_CHECKSUM_DATA[8]        =  {0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x9E, 0xFF, 0x00};


// =======================================================================================================


// IAP response data --> 0xFF = don't care bytes

// ID = 0x60
uint8_t IN_IAP_MODE_DATA[5]                           =  {0x80, 0x00, 0x00, 0x00, 0x00};
uint8_t SELF_CALCULATED_PAGE_CHECKSUM_DATA[5]         =  {0x84, 0xFF, 0xFF, 0xFF, 0xFF};

//ID = 0x067
uint8_t FW_VERSION_RESPONSE_DATA[8]                  =  {0xFF, 0xFF, 0x5E, 0xFF, 0xFF, 0x00, 0x00, 0x00};

// ID = 0x069
uint8_t SEND_BYTES_RESPONSE_DATA[8]                   =  {0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
uint8_t SEND_START_ADDRESS_RESPONSE_DATA[8]           =  {0x02, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
uint8_t SEND_CHECKSUM_DATA_RESPONSE_DATA[8]           =  {0x03, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
uint8_t SEND_DATA_SIZE_RESPONSE_DATA[8]               =  {0x04, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
uint8_t END_OF_HEX_FILE_RESPONSE_DATA[8]              =  {0x05, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
uint8_t CALCULATE_TOTAL_CHECKSUM_RESPONSE_DATA[8]     =  {0x06, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
uint8_t CALCULATE_PAGE_CHECKSUM_RESPONSE_DATA[8]      =  {0x07, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
uint8_t RECEIVED_32_BYTES_DATA[8]                     =  {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

// ID = 0x80
uint8_t HEART_BEAT_DATA[8]                            =  {0x1D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ID = 0x81
uint8_t ENTER_IAP_MODE_SELECTIVE_RESPONSE_DATA[5]     =  {0x1D, 0x03, 0x27, 0x00, 0x00};


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
iap_response get_response_type(uint32_t can_id, uint8_t* data, uint8_t num_bytes)
{
    if((kt_can_id)can_id == IAP_RESPONSE_ID) // 0x69
    {
        if(array_compare(RECEIVED_32_BYTES_DATA, sizeof(RECEIVED_32_BYTES_DATA), data, num_bytes))
        {
            return RECEIVED_32_BYTES;
        }
        else if(array_compare(SEND_BYTES_RESPONSE_DATA, sizeof(SEND_BYTES_RESPONSE_DATA), data, num_bytes))
        {
            return SEND_BYTES_RESPONSE;
        }
        else if(array_compare(SEND_START_ADDRESS_RESPONSE_DATA, sizeof(SEND_START_ADDRESS_RESPONSE_DATA), data, num_bytes))
        {
            return SEND_START_ADDRESS_RESPONSE;
        }
        else if(array_compare(SEND_CHECKSUM_DATA_RESPONSE_DATA, sizeof(SEND_CHECKSUM_DATA_RESPONSE_DATA), data, num_bytes))
        {
            return SEND_CHECKSUM_DATA_RESPONSE;
        }
        else if(array_compare(SEND_DATA_SIZE_RESPONSE_DATA, sizeof(SEND_DATA_SIZE_RESPONSE_DATA), data, num_bytes))
        {
            return SEND_DATA_SIZE_RESPONSE;
        }
        else if(array_compare(END_OF_HEX_FILE_RESPONSE_DATA, sizeof(END_OF_HEX_FILE_RESPONSE_DATA), data, num_bytes))
        {
            return END_OF_HEX_FILE_RESPONSE;
        }
        else if(array_compare(CALCULATE_TOTAL_CHECKSUM_RESPONSE_DATA, sizeof(CALCULATE_TOTAL_CHECKSUM_RESPONSE_DATA), data, num_bytes))
        {
            return CALCULATE_TOTAL_CHECKSUM_RESPONSE;
        }
        else if(array_compare(CALCULATE_PAGE_CHECKSUM_RESPONSE_DATA, sizeof(CALCULATE_PAGE_CHECKSUM_RESPONSE_DATA), data, num_bytes))
        {
            return CALCULATE_PAGE_CHECKSUM_RESPONSE;
        }
    }
    else if((kt_can_id)can_id == HEART_BEAT_ID && data[0] == HEART_BEAT_DATA[0]) // 0x080
    {
        printf("heart beat");
        return HEART_BEAT;
    }
    else if((kt_can_id)can_id == KINETEK_MESSAGE_ID) // 0x60, 0x080
    {
        if(array_compare(IN_IAP_MODE_DATA, sizeof(IN_IAP_MODE_DATA), data, num_bytes))
        {
            return IN_IAP_MODE;
        }
        else if(data[0] == 0x84)
        {
            return SELF_CALCULATED_PAGE_CHECKSUM;
        }
    }
    else if((kt_can_id)can_id == FW_VERSION_RESPONSE_ID) // 0x067
    {
        if(data[2] == 0x5E)
        return FW_VERSION_RESPONSE;
    } 
    else if ((kt_can_id)can_id == KINETEK_RESPONSE_ID) // 0x081
    {
        if(array_compare(ENTER_IAP_MODE_SELECTIVE_RESPONSE_DATA, sizeof(ENTER_IAP_MODE_SELECTIVE_RESPONSE_DATA), data, num_bytes))
        {
            return ENTER_IAP_MODE_SELECTIVE_RESPONSE;
        }
    }
}

#endif