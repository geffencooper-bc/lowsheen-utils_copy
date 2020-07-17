#include "IAP.h"
#include "KinetekCodes.h"
#include 

using std::to_string;

IAP::IAP()
{
    memset(start_address, 0, 4);
    memset(data_size, 0, 4);
    memset(total_checksum, 0, 4);
    in_iap_mode = false;
    resend_msg = true;
    sc = new SocketCanHelper;
}

IAP::~IAP()
{
    printf("IAP Destructor\n");
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

void IAP::init_can(const char* channel_name)
{
    sc->init_socketcan(channel_name);
}

string decode_can_msg(const CO_CANrxMsg_t* can_msg)
{
    string can_id = to_string(can_msg->ident);
    string data = "";
    for(int i = 0; i < can_msg->DLC; i++)
    {
        data += to_string(can_msg->data[i]);
    }
    return (can_id + "|" + data);
}

void resp_call_back(void* cbc, const CO_CANrxMsg_t* can_msg)
{
    iap_response resp = lookup(decode_can_msg(can_msg), iap_response_table);
    call_back_checker* cbc_obj = ((call_back_checker*)(cbc));
    if(resp != cbc_obj->expected)
    {
        cbc_obj->iap_obj->resend_msg = true;
    }
    else
    {
        cbc_obj->iap_obj->resend_msg = false;
    }
    
}

bool IAP::put_in_iap_mode(bool forced_mode)
{
    #ifdef PRINT_DEBUG
    printf("Putting in IAP mode\n");
    #endif
    if(!forced_mode)
    {
        call_back_checker cbc = {this, IN_IAP_MODE};
        sc->send_frame(IAP_REQUEST, ENTER_IAP_MODE_FORCED, sizeof(ENTER_IAP_MODE_FORCED));
        sc->get_frame(KINETEK_MESSAGE, &cbc, resp_call_back);

    }
    //    print("Putting in IAP mode")
    //     if force_mode == False:
    //         self.send_request_repeated(None, "HEART_BEAT",-1,-1, None)
    //         self.send_request(self.ENTER_IAP_MODE_REQUEST_SELECTIVE, "ENTER_IAP_MODE_RESPONSE_SELECTIVE", 20)
    //         self.send_request_repeated(None, "IN_IAP_MODE",-1,-1, None)
    //         print("entered iap_mode")
    //         return True
    //     else:
    //         self.send_request_repeated(self.ENTER_IAP_MODE_REQUEST_FORCED, "IN_IAP_MODE", -1, -1, None) # -1 ensures request sent indefinetely
    //         # no need to check response because it will request will only return if successful
    //         self.in_iap_mode = True
    //         print("entered iap_mode")
    //         return True
            
}
