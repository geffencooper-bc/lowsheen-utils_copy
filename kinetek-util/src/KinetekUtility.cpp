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

#include "KinetekUtility.h"
#include <argp.h>

KinetekUtility::KinetekUtility()
{
    ku_data = new KU::CanDataList;
    sc = new SocketCanHelper;

    // initialized when tool called
    iap = nullptr;
    stu = nullptr;
    ld = nullptr;

    can_initialized = false;
    can_interface = "can0";  // can0 by default
}

KinetekUtility::~KinetekUtility()
{
    delete iap;
    delete stu;
    delete sc;
    delete ld;
    delete ku_data;
}

// sets up socket can helper object and connects to can interface, channel name defaults to "can0"
KU::StatusCode KinetekUtility::init_can()
{
    // DEBUG_PRINTF("init can\r\n");
    if (can_initialized)
    {
        return KU::INIT_CAN_SUCCESS;
    }
    int err = sc->init_socketcan(can_interface.c_str());
    if (err == -1)
    {
        return KU::INIT_CAN_FAIL;
    }
    can_initialized = true;
    return KU::INIT_CAN_SUCCESS;
}

//  translates status code to a human readable string
string KinetekUtility::translate_status_code(KU::StatusCode status)
{
    switch (status)
    {
        case KU::INIT_CAN_FAIL:
        {
            return "Can initialization failed. Make sure to run ip link commands";
        }
        case KU::INIT_CAN_SUCCESS:
        {
            return "Can initialization successful";
        }
        case KU::IAP_MODE_FAIL:
        {
            return "Could not enter IAP mode. check can trace";
        }
        case KU::IAP_MODE_SUCCESS:
        {
            return "Entered IAP mode successfully";
        }
        case KU::FW_VERSION_REQUEST_FAIL:
        {
            return "Kinetek did not not receive FW revision request. check can trace";
        }
        case KU::START_DOWNLOAD_FAIL:
        {
            return "Kinetek did not receive the request to start download. check can trace";
        }
        case KU::SEND_START_ADDRESS_FAIL:
        {
            return "Kinetek did not receive the start address. check can trace";
        }
        case KU::SEND_CHECKSUM_FAIL:
        {
            return "Kinetek did not receive the hex file checksum. check can trace";
        }
        case KU::SEND_DATA_SIZE_FAIL:
        {
            return "Kinetek did not receive the hex file data size. check can trace";
        }
        case KU::INIT_PACKET_SUCCESS:
        {
            return "The initialization packets were received successfully";
        }
        case KU::PACKET_SENT_SUCCESS:
        {
            return "The hex packet was received by the Kinetek";
        }
        case KU::PACKET_SENT_FAIL:
        {
            return "The hex packet was not receive by the Kinetek. check can trace";
        }
        case KU::PAGE_CHECKSUM_FAIL:
        {
            return "The page checksum was not received or does not match the Kinetek. check can trace";
        }
        case KU::END_OF_FILE_CODE:
        {
            return "The end of the file has been reached";
        }
        case KU::PACKET_RESENT_FAIL:
        {
            return "The Kinetek did not receive the hex packet after resending. check can trace";
        }
        case KU::TOTAL_CHECKSUM_FAIL:
        {
            return "The total checksum was not received or does not match the Kinetek. check can trace";
        }
        case KU::NO_HEART_BEAT:
        {
            return "No heart beat was detected. check can trace";
        }
        case KU::END_OF_FILE_FAIL:
        {
            return "The Kinetek did not receive the end of file confirmation. check can trace";
        }
        case KU::UPLOAD_COMPLETE:
        {
            return "The hex file was uploaded successfully";
        }
        case KU::KU_INIT_ERROR:
        {
            return "The Kinetek Utility was not fully initialized.";
        }
        case KU::STU_READ_LINE_A_FAIL:
        {
            return "The first half of a stu line was not received. check can trace";
        }
        case KU::STU_READ_LINE_B_FAIL:
        {
            return "The second half of a stu line was not received. check can trace";
        }
        case KU::INVALID_STU_FILE:
        {
            return "This stu file is not valid. check can trace for more detail";
        }
        case KU::VALID_STU_FILE:
        {
            return "This stu file is valid";
        }
        case KU::STU_FILE_WRITE_FAIL:
        {
            return "The stu file was not written to the Kinetek successfully. check can trace";
        }
        case KU::STU_FILE_READ_SUCCESS:
        {
            return "The stu parameters were read to the file successfully";
        }
        case KU::STU_FILE_WRITE_SUCCESS:
        {
            return "The stu file was written successfully";
        }
        case KU::STU_PARAM_READ_A_FAIL:
        {
            return "Could not read stu param successfully";
        }
        case KU::STU_PARAM_READ_B_FAIL:
        {
            return "Could not read stu param successfully";
        }
        case KU::STU_PARAM_WRITE_FAIL:
        {
            return "Could not write stu param successfully";
        }
        case KU::STU_PARAM_READ_SUCCESS:
        {
            return "The stu parameter was read successfully";
        }
        case KU::STU_PARAM_WRITE_SUCCESS:
        {
            return "The stu parameter was written successfully";
        }
        case KU::INVALID_INI_FILE:
        {
            return "The ini sections are invalid. Delete the file and run the tool again";
        }
        case KU::NO_ERROR:
        {
            return "No error";
        }
        default:
        {
            return "Unknown status code";
        }
    }
}

KU::StatusCode KinetekUtility::run_iap(const string& file_path, bool iap_mode)
{
    if (iap == nullptr)
    {
        iap = new IAP(sc, ku_data);
    }

    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF("Can not initialized. Call init_can\r\n");
        return KU::INIT_CAN_FAIL;
    }

    iap->load_hex_file(file_path);
    iap->print();  // hex file information

    // step 2: put Kinetek into IAP mode (fw download mode)
    KU::StatusCode status = iap->put_in_iap_mode(iap_mode);
    if (status == KU::IAP_MODE_SUCCESS)
    {
        // step 3: send initialization frames (hex file size, checksum, start address, etc)
        status = iap->send_init_frames();
        if (status == KU::INIT_PACKET_SUCCESS)
        {
            // step 4: upload the hex file
            status = iap->upload_hex_file();
            if (status == KU::UPLOAD_COMPLETE)
            {
                DEBUG_PRINTF("\n\n====== SUCCSESS ======\r\n");
            }
        }
    }
    if (status != KU::UPLOAD_COMPLETE)
    {
        DEBUG_PRINTF("Error: %s\r\n", translate_status_code(status).c_str());
    }
    return status;
}

KU::StatusCode KinetekUtility::read_stu_to_file(const string& file_path)
{
    if (stu == nullptr)
    {
        stu = new STUparam(sc, ku_data);
    }

    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF("Can not initialized. Call init_can\r\n");
        return KU::INIT_CAN_FAIL;
    }

    // step 2: read the stu params to the file
    KU::StatusCode status = stu->read_stu_params(file_path);
    if (status != KU::STU_FILE_READ_SUCCESS)
    {
        DEBUG_PRINTF("Error: %s\r\n", translate_status_code(status).c_str());
        return KU::STU_FILE_READ_FAIL;
    }
    else
    {
        DEBUG_PRINTF("Success: %s\r\n", translate_status_code(status).c_str());
        return KU::STU_FILE_READ_SUCCESS;
    }
}

// write stu to file
KU::StatusCode KinetekUtility::write_stu_from_file(const string& file_path)
{
    if (stu == nullptr)
    {
        stu = new STUparam(sc, ku_data);
    }

    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF("Can not initialized. Call init_can\r\n");
        return KU::INIT_CAN_FAIL;
    }
    // step 2: write the stu params to the file
    KU::StatusCode status = stu->write_stu_params(file_path);
    if (status != KU::STU_FILE_WRITE_SUCCESS)
    {
        DEBUG_PRINTF("Error: %s\r\n", translate_status_code(status).c_str());
        return KU::STU_FILE_WRITE_FAIL;
    }
    else
    {
        DEBUG_PRINTF("Success: %s\r\n", translate_status_code(status).c_str());
        return KU::STU_FILE_WRITE_SUCCESS;
    }
}

// read stu param
KU::StatusCode KinetekUtility::get_stu_param(int param_num)
{
    if (stu == nullptr)
    {
        stu = new STUparam(sc, ku_data);
    }

    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF(("Can not initialized. Call init_can\r\n"));
        return KU::INIT_CAN_FAIL;
    }
    // step 2: read stu param
    if (param_num > 254 || param_num < 0)
    {
        DEBUG_PRINTF("Error: value invalid. Must be 0-255\r\n");
        exit(EXIT_FAILURE);
    }
    int param_value = stu->get_stu_param(param_num);
    if (param_value < 0)
    {
        DEBUG_PRINTF("Error: %s\r\n", translate_status_code((KU::StatusCode)param_value).c_str());
        return KU::STU_PARAM_READ_FAIL;
    }
    if (param_value == 255)
    {
        printf("Parameter not used\r\n");
    }
    else
    {
        printf("STU PARAM #%i: %i\r\n", param_num, param_value);
    }
    return KU::STU_PARAM_READ_SUCCESS;
}

// write stu param
KU::StatusCode KinetekUtility::set_stu_param(int param_num, int new_value)
{
    if (stu == nullptr)
    {
        stu = new STUparam(sc, ku_data);
    }

    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF("Can not initialized. Call init_can\r\n");
        return KU::INIT_CAN_FAIL;
    }
    // step 2: write stu param
    if (param_num > 254 || param_num < 0)
    {
        DEBUG_PRINTF("Error: value invalid. Must be 0-254\r\n");
        exit(EXIT_FAILURE);
    }
    if (new_value > 254 || new_value < 0)
    {
        DEBUG_PRINTF("Error: value invalid. Must be 0-254\r\n");
        exit(EXIT_FAILURE);
    }
    KU::StatusCode status = stu->set_stu_param(param_num, new_value);
    if (status != KU::STU_PARAM_WRITE_SUCCESS)
    {
        DEBUG_PRINTF("Error: %s\r\n", translate_status_code(status).c_str());
    }
    else
    {
        DEBUG_PRINTF("Success: %s\r\n", translate_status_code(status).c_str());
    }
    return status;
}

KU::StatusCode KinetekUtility::reset_xt_can()
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF("Can not initialized. Call init_can\r\n");
        return KU::INIT_CAN_FAIL;
    }

    sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->reset_xt_can_data, sizeof(ku_data->reset_xt_can_data));
    return KU::NO_ERROR;
}

KU::StatusCode KinetekUtility::toggle_estop(int mode)
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        DEBUG_PRINTF("Can not initialized. Call init_can\r\n");
        return KU::INIT_CAN_FAIL;
    }
    if (mode == 1)
    {
        sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->disable_kinetek_data, sizeof(ku_data->disable_kinetek_data));
    }
    else if (mode == 2)
    {
        sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->enable_kinetek_data, sizeof(ku_data->enable_kinetek_data));
    }
    return KU::NO_ERROR;
}

// ===================================== Argument Parsing ================================================

// determines if arg is a number, returns number if it is and -1 if not
static int is_number(char* arg)
{
    // if hex, remove the 0x
    if (strlen(arg) >= 3)
    {
        if (arg[0] == '0' && arg[1] == 'x')
        {
            arg += 2;
            return strtol(arg, NULL, 16);
        }
    }

    // determine if it is a number or file
    for (int i = 0; i < strlen(arg); i++)
    {
        if (isdigit(arg[i]) == false)
        {
            return -1;
        }
    }
    return strtol(arg, NULL, 10);
}

static struct argp_option options[] = {
    {"read", 'r', "ARG", 0, "Read a parameter or file"},
    {"write", 'w', "ARG", 0, "Write a parameter or file\nParameter requires value argument"},
    {"interface", 'i', "NAME", 0, "Specify interface name, can0 by default"},
    {"cycle", 'c', 0, 0, "Reset xt can"},
    {"estop", 'e', "STATE", 0, "Toggle estop, 1 = trigger estop 2 = disable estop"},
    {"value", 'v', "VAL", 0, "Value arg for write parameter"},
    {"heartbeat", 'h', 0, 0, "Launch the live data output"},
    {"selective", 's', "ARG", 0, "sel mode"},
    {0}};

// callback function
static int parse_opt(int key, char* arg, struct argp_state* state)
{
    // pass in the utility object to the callback function
    KinetekUtility* ku = (KinetekUtility*)(state->input);

    // make sure a param num is set before writing stu parameter, see case 'v'
    static int param_num = -1;

    // remove leading spaces
    if (arg != NULL)
    {
        int start = 0;
        while (arg[start] == ' ')
        {
            start++;
        }
        arg += start;
    }

    // if this is the first call and no can interface specified, use default;
    static bool first_call = true;
    if (key != 'i' && first_call)
    {
        ku->CL_status = ku->init_can();
        first_call = false;
        if (ku->CL_status != KU::INIT_CAN_SUCCESS)
        {
            return -1;
        }
    }

    switch (key)
    {
        case 's':
        {
            ku->CL_status = ku->run_iap((string)arg, 0);
            break;
        }
        case 'r':
        {
            // distinguish between individual parameter and file
            int num = is_number(arg);
            if (num >= 0)
            {
                ku->CL_status = ku->get_stu_param(num);
            }
            else
            {
                ku->CL_status = ku->read_stu_to_file(string(arg));
            }
            break;
        }
        case 'w':
        {
            int num = is_number(arg);
            if (num >= 0)
            {
                param_num = num;  // call set_stu_param with -v option
            }
            else
            {
                string file_type = string(arg).substr(strlen(arg) - 3, 3);
                if (file_type == "hex")
                {
                    ku->CL_status = ku->run_iap(string(arg), 1);
                }
                else if (file_type == "stu")
                {
                    ku->CL_status = ku->write_stu_from_file(string(arg));
                }
                else
                {
                    DEBUG_PRINTF("Invalid file. Must be .stu or .hex\r\n");
                    return -1;
                }
            }
            break;
        }
        case 'v':
        {
            if (param_num == -1)
            {
                DEBUG_PRINTF("Usage: kintek-util [-w PARAM# -v VAL]\r\n");
                return -1;
            }
            ku->CL_status = ku->set_stu_param(param_num, atoi(arg));
            break;
        }
        case 'c':
        {
            ku->CL_status = ku->reset_xt_can();
            if (ku->CL_status != KU::NO_ERROR)
            {
                return -1;
            }
            break;
        }
        case 'i':
        {
            ku->set_can_interface(string(arg));
            ku->CL_status = ku->init_can();
            first_call = false;
            if (ku->CL_status != KU::INIT_CAN_SUCCESS)
            {
                return -1;
            }
            break;
        }
        case 'h':
        {
            ku->CL_status = ku->get_live_data();
            if (ku->CL_status != KU::NO_ERROR)
            {
                return -1;
            }
            break;
        }
        case 'e':
        {
            ku->CL_status = ku->toggle_estop(atoi(arg));
            if (ku->CL_status != KU::NO_ERROR)
            {
                return -1;
            }
            break;
        }
    }
    return 0;
}

int KinetekUtility::parse_args(int argc, char** argv)
{
    struct argp argp = {options, parse_opt};
    return argp_parse(&argp, argc, argv, 0, 0, this);
}

KU::StatusCode KinetekUtility::get_live_data()
{
    if (ld == nullptr)
    {
        ld = new LiveData(sc, ku_data);
    }
    return ld->update_heartbeat();
}

// callback function for received messages, not used as of now
void test_resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
// nothing needed
}

void KinetekUtility::test_iap_window(int delay_time, int tries)
{
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    CO_CANrxMsg_t* resp;
    int count = 0;

    // first reset the Kinetek by toggling the estop line
    sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->disable_kinetek_data, sizeof(ku_data->disable_kinetek_data));
    usleep(2500000); // sleep for 2.5 seconds
    // turn on the kinetek and wait some time before trying to send the fored command
    sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->enable_kinetek_data, sizeof(ku_data->enable_kinetek_data));

    // start the clock
    begin = std::chrono::steady_clock::now();
    end = std::chrono::steady_clock::now();

    // wait for window time
    while(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0 < delay_time)
    {
        end = std::chrono::steady_clock::now();
    }
    uint8_t mask = 0b00011111;
    // after window finishes, attempt to enter iap mode
    while(true)
    {
        if(count == tries)
        {
            printf("passed count");
            exit(EXIT_FAILURE);
        }
        // start sending the forced command
        sc->send_frame(KU::FORCE_ENTER_IAP_MODE_ID, ku_data->force_enter_iap_mode_data,
        sizeof(ku_data->force_enter_iap_mode_data));

        resp = sc->get_frame(KINETEK_STATUS_1_ID, this, test_resp_call_back, 1, mask);

        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::IN_IAP_MODE)
        {
            end = std::chrono::steady_clock::now();
            DEBUG_PRINTF("\nSTATE ID: %02X\r\n", resp->ident);
            break;
        }
        else if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) == KU::HEART_BEAT)
        {
            end = std::chrono::steady_clock::now();
            printf("\nFAILED\n");
            exit(EXIT_FAILURE);
        }
        count++;
    }
    printf("\n-----------------------\nIN IAP");
    printf("\nCount: %i Time: %fms", count, std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0);
}