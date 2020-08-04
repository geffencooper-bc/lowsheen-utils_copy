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

using std::string;
using std::ifstream;
using std::ofstream;

#define NUM_STU_ROWS 16
#define ROW_SIZE 16 // bytes

class STUparam
{
    public:
    STUparam();
    ~STUparam();

    int read_stu_params(const string& output_file);
    void write_stu_params(const string& input_file);

    private:
    SocketCanHelper* sc; // helps to tx rx can frames
    KinetekCodes* kt;

    friend void resp_call_back_stu(
        void* msg,
        const CO_CANrxMsg_t* can_msg);  // the call back may need access to private member variables
};

#endif