#include "IAP.h"
#include "KinetekCodes.h"
#include <unistd.h>

// these are timeout values used for waiting for response frames
#define LONG_WAIT_TIME 5000 // ms --> 5 sec
#define MEDIUM_WAIT_TIME 100 // ms
#define SHORT_WAIT_TIME 10 // ms

#define PRINT_LOG
//#define PROGRESS_BAR

IAP::IAP()
{
    data_size_bytes = 0;
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
    delete ut;
    delete sc;
}

void IAP::load_hex_file(string file_path)
{
    ut = new HexUtility(file_path);
    data_size_bytes = ut->get_file_data_size(data_size, 4);
    ut->get_total_cs(total_checksum, 4);
    ut->get_start_address(start_address, 4);
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
    printf("\n================= IAP DETAILS ===============");
    printf("\nHEX FILE DATA SIZE:\t\t%i bytes", data_size_bytes);
    printf("\nHEX FILE DATA TOTAL CHECKSUM:\t"); print_array(total_checksum, sizeof(total_checksum));
    printf("\nSTART ADDRESS:\t\t\t"); print_array(start_address, sizeof(start_address));
    printf("\n=============================================\n");
    printf("last packet size: %i", data_size_bytes%32);
}

int IAP::init_can(const char* channel_name)
{
   int err = sc->init_socketcan(channel_name);
   if(err == -1)
   {
       return INIT_CAN_FAIL;
   }
}

void IAP::progress_bar(int current, int total, int bar_length)
{
  float percent = (current*100)/total;
  string arrow = "";
  string spaces = "";
  int i;
  for(i = 0; i < (percent/100)*bar_length-1; i++)
  {
    arrow += "-";
  }
  arrow += ">";
  for(int j = 0; j < bar_length-i; j++)
  {
    spaces += " ";
  }
  printf("  Progress [%s%s] %f %% \r", arrow.c_str(), spaces.c_str(), percent);
}

void resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

status_code IAP::put_in_iap_mode(bool forced_mode)
{
    #ifdef PRINT_LOG
    printf("Putting in IAP mode\n");
    #endif

    // selective mode
    if(!forced_mode)
    {
        // first do soft reset by sending selective download command
        sc->send_frame(KINETEK_COMMAND_ID, ENTER_IAP_MODE_SELECTIVE_DATA, sizeof(ENTER_IAP_MODE_SELECTIVE_DATA));
        CO_CANrxMsg_t * resp = sc->get_frame(KINETEK_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
        if(get_response_type(resp->ident, resp->data, resp->DLC) != ENTER_IAP_MODE_SELECTIVE_RESPONSE)
        {
            #ifdef PRINT_LOG
            printf("Selective FW Request time out\n");
            #endif
            return IAP_MODE_FAIL;
        }
        #ifdef PRINT_LOG
        printf("Selective FW Request success\n");
        #endif

        // next check if in iap mode
        resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
        if(get_response_type(resp->ident, resp->data, resp->DLC) != IN_IAP_MODE)
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
        // repeatedly send the force enter iap mode command
        int count = 0;
        sc->send_frame(IAP_REQUEST_ID, ENTER_IAP_MODE_FORCED_DATA, sizeof(ENTER_IAP_MODE_FORCED_DATA));
        CO_CANrxMsg_t * resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back, 3);
        while(get_response_type(resp->ident, resp->data, resp->DLC) != IN_IAP_MODE)
        {
            if(count > 5000)
            {
                #ifdef PRINT_LOG
                printf("Forced IAP mode time out\n");
                #endif
                return IAP_MODE_FAIL;
            }
            sc->send_frame(IAP_REQUEST_ID, ENTER_IAP_MODE_FORCED_DATA, sizeof(ENTER_IAP_MODE_FORCED_DATA));
            resp = sc->get_frame(KINETEK_MESSAGE_ID, this, resp_call_back, 3);
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
    // first send the fw version request
    sc->send_frame(FW_VERSION_REQUEST_ID, FW_VERSION_REQUEST_DATA, sizeof(FW_VERSION_REQUEST_DATA));
    CO_CANrxMsg_t * resp = sc->get_frame(FW_VERSION_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != FW_VERSION_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("FW_VERSION_REQUEST TIMEOUT\n");
        #endif
        return FW_VERSION_REQUEST_FAIL;
    }
    #ifdef PRINT_LOG
    printf("GOT FW VERSION RESPONSE\n");
    #endif

    printf("\nKinetek Bootloader Version: %i.%i", resp->data[0], resp->data[1]);

    // next send  a request to sent bytes
    sc->send_frame(IAP_REQUEST_ID, SEND_BYTES_DATA, sizeof(SEND_BYTES_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_BYTES_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("SEND_BYTES TIMEOUT\n");
        #endif
        return SEND_BYTES_FAIL;
    }
    #ifdef PRINT_LOG
    printf("CAN START SENDING BYTES\n");
    #endif

    // next send the start address
    memcpy(SEND_START_ADDRESS_DATA + 1, start_address, 4); // copy in start address bytes into message
    sc->send_frame(IAP_REQUEST_ID, SEND_START_ADDRESS_DATA, sizeof(SEND_START_ADDRESS_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_START_ADDRESS_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("SEND_START_ADDRESS TIMEOUT\n");
        #endif
        return SEND_START_ADDRESS_FAIL;
    }
    #ifdef PRINT_LOG
    printf("SENT START ADDRESS\n");
    #endif

    // next send the total checksum
    memcpy(SEND_CHECKSUM_DATA + 1, total_checksum, 4); // copy in checksum bytes into message
    sc->send_frame(IAP_REQUEST_ID, SEND_CHECKSUM_DATA, sizeof(SEND_CHECKSUM_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_CHECKSUM_DATA_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("SEND_CHECKSUM TIMEOUT\n");
        #endif
        return SEND_CHECKSUM_FAIL;
    }
    #ifdef PRINT_LOG
    printf("GOT CHECKSUM DATA RESPONSE\n");
    #endif

    // finally send the data size
    memcpy(SEND_DATA_SIZE_DATA + 1, data_size, 4); // copy in data size bytes into message
    if(data_size_bytes % 8 == 0)
    {
        SEND_DATA_SIZE_DATA[6] = 0x01;
        #ifdef PRINT_LOG
        printf("last data frame has no filler\n");
        #endif
    }
    sc->send_frame(IAP_REQUEST_ID, SEND_DATA_SIZE_DATA, sizeof(SEND_DATA_SIZE_DATA));
    resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);
    if(get_response_type(resp->ident, resp->data, resp->DLC) != SEND_DATA_SIZE_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("SEND_DATA_SIZE TIMEOUT\n");
        #endif
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
        #ifdef PROGRESS_BAR
        progress_bar(num_bytes_uploaded, data_size_bytes);
        #endif

        // reached the end of a page
        if(packet_count > 0 && packet_count % 32 == 0)
        {
            usleep(50000);
            #ifdef PRINT_LOG
            printf("\n======END OF PAGE %i======\n", page_count+1);
            #endif

            // convert page checksum into a list of bytes, copy it into the data of a page checksum frame
            ut->num_to_byte_list(curr_page_cs, SEND_PAGE_CHECKSUM_DATA + 1, 4);
            SEND_PAGE_CHECKSUM_DATA[6] = page_count+1;

            // send the page checksum frame, this frame typically takes longer
            sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM_DATA, sizeof(SEND_PAGE_CHECKSUM_DATA));
            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                #ifdef PRINT_LOG
                printf("PAGE_CHECKSUM TIMEOUT\n");
                #endif
                return PAGE_CHECKSUM_FAIL;
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
            if(status != PACKET_SENT_SUCCESS)
            {
                #ifdef PRINT_LOG
                printf("PACKET_RESEND TIMEOUT\n");
                #endif
                return PACKET_RESENT_FAIL;
            }
            num_bytes_uploaded += 32;
            #ifdef PRINT_LOG
            printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
            #endif
            packet_count += 1;
            continue;
        }
        // if reached end of file then send end of hex file, total checksum, the last page checksum, and check for a heartbeat
        else if(status == END_OF_FILE_CODE)
        {   
            // send end of file notification with the number of bytes of the last data packet (account for filler when doing checksum), takes longer to receive response
            uint8_t last_packet_size = data_size_bytes % 32;
            if(last_packet_size == 0)
            {
                last_packet_size = 32;
            }
            SEND_END_OF_FILE_DATA[1] = last_packet_size;
            sc->send_frame(IAP_REQUEST_ID, SEND_END_OF_FILE_DATA, sizeof(SEND_END_OF_FILE_DATA));
            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != END_OF_HEX_FILE_RESPONSE)
            {
                #ifdef PRINT_LOG
                printf("END_OF_FILE TIMEOUT\n");
                #endif
                return END_OF_FILE_FAIL;
            }

            // make last page checksum data
            ut->num_to_byte_list(curr_page_cs, SEND_PAGE_CHECKSUM_DATA + 1, 4);
            SEND_PAGE_CHECKSUM_DATA[6] = page_count+1;

            // send last page checksum, takes longer to receive
            sc->send_frame(IAP_REQUEST_ID, SEND_PAGE_CHECKSUM_DATA, sizeof(SEND_PAGE_CHECKSUM_DATA));
            resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                #ifdef PRINT_LOG
                printf("PAGE_CHECKSUM TIMEOUT\n");
                #endif
                return PAGE_CHECKSUM_FAIL;
            }
            // send total checksum
            for(int i = 0; i < 2; i++)
            {
                int tmp = total_checksum[i];
                total_checksum[i] = total_checksum[4-i-1];
                total_checksum[4-i-1] = tmp;
            }
            memcpy(TOTAL_CHECKSUM_DATA+1, total_checksum,4);
            sc->send_frame(IAP_REQUEST_ID, TOTAL_CHECKSUM_DATA, sizeof(TOTAL_CHECKSUM_DATA));
            resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                #ifdef PRINT_LOG
                printf("TOTAL_CHECKSUM TIMEOUT\n");
                #endif
                return TOTAL_CHECKSUM_FAIL;
            }

            // check for a heartbeat
            resp = sc->get_frame(HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
            if(get_response_type(resp->ident, resp->data, resp->DLC) != HEART_BEAT)
            {
                #ifdef PRINT_LOG
                printf("HEART_BEAT TIMEOUT\n");
                #endif
                return NO_HEART_BEAT;
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
        usleep(50000); // wait 100ms before retry
        // resend the last packet using saved data from "current packet"
        kt_can_id curr_frame_id = RESEND_FRAME_1_ID;
        uint8_t data[8];
        sc->send_frame(RESEND_FRAME_1_ID, current_packet, 8);
        sc->send_frame(RESEND_FRAME_2_ID, current_packet+8, 8);
        sc->send_frame(RESEND_FRAME_3_ID, current_packet+16, 8);
        sc->send_frame(RESEND_FRAME_4_ID, current_packet+24, 8);
        CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
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
                // copy these next 8 bytes into the current packet
                memcpy(current_packet + 8*frame_count, data, 8);
            }
            
            if(sum == -1) // means surpassed last data line
            {
                if(frame_count != 3)
                {
                    num_bytes_uploaded += data_size_bytes % 32;
                }
                else
                {
                    num_bytes_uploaded -= 32;
                    num_bytes_uploaded += data_size_bytes % 32;
                }
                if(curr_frame_id == SEND_FRAME_1_ID) // if the frame id is 1, that means that last packet was completed, so no filler necessary
                { 
                    #ifdef PRINT_LOG
                    printf("\n\n\n====NO FILLER====\n\n\n");
                    #endif
                    return END_OF_FILE_CODE;
                }
                // otherwise need to add filler frames
                #ifdef PRINT_LOG
                printf("\n\n\n====FILLER====\n\n\n");
                #endif
                memset(current_packet + 8*frame_count+1, 0xFF, sizeof(current_packet) - 8*frame_count);
                #ifdef PRINT_LOG
                for(int i = 0; i < 32; i++)
                {
                    printf("%02X", current_packet[i]);
                }
                #endif
                printf("\n");
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
                            CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, LONG_WAIT_TIME);
                            if(get_response_type(resp->ident, resp->data, resp->DLC) != RECEIVED_32_BYTES)
                            {
                                return PACKET_SENT_FAIL;
                            }
                            #ifdef PRINT_LOG
                            printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
                            #endif
                            return END_OF_FILE_CODE;
                        }
                    }
                }
            }
            
            // not the last line, just a normal frame
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
                    CO_CANrxMsg_t * resp = sc->get_frame(IAP_RESPONSE_ID, this, resp_call_back, 150);
                    if(get_response_type(resp->ident, resp->data, resp->DLC) != RECEIVED_32_BYTES)
                    {
                        return PACKET_SENT_FAIL;
                    }
                    num_bytes_uploaded += 32;
                    #ifdef PRINT_LOG
                    printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
                    #endif
                    usleep(5000);
                    return PACKET_SENT_SUCCESS;
                }
            }

        }
    }
    
}
