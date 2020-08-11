#include "KinetekUtility.h"
#include <argp.h>

#define PRINT_LOG

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) \
    do               \
    {                \
    } while (0)
#endif

KinetekUtility::KinetekUtility()
{
    ku_data = new KU::CanDataList;
    sc = new SocketCanHelper;
    iap = new IAP(sc, ku_data);
    stu = new STUparam(sc, ku_data);
}

KinetekUtility::~KinetekUtility()
{
    delete iap;
    delete stu;
    delete sc;
    delete ku_data;
}

KU::StatusCode KinetekUtility::init_can(const char* channel_name)
{
    int err = sc->init_socketcan(channel_name);
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
            return "Could not enter IAP mode. Check print log";
        }
        case KU::IAP_MODE_SUCCESS:
        {
            return "Entered IAP mode successfully";
        }
        case KU::FW_VERSION_REQUEST_FAIL:
        {
            return "Kinetek did not not receive FW revision request. Check print log";
        }
        case KU::SEND_BYTES_FAIL:
        {
            return "Kinetek did not receive the request to start sending bytes. Check print log";
        }
        case KU::SEND_START_ADDRESS_FAIL:
        {
            return "Kinetek did not receive the start address. Check print log";
        }
        case KU::SEND_CHECKSUM_FAIL:
        {
            return "Kinetek did not receive the hex file checksum. Check print log";
        }
        case KU::SEND_DATA_SIZE_FAIL:
        {
            return "Kinetek did not receive the hex file data size. Check print log";
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
            return "The hex packet was not receive by the Kinetek. Check print log";
        }
        case KU::PAGE_CHECKSUM_FAIL:
        {
            return "The page checksum was not received or does not match the Kinetek. Check print log";
        }
        case KU::END_OF_FILE_CODE:
        {
            return "The end of the file has been reached";
        }
        case KU::PACKET_RESENT_FAIL:
        {
            return "The Kinetek did not receive the hex packet after resending. Check print log";
        }
        case KU::TOTAL_CHECKSUM_FAIL:
        {
            return "The total checksum was not received or does not match the Kinetek. Check print log";
        }
        case KU::NO_HEART_BEAT:
        {
            return "No heart beat was detected. Check print log";
        }
        case KU::END_OF_FILE_FAIL:
        {
            return "The Kinetek did not receive the end of file confirmation. Check print log";
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
            return "The first half of a stu line was not received. Check print log";
        }
        case KU::STU_READ_LINE_B_FAIL:
        {
            return "The second half of a stu line was not received. Check print log";
        }
        case KU::INVALID_STU_FILE:
        {
            return "This stu file is not valid. Check print log for more detail";
        }
        case KU::VALID_STU_FILE:
        {
            return "This stu file is valid";
        }
        case KU::STU_FILE_WRITE_FAIL:
        {
            return "The stu file was not written to the Kinetek successfully. Check print log";
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
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        LOG_PRINT(("Can not initialized. Call init_can\n"));
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
                LOG_PRINT(("\n\n====== SUCCSESS ======\n"));
            }
        }
    }
    if (status != KU::UPLOAD_COMPLETE)
    {
        LOG_PRINT(("Error: %s", translate_status_code(status).c_str()));
    }
    return status;
}

KU::StatusCode KinetekUtility::read_stu_to_file(const string& file_path)
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        LOG_PRINT(("Can not initialized. Call init_can\n"));
        return KU::INIT_CAN_FAIL;
    }

    // step 2: read the stu params to the file
    KU::StatusCode status = stu->read_stu_params(file_path);
    if (status != KU::STU_FILE_READ_SUCCESS)
    {
        LOG_PRINT(("Error: %s", translate_status_code(status).c_str()));
        return KU::STU_FILE_READ_FAIL;
    }
    else
    {
        LOG_PRINT(("Success: %s", translate_status_code(status).c_str()));
        return KU::STU_FILE_READ_SUCCESS;
    }
}

// write stu to file
KU::StatusCode KinetekUtility::write_stu_from_file(const string& file_path)
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        LOG_PRINT(("Can not initialized. Call init_can\n"));
        return KU::INIT_CAN_FAIL;
    }
    // step 2: write the stu params to the file
    KU::StatusCode status = stu->write_stu_params(file_path);
    if (status != KU::STU_FILE_WRITE_SUCCESS)
    {
        LOG_PRINT(("Error: %s", translate_status_code(status).c_str()));
        return KU::STU_FILE_WRITE_FAIL;
    }
    else
    {
        LOG_PRINT(("Success: %s", translate_status_code(status).c_str()));
        return KU::STU_FILE_WRITE_SUCCESS;
    }
}

// read stu param
KU::StatusCode KinetekUtility::get_stu_param(uint8_t param_num)
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        LOG_PRINT(("Can not initialized. Call init_can\n"));
        return KU::INIT_CAN_FAIL;
    }
    // step 2: read stu param
    int param_value = stu->get_stu_param(param_num);
    if (param_value < 0)
    {
        LOG_PRINT(("Error: %s", translate_status_code((KU::StatusCode)param_value).c_str()));
        return KU::STU_PARAM_READ_FAIL;
    }
    printf("STU PARAM #%i: %i\n", param_num, param_value);
    return KU::STU_PARAM_READ_SUCCESS;
}

// write stu param
KU::StatusCode KinetekUtility::set_stu_param(uint8_t param_num, uint8_t new_value)
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        LOG_PRINT(("Can not initialized. Call init_can\n"));
        return KU::INIT_CAN_FAIL;
    }
    // step 2: write stu param
    KU::StatusCode status = stu->set_stu_param(param_num, new_value);
    if (status != KU::STU_PARAM_WRITE_SUCCESS)
    {
        LOG_PRINT(("Error: %s", translate_status_code(status).c_str()));
    }
    else
    {
        LOG_PRINT(("Success: %s", translate_status_code(status).c_str()));
    }
    return status;
}

KU::StatusCode KinetekUtility::reset_xt_can()
{
    // step 1: check if interface accessible
    if (!can_initialized)
    {
        LOG_PRINT(("Can not initialized. Call init_can\n"));
        return KU::INIT_CAN_FAIL;
    }

    sc->send_frame(KU::XT_CAN_REQUEST_ID, ku_data->reset_xt_can_data, sizeof(ku_data->reset_xt_can_data));
    return KU::NO_ERROR;
}

// ===================================== Argument Parsing ================================================

static struct argp_option options[] = {{"set_param", 's', "PARAM#", 0, "Set a STU param, [-s PARAM# -v VAL]"},
                                       {"get_param", 'g', "PARAM#", 0, "Get a STU param"},
                                       {"value", 'v', "VAL", 0, "STU param value (used with set_param)"},
                                       {"read_stu", 'r', "FILENAME", 0, "Read STU params to a file"},
                                       {"write_stu", 'w', "FILENAME", 0, "Write STU params from a file"},
                                       {"update", 'u', "FILENAME", 0, "Update the Kinetek firmware"},
                                       {"cycle_xt", 'c', 0, 0, "Reset xt_can"},
                                       {0}};

// callback function
static int parse_opt(int key, char* arg, struct argp_state* state)
{
    KinetekUtility* ku = (KinetekUtility*)(state->input);
    static int param_num = -1;
    switch (key)
    {
        case 's':
        {
            param_num = atoi(arg);
            break;
        }
        case 'g':
        {
            int ar = atoi(arg);
            ku->get_stu_param(ar);
            break;
        }
        case 'r':
        {
            ku->read_stu_to_file(string(arg));
            break;
        }
        case 'w':
        {
            ku->write_stu_from_file(string(arg));
            break;
        }
        case 'u':
        {
            ku->run_iap(string(arg), 1);
            break;
        }
        case 'c':
        {
            ku->reset_xt_can();
            break;
        }
        case 'v':
        {
            if (param_num < 0)
            {
                printf("Need to provide a parameter number and a value\n");
            }
            int ar = atoi(arg);
            ku->set_stu_param(param_num, ar);
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