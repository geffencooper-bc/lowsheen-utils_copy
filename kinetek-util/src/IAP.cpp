#include "IAP.h"
#include "KinetekCodes.h"

#define PRINT_DEBUG
using std::to_string;

IAP::IAP()
{
    memset(start_address, 0, 4);
    memset(data_size, 0, 4);
    memset(total_checksum, 0, 4);
    in_iap_mode = false;
    sc = new SocketCanHelper;
}

IAP::~IAP()
{
    printf("IAP Destructor\n");
    delete ut;
    delete sc;
}


void IAP::load_hex_file(string file_path)
{
    ut = new HexUtility(file_path);
    data_size_bytes = ut->get_file_data_size(data_size, 4);
    ut->get_total_cs(total_checksum, 4);
    ut->get_start_address(start_address, 4);
    last_line_data_size = ut->get_last_data_line_size();
}

void print_array(uint8_t* arr, uint8_t size)
{
    for(int i = 0; i < size; i++)
    {
        printf("%i ", arr[i]);
    }
}

void IAP::print()
{
    printf("\n");
    printf("\n\t\t\t\t\t=============================================");
    printf("\n\t\t\t\t\t============= IAP UTILITY PRINT== ===========");
    printf("\n\t\t\t\t\t=============================================\n\n");
    printf("\nHEX FILE DATA SIZE:\t\t%i bytes", data_size_bytes);
    printf("\nHEX FILE DATA TOTAL CHECKSUM:\t"); print_array(total_checksum, sizeof(total_checksum));
    printf("\nSTART ADDRESS:\t\t\t"); print_array(start_address, sizeof(start_address));
    // printf("\n\n==== INIT COMMANDS====\n\nFW_REVISION_REQUEST:\t\t\t\t", self.FW_REVISION_REQUEST);
    // printf("\nENTER_IAP_MODE_REQUEST_FORCED:\t\t\t", self.ENTER_IAP_MODE_REQUEST_FORCED);
    // printf("\nENTER_IAP_MODE_REQUEST_SELECTIVE:\t\t", self.ENTER_IAP_MODE_REQUEST_SELECTIVE);
    // printf("\nSEND_BYTES_REQUEST:\t\t\t\t", self.SEND_BYTES_REQUEST);
    // printf("\nSEND_START_ADDRESS_REQUEST:\t\t\t", self.SEND_START_ADDRESS_REQUEST);
    // printf("\nSEND_CHECKSUM_DATA_REQUEST:\t\t\t", self.SEND_CHECKSUM_DATA_REQUEST);
    // printf("\nSEND_DATA_SIZE_REQUEST:\t\t\t\t", self.SEND_DATA_SIZE_REQUEST);
    printf("\n");
    printf("\n\t\t\t\t\t=============================================");
    printf("\n\t\t\t\t\t============= IAP UTILITY PRINT =============");
    printf("\n\t\t\t\t\t=============================================\n\n\n\n");
    
}

void IAP::init_can(const char* channel_name)
{
    sc->init_socketcan(channel_name);
}

void resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    
}

bool IAP::put_in_iap_mode(bool forced_mode)
{
    #ifdef PRINT_DEBUG
    printf("Putting in IAP mode\n");
    #endif
    if(!forced_mode)
    {
        sc->send_frame(KINETEK_COMMAND_ID, ENTER_IAP_MODE_SELECTIVE, sizeof(ENTER_IAP_MODE_SELECTIVE));
        CO_CANrxMsg_t * resp = sc->get_frame(KINETEK_RESPONSE_ID, this, resp_call_back);
        // check if in iap mode, figure out how to set timeout diff than forced
        printf("\n\n======IN IAP MODE=========\n\n");
        return true;
    }
    else
    {
        sc->send_frame(IAP_REQUEST_ID, ENTER_IAP_MODE_FORCED, sizeof(ENTER_IAP_MODE_FORCED));
        CO_CANrxMsg_t * resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back);
        while(get_response_type(resp->ident, resp->data, resp->DLC) != IN_IAP_MODE)
        {
            sc->send_frame(IAP_REQUEST_ID, ENTER_IAP_MODE_FORCED, sizeof(ENTER_IAP_MODE_FORCED));
            resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back);
        }
        printf("\n\n======IN IAP MODE=========\n\n");
        return true;
    }       
}

void IAP::send_init_packets()
{
    sc->send_frame(FW_REVISION_REQUEST_ID, FW_REVISION_REQUEST, sizeof(FW_REVISION_REQUEST));
    CO_CANrxMsg_t * resp = sc->get_frame(FW_REVISION_RESPONSE_ID, this, resp_call_back);
    while(get_response_type(resp->ident, resp->data, resp->DLC) != FW_REVISION_RESPONSE)
    {
        sc->send_frame(FW_REVISION_REQUEST_ID, FW_REVISION_REQUEST, sizeof(FW_REVISION_REQUEST));
        resp = sc->get_frame(FW_REVISION_RESPONSE_ID, this, resp_call_back);
    }
    printf("GOT FW REVISION RESPONSE\n");

    sc->send_frame(IAP_REQUEST_ID, SEND_BYTES, sizeof(SEND_BYTES));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    while(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_BYTES_RESPONSE)
    {
        sc->send_frame(IAP_REQUEST_ID, SEND_BYTES, sizeof(SEND_BYTES));
        resp = sc->get_frame(SEND_BYTES_RESPONSE, this, resp_call_back);
    }
    printf("CAN START SENDING BYTES\n");

    memcpy(SEND_START_ADDRESS + 1, start_address, 4);
    sc->send_frame(IAP_REQUEST_ID, SEND_START_ADDRESS, sizeof(SEND_START_ADDRESS));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    while(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_START_ADDRESS_RESPONSE)
    {
        sc->send_frame(IAP_REQUEST_ID, SEND_START_ADDRESS, sizeof(SEND_START_ADDRESS));
        resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    }
    printf("SENT START ADDRESS\n");

    memcpy(SEND_CHECKSUM_DATA + 1, total_checksum, 4);
    sc->send_frame(IAP_REQUEST_ID, SEND_CHECKSUM_DATA, sizeof(SEND_CHECKSUM_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    while(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_CHECKSUM_DATA_RESPONSE)
    {
        sc->send_frame(IAP_REQUEST_ID, SEND_CHECKSUM_DATA, sizeof(SEND_CHECKSUM_DATA));
        resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    }
    printf("GOT CHECKSUM DATA RESPONSE\n");

    memcpy(SEND_DATA_SIZE + 1, data_size, 4);
    sc->send_frame(IAP_REQUEST_ID, SEND_DATA_SIZE, sizeof(SEND_DATA_SIZE));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    while(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_DATA_SIZE_RESPONSE)
    {
        sc->send_frame(IAP_REQUEST_ID, SEND_DATA_SIZE, sizeof(SEND_DATA_SIZE));
        resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
    }
    printf("SENT DATA SIZE\n =========DONE WITH INIT PACKETS\n");
}
