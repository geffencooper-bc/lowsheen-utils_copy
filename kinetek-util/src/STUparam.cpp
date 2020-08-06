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

    sc->init_socketcan("can0");
}

// deallocate memory
STUparam::~STUparam()
{
    delete sc;
    delete kt;
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
    // try to open the file(overwrite), if DNE then create a new file  
    ofstream output;
    output.open(output_file, std::ofstream::trunc);
    if(output.fail())
    {
        output.open(output_file); // create a new file
    }
    stringstream stu_string; // store stu output as stringstream for formatting

    // Part 1: stu header 
    string stu_header = ""; // stores stu header until gets written at end
    int stu_header_checksum = 29; // controller id is always 29
    int num_params = MAX_NUM_STU_PARAMS - INITIAL_UNUSED_PARAMS; // start at max then subtract away unused params

    stu_header += "29, ";
    // get the firmware version from heart beat page 9
    CO_CANrxMsg_t* resp =  sc->get_frame(KinetekCodes::HEART_BEAT_ID, this, resp_call_back_stu, 20000);
    while(resp->data[1] != 9) // wait till page 9 of heart beat
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
    int current_checksum = 0;
    CO_CANrxMsg_t respA; // first 8 bytes
    CO_CANrxMsg_t respB; // second 8 bytes
    while(stu_row_i < NUM_STU_ROWS) // keep reading parameters until all rows read
    {
        // initialize read request data with according eeprom address and get the row response data
        kt->eeprom_access_read_request_data[3] = ROW_SIZE*stu_row_i;
        sc->send_frame(KinetekCodes::EEPROM_ACCESS_MESSAGE_ID, kt->eeprom_access_read_request_data, sizeof(kt->eeprom_access_read_request_data));
    
        memcpy(&respA, sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_A_ID, this, resp_call_back_stu, 1000), sizeof(CO_CANrxMsg_t));
        memcpy(&respB, sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_B_ID, this, resp_call_back_stu, 1000), sizeof(CO_CANrxMsg_t));

        // get the first 8 bytes of the row
        if(kt->get_response_type(respA.ident, respA.data, respA.DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
        {
            LOG_PRINT(("DID NOT RECEIVE A"));
            return READ_A_FAIL;
        }

        stu_stream(stu_string, ROW_SIZE*stu_row_i, 4) << ", "; // row address

        for(int j = 0; j < respA.DLC; j+=2) // get the data from the row and increment checksum
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
            current_checksum += respA.data[j] + respA.data[j+1];
            stu_stream(stu_string, respA.data[j], 2); stu_stream(stu_string, respA.data[j+1], 2) << ", ";
        } 

        // get the second 8 bytes of the row
        if(kt->get_response_type(respB.ident, respB.data, respB.DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
        {
            LOG_PRINT(("DID NOT RECEIVE B"));
            return READ_B_FAIL;
        }
        
        for(int j = 0; j < respB.DLC; j+=2) // get the data from the row and increment checksum
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
            current_checksum += respB.data[j] + respB.data[j+1];
            stu_stream(stu_string, respB.data[j], 2); stu_stream(stu_string, respB.data[j+1], 2) << ", ";
        } 

        // write the row checksum to the stu file
        current_checksum += ROW_SIZE*stu_row_i;
        if(stu_row_i == NUM_STU_ROWS - 1)
        {
            stu_stream(stu_string, current_checksum, 4);
        }
        else
        {
            stu_stream(stu_string, current_checksum, 4) << '\n';
        }
        stu_row_i++;
        current_checksum = 0;
    }

    // write the stu header
    stu_header_checksum += num_params;
    stu_header += std::to_string(num_params) + ", " + std::to_string(stu_header_checksum) + "\n";

    output << stu_header << stu_string.str();
    output.close(); // close the stu output file
    return STU_READ_SUCCESS;
}

STUparam::stu_status STUparam::write_stu_params(const string& input_file)
{
    // make sure the stu file is valid
    stu_status status = validate_stu_file(input_file);
    if(status == INVALID_STU_FILE)
    {
        LOG_PRINT(("BAD STU FILE"));
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

    // Check 3: zero-out the first 28 parameters, rewrite the file
    string first_two_lines = curr_line + "\n";
    hu_getline(stu_file, curr_line); // read stu line 1
    hu_getline(stu_file, curr_line); // read stu line 2
    first_two_lines += "0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000\n"; // first stu line zero
    first_two_lines += "0010, 0000, 0000, 0000, 0000, 0000, 0000, " + curr_line.substr(42, 10); // grab parameters 28-32 from stu line 2

    // get the sum of these parameters (row checksum)
    int start_params_sum = std::stoi(curr_line.substr(42, 2), 0, 16) + 
                           std::stoi(curr_line.substr(44, 2), 0, 16) +
                           std::stoi(curr_line.substr(48, 2), 0, 16) +
                           std::stoi(curr_line.substr(50, 2), 0, 16) + ROW_SIZE;
    // append the row checksum
    stringstream stream;
    stream << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << start_params_sum;
    first_two_lines += ", " + stream.str() + "\n";

    // get the rest of the file as a string and combine with edited first two lines
    while(hu_getline(stu_file, curr_line))
    {
        first_two_lines += curr_line + "\n";
    }

    stu_file.clear();
    stu_file.seekp(0, std::ios::beg);
    stu_file << first_two_lines;

    stu_file.clear();
    stu_file.seekp(0, std::ios::beg); 

    // Check 4: validate row checksums
    int curr_line_i = 0;
    int total_stu_checksum = 0;
    hu_getline(stu_file, curr_line); // start at line 1
    // read all lines in the file, calculate the row checksums, compare to expected checkums. Also increment pure stu checksum
    while(hu_getline(stu_file, curr_line))
    {
        int line_checksum = 16*curr_line_i; // include row address in row checksum
        int expected_checksum = std::stoi(curr_line.substr(curr_line.size()-4, 4), 0, 16);
        for(int i = 1; i < ROW_SIZE/2 + 1; i++) // sum the bytes
        {
            line_checksum += std::stoi(curr_line.substr(i*6, 2), 0, 16) + std::stoi(curr_line.substr(i*6 + 2, 2), 0, 16);
        }
        if(line_checksum != expected_checksum)
        {
            LOG_PRINT(("BAD LINE CHECKSUM\n"));
            return INVALID_STU_FILE;
        }
        total_stu_checksum += line_checksum - 16*curr_line_i; // don't include row address in pure stu checksum
        curr_line_i++;
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