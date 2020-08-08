#include "KinetekUtility.h"

KinetekUtility::KinetekUtility()
{
    parse_args()
}

KU::StatusCode KinetekUtility::init_can(const char* channel_name)
{
    int err = sc->init_socketcan(channel_name);
    if (err == -1)
    {
        return KU::INIT_CAN_FAIL;
    }
    return KU::INIT_CAN_SUCCESS;
}
// translates status code to a human readable string
// string IAP::translate_status_code(status_code code)
// {
//     switch(code)
//     {
//         case INIT_CAN_FAIL:
//         {
//             return "Can initialization failed";
//         }
//         case INIT_CAN_SUCCESS:
//         {
//             return "Can initialization successful";
//         }
//         case IAP_MODE_FAIL:
//         {
//             return "Could not enter IAP mode";
//         }
//         case IAP_MODE_SUCCESS:
//         {
//             return "Entered IAP mode successfully";
//         }
//         case FW_VERSION_REQUEST_FAIL:
//         {
//             return "Kinetek did not not receive FW revision request";
//         }
//         case SEND_BYTES_FAIL:
//         {
//             return "Kinetek did not receive the request to start sending bytes";
//         }
//         case SEND_START_ADDRESS_FAIL:
//         {
//             return "Kinetek did not receive the start address";
//         }
//         case SEND_CHECKSUM_FAIL:
//         {
//             return "Kinetek did not receive the hex file checksum";
//         }
//         case SEND_DATA_SIZE_FAIL:
//         {
//             return "Kinetek did not receive the hex file data size";
//         }
//         case INIT_PACKET_SUCCESS:
//         {
//             return "The initialization packets were received successfully";
//         }
//         case PACKET_SENT_SUCCESS:
//         {
//             return "The hex packet was received by the Kinetek";
//         }
//         case PACKET_SENT_FAIL:
//         {
//             return "The hex packet was not receive by the Kinetek";
//         }
//         case PAGE_CHECKSUM_FAIL:
//         {
//             return "The page checksum was not received or does not match the Kinetek";
//         }
//         case END_OF_FILE_CODE:
//         {
//             return "The end of the file has been reached";
//         }
//         case PACKET_RESENT_FAIL:
//         {
//             return "The Kinetek did not receive the hex packet after resending";
//         }
//         case TOTAL_CHECKSUM_FAIL:
//         {
//             return "The total checksum was not received or does not match the Kinetek";
//         }
//         case NO_HEART_BEAT:
//         {
//             return "No heart beat was detected";
//         }
//         case END_OF_FILE_FAIL:
//         {
//             return "The Kinetek did not receive the end of file confirmation";
//         }
//         case UPLOAD_COMPLETE:
//         {
//             return "The hex file was uploaded successfully";
//         }
//     }
// }