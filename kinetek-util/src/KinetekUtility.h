#ifndef KINETEK_UTILITY_H
#define KINETEK_UTILITY_H

#include "IAP.h"
#include "STUparam.h"
#include "KinetekCodes.h"


// these codes are used to determine the status of the kinetek utility
enum status_code
{
    // IAP status codes
    IAP_MODE_FAIL,
    IAP_MODE_SUCCESS,
    FW_VERSION_REQUEST_FAIL,
    SEND_BYTES_FAIL,
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

    // STU status codes
    STU_READ_LINE_A_FAIL,
    STU_READ_LINE_B_FAIL,
    INVALID_STU_FILE,
    VALID_STU_FILE,
    STU_FILE_WRITE_FAIL,
    STU_FILE_READ_SUCCESS,
    STU_FILE_WRITE_SUCCESS,
    STU_PARAM_READ_A_FAIL,
    STU_PARAM_READ_B_FAIL,
    STU_PARAM_WRITE_FAIL,

    // Standard status codes
    INIT_CAN_FAIL = 0,
    INIT_CAN_SUCCESS,
    NO_HEART_BEAT,
};

// this class is the interface and entry point into the kinetek utilities
class KinetekUtility
{
    public:
    // parses the command line input and runs the correct function
    void parse_args(const string& input);

    void run_tool();

    private:
};

#endif