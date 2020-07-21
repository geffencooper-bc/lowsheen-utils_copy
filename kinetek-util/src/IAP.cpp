#include "IAP.h"
#include "KinetekCodes.h"

#define PRINT_LOG
using std::to_string;

IAP::IAP()
{
    data_size_bytes = 0;
    last_line_data_size = 0;
    memset(start_address, 0, 4);
    memset(data_size, 0, 4);
    memset(total_checksum, 0, 4);

    packet_count = 0;
    page_count = 0;
    num_bytes_uploaded = 0;
    curr_page_cs = 0;
    in_iap_mode = false;
    memset(current_packet, 0, 32);

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
    printf("\n");
    printf("\n\t\t\t\t\t=============================================");
    printf("\n\t\t\t\t\t============= IAP UTILITY PRINT =============");
    printf("\n\t\t\t\t\t=============================================\n\n\n\n");
    
}

void IAP::init_can(const char* channel_name)
{
    sc->init_socketcan(channel_name);
}

void IAP::progress_bar()
{
    
}

void resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    
}

status_code IAP::put_in_iap_mode(bool forced_mode)
{
    #ifdef PRINT_LOG
    printf("Putting in IAP mode\n");
    #endif

    // selective mode
    if(!forced_mode)
    {
        sc->send_frame(KINETEK_COMMAND_ID, ENTER_IAP_MODE_SELECTIVE_DATA, sizeof(ENTER_IAP_MODE_SELECTIVE_DATA));
        CO_CANrxMsg_t * resp = sc->get_frame(KINETEK_RESPONSE_ID, this, resp_call_back, 10);
        if(get_response_type(resp->ident, resp->data, resp->DLC) != ENTER_IAP_MODE_SELECTIVE_RESPONSE)
        {
            #ifdef PRINT_LOG
            printf("IAP mode time out\n");
            #endif
            return IAP_MODE_FAIL;
        }
    }
    // forced mode
    else
    {
        int count = 0;
        sc->send_frame(IAP_REQUEST_ID, ENTER_IAP_MODE_FORCED_DATA, sizeof(ENTER_IAP_MODE_FORCED_DATA));
        CO_CANrxMsg_t * resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back, 5);
        while(get_response_type(resp->ident, resp->data, resp->DLC) != IN_IAP_MODE)
        {
            if(count > 500)
            {
                #ifdef PRINT_LOG
                printf("IAP mode time out\n");
                #endif
                return IAP_MODE_FAIL;
            }
            sc->send_frame(IAP_REQUEST_ID, ENTER_IAP_MODE_FORCED_DATA, sizeof(ENTER_IAP_MODE_FORCED_DATA));
            resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back);
            count++;
        }
    }     

    #ifdef PRINT_LOG
    printf("\n\n======IN IAP MODE=========\n\n");
    #endif
    
    return IAP_MODE_SUCCESS;  
}

status_code IAP::send_init_packets()
{
    // first send the fw revision request
    sc->send_frame(FW_REVISION_REQUEST_ID, FW_REVISION_REQUEST_DATA, sizeof(FW_REVISION_REQUEST_DATA));
    CO_CANrxMsg_t * resp = sc->get_frame(FW_REVISION_RESPONSE_ID, this, resp_call_back, 20);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != FW_REVISION_RESPONSE)
    {
        return FW_REVISION_REQUEST_FAIL;
    }
    #ifdef PRINT_LOG
    printf("GOT FW REVISION RESPONSE\n");
    #endif

    // next send  a request to sent bytes
    sc->send_frame(IAP_REQUEST_ID, SEND_BYTES_DATA, sizeof(SEND_BYTES_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, 20);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_BYTES_RESPONSE)
    {
        return SEND_BYTES_FAIL;
    }
    #ifdef PRINT_LOG
    printf("CAN START SENDING BYTES\n");
    #endif

    // next send the start address
    memcpy(SEND_START_ADDRESS_DATA + 1, start_address, 4); // copy in start address bytes into message
    sc->send_frame(IAP_REQUEST_ID, SEND_START_ADDRESS_DATA, sizeof(SEND_START_ADDRESS_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, 20);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_START_ADDRESS_RESPONSE)
    {
        return SEND_START_ADDRESS_FAIL;
    }
    #ifdef PRINT_LOG
    printf("SENT START ADDRESS\n");
    #endif

    // next send the total checksum
    memcpy(SEND_CHECKSUM_DATA + 1, total_checksum, 4); // copy in checksum bytes into message
    sc->send_frame(IAP_REQUEST_ID, SEND_CHECKSUM_DATA, sizeof(SEND_CHECKSUM_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, 20);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_CHECKSUM_DATA_RESPONSE)
    {
        return SEND_CHECKSUM_FAIL;
    }
    #ifdef PRINT_LOG
    printf("GOT CHECKSUM DATA RESPONSE\n");
    #endif;

    // finally send the data size
    memcpy(SEND_DATA_SIZE_DATA + 1, data_size, 4); // copy in data size bytes into message
    sc->send_frame(IAP_REQUEST_ID, SEND_DATA_SIZE_DATA, sizeof(SEND_DATA_SIZE_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, 20);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_DATA_SIZE_RESPONSE)
    {
        return SEND_DATA_SIZE_FAIL;
    }
    #ifdef PRINT_LOG
    printf("SENT DATA SIZE\n =========DONE WITH INIT PACKETS\n");
    #endif
    return INIT_PACKET_SUCCESS;
}

status_code IAP::upload_hex_file()
{
    #ifdef PRINT_LOG
    printf("\n ======== Uploading hex file =======\n");
    #endif
    
    while(true) // keep sending packets until reached end of file or fail and function returns
    {
        // progress bar

        // reached the end of a page
        if(packet_count > 0 && packet_count % 32 == 0)
        {
            #ifdef PRINT_LOG
            printf("\n======END OF PAGE %i======\n", page_count+1);
            #endif

            // convert page checksum into a list of bytes, copy it into the data of a page checksum frame
            ut->num_to_byte_list(curr_page_cs, SEND_PAGE_CHECKSUM_DATA + 1, 4);
            SEND_PAGE_CHECKSUM_DATA[6] = page_count+1;

            // send the page checksum frame
            sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM_DATA, sizeof(SEND_PAGE_CHECKSUM_DATA));
            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            while(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM_DATA, sizeof(SEND_PAGE_CHECKSUM_DATA));
                resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            }
            page_count +=1;
            curr_page_cs = 0;
        }

        // if not the end of a page, then send the next packet
        status_code status = send_hex_packet();
        if(status == PACKET_SENT_SUCCESS)
        {
            packet_count +=1;
        }
        // if send_hex_packet fails then resend the packet
        else if(status == PACKET_SENT_FAIL)
        {
            status = send_hex_packet(true);
            if(status == PACKET_SENT_FAIL)
            {
                return PACKET_RESENT_FAIL;
            }
            num_bytes_uploaded += 32;
            printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
            packet_count += 1;
            continue;
        }
        // if reached end of file then send end of hex file, total checksum, the last page checksum, and check for a heartbeat
        else if(status == END_OF_FILE_CODE)
        {   
            // send end of file notification with the number of bytes of the last data line (account for filler when doing checksum)
            SEND_END_OF_FILE[1] = last_line_data_size;
            sc->send_frame(IAP_REQUEST_ID, SEND_END_OF_FILE, sizeof(SEND_END_OF_FILE));
            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            while(get_response_type(resp->ident, resp->data, resp->DLC) != END_OF_HEX_FILE_RESPONSE)
            {
                sc->send_frame(IAP_REQUEST_ID, SEND_END_OF_FILE, sizeof(SEND_END_OF_FILE));
                resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            }

            // make last page checksum data
            ut->num_to_byte_list(curr_page_cs, SEND_PAGE_CHECKSUM + 1, 4);
            SEND_PAGE_CHECKSUM[6] = page_count+1;

            // send last page checksum
            sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM, sizeof(SEND_PAGE_CHECKSUM));
            resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            while(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM, sizeof(SEND_PAGE_CHECKSUM));
                resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            }

            // send total checksum
            for(int i = 0; i < 2; i++)
            {
                int tmp = total_checksum[i];
                total_checksum[i] = total_checksum[4-i-1];
                total_checksum[4-i-1] = tmp;
            }
            memcpy(TOTAL_CHECKSUM_REQUEST+1, total_checksum,4);
            sc->send_frame(IAP_REQUEST_ID, TOTAL_CHECKSUM_REQUEST, sizeof(TOTAL_CHECKSUM_REQUEST));
            resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            while(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                sc->send_frame(IAP_REQUEST_ID, TOTAL_CHECKSUM_REQUEST, sizeof(TOTAL_CHECKSUM_REQUEST));
                resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
            }

            // check for a heartbeat
            resp = sc->get_frame(HEART_BEAT_ID, this, resp_call_back);
            while(get_response_type(resp->ident, resp->data, resp->DLC) != HEART_BEAT)
            {
                resp = sc->get_frame(HEART_BEAT_ID, this, resp_call_back);
            }
            return UPLOAD_COMPLETE;
        }
    }
}

status_code IAP::send_hex_packet(bool is_retry)
{
    // first check if the hex packet to be sent is a retry
    if(is_retry)
    {
        // resend the last packet using saved data from "current packet"
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

    // otherwise send the next hex packet
    else
    {
        int frame_count = 0;
        uint8_t data[8];
        int sum;
        kt_can_id curr_frame_id = SEND_FRAME_1_ID;


        while(true) // keep getting the next 8 bytes until have a complete packet
        {
            // get the next 8 bytes from the hex file
            
            sum = ut->get_next_8_bytes(data, 8);
            if(sum != -1)
            {
                curr_page_cs += sum;
            }
           //printf("CURRENT PAGE CHECKSUM: %i", curr_page_cs);

            // copy these next 8 bytes into the current packet
            memcpy(current_packet + 8*frame_count, data, 8);
            
            if(sum == -1) // means surpassed last data line
            {
                num_bytes_uploaded += last_line_data_size;
                if(curr_frame_id == SEND_FRAME_1_ID) // if the frame id is 1, that means that last packet was completed, so no filler necessary
                {
                    printf("\n\n\n====NO FILLER====\n\n\n");
                    return END_OF_FILE_CODE;
                }
                // otherwise need to add filler frames
                printf("\n\n\n====FILLER====\n\n\n");
                memset(current_packet + 8*frame_count+1, 0xFF, sizeof(current_packet) - 8*frame_count);
                for(int i = 0; i < 32; i++)
                {
                    printf("%02X", current_packet[i]);
                }
                while(true)
                {
                    switch(curr_frame_id)
                    {
                        case SEND_FRAME_1_ID:
                        {
                            sc->send_frame(SEND_FRAME_1_ID, current_packet, 8);
                            curr_frame_id = SEND_FRAME_2_ID;
                            frame_count +=1;
                        }
                        case SEND_FRAME_2_ID:
                        {
                            sc->send_frame(SEND_FRAME_2_ID, current_packet+8, 8);
                            curr_frame_id = SEND_FRAME_3_ID;
                            frame_count +=1;
                        }
                        case SEND_FRAME_3_ID:
                        {
                            sc->send_frame(SEND_FRAME_3_ID, current_packet+16, 8);
                            curr_frame_id = SEND_FRAME_4_ID;
                            frame_count +=1;
                        }
                        case SEND_FRAME_4_ID:
                        {
                            sc->send_frame(SEND_FRAME_4_ID, current_packet+24, 8);
                            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back);
                            if(get_response_type(resp->ident, resp->data, resp->DLC) != RECEIVED_32_BYTES)
                            {
                                return PACKET_SENT_FAIL;
                            }
                            printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
                            return END_OF_FILE_CODE;
                        }
                    }
                }
            }
            
            // nor the last line, just a normal frame
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
                        return PACKET_SENT_FAIL;
                    }
                    num_bytes_uploaded += 32;
                    printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
                    return PACKET_SENT_SUCCESS;
                }
            }

        }
    }
    
}
