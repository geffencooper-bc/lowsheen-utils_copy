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
#include <iomanip>
#include <iostream>
#include "SocketCanHelper.h"
#include "KinetekUtilityCodes.h"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::stringstream;

#define NUM_STU_ROWS 16 // 16 rows in a stu file
#define ROW_SIZE 16 // each row holds 16 stu parameters
#define MAX_NUM_STU_PARAMS 254 // max stu params is 254 because last two bytes are checksum
#define INITIAL_UNUSED_PARAMS 28 // first 28 params are runtime values

class STUparam
{
    public:

    // initializes objects
    STUparam(SocketCanHelper* sc, KU::CanDataList* ku_data);

    // deallocates memory
    ~STUparam();

    // gets stu parameters from kinetek and outputs to a file
    KU::StatusCode read_stu_params(const string& output_file);

    // writes stu parameters from a file to the kinetek
    KU::StatusCode write_stu_params(const string& input_file);

    // gets a single stu parameter during runtime, returns stu param value
    int get_stu_param(uint8_t param_num);

    // changes a single stu param during runtime
    KU::StatusCode set_stu_param(uint8_t param_num, uint8_t new_value);

    private:
    SocketCanHelper* sc; // helps to tx/rx can frames
    KU::CanDataList* ku_data; // stores kinetek can ids/data frames

    // the call back may need access to private member variables
    friend void STU_resp_call_back(
        void* msg,
        const CO_CANrxMsg_t* can_msg);  

    // confirms checksums before uploading file
    KU::StatusCode validate_stu_file(const string& input_file); 

    // converts a line in a stu file into an array of bytes, return sum of the bytes
    int stu_line_to_byte_array(const string& stu_line, uint8_t* byte_array, uint8_t arr_size);
};

#endif