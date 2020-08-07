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

#include "STUparam.h"
#include "HexUtility.h"
#include <sstream>
#include <memory>
// debug print macro to see error messages
#define PRINT_LOG

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) do {} while (0)
#endif

// initializes objects, starts up socket can
STUparam::STUparam()
{
    sc = new SocketCanHelper;
    kt = new KinetekCodes;
}

// deallocate memory
STUparam::~STUparam()
{
    delete sc;
    delete kt;
}

// initialize the SocketCanHelper object and can communication
STUparam::stu_status STUparam::init_can(const char* channel_name)
{
    int err = sc->init_socketcan(channel_name);
    if (err == -1)
    {
        return INIT_CAN_FAIL;
    }
    return INIT_CAN_SUCCESS;
}

// callback function for received messages, not used as of now
void resp_call_back_stu(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

// formats stu data when writing to file
stringstream& stu_stream(stringstream& ss, int val, int fill_width)
{
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(fill_width) << val;
    return ss;
}

// gets stu parameters from kinetek and outputs to a file
STUparam::stu_status STUparam::read_stu_params(const string& output_file)
{
    // try to open the file(will overwrite), if DNE then create a new file  
    ofstream output;
    output.open(output_file, std::ofstream::trunc);
    if(output.fail())
    {
        output.open(output_file); // create a new file
    }

    stringstream stu_string; // store total stu output as stringstream for formatting

    // Part 1: get the stu header 
    string stu_header = ""; // stores stu header until gets written at end
    int stu_header_checksum = 0;
    int num_params = MAX_NUM_STU_PARAMS - INITIAL_UNUSED_PARAMS; // start at max then subtract away unused params
    printf("NUM STU: %i\n", num_params);
    // Kinetek id is always 29
    stu_header += "29, ";
    stu_header_checksum += 29;

    // get the firmware version from heart beat page 9
    CO_CANrxMsg_t* resp =  sc->get_frame(KinetekCodes::HEART_BEAT_ID, this, resp_call_back_stu, 20000);
    while(resp->data[1] != 9)
    {
        resp =  sc->get_frame(KinetekCodes::HEART_BEAT_ID, this, resp_call_back_stu, 20000);
        if(kt->get_response_type(resp->ident, resp->data, resp->DLC) != KinetekCodes::HEART_BEAT)
        {
            LOG_PRINT(("NO HEART BEAT\n"));
            return NO_HEART_BEAT;
        }
    }
    // write the rest of the header: firmware version major/minor, number of params
    stu_header += std::to_string(resp->data[3]) + ", " + std::to_string(resp->data[4]) + ", ";
    stu_header_checksum += resp->data[3] + resp->data[4];

    // Part 2: get the stu parameters
    int stu_row_i = 0;
    int row_checksum = 0;
    CO_CANrxMsg_t respA; // first 8 bytes
    CO_CANrxMsg_t respB; // second 8 bytes

    // keep reading parameters until all rows read
    while(stu_row_i < NUM_STU_ROWS)
    {
        PRINT_LOG(("READING ROW %i\n", stu_row_i));
        // initialize read request data with according eeprom address and get the row response data
        kt->eeprom_access_read_request_data[3] = ROW_SIZE*stu_row_i;
        sc->send_frame(KinetekCodes::EEPROM_ACCESS_MESSAGE_ID, kt->eeprom_access_read_request_data, sizeof(kt->eeprom_access_read_request_data));
    
        memcpy(&respA, sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_A_ID, this, resp_call_back_stu, 100), sizeof(CO_CANrxMsg_t));
        memcpy(&respB, sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_B_ID, this, resp_call_back_stu, 100), sizeof(CO_CANrxMsg_t));

        // validate response A, first 8 bytes
        if(kt->get_response_type(respA.ident, respA.data, respA.DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
        {
            LOG_PRINT(("DID NOT RECEIVE A"));
            return READ_A_FAIL;
        }

        stu_stream(stu_string, ROW_SIZE*stu_row_i, 4) << ", "; // grab row address

        // grab the stu data from the response A and increment checksum
        for(int j = 0; j < respA.DLC; j+=2)
        {
            if(stu_row_i > 1)
            {
                // if have unused params, then subtract from number of params
                if(respA.data[j] == 0xFF)
                {
                    num_params--;
                }
                if(respA.data[j+1] == 0xFF)
                {
                    num_params--;
                }
            }
            row_checksum += respA.data[j] + respA.data[j+1];
            stu_stream(stu_string, respA.data[j], 2); stu_stream(stu_string, respA.data[j+1], 2) << ", ";
        } 

        // validate response B, second 8 bytes
        if(kt->get_response_type(respB.ident, respB.data, respB.DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
        {
            LOG_PRINT(("DID NOT RECEIVE B"));
            return READ_B_FAIL;
        }
        
        // grab the stu data from response B and increment checksum
        for(int j = 0; j < respB.DLC; j+=2)
        {
            if(stu_row_i > 1)
            {
                // if have unused params, then subtract from number of params
                if(respB.data[j] == 0xFF)
                {
                    num_params--;
                }
                if(respB.data[j+1] == 0xFF)
                {
                    num_params--;
                }
            }
            row_checksum += respB.data[j] + respB.data[j+1];
            stu_stream(stu_string, respB.data[j], 2); stu_stream(stu_string, respB.data[j+1], 2) << ", ";
        } 
        row_checksum += ROW_SIZE*stu_row_i; // add the address to the row checksum

        // append the row checksum to the stu stream
        if(stu_row_i == NUM_STU_ROWS - 1)
        {
            stu_stream(stu_string, row_checksum, 4);
        }
        else
        {
            stu_stream(stu_string, row_checksum, 4) << '\n';
        }
        stu_row_i++;
        row_checksum = 0;
        printf("NUM STU: %i\n", num_params);
    }

    // complete the stu header
    stu_header_checksum += num_params;
    stu_header += std::to_string(num_params) + ", " + std::to_string(stu_header_checksum) + "\n";

    // write stu header and stu data to the file
    output << stu_header << stu_string.str();
    output.close();
    return STU_READ_SUCCESS;
}

STUparam::stu_status STUparam::write_stu_params(const string& input_file)
{
    // make sure the stu file is valid
    stu_status status = validate_stu_file(input_file);
    if(status == INVALID_STU_FILE)
    {
        LOG_PRINT(("BAD STU FILE\n"));
        return status;
    }

    LOG_PRINT(("VALID STU FILE\n"));
    // write stu file
    ifstream stu_file;
    stu_file.open(input_file);
    if(stu_file.fail())
    {
        LOG_PRINT(("CAN'T OPEN STU FILE\n"));
        return INVALID_STU_FILE;
    }

     // send request, then the row part A and B
    string curr_line = "";
    int curr_line_i = 0;
    hu_getline(stu_file, curr_line); // skip the header
    
    while(hu_getline(stu_file, curr_line)) // keep writing until go through all lines, or fail
    {
        kt->eeprom_access_write_request_data[3] = 16*curr_line_i; // fill in address
        // convert the row into an array of bytes
        stu_line_to_byte_array(curr_line, kt->eeprom_access_line_write_data, sizeof(kt->eeprom_access_line_write_data));
        sc->send_frame(KinetekCodes::EEPROM_ACCESS_MESSAGE_ID, kt->eeprom_access_write_request_data, sizeof(kt->eeprom_access_write_request_data));
        usleep(1000);
        sc->send_frame(KinetekCodes::EEPROM_LINE_WRITE_A_ID, kt->eeprom_access_line_write_data, CAN_DATA_LEN);
        usleep(1000);
        sc->send_frame(KinetekCodes::EEPROM_LINE_WRITE_B_ID, kt->eeprom_access_line_write_data+8, CAN_DATA_LEN);
        usleep(1000);

        CO_CANrxMsg_t* resp = sc->get_frame(KinetekCodes::EEPROM_LINE_WRITE_RESPONSE_ID, this, resp_call_back_stu, 100000);
        if(kt->get_response_type(resp->ident, resp->data, resp->DLC) != KinetekCodes::EEPROM_ACCESS_WRITE_RESPONSE)
        {
            LOG_PRINT(("NO WRITE RESPONSE"));
            return WRITE_FAIL;
        }
        curr_line_i++;
    }
    stu_file.close();

    // reset the kinetek, check for error message
    sc->send_frame(KinetekCodes::ESTOP_ID, kt->disable_kinetek_data, sizeof(kt->disable_kinetek_data));
    usleep(2000000);  // sleep for 2 seconds
    sc->send_frame(KinetekCodes::ESTOP_ID, kt->enable_kinetek_data, sizeof(kt->enable_kinetek_data));
    // check for errors?
    return STU_WRITE_SUCCESS;
}

STUparam::stu_status STUparam::validate_stu_file(const string& input_file)
{
    // Check 1: try to open the file
    fstream stu_file;
    stu_file.open(input_file, std::ios::out | std::ios::in); // read/write
    if(stu_file.fail())
    {
        LOG_PRINT(("CANT OPEN\n"));
        return INVALID_STU_FILE;
    }

    // Check 2: validate header checksum
    string curr_line = "";
    hu_getline(stu_file, curr_line); 
    int header_checksum = 0;
    string value = "";
    // go character by character to get the values
    for(int i = 0; i < curr_line.size(); i++)
    {
        if(curr_line[i] == ',')
        {
            header_checksum += std::stoi(value);
            value = "";
            i++; // acount for the space
        }
        else
        {
            value += curr_line[i];
        }
    }
    // compare the calculated checksum to the expected checksum
    if(header_checksum != std::stoi(value))
    {
        LOG_PRINT(("BAD HEADER CHECKSUM\n"));
        return INVALID_STU_FILE;
    }

    // Check 3: zero-out the first 28 parameters
    string stu_file_string = curr_line + "\n";
    hu_getline(stu_file, curr_line); // read stu line 1
    hu_getline(stu_file, curr_line); // read stu line 2
    stu_file_string += "0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000\n"; // first stu line zero
    stu_file_string += "0010, 0000, 0000, 0000, 0000, 0000, 0000, " + curr_line.substr(42, 10); // grab parameters 29-32 from stu line 2

    // get the sum of parameters 29-32 (row checksum)
    int start_params_sum = std::stoi(curr_line.substr(42, 2), 0, 16) + 
                           std::stoi(curr_line.substr(44, 2), 0, 16) +
                           std::stoi(curr_line.substr(48, 2), 0, 16) +
                           std::stoi(curr_line.substr(50, 2), 0, 16) + ROW_SIZE;
    int total_stu_checksum = start_params_sum - ROW_SIZE; // pure stu checksum

    // append row 2 (params 29-32) checksum
    stringstream stream;
    stu_stream(stream, start_params_sum, 4);
    stu_file_string += ", " + stream.str() + "\n";

    // append the rest of the file to the stu string
    while(hu_getline(stu_file, curr_line))
    {
        stu_file_string += curr_line + "\n";
    }

    // write back to the file before validation
    stu_file.clear();
    stu_file.seekp(0, std::ios::beg);
    stu_file << stu_file_string;

    stu_file.clear();
    stu_file.seekp(0, std::ios::beg); 

    // Check 4: validate row checksums
    int curr_line_i = 0;
    string last_4_bytes = ""; // stores the pure stu checksum
    hu_getline(stu_file, curr_line); // start at line 1

    // read all lines in file, calculate row checksums, compare to expected checkums, increment pure stu checksum
    string first_byte;
    string second_byte;
    while(hu_getline(stu_file, curr_line))
    {
        int line_checksum = 16*curr_line_i; // include row address in row checksum
        int expected_checksum = std::stoi(curr_line.substr(curr_line.size()-4, 4), 0, 16);

        // calculate row checksums
        for(int i = 1; i < ROW_SIZE/2 + 1; i++)
        {
            first_byte = curr_line.substr(i*6, 2);
            second_byte = curr_line.substr(i*6+2, 2);
            if(curr_line_i > 1)
            {
                if(first_byte != "FF" )
                {
                    total_stu_checksum += std::stoi(first_byte, 0, 16);
                }
                if(second_byte != "FF")
                {
                    total_stu_checksum += std::stoi(second_byte, 0, 16);
                }
            }
            line_checksum += std::stoi(first_byte, 0, 16) + std::stoi(second_byte, 0, 16);
            last_4_bytes = first_byte + second_byte;
        }
        if(line_checksum != expected_checksum)
        {
            LOG_PRINT(("BAD CHECKSUM. LINE: %i\n", curr_line_i));
            return INVALID_STU_FILE;
        }
        curr_line_i++;
    }
    // don't include the last 4 bytes into the checksum
    total_stu_checksum -= (std::stoi(first_byte, 0, 16) + std::stoi(second_byte, 0, 16));
    if(std::stoi(last_4_bytes, 0, 16) != __builtin_bswap16(total_stu_checksum))
    {
        LOG_PRINT(("BAD TOTAL STU CHECKSUM"));
        return INVALID_STU_FILE;
    }

    stu_file.close();
    return VALID_STU_FILE;
}

int STUparam::stu_line_to_byte_array(const string& stu_line, uint8_t* byte_array, uint8_t arr_size)
{
    int sum = 0;
    if(arr_size < ROW_SIZE)
    {
        LOG_PRINT(("ARRAY SIZE TOO SMALL"));
        exit(EXIT_FAILURE);
    }
    for(int i = 1; i < ROW_SIZE/2 + 1; i++)
    {
        byte_array[2*i - 2] = std::stoi(stu_line.substr(i*6, 2), 0, 16);
        byte_array[2*i - 1] = std::stoi(stu_line.substr(i*6 + 2, 2), 0, 16);
        sum += byte_array[2*i - 2] + byte_array[2*i - 1];
    }
    return sum;
}

// gets a single stu parameter during runtime
int STUparam::get_stu_param(uint8_t param_num)
{
    sc->send_frame(KinetekCodes::READ_STU_PARAM_ID, &param_num, 1);
    CO_CANrxMsg_t respA, respB;
    memcpy(&respA, sc->get_frame(KinetekCodes::STU_PARAM_DATA_A_ID, this, resp_call_back_stu, 100), sizeof(CO_CANrxMsg_t));
    memcpy(&respB, sc->get_frame(KinetekCodes::STU_PARAM_DATA_B_ID, this, resp_call_back_stu, 100), sizeof(CO_CANrxMsg_t));

    if(kt->get_response_type(respA.ident, respA.data, respA.DLC) != KinetekCodes::STU_PARAM_READ_RESPONSE)
    {
        LOG_PRINT(("DID NOT RECEIVE DATA A\n"));
        return STU_PARAM_A_FAIL;
    }
    uint8_t value = respA.data[4];
    return value;

}

// changes a single stu param during runtime
int STUparam::change_stu_param(uint8_t param_num, uint8_t new_value)
{
    uint8_t write[3] = {param_num, 0x0, new_value};
    sc->send_frame(KinetekCodes::WRITE_STU_PARAM_ID, write, sizeof(write));
    CO_CANrxMsg_t* resp = sc->get_frame(KinetekCodes::STU_PARAM_WRITE_RESPONSE_ID, this, resp_call_back_stu, 500);
    if(kt->get_response_type(resp->ident, resp->data, resp->DLC) != KinetekCodes::STU_PARAM_WRITE_RESPONSE)
    {
        LOG_PRINT(("DID NOT RECEIVE WRITE CONFIRM\n"));
        return STU_WRITE_PARAM_FAIL;
    }
}