// main file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include "IAP.h"
#include <sys/timerfd.h>
#include <sys/time.h>

// arg 1 = file path, arg2 = iap_mode
int main(int argc, char** argv)
{
    int iap_type = atoi(argv[2]); // 0 = selective, 1 = forced

    IAP iap;
    iap.load_hex_file(argv[1]); // file path
    iap.print();
    if(iap.init_can("can0") != INIT_CAN_FAIL)
    {
        if(iap.put_in_iap_mode(iap_type) == IAP_MODE_SUCCESS)
        {
            status_code status = iap.send_init_packets();
            if(status == INIT_PACKET_SUCCESS)
            {
                status = iap.upload_hex_file();
                if(status != UPLOAD_COMPLETE)
                {
                    printf("\nError: %i\n", status);
                }
                else
                {
                    printf("\n\n====== SUCCSESS =======\n");
                }
            }
            else
            {
                printf("Error: %i", status);
            }
        }
    }
}

