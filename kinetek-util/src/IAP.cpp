//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions
//==================================================================

#include "IAP.h"

// debug print macro to see error messages
#define PRINT_LOG
//#define PROGRESS_BAR

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) \
    do               \
    {                \
    } while (0)
#endif

// these are timeout values used for waiting for response frames
#define LONG_WAIT_TIME 3000   // ms --> 3 sec
#define MEDIUM_WAIT_TIME 100  // ms
#define SHORT_WAIT_TIME 10    // ms

// init member variables, IAP needs access to the Kinetek Utility can data
IAP::IAP(SocketCanHelper* sc, KU::CanDataList* ku_data)
{
    if(sc == nullptr)
    {
        PRINT_LOG(("Socket Can Helper is not initialized\n"));
        exit(EXIT_FAILURE);
    }
    this->sc = sc;

    if(ku_data == nullptr)
    {
        PRINT_LOG(("Kinetek Utility CanDataList is not initialized\n"));
        exit(EXIT_FAILURE);
    }
    this->ku_data = ku_data;

    hex_data_size = 0;
    packet_count = 0;
    page_count = 0;
    num_bytes_uploaded = 0;
    curr_page_cs = 0;
    in_iap_mode = false;
    memset(current_packet, 0, sizeof(current_packet));

    set_7th = 0;                   // 0 by default
    iap_can_id_mask = 0b00001111;  // only want four lsb
}

// deallocate memory
IAP::~IAP()
{
    delete ut;
}

// init hex utility object and grab needed data from hex file
void IAP::load_hex_file(string file_path)
{
    // loads hex file
    ut = new HexUtility(file_path);

    // initialize iap request data frames using hex file info
    hex_data_size = ut->get_file_data_size(ku_data->hex_data_size_data + 1, KT_DATA_SIZE_LEN);
    total_checksum = ut->get_total_cs(ku_data->total_checksum_data + 1, KT_CS_LEN);  // sent as an init frame
    ut->get_total_cs(ku_data->calculate_total_checksum_data + 1, KT_CS_LEN, true);   // sent after upload in little endian
    start_address = ut->get_start_address(ku_data->start_address_data + 1, KT_ADDRESS_LEN);

    // // Kinetek needs to know last packet size
    last_packet_size = hex_data_size % PACKET_SIZE;
}

// sanity check to make sure reading file correctly
void IAP::print()
{
    PRINT_LOG(("\n"));
    PRINT_LOG(("\n================= IAP DETAILS ==============="));
    PRINT_LOG(("\nHEX FILE DATA SIZE:             %i bytes", hex_data_size));
    PRINT_LOG(("\nHEX FILE DATA TOTAL CHECKSUM:   %08X", total_checksum));
    PRINT_LOG(("\nSTART ADDRESS:                  %08X", start_address));
    PRINT_LOG(("\nLAST PACKET SIZE:               %i", last_packet_size));
    PRINT_LOG(("\n=============================================\n"));
}

// displays upload progress as a growing arrow
void IAP::progress_bar(int current, int total, int bar_length)
{
    int percent = (current * 100) / total;
    string arrow = "";
    string spaces = "";
    int i;
    for (i = 0; i < (percent * bar_length - 1) / 100; i++)
    {
        arrow += "-";
    }
    arrow += ">";
    for (int j = 0; j < bar_length - i; j++)
    {
        spaces += " ";
    }
    printf("Progress [%s%s] %i %% PAGE: %i\r", arrow.c_str(), spaces.c_str(), percent, page_count);
}

// callback function for received messages, not used as of now
void resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

/*
   The IAP state (STATUS_ID) is determined upon entering IAP mode and is independent
   of the mode (forced vs selective). There are two IAP states which each use a
   different but related set of CAN IDs (have the same function). State 1 uses the CAN
   IDs defined in the IAP section of KU_can_id. State 2 uses the same CAN IDs with
   a slight modification for REQUESTS and RESPONSES as shown below.
   
   REQUESTS: (State 1 can id) | 01000000 (set the 7th bit high).
   RESPONSES: Since the 4 least significant bits are the same for RESPONSES for both states,
   the can id will have a bit mask of 00001111 --> (can id) & 00001111.

   The REQUESTS will be bitwise ORed with a variable called "set_7th".
   This function determines the IAP state and if in State 1 then set_7th = 0 and
   the can ids will remain unchanged by the bitwise OR. If in State 2 then
   set_7th = 0b01000000 and the can_ids will changed accordingly.
*/
KU::StatusCode IAP::check_iap_state(int wait_time)
{
    // check if in IAP state 1, if yes then set_7th = 0 and can ids are unchanged
    CO_CANrxMsg_t* resp = sc->get_frame(KINETEK_STATUS_1_ID, this, resp_call_back, wait_time);
    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::IN_IAP_MODE)
    {
        LOG_PRINT(("\nSTATE ID: %02X\n", resp->ident));
        set_7th = 0;
        iap_state = KINETEK_STATUS_1_ID;
    }
    // check if in IAP state 2, if yes then set_7th = 0b01000000 and can ids changed
    else
    {
        resp = sc->get_frame(KINETEK_STATUS_2_ID, this, resp_call_back, wait_time);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::IN_IAP_MODE)
        {
            LOG_PRINT(("\nSTATE ID: %02X\n", resp->ident));
            set_7th = 0b01000000;
            iap_state = KINETEK_STATUS_2_ID;
        }
        else  // if neither states detected then return fail
        {
            LOG_PRINT(("Check IAP state time out\n"));
            return KU::IAP_MODE_FAIL;
        }
    }

    LOG_PRINT(("\n\n======IN IAP MODE=========\n\n"));
    return KU::IAP_MODE_SUCCESS;
}

KU::StatusCode IAP::put_in_iap_mode(bool forced_mode)
{
    LOG_PRINT(("Putting in IAP mode\n"));

    // selective mode
    if (!forced_mode)
    {
        // wait for a heart beat before trying soft reset
        CO_CANrxMsg_t* resp = sc->get_frame(KU::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
        {
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEART_BEAT)
            {
                LOG_PRINT(("No Heart Beat detected, Selective Mode failed\n"));
                return KU::IAP_MODE_FAIL;
            }
        }

        // next do soft reset by sending selective download command
        sc->send_frame(KU::KINETEK_REQUEST_ID, ku_data->enter_iap_mode_selective_data,
                       sizeof(ku_data->enter_iap_mode_selective_data));
        resp = sc->get_frame(KU::KINETEK_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ENTER_IAP_MODE_SELECTIVE_RESPONSE)
        {
            // try again
            resp = sc->get_frame(KU::KINETEK_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ENTER_IAP_MODE_SELECTIVE_RESPONSE)
            {
                LOG_PRINT(("Selective FW Request time out\n"));
                return KU::IAP_MODE_FAIL;
            }
        }

        // next check iap mode status, will init set_7th and return success/fail
        KU::StatusCode iap_status = check_iap_state(MEDIUM_WAIT_TIME);
        return iap_status;
    }
    // forced mode
    else
    {
        // first reset the Kinetek by toggling the estop line
        sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->disable_kinetek_data, sizeof(ku_data->disable_kinetek_data));
        usleep(2500000);  // sleep for 2 seconds

        // turn on the kinetek and repeatedly send the force enter iap mode command
        sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->enable_kinetek_data, sizeof(ku_data->enable_kinetek_data));
        sc->send_frame(KU::FORCE_ENTER_IAP_MODE_ID, ku_data->force_enter_iap_mode_data,
                       sizeof(ku_data->force_enter_iap_mode_data));

        KU::StatusCode iap_status = check_iap_state(3);  // want to check the iap state quickly (3ms timeout)
        int count = 0;
        while (iap_status != KU::IAP_MODE_SUCCESS)
        {
            if (count > 50)  // try 50 times
            {
                LOG_PRINT(("Forced IAP mode time out\n"));
                return KU::IAP_MODE_FAIL;
            }
            sc->send_frame(KU::FORCE_ENTER_IAP_MODE_ID, ku_data->force_enter_iap_mode_data,
                           sizeof(ku_data->force_enter_iap_mode_data));
            iap_status = check_iap_state(3);
            count++;
        }
    }

    return KU::IAP_MODE_SUCCESS;
}

KU::StatusCode IAP::send_init_frames()
{
    // first send the fw version request
    sc->send_frame(KU::FW_VERSION_REQUEST_ID | set_7th, ku_data->fw_version_request_data,
                   sizeof(ku_data->fw_version_request_data));
    CO_CANrxMsg_t* resp =
        sc->get_frame(KU::FW_VERSION_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::FW_VERSION_RESPONSE)
    {
        LOG_PRINT(("FW_VERSION_REQUEST TIMEOUT\n"));
        return KU::FW_VERSION_REQUEST_FAIL;
    }
    LOG_PRINT(("GOT FW VERSION RESPONSE\n"));

    printf("\nKinetek Bootloader Version: %i.%i\n", resp->data[0], resp->data[1]);

    usleep(1000);
    // next send  a request to sent bytes
    sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->send_bytes_data, sizeof(ku_data->send_bytes_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::SEND_BYTES_RESPONSE)
    {
        LOG_PRINT(("SEND_BYTES TIMEOUT\n"));
        return KU::SEND_BYTES_FAIL;
    }
    LOG_PRINT(("CAN START SENDING BYTES\n"));

    usleep(1000);
    // next send the start address
    sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->start_address_data, sizeof(ku_data->start_address_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::START_ADDRESS_RESPONSE)
    {
        LOG_PRINT(("SEND_START_ADDRESS TIMEOUT\n"));
        return KU::SEND_START_ADDRESS_FAIL;
    }
    LOG_PRINT(("SENT START ADDRESS\n"));

    usleep(1000);
    // next send the total checksum
    sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->total_checksum_data, sizeof(ku_data->total_checksum_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::TOTAL_CHECKSUM_RESPONSE)
    {
        LOG_PRINT(("SEND_CHECKSUM TIMEOUT\n"));
        return KU::SEND_CHECKSUM_FAIL;
    }
    LOG_PRINT(("GOT CHECKSUM DATA RESPONSE\n"));

    usleep(1000);
    // finally send the data size
    sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->hex_data_size_data, sizeof(ku_data->hex_data_size_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEX_DATA_SIZE_RESPONSE)
    {
        LOG_PRINT(("SEND_DATA_SIZE TIMEOUT\n"));
        return KU::SEND_DATA_SIZE_FAIL;
    }

    LOG_PRINT(("SENT DATA SIZE\n====== DONE WITH INIT PACKETS ======\n"));
    return KU::INIT_PACKET_SUCCESS;
}

KU::StatusCode IAP::upload_hex_file()
{
    LOG_PRINT(("\n ======== Uploading hex file =======\n"));

    while (true)  // keep sending packets until reached end of file or fail and function returns
    {
// update the progress bar
#ifdef PROGRESS_BAR
        progress_bar(num_bytes_uploaded, data_size_bytes);
#endif

        // reached the end of a page
        if (packet_count > 0 && packet_count % PAGE_SIZE == 0)
        {
            LOG_PRINT(("\n======END OF PAGE %i======\n", page_count + 1));

            // wait for Kinetek to send page checksum, try twice. If don't receive can still get confirmation by sending
            // page checksum
            CO_CANrxMsg_t* resp = sc->get_frame(iap_state, this, resp_call_back, MEDIUM_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::KT_CALCULATED_PAGE_CHECKSUM)
            {
                // try again
                resp = sc->get_frame(iap_state, this, resp_call_back, MEDIUM_WAIT_TIME);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::KT_CALCULATED_PAGE_CHECKSUM)
                {
                    LOG_PRINT(("KT CALCULATED PAGE_CHECKSUM TIMEOUT\n"));
                }
            }

            // convert page checksum into a list of bytes, copy it into the data of a page checksum frame
            ut->num_to_byte_list(curr_page_cs, ku_data->page_checksum_data + 1, KT_CS_LEN);
            ku_data->page_checksum_data[6] = page_count + 1;

            // send the page checksum frame to the kinetek, wait for confirmation, wait twice if need to
            sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->page_checksum_data, sizeof(ku_data->page_checksum_data));
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    LOG_PRINT(("last effort compare\n"));
                    // if don't receive cs frame, may have missed it, as a last effort compare the KT calculated CS and
                    // your calculated CS
                    if (ku_data->array_compare(ku_data->page_checksum_data + 1, KT_CS_LEN, resp->data + 1, KT_CS_LEN) == false)
                    {
                        LOG_PRINT(("PAGE_CHECKSUM TIMEOUT\n"));
                        return KU::PAGE_CHECKSUM_FAIL;
                    }
                }
            }
            page_count += 1;
            curr_page_cs = 0;
        }

        // if not the end of a page, then send the next packet
        KU::StatusCode status = send_hex_packet();
        if (status == KU::PACKET_SENT_SUCCESS)
        {
            packet_count += 1;
            continue;
        }

        // if send_hex_packet fails then resend the packet
        else if (status == KU::PACKET_SENT_FAIL)
        {
            status = send_hex_packet(true);
            if (status != KU::PACKET_SENT_SUCCESS)
            {
                LOG_PRINT(("PACKET_RESEND TIMEOUT\n"));
                return KU::PACKET_RESENT_FAIL;
            }
            num_bytes_uploaded += PACKET_SIZE;
            LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));
            packet_count += 1;
            continue;
        }

        // if reached end of file then send end of hex file, total checksum, the last page checksum, and check for a
        // heartbeat
        else if (status == KU::END_OF_FILE_CODE)
        {
            // send end of file notification with the number of bytes of the last data packet (accounts for filler when
            // doing checksum)
            if (last_packet_size == 0)  // do this because 32 % 32 = 0
            {
                last_packet_size = PACKET_SIZE;
            }
            ku_data->end_of_file_data[1] = last_packet_size;  // need to tell Kinetek what the last packet size is

            sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->end_of_file_data, sizeof(ku_data->end_of_file_data));
            CO_CANrxMsg_t* resp =
                sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::END_OF_HEX_FILE_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::END_OF_HEX_FILE_RESPONSE)
                {
                    LOG_PRINT(("END_OF_FILE TIMEOUT\n"));
                    return KU::END_OF_FILE_FAIL;
                }
            }

            // wait for the kinetek to send its calculated page checksum
            resp = sc->get_frame(iap_state, this, resp_call_back, MEDIUM_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::KT_CALCULATED_PAGE_CHECKSUM)
            {
                LOG_PRINT(("KT_CALCULATED_PAGE_CHECKSUM TIMEOUT\n"));
            }

            // make last page checksum data
            ut->num_to_byte_list(curr_page_cs, ku_data->page_checksum_data + 1, 4);
            ku_data->page_checksum_data[6] = page_count + 1;

            // send last page checksum
            sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->page_checksum_data, sizeof(ku_data->page_checksum_data));
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    LOG_PRINT(("PAGE_CHECKSUM TIMEOUT\n"));
                    return KU::PAGE_CHECKSUM_FAIL;
                }
            }
            // send total checksum
            sc->send_frame(KU::IAP_REQUEST_ID | set_7th, ku_data->calculate_total_checksum_data,
                           sizeof(ku_data->calculate_total_checksum_data));
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                LOG_PRINT(("TOTAL_CHECKSUM TIMEOUT\n"));
                return KU::TOTAL_CHECKSUM_FAIL;
            }

            // check for a heartbeat
            resp = sc->get_frame(KU::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEART_BEAT)
            {
                LOG_PRINT(("HEART_BEAT TIMEOUT\n"));
                return KU::NO_HEART_BEAT;
            }

            usleep(3000);

            // check for a heartbeat again
            resp = sc->get_frame(KU::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEART_BEAT)
            {
                LOG_PRINT(("HEART_BEAT TIMEOUT\n"));
                return KU::NO_HEART_BEAT;
            }
            return KU::UPLOAD_COMPLETE;
        }
    }
}

KU::StatusCode IAP::send_hex_packet(bool is_retry)
{
    // first check if the hex packet to be sent is a retry
    if (is_retry)
    {
        // resend the last packet using saved data from "current packet"
        KU::CanId curr_frame_id = KU::RESEND_FRAME_1_ID;
        uint8_t data[CAN_DATA_LEN];
        sc->send_frame(KU::RESEND_FRAME_1_ID | set_7th, current_packet, sizeof(data));
        usleep(3000);  // delay to prevent TX overflow
        sc->send_frame(KU::RESEND_FRAME_2_ID | set_7th, current_packet + 8, sizeof(data));
        usleep(3000);  // delay to prevent TX overflow
        sc->send_frame(KU::RESEND_FRAME_3_ID | set_7th, current_packet + 16, sizeof(data));
        usleep(3000);  // delay to prevent TX overflow
        sc->send_frame(KU::RESEND_FRAME_4_ID | set_7th, current_packet + 24, sizeof(data));
        CO_CANrxMsg_t* resp =
            sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME, iap_can_id_mask);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
        {
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME, iap_can_id_mask);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
            {
                return KU::PACKET_RESENT_FAIL;
            }
        }
        return KU::PACKET_SENT_SUCCESS;
    }

    // otherwise send the next hex packet
    else
    {
        int frame_count = 0;
        uint8_t data[CAN_DATA_LEN];
        int sum;
        KU::CanId curr_frame_id = KU::SEND_FRAME_1_ID;

        while (true)  // keep getting the next 8 bytes until have a complete packet
        {
            // get the next 8 bytes from the hex file
            sum = ut->get_next_8_bytes(data, sizeof(data));
            if (sum != -1)
            {
                curr_page_cs += sum;
                // copy these next 8 bytes into the according spot of the current packet
                memcpy(current_packet + CAN_DATA_LEN * frame_count, data, sizeof(data));
            }

            else if (sum == -1)  // means surpassed last data line
            {
                // have not reached the end of the packet
                if (frame_count != 0)
                {
                    num_bytes_uploaded += hex_data_size % PACKET_SIZE;
                }
                else  // reached the end of the frame,, need to correct number of bytes uploaded
                {
                    num_bytes_uploaded -= PACKET_SIZE;
                    num_bytes_uploaded += hex_data_size % PACKET_SIZE;
                }
                LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));

                // if the frame id is 1, that means that last packet was completed, so no filler frames
                if (curr_frame_id == KU::SEND_FRAME_1_ID)
                {
                    LOG_PRINT(("\n\n\n====NO FILLER====\n\n\n"));
                    return KU::END_OF_FILE_CODE;
                }

                // otherwise need to add filler frames
                LOG_PRINT(("\n\n\n====FILLER====\n\n\n"));

                memset(current_packet + CAN_DATA_LEN * frame_count, 0xFF,
                       sizeof(current_packet) - CAN_DATA_LEN * frame_count);
                for (int i = 0; i < PACKET_SIZE; i++)
                {
                    LOG_PRINT(("%02X", current_packet[i]));
                }
                LOG_PRINT(("\n"));

                // send the filler frames, need a switch statement because don't know which frame to start at
                // pause for 1ms before sending the next frame, avoid overflow
                while (true)
                {
                    switch (curr_frame_id)
                    {
                        case KU::SEND_FRAME_1_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_1_ID | set_7th, current_packet, CAN_DATA_LEN);
                            curr_frame_id = KU::SEND_FRAME_2_ID;
                            frame_count += 1;
                        }
                        case KU::SEND_FRAME_2_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_2_ID | set_7th, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            curr_frame_id = KU::SEND_FRAME_3_ID;
                            frame_count += 1;
                        }
                        case KU::SEND_FRAME_3_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_3_ID | set_7th, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            curr_frame_id = KU::SEND_FRAME_4_ID;
                            frame_count += 1;
                        }
                        case KU::SEND_FRAME_4_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_4_ID | set_7th, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            CO_CANrxMsg_t* resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back,
                                                                MEDIUM_WAIT_TIME, iap_can_id_mask);
                            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                            {
                                // try again
                                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME,
                                                     iap_can_id_mask);
                                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                                {
                                    return KU::PACKET_SENT_FAIL;
                                }
                            }
                            LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));
                            return KU::END_OF_FILE_CODE;
                        }
                    }
                }
            }

            // not the last line, just a normal frame. Pause for 1ms before next frame sent, avoid overflow
            switch (curr_frame_id)
            {
                case KU::SEND_FRAME_1_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_1_ID | set_7th, data, sizeof(data));
                    curr_frame_id = KU::SEND_FRAME_2_ID;
                    frame_count += 1;
                    break;
                }
                case KU::SEND_FRAME_2_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_2_ID | set_7th, data, sizeof(data));
                    curr_frame_id = KU::SEND_FRAME_3_ID;
                    frame_count += 1;
                    break;
                }
                case KU::SEND_FRAME_3_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_3_ID | set_7th, data, sizeof(data));
                    curr_frame_id = KU::SEND_FRAME_4_ID;
                    frame_count += 1;
                    break;
                }
                case KU::SEND_FRAME_4_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_4_ID | set_7th, data, sizeof(data));
                    CO_CANrxMsg_t* resp =
                        sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);
                    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                    {
                        // try again
                        resp =
                            sc->get_frame(KU::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);
                        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                        {
                            return KU::PACKET_SENT_FAIL;
                        }
                    }
                    num_bytes_uploaded += 32;
                    if (num_bytes_uploaded < hex_data_size)
                    {
                        LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));
                    }
                    return KU::PACKET_SENT_SUCCESS;
                }
            }
        }
    }
}
