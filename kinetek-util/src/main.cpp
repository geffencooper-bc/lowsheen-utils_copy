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

//#define FULL_IAP_TEST
//#define TEST_FORCED_WINDOW
#define TEST_IAP_MODE
//#define COMMAND_LINE_MODE

int main(int argc, char** argv)
{
    // make the KinetekUtility object
    KinetekUtility ku;

#ifdef FULL_IAP_TEST
    // execute according to the command line options
    ku.parse_args(argc, argv);

    // show the status of what was executed (success or error)
    if(ku.CL_status == KU::UPLOAD_COMPLETE)
    {
        printf("+\n");
    }
    else
    {
        printf("-\n");
    }
    return 0;

#endif

#ifdef TEST_FORCED_WINDOW
    // test the iap forced window time
    if (argc != 3)
    {
        printf("ARG1: window delay ARG2: number of requests\n");
        exit(EXIT_FAILURE);
    }
    ku.init_can();
    ku.test_forced_window(atoi(argv[1]), atoi(argv[2]));

#endif
// test reliability and timing of entering iap mode
#ifdef TEST_IAP_MODE

    ku.init_can();

    // execute according to the command line options
    ku.parse_args(argc, argv);

    if (ku.CL_status == KU::UPLOAD_COMPLETE)
    {
        printf("+,%s\n", ku.IAP_test_string.c_str());
    }
    else
    {
        printf("-,%s\n", ku.IAP_test_string.c_str());
    }
    return 0;
#endif

#ifdef COMMAND_LINE_MODE
    // execute according to the command line options
    ku.parse_args(argc, argv);

    // show the status of what was executed (success or error)
    printf("STATUS: %s\n", ku.translate_status_code(ku.CL_status).c_str());
    return 0;
#endif
}
