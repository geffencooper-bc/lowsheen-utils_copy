// this file is essentially a config file with all the kinetek command and response codes

#ifndef KINETEK_CODES_H
#define KINETEK_CODES_H

#include <string>
#include <regex>

using std::string;
using std::regex;
using std::regex_match;

enum iap_response
{
    IN_IAP_MODE = 2,
    ENTER_IAP_MODE_RESPONSE_SELECTIVE,
    RECEIVED_32_BYTES,
    SEND_BYTES_RESPONSE,
    FW_REVISION_REQUEST_RESPONSE,
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

struct iap_resp_pair
{
    string resp_form;
    iap_response resp_name;
};

// this table and the function below are used to get the according response string from the response data
iap_resp_pair iap_response_table[]=
{
    {"060\\|80(0){8}",                             IN_IAP_MODE},
    {"081\\|1D03270000" ,                          ENTER_IAP_MODE_RESPONSE_SELECTIVE},
    {"069\\|(10){8}" ,                             RECEIVED_32_BYTES},
    {"069\\|(99){8}" ,                             SEND_BYTES_RESPONSE},
    {"067\\|[0-9A-F]{4}5E[0-9A-F]{4}(0){6}" ,      FW_REVISION_REQUEST_RESPONSE},
    {"069\\|02(10){7}" ,                           SEND_START_ADDRESS_RESPONSE},
    {"069\\|03(10){7}" ,                           SEND_CHECKSUM_DATA_RESPONSE},
    {"069\\|04(10){7}" ,                           SEND_DATA_SIZE_RESPONSE},
    {"069\\|05(20){7}" ,                           END_OF_HEX_FILE_RESPONSE},
    {"069\\|06(30){7}" ,                           CALCULATE_TOTAL_CHECKSUM_RESPONSE},
    {"069\\|07(40){7}" ,                           CALCULATE_PAGE_CHECKSUM_RESPONSE},
    {"060\\|84[0-9A-F]{10}",                       SELF_CALCULATED_PAGE_CHECKSUM},
    {"080\\|1D[0-9A-F]{14}",                       HEART_BEAT}
};

// find the according pattern in the above table
iap_response lookup(string data, iap_resp_pair table[], int num_rows=sizeof(iap_response_table))
{
    for(int i = 0; i < num_rows; i++)
    {
      if (std::regex_match (data, std::regex(table[i].resp_form)))
      {
          return table[i].resp_name;
      }
    }
    return NONE;
}

enum kt_can_id
{
    KINETEK_COMMAND =       0x001,
    FW_REVISION_REQUEST =   0x045,
    IAP_REQUEST =           0x048,
    SEND_PACKET_1 =         0x04F,
    SEND_PACKET_2 =         0x050,
    SEND_PACKET_3 =         0x051,
    SEND_PACKET_4 =         0x052,
    RESEND_PACKET_1 =       0x053,
    RESEND_PACKET_2 =       0x054,
    RESEND_PACKET_3 =       0x055,
    RESEND_PACKET_4 =       0x056,
    KINETEK_MESSAGE =       0x060,
    IAP_RESPONSE =          0x069
};

// list of data bytes for commands
uint8_t DEFAULT[8]                   =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t ENTER_IAP_MODE_FORCED[8]     =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t ENTER_IAP_MODE_SELECTIVE[5]  =  {0x1D, 0x03, 0x27, 0x00, 0x00};
uint8_t SEND_BYTES[8]                =  {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
uint8_t CODE_START_ADDRESS_PREFIX[1] =  {0x02};
uint8_t CODE_START_ADDRESS_SUFFIX[3] =  {0x9A, 0x00, 0x00};
uint8_t SEND_CHECKSUM_PREFIX[1]      =  {0x03};
uint8_t SEND_CHECKSUM_SUFFIX[3]      =  {0x9B, 0x00, 0x00};
uint8_t SEND_DATA_SIZE_PREFIX[1]     =  {0x04};
uint8_t SEND_DATA_SIZE_SUFFIX[3]     =  {0x9C, 0x00, 0x00};
uint8_t SEND_END_OF_FILE_PREFIX[1]   =  {0x05};
uint8_t SEND_END_OF_FILE_SUFFIX[6]   =  {0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
uint8_t TOTAL_CHECKSUM_PREFIX[1]     =  {0x06};
uint8_t TOTAL_CHECKSUM_SUFFIX[3]     =  {0x9D, 0x00, 0x00};
uint8_t PAGE_CHECKSUM_PREFIX[1]      =  {0x07};
uint8_t PAGE_CHECKSUM_MID[1]         =  {0x9E};
uint8_t PAGE_CHECKSUM_SUFFIX[1]      =  {0x00};

#endif