//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//==================================================================

#include "IAP.h"

//#define PROGRESS_BAR

// these are timeout values used for waiting for response frames
#define LONG_WAIT_TIME 3000   // ms --> 3 sec
#define MEDIUM_WAIT_TIME 100  // ms
#define SHORT_WAIT_TIME 10    // ms

// init member variables, IAP needs access to the Kinetek Utility can data and socket can helper
IAP::IAP(SocketCanHelper* sc, KU::CanDataList* ku_data)
{
    if (sc == nullptr)
    {
        DEBUG_PRINTF("ERROR: Socket Can Helper is not initialized\r\n");
        exit(EXIT_FAILURE);
    }
    this->sc = sc;

    if (ku_data == nullptr)
    {
        DEBUG_PRINTF("ERROR: Kinetek Utility CanDataList is not initialized\r\n");
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

    ut = nullptr;
}

// deallocate memory
IAP::~IAP()
{
    delete ut;
}

// init hex utility object and grab needed data from hex file
void IAP::load_hex_file(string file_path)
{
    if (ut == nullptr)
    {
        ut = new HexUtility();
    }
    else
    {
        // already called before this, reset member variables
        clear();
    }

    // initialize the hex utility object by loading the hex file
    ut->load_hex_file_data(file_path);

    // initialize iap request data frames using hex file info
    hex_data_size = ut->get_file_data_size(ku_data->hex_data_size_data + 1, KT_DATA_SIZE_LEN);
    total_checksum = ut->get_total_cs(ku_data->total_checksum_data + 1, KT_CS_LEN);  // sent as an init frame
    ut->get_total_cs(ku_data->calculate_total_checksum_data + 1, KT_CS_LEN,
                     true);  // sent after upload in little endian
    start_address = ut->get_start_address(ku_data->start_address_data + 1, KT_ADDRESS_LEN);

    // Kinetek needs to know last packet size
    last_packet_size = hex_data_size % PACKET_SIZE;
}

// sanity check to make sure reading file correctly
void IAP::print()
{
    DEBUG_PRINTF("\n================= IAP DETAILS ===============\r\n");
    DEBUG_PRINTF("HEX FILE DATA SIZE:             %i bytes\r\n", hex_data_size);
    DEBUG_PRINTF("HEX FILE DATA TOTAL CHECKSUM:   %08X\r\n", total_checksum);
    DEBUG_PRINTF("START ADDRESS:                  %08X\r\n", start_address);
    DEBUG_PRINTF("LAST PACKET SIZE:               %i\r\n", last_packet_size);
    DEBUG_PRINTF("=============================================\n\r\n");
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
void IAP_resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

KU::StatusCode IAP::put_in_iap_mode(bool forced_mode)
{
    if(forced_mode == true)
    {
        DEBUG_PRINTF("Putting in IAP mode using Forced command\r\n");
    }
    else
    {
        DEBUG_PRINTF("Putting in IAP mode using Selective command\r\n");
    }

    // selective mode
    if (!forced_mode)
    {
        // first wait for a normal heart beat before trying to send selective command
        CO_CANrxMsg_t* resp = sc->get_frame(KU::HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEARTBEAT)
        {
            resp = sc->get_frame(KU::HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
            DEBUG_PRINTF("ERROR: No Heartbeat detected, can't attempt selective command\r\n");
            return KU::NO_HEARTBEAT_DETECTED;
        }

        // do soft reset by sending selective download command
        sc->send_frame(KU::KINETEK_REQUEST_ID, ku_data->enter_iap_mode_selective_data,
                    sizeof(ku_data->enter_iap_mode_selective_data));
        
        // check if received a 0x081 selective response from the Kinetek
        resp = sc->get_frame(KU::KINETEK_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ENTER_IAP_MODE_SELECTIVE_RESPONSE)
        {
            // may have missed respoonse, continue anyways in case entered IAP mode
            DEBUG_PRINTF("WARNING: Kinetek did not confirm selective IAP request\r\n");
        }

        // check if entered IAP mode, selective defaults to 0x080 which according to Laurence is the id the LCD uses for IAP
        resp = sc->get_frame(KU::LCD_IAP_HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::IN_IAP_MODE)
        {
            // force the Kinetek to use hand held programmer Can Ids (0x060 Heartbeat)
            sc->send_frame(KU::HAND_HELD_PROGRAMMER_ID, ku_data->use_hand_held_programmer_ids_data, sizeof(ku_data->use_hand_held_programmer_ids_data));
            
            // make sure that the new heartbeat is received
            resp = sc->get_frame(KU::IAP_HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::IN_IAP_MODE)
            {
                return KU::IAP_MODE_SUCCESS;
            }
        }
        return KU::IAP_HEARTBEAT_TIMEOUT;
    }
    // forced mode
    else
    {
        KU::StatusCode iap_status;
        std::chrono::steady_clock::time_point begin;
        std::chrono::steady_clock::time_point end;
        int forced_tries = 0; // number of forced commands sent

        // first turn off the Kinetek by toggling the estop line
        sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->disable_kinetek_data, sizeof(ku_data->disable_kinetek_data));
        usleep(2500000);  // sleep for 2.5 seconds

        // then turn on the kinetek
        sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->enable_kinetek_data, sizeof(ku_data->enable_kinetek_data));
        begin = std::chrono::steady_clock::now();
        end = std::chrono::steady_clock::now();
    
        // wait until get close the 6ms forced window
        while(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0 < 38)
        {
            end = std::chrono::steady_clock::now();
        }

        // send IAP request to force the Kinetek to enter IAP mode
        sc->send_frame(KU::IAP_REQUEST_ID, ku_data->force_enter_iap_mode_data,
                    sizeof(ku_data->force_enter_iap_mode_data));

        // send the request 10 times at a 1ms interval
        while (forced_tries < 10)
        {
            sc->send_frame(KU::IAP_REQUEST_ID, ku_data->force_enter_iap_mode_data, sizeof(ku_data->force_enter_iap_mode_data));
            usleep(1500);
            forced_tries++;
        }

        // wait until get close to first possible IAP heartbeat
        while(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0 < 80)
        {
            end = std::chrono::steady_clock::now();
        }

        // force the Kinetek to use hand held programmer Can Ids (0x060 Heartbeat)
        sc->send_frame(KU::HAND_HELD_PROGRAMMER_ID, ku_data->use_hand_held_programmer_ids_data, sizeof(ku_data->use_hand_held_programmer_ids_data));

        // check if entered IAP mode
        CO_CANrxMsg_t* resp = sc->get_frame(KU::IAP_HEARTBEAT_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::IN_IAP_MODE)
        {
            return KU::IAP_MODE_SUCCESS;
        }

        // if did not enter IAP mode, check for a normal heartbeat
        else
        {
            resp = sc->get_frame(KU::HEARTBEAT_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
            if(ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::HEARTBEAT)
            {
                return KU::HEARTBEAT_DETECTED;
            }
        }
        return KU::NO_HEARTBEAT_DETECTED;
    }
}

KU::StatusCode IAP::send_init_frames()
{
    DEBUG_PRINTF("\n====== SEND INIT PACKETS ======\r\n");
    // first send the fw version request
    sc->send_frame(KU::FW_VERSION_REQUEST_ID, ku_data->fw_version_request_data,
                   sizeof(ku_data->fw_version_request_data));
    CO_CANrxMsg_t* resp =
        sc->get_frame(KU::FW_VERSION_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::FW_VERSION_RESPONSE)
    {
        DEBUG_PRINTF("ERROR: Kinetek version request timeout\r\n");
        return KU::FW_VERSION_REQUEST_FAIL;
    }

    DEBUG_PRINTF("Kinetek Bootloader Version: %i.%i\r\n", resp->data[0], resp->data[1]);

    // next send  a request to sent bytes
    sc->send_frame(KU::IAP_REQUEST_ID, ku_data->start_download_data, sizeof(ku_data->start_download_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::START_DOWNLOAD_RESPONSE)
    {
        DEBUG_PRINTF("ERROR: start download timeout\r\n");
        return KU::START_DOWNLOAD_FAIL;
    }
    DEBUG_PRINTF("Starting download\r\n");

    // next send the start address
    sc->send_frame(KU::IAP_REQUEST_ID, ku_data->start_address_data, sizeof(ku_data->start_address_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::START_ADDRESS_RESPONSE)
    {
        DEBUG_PRINTF("ERROR: start address timeout\r\n");
        return KU::SEND_START_ADDRESS_FAIL;
    }
    DEBUG_PRINTF("start address received\r\n");

    // next send the total checksum
    sc->send_frame(KU::IAP_REQUEST_ID, ku_data->total_checksum_data, sizeof(ku_data->total_checksum_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::TOTAL_CHECKSUM_RESPONSE)
    {
        DEBUG_PRINTF("ERROR: checksum timeout\r\n");
        return KU::SEND_CHECKSUM_FAIL;
    }
    DEBUG_PRINTF("checksum data received\r\n");

    // finally send the data size
    sc->send_frame(KU::IAP_REQUEST_ID, ku_data->hex_data_size_data, sizeof(ku_data->hex_data_size_data));
    resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEX_DATA_SIZE_RESPONSE)
    {
        DEBUG_PRINTF("ERROR: data size timeout\n\r\n");
        return KU::SEND_DATA_SIZE_FAIL;
    }

    DEBUG_PRINTF("data size received\r\n");
    return KU::INIT_PACKET_SUCCESS;
}

KU::StatusCode IAP::upload_hex_file()
{
    DEBUG_PRINTF("\n======== UPLOADING HEX FILE =======\r\n");

    while (true)  // keep sending packets until reached end of file or fail and function returns
    {
// update the progress bar
#ifdef PROGRESS_BAR
        progress_bar(num_bytes_uploaded, hex_data_size);
#endif

        // reached the end of a page
        if (packet_count > 0 && packet_count % PAGE_SIZE == 0)
        {
            // wait for Kinetek to send page checksum, try twice. If don't receive can still get confirmation by sending
            // page checksum
            CO_CANrxMsg_t* resp = sc->get_frame(KU::IAP_HEARTBEAT_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::KT_CALCULATED_PAGE_CHECKSUM)
            {
                // try again
                resp = sc->get_frame(KU::IAP_HEARTBEAT_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::KT_CALCULATED_PAGE_CHECKSUM)
                {
                    DEBUG_PRINTF("WARNING: Kinetek calculated page checksum timeout\r\n");
                }
            }
            // store the kinetek calculated page checksum
            memcpy(ku_data->kt_calculated_page_checksum_data, resp->data, sizeof(ku_data->kt_calculated_page_checksum_data));

            // convert self calculated page checksum into a list of bytes, copy it into the data of a page checksum frame
            ut->num_to_byte_list(curr_page_cs, ku_data->page_checksum_data + 1, KT_CS_LEN);
            ku_data->page_checksum_data[6] = page_count + 1;

            // send the page checksum frame to the kinetek, wait for confirmation, wait twice if need to
            sc->send_frame(KU::IAP_REQUEST_ID, ku_data->page_checksum_data,
                           sizeof(ku_data->page_checksum_data));
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) !=
                    KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    // if don't receive cs frame, may have missed it, as a last effort compare the KT calculated CS and
                    // your calculated CS
                    if (ku_data->array_compare(ku_data->page_checksum_data + 1, KT_CS_LEN,
                                               ku_data->kt_calculated_page_checksum_data + 1, KT_CS_LEN) == false)
                    {
                        DEBUG_PRINTF("ERROR: page checksum timeout\r\n");
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
                DEBUG_PRINTF("ERROR: packet resend timeout\r\n");
                return KU::PACKET_RESENT_FAIL;
            }
            num_bytes_uploaded += PACKET_SIZE;
            DEBUG_PRINTF("Bytes Uploaded: %i\r", num_bytes_uploaded);  // no-crlf-check
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

            sc->send_frame(KU::IAP_REQUEST_ID, ku_data->end_of_file_data, sizeof(ku_data->end_of_file_data));
            CO_CANrxMsg_t* resp =
                sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::END_OF_HEX_FILE_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::END_OF_HEX_FILE_RESPONSE)
                {
                    DEBUG_PRINTF("ERROR: end of file timeout\r\n");
                    return KU::END_OF_FILE_FAIL;
                }
            }

            // wait for the kinetek to send its calculated page checksum
            resp = sc->get_frame(KU::IAP_HEARTBEAT_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::KT_CALCULATED_PAGE_CHECKSUM)
            {
                DEBUG_PRINTF("WARNING: Kinetek calculated page checksum timeout\r\n");
            }

            // make last page checksum data
            ut->num_to_byte_list(curr_page_cs, ku_data->page_checksum_data + 1, 4);
            ku_data->page_checksum_data[6] = page_count + 1;

            // send last page checksum
            sc->send_frame(KU::IAP_REQUEST_ID, ku_data->page_checksum_data,
                           sizeof(ku_data->page_checksum_data));
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
            {
                // try again
                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) !=
                    KU::CALCULATE_PAGE_CHECKSUM_RESPONSE)
                {
                    DEBUG_PRINTF("ERROR: page checksum timeout\r\n");
                    return KU::PAGE_CHECKSUM_FAIL;
                }
            }
            // send total checksum
            sc->send_frame(KU::IAP_REQUEST_ID, ku_data->calculate_total_checksum_data,
                           sizeof(ku_data->calculate_total_checksum_data));
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);

            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::CALCULATE_TOTAL_CHECKSUM_RESPONSE)
            {
                DEBUG_PRINTF("ERROR: total checksum timeout\r\n");
                return KU::TOTAL_CHECKSUM_FAIL;
            }

            // first heartbeat message after update misses some information, wait till page one comes around again
            resp = sc->get_frame(KU::HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
            resp = sc->get_frame(KU::HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
            while (resp->data[1] != 1)
            {
                resp = sc->get_frame(KU::HEARTBEAT_ID, this, IAP_resp_call_back, LONG_WAIT_TIME);
                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEARTBEAT)
                {
                    DEBUG_PRINTF("ERROR: no heartbeat\r\n");
                    return KU::NO_HEARTBEAT_DETECTED;
                }
            }
            // error value is on page 1, byte 4
            if (resp->data[3] == 0)
            {
                return KU::UPLOAD_COMPLETE;
            }
            else
            {
                DEBUG_PRINTF("FW uploaded successfully but Kinetek has ERROR CODE: %02X\n", resp->data[3]);
                return KU::KINETEK_ERROR;
            }
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
        sc->send_frame(KU::RESEND_FRAME_1_ID, current_packet, sizeof(data));
        usleep(1500);  // delay to prevent TX overflow
        sc->send_frame(KU::RESEND_FRAME_2_ID, current_packet + 8, sizeof(data));
        usleep(1500);  // delay to prevent TX overflow
        sc->send_frame(KU::RESEND_FRAME_3_ID, current_packet + 16, sizeof(data));
        usleep(1500);  // delay to prevent TX overflow
        sc->send_frame(KU::RESEND_FRAME_4_ID, current_packet + 24, sizeof(data));
        CO_CANrxMsg_t* resp =
            sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, SHORT_WAIT_TIME);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
        {
            resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, SHORT_WAIT_TIME);
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
                else  // reached the end of the frame, need to correct number of bytes uploaded
                {
                    num_bytes_uploaded -= PACKET_SIZE;
                    num_bytes_uploaded += hex_data_size % PACKET_SIZE;
                }
                DEBUG_PRINTF("Bytes Uploaded: %i\r", num_bytes_uploaded);  // no-crlf-check

                // if the frame id is 1, that means that last packet was completed, so no filler frames
                if (curr_frame_id == KU::SEND_FRAME_1_ID)
                {
                   // DEBUG_PRINTF("\n\n====NO FILLER====\n\r\n");
                    return KU::END_OF_FILE_CODE;
                }

                memset(current_packet + CAN_DATA_LEN * frame_count, 0xFF,
                       sizeof(current_packet) - CAN_DATA_LEN * frame_count);
                DEBUG_PRINTF("\r\n");

                // send the filler frames, need a switch statement because don't know which frame to start at
                // pause for 1ms before sending the next frame, avoid overflow
                while (true)
                {
                    switch (curr_frame_id)
                    {
                        case KU::SEND_FRAME_1_ID:
                        {
                            usleep(1500);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_1_ID, current_packet, CAN_DATA_LEN);
                            curr_frame_id = KU::SEND_FRAME_2_ID;
                            frame_count += 1;
                        }
                        case KU::SEND_FRAME_2_ID:
                        {
                            usleep(1500);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_2_ID, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            curr_frame_id = KU::SEND_FRAME_3_ID;
                            frame_count += 1;
                        }
                        case KU::SEND_FRAME_3_ID:
                        {
                            usleep(1500);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_3_ID, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            curr_frame_id = KU::SEND_FRAME_4_ID;
                            frame_count += 1;
                        }
                        case KU::SEND_FRAME_4_ID:
                        {
                            usleep(1500);  // delay to prevent TX overflow
                            sc->send_frame(KU::SEND_FRAME_4_ID, current_packet + CAN_DATA_LEN * frame_count,
                                           CAN_DATA_LEN);
                            CO_CANrxMsg_t* resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back,
                                                                MEDIUM_WAIT_TIME);
                            if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                            {
                                // try again
                                resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
                                if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                                {
                                    return KU::PACKET_SENT_FAIL;
                                }
                            }
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
                    usleep(1500);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_1_ID, data, sizeof(data));
                    curr_frame_id = KU::SEND_FRAME_2_ID;
                    frame_count += 1;
                    break;
                }
                case KU::SEND_FRAME_2_ID:
                {
                    usleep(1500);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_2_ID, data, sizeof(data));
                    curr_frame_id = KU::SEND_FRAME_3_ID;
                    frame_count += 1;
                    break;
                }
                case KU::SEND_FRAME_3_ID:
                {
                    usleep(1500);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_3_ID, data, sizeof(data));
                    curr_frame_id = KU::SEND_FRAME_4_ID;
                    frame_count += 1;
                    break;
                }
                case KU::SEND_FRAME_4_ID:
                {
                    usleep(1500);  // delay to prevent TX overflow
                    sc->send_frame(KU::SEND_FRAME_4_ID, data, sizeof(data));
                    CO_CANrxMsg_t* resp =
                        sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
                    if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                    {
                        // try again
                        resp = sc->get_frame(KU::IAP_RESPONSE_ID, this, IAP_resp_call_back, MEDIUM_WAIT_TIME);
                        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::ACK_32_BYTES)
                        {
                            return KU::PACKET_SENT_FAIL;
                        }
                    }
                    num_bytes_uploaded += 32;
                    if (num_bytes_uploaded < hex_data_size)
                    {
                        DEBUG_PRINTF("Bytes Uploaded: %i\r", num_bytes_uploaded);  // no-crlf-check
                    }
                    return KU::PACKET_SENT_SUCCESS;
                }
            }
        }
    }
}

void IAP::clear()
{
    hex_data_size = 0;
    packet_count = 0;
    page_count = 0;
    num_bytes_uploaded = 0;
    curr_page_cs = 0;
    in_iap_mode = false;
    memset(current_packet, 0, sizeof(current_packet));

    ut->clear();
}
