#include "IAP.h"
#include "KinetekCodes.h"

// these are timeout values used for waiting for response frames
#define LONG_WAIT_TIME 5000 // ms --> 5 sec
#define MEDIUM_WAIT_TIME 100 // ms
#define SHORT_WAIT_TIME 10 // ms

#define PRINT_LOG
//#define PROGRESS_BAR

IAP::IAP()
{
    data_size_bytes = 0;
    packet_count = 0;
    page_count = 0;
    num_bytes_uploaded = 0;
    curr_page_cs = 0;
    in_iap_mode = false;
    memset(current_packet, 0, sizeof(current_packet));

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
    // size of hex file
    data_size_bytes = ut->get_file_data_size(KT::data_size_data + 1, KT_DATA_SIZE_LEN);

    // total checksum init frame
    total_checksum = ut->get_total_cs(KT::total_checksum_data + 1, KT_CS_LEN);

    // total checksum calculation frame (sent after upload, CS bytes sent in reverse)
    ut->get_total_cs(KT::calculate_total_checksum_data + 1, KT_CS_LEN, true);

    start_address = ut->get_start_address(KT::start_address_data + 1, KT_ADDRESS_LEN);
    last_packet_size = data_size_bytes % PACKET_SIZE;
}

void IAP::print()
{
    printf("\n");
    printf("\n================= IAP DETAILS ===============");
    printf("\nHEX FILE DATA SIZE:             %i bytes", data_size_bytes);
    printf("\nHEX FILE DATA TOTAL CHECKSUM:   %08X", total_checksum);
    printf("\nSTART ADDRESS:                  %08X", start_address);
    printf("\nLAST PACKET SIZE:               %i", last_packet_size);
    printf("\n=============================================\n");
    
}

status_code IAP::init_can(const char* channel_name)
{
   int err = sc->init_socketcan(channel_name);
   if(err == -1)
   {
       return INIT_CAN_FAIL;
   }
   return INIT_CAN_SUCCESS;
}

void IAP::progress_bar(int current, int total, int bar_length)
{
  int percent = (current*100)/total;
  string arrow = "";
  string spaces = "";
  int i;
  for(i = 0; i < (percent*bar_length-1)/100; i++)
  {
    arrow += "-";
  }
  arrow += ">";
  for(int j = 0; j < bar_length-i; j++)
  {
    spaces += " ";
  }
  printf("Progress [%s%s] %i %% PAGE: %i\r", arrow.c_str(), spaces.c_str(), percent, page_count);
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
        sc->send_frame(KT::KINETEK_COMMAND_ID, KT::enter_iap_mode_selective_data, sizeof(KT::enter_iap_mode_selective_data));
        CO_CANrxMsg_t * resp = sc->get_frame(KT::KINETEK_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

        if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ENTER_IAP_MODE_SELECTIVE_RESPONSE)
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
        resp = sc->get_frame(KT::KINETEK_STATUS_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
        if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::IN_IAP_MODE)
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
        sc->send_frame(KT::IAP_REQUEST_ID, KT::enter_iap_mode_forced_data, sizeof(KT::enter_iap_mode_forced_data));
        CO_CANrxMsg_t * resp = sc->get_frame(KT::KINETEK_STATUS_ID, this, resp_call_back, 3);

        while(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::IN_IAP_MODE)
        {
            if(count > 5000) // try 5000 times
            {
                #ifdef PRINT_LOG
                printf("Forced IAP mode time out\n");
                #endif
                return IAP_MODE_FAIL;
            }
            sc->send_frame(KT::IAP_REQUEST_ID, KT::enter_iap_mode_forced_data, sizeof(KT::enter_iap_mode_forced_data));
            resp = sc->get_frame(KT::KINETEK_STATUS_ID, this, resp_call_back, 3);
            count++;
        }
    }     

    #ifdef PRINT_LOG
    printf("\n\n======IN IAP MODE=========\n\n");
    #endif
    
    return IAP_MODE_SUCCESS;  
}

status_code IAP::send_init_frames()
{
    // first send the fw version request
    sc->send_frame(KT::FW_VERSION_REQUEST_ID, KT::fw_version_request_data, sizeof(KT::fw_version_request_data));
    CO_CANrxMsg_t * resp = sc->get_frame(KT::FW_VERSION_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);

    if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::FW_VERSION_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("FW_VERSION_REQUEST TIMEOUT\n");
        #endif
        return FW_VERSION_REQUEST_FAIL;
    }
    #ifdef PRINT_LOG
    printf("GOT FW VERSION RESPONSE\n");
    #endif

    printf("\nKinetek Bootloader Version: %i.%i\n", resp->data[0], resp->data[1]);

    // next send  a request to sent bytes
    sc->send_frame(KT::IAP_REQUEST_ID, KT::send_bytes_data, sizeof(KT::send_bytes_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);

    if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::SEND_BYTES_RESPONSE)
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
    sc->send_frame(KT::IAP_REQUEST_ID, KT::start_address_data, sizeof(KT::start_address_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);

    if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::START_ADDRESS_RESPONSE)
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
    sc->send_frame(KT::IAP_REQUEST_ID, KT::total_checksum_data, sizeof(KT::total_checksum_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);

    if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::TOTAL_CHECKSUM_RESPONSE)
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
    if(last_packet_size == 0)
    {
        KT::data_size_data[6] = 0x01;
        #ifdef PRINT_LOG
        printf("last data frame has no filler\n");
        #endif
    }
    sc->send_frame(KT::IAP_REQUEST_ID, KT::data_size_data, sizeof(KT::data_size_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME);

    if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::DATA_SIZE_RESPONSE)
    {
        #ifdef PRINT_LOG
        printf("SEND_DATA_SIZE TIMEOUT\n");
        #endif
        return SEND_DATA_SIZE_FAIL;
    }

    #ifdef PRINT_LOG
    printf("SENT DATA SIZE\n====== DONE WITH INIT PACKETS ======\n");
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
        if(packet_count > 0 && packet_count % PAGE_SIZE == 0)
        {
            // wait for Kinetek to send page checksum, try twice. If don't receive can still get confirmation by sending page checksum
            CO_CANrxMsg_t* resp = sc->get_frame(KT::KINETEK_STATUS_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::KT_CALCULATED_PAGE_CHECKSUM)
            {
                resp = sc->get_frame(KT::KINETEK_STATUS_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::KT_CALCULATED_PAGE_CHECKSUM)
                {
                    #ifdef PRINT_LOG
                    printf("KT CALCULATED PAGE_CHECKSUM TIMEOUT\n");
                    #endif
                }
            }

            #ifdef PRINT_LOG
            printf("\n======END OF PAGE %i======\n", page_count+1);
            #endif

            // convert page checksum into a list of bytes, copy it into the data of a page checksum frame
            ut->num_to_byte_list(curr_page_cs, KT::page_checksum_data + 1, KT_CS_LEN);
            KT::page_checksum_data[6] = page_count+1;

            // send the page checksum frame to the kinetek, wait for confirmation, send twice if need to
            sc->send_frame(KT::IAP_REQUEST_ID, KT::page_checksum_data, sizeof(KT::page_checksum_data));
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                sc->send_frame(KT::IAP_REQUEST_ID, KT::page_checksum_data, sizeof(KT::page_checksum_data));
                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

                if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    printf("last effort compare\n");
                    // if don't receive cs frame, may have missed it, as a last effort compare the KT calculated CS and your calculated CS
                    if(KT::array_compare(KT::page_checksum_data + 1, KT_CS_LEN, resp->data + 1, KT_CS_LEN) == false)
                    {
                        #ifdef PRINT_LOG
                        printf("PAGE_CHECKSUM TIMEOUT\n");
                        #endif
                        return PAGE_CHECKSUM_FAIL;
                    }
                }
            }
            page_count +=1;
            curr_page_cs = 0;
        }

        // if not the end of a page, then send the next packet
        status_code status = send_hex_packet();
        if(status == PACKET_SENT_SUCCESS)
        {
            packet_count +=1;
            continue;
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
            num_bytes_uploaded += PACKET_SIZE;
            #ifdef PRINT_LOG
            printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
            #endif
            packet_count += 1;
            continue;
        }

        // if reached end of file then send end of hex file, total checksum, the last page checksum, and check for a heartbeat
        else if(status == END_OF_FILE_CODE)
        {   
            // send end of file notification with the number of bytes of the last data packet (accounts for filler when doing checksum)
            if(last_packet_size == 0) // do this because 32 % 32 = 0
            {
                last_packet_size = PACKET_SIZE;
            }
            KT::end_of_file_data[1] = last_packet_size; // need to tell Kinetek what the last packet size is

            sc->send_frame(KT::IAP_REQUEST_ID, KT::end_of_file_data, sizeof(KT::end_of_file_data));
            CO_CANrxMsg_t * resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::END_OF_HEX_FILE_RESPONSE)
            {
                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::END_OF_HEX_FILE_RESPONSE)
                {
                    #ifdef PRINT_LOG
                    printf("END_OF_FILE TIMEOUT\n");
                    #endif
                    return END_OF_FILE_FAIL;
                }
            }

            // wait for the kinetek to send its calculated page checksum
            resp = sc->get_frame(KT::KINETEK_STATUS_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::KT_CALCULATED_PAGE_CHECKSUM)
            {
                #ifdef PRINT_LOG
                printf("KT_CALCULATED_PAGE_CHECKSUM TIMEOUT\n");
                #endif
            }

            // make last page checksum data
            ut->num_to_byte_list(curr_page_cs, KT::page_checksum_data + 1, 4);
            KT::page_checksum_data[6] = page_count+1;

            // send last page checksum
            sc->send_frame(KT::IAP_REQUEST_ID, KT::page_checksum_data, sizeof(KT::page_checksum_data));
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                sc->send_frame(KT::IAP_REQUEST_ID, KT::page_checksum_data, sizeof(KT::page_checksum_data));
                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

                if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    #ifdef PRINT_LOG
                    printf("PAGE_CHECKSUM TIMEOUT\n");
                    #endif
                    return PAGE_CHECKSUM_FAIL;
                }
            }
            // send total checksum
            sc->send_frame(KT::IAP_REQUEST_ID, KT::calculate_total_checksum_data, sizeof(KT::calculate_total_checksum_data));
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                #ifdef PRINT_LOG
                printf("TOTAL_CHECKSUM TIMEOUT\n");
                #endif
                return TOTAL_CHECKSUM_FAIL;
            }

            // check for a heartbeat
            resp = sc->get_frame(KT::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::HEART_BEAT)
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
        // resend the last packet using saved data from "current packet"
        KT::can_id curr_frame_id = KT::RESEND_FRAME_1_ID;
        uint8_t data[CAN_DATA_LEN];
        sc->send_frame(KT::RESEND_FRAME_1_ID, current_packet, sizeof(data));
        sc->send_frame(KT::RESEND_FRAME_2_ID, current_packet+8, sizeof(data));
        sc->send_frame(KT::RESEND_FRAME_3_ID, current_packet+16, sizeof(data));
        sc->send_frame(KT::RESEND_FRAME_4_ID, current_packet+24, sizeof(data));
        CO_CANrxMsg_t * resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
        if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
        {
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
            {
                return PACKET_RESENT_FAIL;
            }   
        }
        // increment num bytes uploaded, need to figure out when last line not full
        return PACKET_SENT_SUCCESS;
    }

    // otherwise send the next hex packet
    else
    {
        int frame_count = 0;
        uint8_t data[CAN_DATA_LEN];
        int sum;
        KT::can_id curr_frame_id = KT::SEND_FRAME_1_ID;


        while(true) // keep getting the next 8 bytes until have a complete packet
        {
            // get the next 8 bytes from the hex file
            sum = ut->get_next_8_bytes(data, sizeof(data));
            if(sum != -1)
            {
                curr_page_cs += sum;
                // copy these next 8 bytes into the according spot of the current packet
                memcpy(current_packet + CAN_DATA_LEN*frame_count, data, sizeof(data));
            }
            
            if(sum == -1) // means surpassed last data line
            {
                // have not reached the end of the packet
                if(frame_count != 0)
                {
                    num_bytes_uploaded += data_size_bytes % PACKET_SIZE;
                }
                else // reached the end of the frame
                {
                    num_bytes_uploaded -= PACKET_SIZE;
                    num_bytes_uploaded += data_size_bytes % PACKET_SIZE;
                }
                #ifdef PRINT_LOG
                printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
                #endif

                // if the frame id is 1, that means that last packet was completed, so no filler frames
                if(curr_frame_id == KT::SEND_FRAME_1_ID) 
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

                memset(current_packet + CAN_DATA_LEN*frame_count, 0xFF, sizeof(current_packet) - CAN_DATA_LEN*frame_count);
                #ifdef PRINT_LOG
                for(int i = 0; i < PACKET_SIZE; i++)
                {
                    printf("%02X", current_packet[i]);
                }
                printf("\n");
                #endif
                
                while(true)
                {
                    switch(curr_frame_id)
                    {
                        case KT::SEND_FRAME_1_ID:
                        {
                            sc->send_frame(KT::SEND_FRAME_1_ID, current_packet, CAN_DATA_LEN);
                            curr_frame_id = KT::SEND_FRAME_2_ID;
                            frame_count +=1;
                        }
                        case KT::SEND_FRAME_2_ID:
                        {
                            sc->send_frame(KT::SEND_FRAME_2_ID, current_packet+CAN_DATA_LEN*frame_count, CAN_DATA_LEN);
                            curr_frame_id = KT::SEND_FRAME_3_ID;
                            frame_count +=1;
                        }
                        case KT::SEND_FRAME_3_ID:
                        {
                            sc->send_frame(KT::SEND_FRAME_3_ID, current_packet+CAN_DATA_LEN*frame_count, CAN_DATA_LEN);
                            curr_frame_id = KT::SEND_FRAME_4_ID;
                            frame_count +=1;
                        }
                        case KT::SEND_FRAME_4_ID:
                        {
                            sc->send_frame(KT::SEND_FRAME_4_ID, current_packet+CAN_DATA_LEN*frame_count, CAN_DATA_LEN);
                            CO_CANrxMsg_t * resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                            if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                            {
                                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                                if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                                {
                                    return PACKET_SENT_FAIL;
                                }
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
                case KT::SEND_FRAME_1_ID:
                {
                    usleep(3000);
                    sc->send_frame(KT::SEND_FRAME_1_ID, data, sizeof(data));
                    curr_frame_id = KT::SEND_FRAME_2_ID;
                    frame_count +=1;
                    break;
                }
                case KT::SEND_FRAME_2_ID:
                {
                    sc->send_frame(KT::SEND_FRAME_2_ID, data, sizeof(data));
                    curr_frame_id = KT::SEND_FRAME_3_ID;
                    frame_count +=1;
                    break;
                }
                case KT::SEND_FRAME_3_ID:
                {
                    sc->send_frame(KT::SEND_FRAME_3_ID, data, sizeof(data));
                    curr_frame_id = KT::SEND_FRAME_4_ID;
                    frame_count +=1;
                    break;
                }
                case KT::SEND_FRAME_4_ID:
                {
                    sc->send_frame(KT::SEND_FRAME_4_ID, data, sizeof(data));
                    CO_CANrxMsg_t * resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                    if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                    {
                        resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                        if(KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                        {
                            return PACKET_SENT_FAIL;
                        }
                    }
                    num_bytes_uploaded += 32;
                    if(num_bytes_uploaded < data_size_bytes)
                    {
                        #ifdef PRINT_LOG
                        printf("BYTES UPLOADED: %i\n", num_bytes_uploaded);
                        #endif
                    }
                    return PACKET_SENT_SUCCESS;
                }
            }

        }
    }
    
}
