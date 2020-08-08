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
    // arg 1 = file path, arg2 = stu_mode
    // if (argc != 3)
    // {
    //     printf("ARGS: [FILE PATH] [STU MODE] (0 = Read, 1 = Write)");
    //     exit(EXIT_FAILURE);
    // }

    // int stu_mode = atoi(argv[2]);  // 0 = read, 1 = write
    // string file_path = argv[1];

    // STUparam stu;
    // STUparam::stu_status status = stu.init_can("can0");

    // if (status == INIT_CAN_SUCCESS)
    // {
    //     if(stu_mode == 0)
    //     {
    //         STUparam::stu_status status = stu.read_stu_params(file_path);
    //         if(status != STUparam::STU_READ_SUCCESS)
    //         {
    //             printf("Error: %i", status);
    //         }
    //         else
    //         {
    //             printf("SUCCESS\n");
    //         }
    //     }
    //     else if(stu_mode == 1)
    //     {
    //         STUparam::stu_status status = stu.write_stu_params(file_path);
    //         if(status != STUparam::STU_WRITE_SUCCESS)
    //         {
    //             printf("Error: %i", status);
    //         }
    //         else
    //         {
    //             printf("SUCCESS\n");
    //         }
    //     }
    // }
    // STUparam stu;
    // STUparam::stu_status status = stu.init_can("can0");
    // printf("BRUSH S LIMIT: %i\n", stu.get_stu_param(4));
    // stu.change_stu_param(4, 148);
    // printf("BRUSH S LIMIT: %i", stu.get_stu_param(4));

    KinetekUtility ku;
    ku.init_can("can0");
    ku.run_iap("/home/brain/2.27.hex", 1);
}
