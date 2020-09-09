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

// #define TEST_IAP
#define COMMAND_LINE_MODE

int main(int argc, char** argv)
{
    // make the KinetekUtility object
    KinetekUtility ku;

// test reliability and timing of entering iap mode
#ifdef TEST_IAP

    ku.init_can();

    // execute according to the command line options
    ku.parse_args(argc, argv);

    if (ku.CL_status == KU::UPLOAD_COMPLETE)
    {
        printf("S,%s\n", ku.IAP_test_string.c_str());
    }
    else
    {
        printf("F,%s\n", ku.IAP_test_string.c_str());
    }

    // test IAP modes as stand alone functions, not in terms of state machine
    // if (argc != 4)
    // {
    //     printf("ARG1: window delay ARG2: number of requests ARG3: IAP mode\n");
    //     exit(EXIT_FAILURE);
    // }

    // ku.test_iap(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
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
