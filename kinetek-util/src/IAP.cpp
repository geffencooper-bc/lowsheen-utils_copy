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

status_code IAP::upload_hex_file()
{
    printf("\n ======== Uploading hex file =======\n");
    page_count = 0;
    packet_count = 0;
    num_bytes_uploaded = 0;
    int curr_page_cs = 0;

    while(true) // keep sending 32 byte packets until reached end of file or fail
    {
        // progress bar

        // every 32 pages there is a page checksum
        if(packet_count > 0 && packet_count % 32 == 0)
        {
            printf("\n======END OF PAGE %i======\n", page_count+1);
            // make the page_cs frame and send it
            uint8_t page_cs[4];
            ut->num_to_byte_list(curr_page_cs, page_cs, 4);
            memcpy(SEND_PAGE_CHECKSUM, page_cs, 4);
            SEND_PAGE_CHECKSUM[6] = page_count+1;

            // send the page checksum frame
            sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM, sizeof(SEND_PAGE_CHECKSUM));
            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                return PAGE_CHECKSUM_FAIL;
            }
            page_count +=1;
        }

        // if not the end of a page, then send the next packet
        status_code status = send_hex_packet();
        if(status == PACKET_SENT_SUCCESS)
        {
            packet_count +=1;
        }
        //if it fails then resend the packet
        else if(status == PACKET_SENT_FAIL)
        {
            status = send_hex_packet(true);
            if(status == PACKET_SENT_FAIL)
            {
                return PACKET_RESENT_FAIL;
            }
            // increment bytes
            packet_count += 1;
            continue;
        }
        // if reached end of file then take care of the last page checksum, then send end of hex file, total checksum, heartbeat
        else if(status == END_OF_FILE)
        {
            uint8_t page_cs[4];
            ut->num_to_byte_list(curr_page_cs, page_cs, 4);
            memcpy(SEND_PAGE_CHECKSUM, page_cs, 4);
            SEND_PAGE_CHECKSUM[6] = page_count+1;
            
            SEND_END_OF_FILE[1] = last_line_data_size;
            sc->send_frame(IAP_REQUEST_ID, SEND_END_OF_FILE, sizeof(SEND_END_OF_FILE));
            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            while(get_response_type(resp->ident, resp->data, resp->DLC) != END_OF_HEX_FILE_RESPONSE)
            {
                sc->send_frame(IAP_REQUEST_ID, SEND_END_OF_FILE, sizeof(SEND_END_OF_FILE));
                resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            }

            sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM, sizeof(SEND_PAGE_CHECKSUM));
            resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                return PAGE_CHECKSUM_FAIL;
            }

            sc->send_frame(IAP_REQUEST_ID, TOTAL_CHECKSUM_REQUEST, sizeof(TOTAL_CHECKSUM_REQUEST));
            resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                return TOTAL_CHECKSUM_FAIL;
            }

            resp = sc->get_frame(HEART_BEAT_ID, this, resp_call_back);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != HEART_BEAT)
            {
                return NO_HEART_BEAT;
            }
        }
    }

}

status_code IAP::send_hex_packet(bool is_retry)
{
    if(is_retry)
    {
        kt_can_id curr_frame_id = RESEND_FRAME_1_ID;
        uint8_t data[8];
        sc->send_frame(RESEND_FRAME_1_ID, current_packet, 8);
        sc->send_frame(RESEND_FRAME_2_ID, current_packet+8, 8);
        sc->send_frame(RESEND_FRAME_3_ID, current_packet+16, 8);
        sc->send_frame(RESEND_FRAME_4_ID, current_packet+24, 8);
        CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
        if(get_response_type(resp->ident, resp->data, resp->DLC) != RECEIVED_32_BYTES)
        {
            return PACKET_RESENT_FAIL; // need to resend frames
        }
        // increment num bytes uploaded, need to figure out when last line not full
        return PACKET_SENT_SUCCESS;
    }

    else
    {
        int frame_count = 0;
        uint8_t data[8];
        int err;
        kt_can_id curr_frame_id = SEND_FRAME_1_ID;
        while(true)
        {
            
            if(num_bytes_uploaded = data_size_bytes)
            {
                return true;
            }
            err = ut->get_next_8_bytes(data, 8);
            if(true)//last line)
            {

            }
            memcpy(current_packet + 8*frame_count, data, 8);
            if(err == -1)
            {
                return true;
            }
            
            switch(curr_frame_id)
            {
                case SEND_FRAME_1_ID:
                {
                    sc->send_frame(SEND_FRAME_1_ID, data, 8);
                    curr_frame_id = SEND_FRAME_2_ID;
                    frame_count +=1;
                    break;
                }
                case SEND_FRAME_2_ID:
                {
                    sc->send_frame(SEND_FRAME_2_ID, data, 8);
                    curr_frame_id = SEND_FRAME_3_ID;
                    frame_count +=1;
                    break;
                }
                case SEND_FRAME_3_ID:
                {
                    sc->send_frame(SEND_FRAME_3_ID, data, 8);
                    curr_frame_id = SEND_FRAME_4_ID;
                    frame_count +=1;
                    break;
                }
                case SEND_FRAME_4_ID:
                {
                    sc->send_frame(SEND_FRAME_4_ID, data, 8);
                    CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
                    if(get_response_type(resp->ident, resp->data, resp->DLC) != RECEIVED_32_BYTES)
                    {
                        return false; // need to resend frames
                    }
                    // increment num bytes uploaded, need to figure out when last line not full
                    return true;
                    curr_frame_id = SEND_FRAME_1_ID;
                    frame_count = 0;
                }
            }

        }
    }
    
}
