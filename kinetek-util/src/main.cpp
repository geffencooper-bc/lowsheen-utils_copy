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

#include "KinetekUtility.h"

//#define LIB_EXAMPLE
#define CL_EXAMPLE

int main(int argc, char** argv)
{
// the command line example shows how to use the kinetek utility through the command line
//#ifdef CL_EXAMPLE
    KinetekUtility ku;
    ku.parse_args(argc, argv);
//#endif

// the library example shows how to use the kinetek utility through direct functions calls
#ifdef LIB_EXAMPLE
    // first create the kinetek utility object and initialize can
    KinetekUtility ku;
    ku.set_can_interface("can0");
    KU::StatusCode status = ku.init_can();
    if (status == KU::INIT_CAN_SUCCESS)
    {
        // update the kinetek fw using forced mode
        status = ku.run_iap("/home/brain/2.27.hex", 1);
        if (status == KU::UPLOAD_COMPLETE)
        {
            // update the kinetek fw using selective mode
            status = ku.run_iap("/home/brain/2.28.hex", 0);
            if (status == KU::UPLOAD_COMPLETE)
            {
                // read the stu parameters to a file
                status = ku.read_stu_to_file("read_test.stu");
                if (status == KU::STU_FILE_READ_SUCCESS)
                {
                    // write the stu parameters from a file
                    status = ku.write_stu_from_file("read_test.stu");
                    if (status == KU::STU_FILE_WRITE_SUCCESS)
                    {
                        // change a single stu parameter
                        status = ku.write_stu_param(4, 150);
                        if (status == KU::STU_PARAM_WRITE_SUCCESS)
                        {
                            // get a single stu parameter
                            status = ku.read_stu_param(4);
                        }
                    }
                }
            }
        }
    }
    if (status == KU::STU_PARAM_READ_SUCCESS)
    {
        printf("SUCCESS: %s\n", ku.translate_status_code(status).c_str());
    }
    else
    {
        printf("ERROR: %s\n", ku.translate_status_code(status).c_str());
    }
#endif
}
