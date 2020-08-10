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

//#define IAP_UTIL
#define STU_PARAM

int main(int argc, char** argv)
{
    KinetekUtility ku;
    KU::StatusCode status = ku.init_can("can0");
    if(status == KU::INIT_CAN_SUCCESS)
    {
        status = ku.run_iap("/home/brain/2.27.hex", 1);
        if(status == KU::UPLOAD_COMPLETE)
        {
            usleep(4500000);
            status = ku.run_iap("/home/brain/2.28.hex", 0);
            // if(status == KU::UPLOAD_COMPLETE)
            // {
            //     usleep(3500000);
            //     status = ku.read_stu_to_file("read_test.stu");
            //     if(status == KU::STU_FILE_READ_SUCCESS)
            //     {
            //         usleep(1500000);
            //         status = ku.write_stu_from_file("read_test.stu");
            //         if(status == KU::STU_FILE_WRITE_SUCCESS)
            //         {
            //             usleep(1500000);
            //             status = ku.write_stu_param(4, 150);
            //             if(status == KU::STU_PARAM_WRITE_SUCCESS)
            //             {
            //                 status = ku.read_stu_param(4);
            //             }
            //         }
            //     }
            // }   
        }
    }
    if(status == KU::STU_PARAM_READ_SUCCESS)
    {
        printf("SUCCESS: %s\n", ku.translate_status_code(status).c_str());
    }
    else
    {
        printf("ERROR: %s\n", ku.translate_status_code(status).c_str());
    }
    
}
