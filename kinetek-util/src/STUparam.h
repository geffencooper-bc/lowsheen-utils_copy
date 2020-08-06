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

// a class to facilitate reading and writing STU parameters from the kinetek

#ifndef STU_PARAM_H
#define STU_PARAM_H

#include <string>
#include <fstream>
#include <stdint.h>
#include <ostream>
#include "SocketCanHelper.h"
#include "KinetekCodes.h"
#include <iomanip>
#include <iostream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::stringstream;

#define NUM_STU_ROWS 16 // 16 rows in a st file
#define ROW_SIZE 16 // each row holds 16 stu parameters
#define MAX_NUM_STU_PARAMS 254 // max stu params is 254 because last two bytes are checksum
#define INITIAL_UNUSED_PARAMS 28 // first 28 params are runtime values

class STUparam
{
    public:
    // these codes are used to determine if to continue stu process
    enum stu_status
    {
        INIT_CAN_FAIL = 0,
        INIT_CAN_SUCCESS,
        NO_HEART_BEAT,
        READ_A_FAIL,
        READ_B_FAIL,
        INVALID_STU_FILE,
        VALID_STU_FILE,
        WRITE_FAIL,
        STU_READ_SUCCESS,
        STU_WRITE_SUCCESS
    };

    // initializes objects
    STUparam();

    // deallocates memory
    ~STUparam();

    // initialize the SocketCanHelper object and can communication
    stu_status init_can(const char* channel_name);

    // gets stu parameters from kinetek and outputs to a file
    stu_status read_stu_params(const string& output_file);

    // writes stu parameters from a file to the kinetek
    stu_status write_stu_params(const string& input_file);

    private:
    SocketCanHelper* sc; // helps to tx/rx can frames
    KinetekCodes* kt; // stores kinetek can ids/data frames

    friend void resp_call_back_stu(
        void* msg,
        const CO_CANrxMsg_t* can_msg);  // the call back may need access to private member variables

    stu_status validate_stu_file(const string& input_file); // confirms checksums before uploading file
    // converts a line in a stu file into an array of bytes, return sum of the bytes
    int stu_line_to_byte_array(const string& stu_line, uint8_t* byte_array, uint8_t arr_size);
};

#endif