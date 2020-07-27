#include "IAP.h"

// arg 1 = file path, arg2 = iap_mode
int main(int argc, char** argv)
{
    int iap_type = atoi(argv[2]); // 0 = selective, 1 = forced

    IAP iap;
    iap.load_hex_file(argv[1]); // file path
    iap.print(); // hex file information

    // step 1: check if interface accessible
    status_code status = iap.init_can("can0");
    if(status == INIT_CAN_SUCCESS)
    {   
        // step 2: put Kinetek into IAP mode (fw download mode)
        status = iap.put_in_iap_mode(iap_type);
        if(status == IAP_MODE_SUCCESS)
        {
            // step 3: send initialization frames (hex file size, checksum, start address, etc)
            status = iap.send_init_frames();
            if(status == INIT_PACKET_SUCCESS)
            {
                // step 4: upload the hex file
                status = iap.upload_hex_file();
                if(status == UPLOAD_COMPLETE)
                {
                    printf("\n\n====== SUCCSESS ======\n");
                }
            }
        }
    }
    if(status != UPLOAD_COMPLETE)
    {
        printf("Error: %i", status);
    }
}

