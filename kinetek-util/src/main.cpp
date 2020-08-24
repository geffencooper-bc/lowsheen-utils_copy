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

// #define LIB_EXAMPLE
//#define CL_EXAMPLE

int main(int argc, char** argv)
{
    // the command line example shows how to use the kinetek utility through the shell
    //#ifdef CL_EXAMPLE
    KinetekUtility ku;
    ku.parse_args(argc, argv);

    // check the status
    printf("STATUS: %s\n", ku.translate_status_code(ku.CL_status).c_str());

//#endif

// this example shows how to use the kinetek utility through direct functions calls (as a library)
#ifdef LIB_EXAMPLE
    // first create the kinetek utility object and initialize can
    KinetekUtility ku;
    ku.set_can_interface("can0");
    KU::StatusCode status = ku.init_can();
    if (status == KU::INIT_CAN_SUCCESS)
    {
        // IAP Utility: update the kinetek fw using forced mode, retry three times
        int num_tries = 0;
        status = ku.run_iap("/home/brain/2.28.hex", 1);
        while (status != KU::UPLOAD_COMPLETE)
        {
            if (num_tries == 3)
            {
                break;
            }
            // retry if failed
            status = ku.run_iap("/home/brain/2.28.hex", 1);
            num_tries++;
        }
    }
    // check the status so far
    printf("STATUS: %s\n", ku.translate_status_code(status).c_str());

#endif
}
