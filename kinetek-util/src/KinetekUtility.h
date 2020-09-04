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

// This is the top level class that facilitates the Kinetek Utility
// by parsing command line arguments and running the according tool.

#ifndef KINETEK_UTILITY_H
#define KINETEK_UTILITY_H

#include "IAP.h"
#include "STUparam.h"
#include "LiveData.h"
#include "KinetekUtilityCodes.h"

// This class is the interface and entry point into the kinetek tools
// and can be used in two ways:

// 1. If the Kinetek Utility is being used by running the executable in the shell
// then parse_args can be called with the command line arguments. parse_args will
// then call the according functions to run the desried tool.

// 2. If the Kinetek Utility is being used as a "library", then init_can and the
// the desired function under "tools" can be called directly.
class KinetekUtility
{
   public:
    // init objects
    KinetekUtility();

    ~KinetekUtility();

    // parses the command line input and runs the correct function
    int parse_args(int argc, char** argv);

    // sets up socket can helper object and connects to can interface, channel name defaults to "can0"
    KU::StatusCode init_can();

    // translates a status code into a human readable string
    string translate_status_code(KU::StatusCode status);

    // tools
    KU::StatusCode run_iap(const string& file_path, bool iap_mode);
    KU::StatusCode read_stu_to_file(const string& file_path);
    KU::StatusCode write_stu_from_file(const string& file_path);
    KU::StatusCode get_stu_param(int param_num);
    KU::StatusCode set_stu_param(int param_num, int new_value);
    KU::StatusCode get_live_data();

    // 1 = enable 2 = disable
    KU::StatusCode toggle_estop(int mode);
    KU::StatusCode reset_xt_can();

    // function to analyze IAP mode timing and reliability, forced and selective
    // for forced specify the amount of time to delay after power up, and the number of requests to send
    void test_iap(int window_time, int tries, bool mode);

    // set interface when don't want default "can0"
    void set_can_interface(const string& interface) { can_interface = interface; }
    // holds the latest status of Kinetek Utility after executing a tool
    // in command line mode, see parse_opt() in KinetekUtility.cpp
    KU::StatusCode CL_status;

   private:
    SocketCanHelper* sc;
    KU::CanDataList* ku_data;  // holds all can data used by the utilities
    IAP* iap;
    STUparam* stu;
    LiveData* ld;

    bool can_initialized;  // keeps track of whether init_can has been called
    string can_interface;
};

#endif