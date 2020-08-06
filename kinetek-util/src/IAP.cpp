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
#include "KinetekCodes.h"

// debug print macro to see error messages
#define PRINT_LOG

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) \
    do               \
    {                \
    } while (0)
#define PROGRESS_BAR  // show a progress bar instead
#endif

// these are timeout values used for waiting for response frames
#define LONG_WAIT_TIME 3000   // ms --> 3 sec
#define MEDIUM_WAIT_TIME 100  // ms
#define SHORT_WAIT_TIME 10    // ms

// init member variables
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

    set_7th = 0;                   // 0 by default
    iap_can_id_mask = 0b00001111;  // only want four lsb
}

// deallocate memory
IAP::~IAP()
{
    delete ut;
    delete sc;
}

// init hex utility object and grab needed data from hex file
void IAP::load_hex_file(string file_path)
{
    // loads hex file
    ut = new HexUtility(file_path);

    // initialize iap request data frames using hex file info
    data_size_bytes = ut->get_file_data_size(KT::data_size_data + 1, KT_DATA_SIZE_LEN);
    total_checksum = ut->get_total_cs(KT::total_checksum_data + 1, KT_CS_LEN);  // sent as an init frame
    ut->get_total_cs(KT::calculate_total_checksum_data + 1, KT_CS_LEN, true);   // sent after upload in little endian
    start_address = ut->get_start_address(KT::start_address_data + 1, KT_ADDRESS_LEN);

    // // Kinetek needs to know last packet size
    last_packet_size = data_size_bytes % PACKET_SIZE;
}

// sanity check to make sure reading file correctly
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

// initialize the SocketCanHelper object and can communication
status_code IAP::init_can(const char* channel_name)
{
    int err = sc->init_socketcan(channel_name);
    if (err == -1)
    {
        return INIT_CAN_FAIL;
    }
    return INIT_CAN_SUCCESS;
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
   The IAP state is determined upon entering IAP mode and is independent
   of the mode (forced vs selective). There are two iap states which
   each use a different but related set of can ids. State 1 uses iap_can_id
   as defined in KineteCodes.h for commands. State 2 uses
   iap_can_id | 01000000 for commands (set the 7th bit high). Since the
   4 least significant bits are the same for responses for both states, the iap_can_id
   responses will have a bit mask of 00001111 --> iap_can_id & 00001111.

   The iap_can_id commands will be bitwise ORed with a variable called "set_7th".
   This function determines the IAP state and if in State 1 then set_7th = 0 and
   iap_can_ids will remain unchanged by the bitwise OR. If in State 2 then
   set_7th = 0b01000000 and the can_ids will changed accordingly.
*/
status_code IAP::check_iap_state(int wait_time)
{
    // check if in IAP state 1, if yes then set_7th = 0 and can ids are unchanged
    CO_CANrxMsg_t* resp = sc->get_frame(KINETEK_STATUS_1_ID, this, resp_call_back, wait_time);
    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) == KT::IN_IAP_MODE)
    {
        LOG_PRINT(("\nSTATE ID: %02X\n", resp->ident));
        set_7th = 0;
        iap_state = KINETEK_STATUS_1_ID;
    }
    // check if in IAP state 2, if yes then set_7th = 0b01000000 and can ids changed
    else
    {
        resp = sc->get_frame(KINETEK_STATUS_2_ID, this, resp_call_back, wait_time);
        if (KT::get_response_type(resp->ident, resp->data, resp->DLC) == KT::IN_IAP_MODE)
        {
            LOG_PRINT(("\nSTATE ID: %02X\n", resp->ident));
            set_7th = 0b01000000;
            iap_state = KINETEK_STATUS_2_ID;
        }
        else  // if neither states detected then return fail
        {
            LOG_PRINT(("Check IAP state time out\n"));
            return IAP_MODE_FAIL;
        }
    }

    LOG_PRINT(("\n\n======IN IAP MODE=========\n\n"));
    return IAP_MODE_SUCCESS;
}

status_code IAP::put_in_iap_mode(bool forced_mode)
{
    LOG_PRINT(("Putting in IAP mode\n"));

    // selective mode
    if (!forced_mode)
    {
        // wait for a heart beat before trying soft reset
        CO_CANrxMsg_t* resp = sc->get_frame(KT::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
        {
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::HEART_BEAT)
            {
                LOG_PRINT(("No Heart Beat detected, Selective Mode failed\n"));
                return IAP_MODE_FAIL;
            }
        }

        // next do soft reset by sending selective download command
        sc->send_frame(KT::KINETEK_COMMAND_ID, KT::enter_iap_mode_selective_data,
                       sizeof(KT::enter_iap_mode_selective_data));
        resp = sc->get_frame(KT::KINETEK_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);

        if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ENTER_IAP_MODE_SELECTIVE_RESPONSE)
        {
            // try again
            resp = sc->get_frame(KT::KINETEK_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ENTER_IAP_MODE_SELECTIVE_RESPONSE)
            {
                LOG_PRINT(("Selective FW Request time out\n"));
                return IAP_MODE_FAIL;
            }
        }

        // next check iap mode status, will init set_7th and return success/fail
        status_code iap_status = check_iap_state(MEDIUM_WAIT_TIME);
        return iap_status;
    }
    // forced mode
    else
    {
        // first reset the Kinetek by toggling the estop line
        sc->send_frame(KT::ESTOP_ID, KT::disable_kinetek_data, sizeof(KT::disable_kinetek_data));
        usleep(2000000);  // sleep for 2 seconds

        // turn on the kinetek and repeatedly send the force enter iap mode command
        sc->send_frame(KT::ESTOP_ID, KT::enable_kinetek_data, sizeof(KT::enable_kinetek_data));
        sc->send_frame(KT::FORCE_ENTER_IAP_MODE_IAP, KT::enter_iap_mode_forced_data,
                       sizeof(KT::enter_iap_mode_forced_data));

        status_code iap_status = check_iap_state(3);  // want to check the iap state quickly (3ms timeout)
        int count = 0;
        while (iap_status != IAP_MODE_SUCCESS)
        {
            if (count > 50)  // try 50 times
            {
                LOG_PRINT(("Forced IAP mode time out\n"));
                return IAP_MODE_FAIL;
            }
            sc->send_frame(KT::FORCE_ENTER_IAP_MODE_IAP, KT::enter_iap_mode_forced_data,
                           sizeof(KT::enter_iap_mode_forced_data));
            iap_status = check_iap_state(3);
            count++;
        }
    }

    return IAP_MODE_SUCCESS;
}

status_code IAP::send_init_frames()
{
    // first send the fw version request
    sc->send_frame(KT::FW_VERSION_REQUEST_ID | set_7th, KT::fw_version_request_data,
                   sizeof(KT::fw_version_request_data));
    CO_CANrxMsg_t* resp =
        sc->get_frame(KT::FW_VERSION_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::FW_VERSION_RESPONSE)
    {
        LOG_PRINT(("FW_VERSION_REQUEST TIMEOUT\n"));
        return FW_VERSION_REQUEST_FAIL;
    }
    LOG_PRINT(("GOT FW VERSION RESPONSE\n"));

    printf("\nKinetek Bootloader Version: %i.%i\n", resp->data[0], resp->data[1]);

    usleep(1000);
    // next send  a request to sent bytes
    sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::send_bytes_data, sizeof(KT::send_bytes_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::SEND_BYTES_RESPONSE)
    {
        LOG_PRINT(("SEND_BYTES TIMEOUT\n"));
        return SEND_BYTES_FAIL;
    }
    LOG_PRINT(("CAN START SENDING BYTES\n"));

    usleep(1000);
    // next send the start address
    sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::start_address_data, sizeof(KT::start_address_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::START_ADDRESS_RESPONSE)
    {
        LOG_PRINT(("SEND_START_ADDRESS TIMEOUT\n"));
        return SEND_START_ADDRESS_FAIL;
    }
    LOG_PRINT(("SENT START ADDRESS\n"));

    usleep(1000);
    // next send the total checksum
    sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::total_checksum_data, sizeof(KT::total_checksum_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::TOTAL_CHECKSUM_RESPONSE)
    {
        LOG_PRINT(("SEND_CHECKSUM TIMEOUT\n"));
        return SEND_CHECKSUM_FAIL;
    }
    LOG_PRINT(("GOT CHECKSUM DATA RESPONSE\n"));

    usleep(1000);
    // finally send the data size
    sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::data_size_data, sizeof(KT::data_size_data));
    resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::DATA_SIZE_RESPONSE)
    {
        LOG_PRINT(("SEND_DATA_SIZE TIMEOUT\n"));
        return SEND_DATA_SIZE_FAIL;
    }

    LOG_PRINT(("SENT DATA SIZE\n====== DONE WITH INIT PACKETS ======\n"));
    return INIT_PACKET_SUCCESS;
}

status_code IAP::upload_hex_file()
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
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::KT_CALCULATED_PAGE_CHECKSUM)
            {
                // try again
                resp = sc->get_frame(iap_state, this, resp_call_back, MEDIUM_WAIT_TIME);
                if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::KT_CALCULATED_PAGE_CHECKSUM)
                {
                    LOG_PRINT(("KT CALCULATED PAGE_CHECKSUM TIMEOUT\n"));
                }
            }

            // convert page checksum into a list of bytes, copy it into the data of a page checksum frame
            ut->num_to_byte_list(curr_page_cs, KT::page_checksum_data + 1, KT_CS_LEN);
            KT::page_checksum_data[6] = page_count + 1;

            // send the page checksum frame to the kinetek, wait for confirmation, wait twice if need to
            sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::page_checksum_data, sizeof(KT::page_checksum_data));
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

                if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    LOG_PRINT(("last effort compare\n"));
                    // if don't receive cs frame, may have missed it, as a last effort compare the KT calculated CS and
                    // your calculated CS
                    if (KT::array_compare(KT::page_checksum_data + 1, KT_CS_LEN, resp->data + 1, KT_CS_LEN) == false)
                    {
                        LOG_PRINT(("PAGE_CHECKSUM TIMEOUT\n"));
                        return PAGE_CHECKSUM_FAIL;
                    }
                }
            }
            page_count += 1;
            curr_page_cs = 0;
        }

        // if not the end of a page, then send the next packet
        status_code status = send_hex_packet();
        if (status == PACKET_SENT_SUCCESS)
        {
            packet_count += 1;
            continue;
        }

        // if send_hex_packet fails then resend the packet
        else if (status == PACKET_SENT_FAIL)
        {
            status = send_hex_packet(true);
            if (status != PACKET_SENT_SUCCESS)
            {
                LOG_PRINT(("PACKET_RESEND TIMEOUT\n"));
                return PACKET_RESENT_FAIL;
            }
            num_bytes_uploaded += PACKET_SIZE;
            LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));
            packet_count += 1;
            continue;
        }

        // if reached end of file then send end of hex file, total checksum, the last page checksum, and check for a
        // heartbeat
        else if (status == END_OF_FILE_CODE)
        {
            // send end of file notification with the number of bytes of the last data packet (accounts for filler when
            // doing checksum)
            if (last_packet_size == 0)  // do this because 32 % 32 = 0
            {
                last_packet_size = PACKET_SIZE;
            }
            KT::end_of_file_data[1] = last_packet_size;  // need to tell Kinetek what the last packet size is

            sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::end_of_file_data, sizeof(KT::end_of_file_data));
            CO_CANrxMsg_t* resp =
                sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::END_OF_HEX_FILE_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);
                if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::END_OF_HEX_FILE_RESPONSE)
                {
                    LOG_PRINT(("END_OF_FILE TIMEOUT\n"));
                    return END_OF_FILE_FAIL;
                }
            }

            // wait for the kinetek to send its calculated page checksum
            resp = sc->get_frame(iap_state, this, resp_call_back, MEDIUM_WAIT_TIME);
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::KT_CALCULATED_PAGE_CHECKSUM)
            {
                LOG_PRINT(("KT_CALCULATED_PAGE_CHECKSUM TIMEOUT\n"));
            }

            // make last page checksum data
            ut->num_to_byte_list(curr_page_cs, KT::page_checksum_data + 1, 4);
            KT::page_checksum_data[6] = page_count + 1;

            // send last page checksum
            sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::page_checksum_data, sizeof(KT::page_checksum_data));
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME);
                if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    LOG_PRINT(("PAGE_CHECKSUM TIMEOUT\n"));
                    return PAGE_CHECKSUM_FAIL;
                }
            }
            // send total checksum
            sc->send_frame(KT::IAP_REQUEST_ID | set_7th, KT::calculate_total_checksum_data,
                           sizeof(KT::calculate_total_checksum_data));
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);

            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                LOG_PRINT(("TOTAL_CHECKSUM TIMEOUT\n"));
                return TOTAL_CHECKSUM_FAIL;
            }

            // check for a heartbeat
            resp = sc->get_frame(KT::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::HEART_BEAT)
            {
                LOG_PRINT(("HEART_BEAT TIMEOUT\n"));
                return NO_HEART_BEAT;
            }

            usleep(3000);

            // check for a heartbeat again
            resp = sc->get_frame(KT::HEART_BEAT_ID, this, resp_call_back, LONG_WAIT_TIME);
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::HEART_BEAT)
            {
                LOG_PRINT(("HEART_BEAT TIMEOUT\n"));
                return NO_HEART_BEAT;
            }
            return UPLOAD_COMPLETE;
        }
    }
}

status_code IAP::send_hex_packet(bool is_retry)
{
    // first check if the hex packet to be sent is a retry
    if (is_retry)
    {
        // resend the last packet using saved data from "current packet"
        KT::iap_can_id curr_frame_id = KT::RESEND_FRAME_1_ID;
        uint8_t data[CAN_DATA_LEN];
        sc->send_frame(KT::RESEND_FRAME_1_ID | set_7th, current_packet, sizeof(data));
        usleep(3000);  // delay to prevent TX overflow
        sc->send_frame(KT::RESEND_FRAME_2_ID | set_7th, current_packet + 8, sizeof(data));
        usleep(3000);  // delay to prevent TX overflow
        sc->send_frame(KT::RESEND_FRAME_3_ID | set_7th, current_packet + 16, sizeof(data));
        usleep(3000);  // delay to prevent TX overflow
        sc->send_frame(KT::RESEND_FRAME_4_ID | set_7th, current_packet + 24, sizeof(data));
        CO_CANrxMsg_t* resp =
            sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME, iap_can_id_mask);
        if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
        {
            resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, SHORT_WAIT_TIME, iap_can_id_mask);
            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
            {
                return PACKET_RESENT_FAIL;
            }
        }
        return PACKET_SENT_SUCCESS;
    }

    // otherwise send the next hex packet
    else
    {
        int frame_count = 0;
        uint8_t data[CAN_DATA_LEN];
        int sum;
        KT::iap_can_id curr_frame_id = KT::SEND_FRAME_1_ID;

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
                    num_bytes_uploaded += data_size_bytes % PACKET_SIZE;
                }
                else  // reached the end of the frame,, need to correct number of bytes uploaded
                {
                    num_bytes_uploaded -= PACKET_SIZE;
                    num_bytes_uploaded += data_size_bytes % PACKET_SIZE;
                }
                LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));

                // if the frame id is 1, that means that last packet was completed, so no filler frames
                if (curr_frame_id == KT::SEND_FRAME_1_ID)
                {
                    LOG_PRINT(("\n\n\n====NO FILLER====\n\n\n"));
                    return END_OF_FILE_CODE;
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
                        case KT::SEND_FRAME_1_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KT::SEND_FRAME_1_ID | set_7th, current_packet, CAN_DATA_LEN);
                            curr_frame_id = KT::SEND_FRAME_2_ID;
                            frame_count += 1;
                        }
                        case KT::SEND_FRAME_2_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KT::SEND_FRAME_2_ID | set_7th, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            curr_frame_id = KT::SEND_FRAME_3_ID;
                            frame_count += 1;
                        }
                        case KT::SEND_FRAME_3_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KT::SEND_FRAME_3_ID | set_7th, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            curr_frame_id = KT::SEND_FRAME_4_ID;
                            frame_count += 1;
                        }
                        case KT::SEND_FRAME_4_ID:
                        {
                            usleep(3000);  // delay to prevent TX overflow
                            sc->send_frame(KT::SEND_FRAME_4_ID | set_7th, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            CO_CANrxMsg_t* resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back,
                                                                MEDIUM_WAIT_TIME, iap_can_id_mask);
                            if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                            {
                                // try again
                                resp = sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME,
                                                     iap_can_id_mask);
                                if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                                {
                                    return PACKET_SENT_FAIL;
                                }
                            }
                            LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));
                            return END_OF_FILE_CODE;
                        }
                    }
                }
            }

            // not the last line, just a normal frame. Pause for 1ms before next frame sent, avoid overflow
            switch (curr_frame_id)
            {
                case KT::SEND_FRAME_1_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KT::SEND_FRAME_1_ID | set_7th, data, sizeof(data));
                    curr_frame_id = KT::SEND_FRAME_2_ID;
                    frame_count += 1;
                    break;
                }
                case KT::SEND_FRAME_2_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KT::SEND_FRAME_2_ID | set_7th, data, sizeof(data));
                    curr_frame_id = KT::SEND_FRAME_3_ID;
                    frame_count += 1;
                    break;
                }
                case KT::SEND_FRAME_3_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KT::SEND_FRAME_3_ID | set_7th, data, sizeof(data));
                    curr_frame_id = KT::SEND_FRAME_4_ID;
                    frame_count += 1;
                    break;
                }
                case KT::SEND_FRAME_4_ID:
                {
                    usleep(3000);  // delay to prevent TX overflow
                    sc->send_frame(KT::SEND_FRAME_4_ID | set_7th, data, sizeof(data));
                    CO_CANrxMsg_t* resp =
                        sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);
                    if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                    {
                        // try again
                        resp =
                            sc->get_frame(KT::IAP_RESPONSE_ID, this, resp_call_back, MEDIUM_WAIT_TIME, iap_can_id_mask);
                        if (KT::get_response_type(resp->ident, resp->data, resp->DLC) != KT::ACK_32_BYTES)
                        {
                            return PACKET_SENT_FAIL;
                        }
                    }
                    num_bytes_uploaded += 32;
                    if (num_bytes_uploaded < data_size_bytes)
                    {
                        LOG_PRINT(("BYTES UPLOADED: %i\n", num_bytes_uploaded));
                    }
                    return PACKET_SENT_SUCCESS;
                }
            }
        }
    }
}

// translates status code to a human readable string
string IAP::translate_status_code(status_code code)
{
    switch(code)
    {
        case INIT_CAN_FAIL:
        {
            return "Can initialization failed";
        }
        case INIT_CAN_SUCCESS:
        {
            return "Can initialization successful";
        }
        case IAP_MODE_FAIL:
        {
            return "Could not enter IAP mode";
        }
        case IAP_MODE_SUCCESS:
        {
            return "Entered IAP mode successfully";
        }
        case FW_VERSION_REQUEST_FAIL:
        {
            return "Kinetek did not not receive FW revision request";
        }
        case SEND_BYTES_FAIL:
        {
            return "Kinetek did not receive the request to start sending bytes";
        }
        case SEND_START_ADDRESS_FAIL:
        {
            return "Kinetek did not receive the start address";
        }
        case SEND_CHECKSUM_FAIL:
        {
            return "Kinetek did not receive the hex file checksum";
        }
        case SEND_DATA_SIZE_FAIL:
        {
            return "Kinetek did not receive the hex file data size";
        }
        case INIT_PACKET_SUCCESS:
        {
            return "The initialization packets were received successfully";
        }
        case PACKET_SENT_SUCCESS:
        {
            return "The hex packet was received by the Kinetek";
        }
        case PACKET_SENT_FAIL:
        {
            return "The hex packet was not receive by the Kinetek";
        }
        case PAGE_CHECKSUM_FAIL:
        {
            return "The page checksum was not received or does not match the Kinetek";
        }
        case END_OF_FILE_CODE:
        {
            return "The end of the file has been reached";
        }
        case PACKET_RESENT_FAIL:
        {
            return "The Kinetek did not receive the hex packet after resending";
        }
        case TOTAL_CHECKSUM_FAIL:
        {
            return "The total checksum was not received or does not match the Kinetek";
        }
        case NO_HEART_BEAT:
        {
            return "No heart beat was detected";
        }
        case END_OF_FILE_FAIL:
        {
            return "The Kinetek did not receive the end of file confirmation";
        }
        case UPLOAD_COMPLETE:
        {
            return "The hex file was uploaded successfully";
        }
    }
}
