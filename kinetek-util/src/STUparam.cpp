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
#include <memory>
// debug print macro to see error messages
#define PRINT_LOG

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) do {} while (0)
#endif

//#define STU_OUT output << std::hex << std::uppercase << std::setfill('0') << std::setw(2)

STUparam::STUparam()
{
    sc = new SocketCanHelper;
    kt = new KinetekCodes;

    sc->init_socketcan("can0");
}

STUparam::~STUparam()
{
    delete sc;
    delete kt;
}

// callback function for received messages, not used as of now
void resp_call_back_stu(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

ofstream& stu_stream(ofstream& os, int val, int fill_width)
{
    os << std::hex << std::uppercase << std::setfill('0') << std::setw(fill_width) << val;// << std::to_string(val);
    return os;
}

int STUparam::read_stu_params(const string& output_file)
{
    // try to open the file, if DNE then create a new file  
    ifstream file;
    ofstream output;
    file.open(output_file);
    if(file.fail())
    {
        output.open(output_file, std::ofstream::trunc);
    }
    // first get the STU header
    output << 29 << ", "; // Kinetek controller id is 29
    
    CO_CANrxMsg_t* resp =  sc->get_frame(KinetekCodes::HEART_BEAT_ID, this, resp_call_back_stu, 20000);
    while(resp->data[1] != 9) // wait till page 9 of heart beat
    {
        resp =  sc->get_frame(KinetekCodes::HEART_BEAT_ID, this, resp_call_back_stu, 20000);
    }
    output << std::to_string(resp->data[3]) << ", " << std::to_string(resp->data[4]) << ", " << 196 << ", " << 254 << "\n";
    
    // next get the stu parameters
    int stu_row_i = 0;
    int current_checksum = 0;
    while(stu_row_i < NUM_STU_ROWS) // keep reading parameters until all rows read
    {
        // initialize read request data with according eeprom address and get the row response data
        kt->eeprom_access_read_request_data[3] = ROW_SIZE*stu_row_i;
        usleep(5000);
        sc->send_frame(KinetekCodes::EEPROM_ACCESS_MESSAGE_ID, kt->eeprom_access_read_request_data, sizeof(kt->eeprom_access_read_request_data));
        // does not work, uses same one under the hood (can rx msg can_msg)
        CO_CANrxMsg_t respA;
        CO_CANrxMsg_t respB;

        memcpy(&respA, sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_A_ID, this, resp_call_back_stu, 100000), sizeof(CO_CANrxMsg_t));
        memcpy(&respB, sc->get_frame(KinetekCodes::EEPROM_LINE_READ_RESPONSE_B_ID, this, resp_call_back_stu, 100000), sizeof(CO_CANrxMsg_t));

        // get the first 8 bytes of the row
        if(kt->get_response_type(respA.ident, respA.data, respA.DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
        {
            LOG_PRINT(("DID NOT RECEIVE A"));
            return -1;
        }

        stu_stream(output, ROW_SIZE*stu_row_i, 4) << ", "; // row address

        for(int j = 0; j < respA.DLC; j+=2) // get the data from the row
        {
            current_checksum += respA.data[j] + respA.data[j+1];
            stu_stream(output, respA.data[j], 2); stu_stream(output, respA.data[j+1], 2) << ", ";
        } 

        // get the second 8 bytes of the row
        if(kt->get_response_type(respB.ident, respB.data, respB.DLC) != KinetekCodes::EEPROM_ACCESS_READ_RESPONSE)
        {
            LOG_PRINT(("DID NOT RECEIVE B"));
            return -1;
        }
        
        for(int j = 0; j < respB.DLC; j+=2)
        {
            current_checksum += respB.data[j] + respB.data[j+1];
            stu_stream(output, respB.data[j], 2); stu_stream(output, respB.data[j+1], 2) << ", ";
        } 
        current_checksum += ROW_SIZE*stu_row_i;
        stu_stream(output, current_checksum, 4) << '\n'; 
        stu_row_i++;
        current_checksum = 0;
    }
    output.close(); // close the stu output file
}

