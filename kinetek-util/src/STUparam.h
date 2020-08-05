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
using std::ifstream;
using std::ofstream;

#define NUM_STU_ROWS 16
#define ROW_SIZE 16 // bytes

class STUparam
{
    public:
    // initializes objects, starts up socket can
    STUparam();

    // deallocates memory
    ~STUparam();

    // gets stu parameters from kinetek and outputs to a file
    int read_stu_params(const string& output_file);

    // writes stu parameters from a file to the kinetek
    int write_stu_params(const string& input_file);

    private:
    SocketCanHelper* sc; // helps to tx rx can frames
    KinetekCodes* kt;

    friend void resp_call_back_stu(
        void* msg,
        const CO_CANrxMsg_t* can_msg);  // the call back may need access to private member variables

    int validate_stu_file(const string& input_file);
    void stu_line_to_byte_array(const string& stu_line, uint8_t* byte_array, uint8_t arr_size);
};

#endif