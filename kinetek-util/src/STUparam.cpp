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

#include "STUparam.h"

// debug print macro to see error messages
#define PRINT_LOG

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) do {} while (0)
#endif

STUparam::STUparam()
{
    sc = new SocketCanHelper;
    kt = new KinetekCodes;
    is_first_8 = true;
}

STUparam::~STUparam()
{
    delete sc;
    delete kt;
}

// callback function for received messages, not used as of now
void resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

int STUparam::read_stu_params(const string& output_file)
{
    // try to open the file, if DNE then create a new file  
    ifstream file;
    ofstream output;
    file.open(output_file);
    if(file.fail())
    {
        output.open(output_file);
    }

    // output << header
    // now start reading the STU params
    for(int i = 0; i < NUM_STU_ROWS; i++)
    {
        is_first_8 = !is_first_8;
        kt->eeprom_access_read_request_data[3] = ROW_SIZE*i;
        sc->send_frame(KinetekCodes::EEPROM_ACCESS_MESSAGE_ID, kt->eeprom_access_read_request_data, sizeof(kt->eeprom_access_read_request_data));
        if(is_first_8)
        {
            CO_CANrxMsg_t* resp =  sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_A_ID, this, resp_call_back, 10000);
            if(kt->get_response_type(resp->ident, resp->data, resp->DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
            {
                LOG_PRINT(("DID NOT RECEIVE A"));
                return -1;
            }
            // output << line address
            for(int j = 0; j < resp->DLC/2; j++)
            {
                output << resp->data[j] << resp->data[j+1] << ", ";
            } 
            // output << checksum
        }
        else
        {
            CO_CANrxMsg_t* resp = sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_B_ID, this, resp_call_back, 10000);
            if(kt->get_response_type(resp->ident, resp->data, resp->DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
            {
                LOG_PRINT(("DID NOT RECEIVE B"));
                return -1;
            }
            
            for(int j = 0; j < resp->DLC/2; j++)
            {
                output << resp->data[j] << resp->data[j+1] << ", ";
            } 
        }
        
    }
    
}

