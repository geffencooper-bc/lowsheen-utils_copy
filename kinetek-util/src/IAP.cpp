#include "IAP.h"

IAP::IAP()
{
    memset(start_address, 0, 4);
    memset(data_size, 0, 4);
    memset(total_checksum, 0, 4);
    in_iap_mode = false;
    sc = new SocketCanHelper;
}

IAP::~IAP()
{
    delete ut;
    delete sc;
}


void IAP::load_hex_file(string file_path)
{
    ut = new HexUtility(file_path);
    data_size_bytes = ut->get_file_data_size(data_size, 4);
    ut->get_total_cs(total_checksum, 4);
    ut->get_start_address(start_address, 4);
    last_line_data_size = ut->get_last_data_line_size();
}

void print_array(uint8_t* arr, uint8_t size)
{
    for(int i = 0; i < size; i++)
    {
        printf("%i ", arr[i]);
    }
}

void IAP::print()
{
    printf("\n");
    printf("\n\t\t\t\t\t=============================================");
    printf("\n\t\t\t\t\t============= IAP UTILITY PRINT== ===========");
    printf("\n\t\t\t\t\t=============================================\n\n");
    printf("\nHEX FILE DATA SIZE:\t\t%i bytes", data_size_bytes);
    printf("\nHEX FILE DATA TOTAL CHECKSUM:\t"); print_array(total_checksum, sizeof(total_checksum));
    printf("\nSTART ADDRESS:\t\t\t"); print_array(start_address, sizeof(start_address));
    // printf("\n\n==== INIT COMMANDS====\n\nFW_REVISION_REQUEST:\t\t\t\t", self.FW_REVISION_REQUEST);
    // printf("\nENTER_IAP_MODE_REQUEST_FORCED:\t\t\t", self.ENTER_IAP_MODE_REQUEST_FORCED);
    // printf("\nENTER_IAP_MODE_REQUEST_SELECTIVE:\t\t", self.ENTER_IAP_MODE_REQUEST_SELECTIVE);
    // printf("\nSEND_BYTES_REQUEST:\t\t\t\t", self.SEND_BYTES_REQUEST);
    // printf("\nSEND_START_ADDRESS_REQUEST:\t\t\t", self.SEND_START_ADDRESS_REQUEST);
    // printf("\nSEND_CHECKSUM_DATA_REQUEST:\t\t\t", self.SEND_CHECKSUM_DATA_REQUEST);
    // printf("\nSEND_DATA_SIZE_REQUEST:\t\t\t\t", self.SEND_DATA_SIZE_REQUEST);
    printf("\n");
    printf("\n\t\t\t\t\t=============================================");
    printf("\n\t\t\t\t\t============= IAP UTILITY PRINT =============");
    printf("\n\t\t\t\t\t=============================================\n\n\n\n");
    
}

