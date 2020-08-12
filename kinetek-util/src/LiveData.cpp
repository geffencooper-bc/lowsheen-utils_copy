#include "LiveData.h"

// debug print macro to see error messages
#define PRINT_LOG

#ifdef PRINT_LOG
#define LOG_PRINT(x) printf x
#else
#define LOG_PRINT(x) \
    do               \
    {                \
    } while (0)
#endif

LiveData::LiveData(SocketCanHelper* sc, KU::CanDataList* ku_data)
{
    if (sc == nullptr)
    {
        PRINT_LOG(("Socket Can Helper is not initialized\n"));
        exit(EXIT_FAILURE);
    }
    this->sc = sc;

    if (ku_data == nullptr)
    {
        PRINT_LOG(("Kinetek Utility CanDataList is not initialized\n"));
        exit(EXIT_FAILURE);
    }
    this->ku_data = ku_data;

    heartbeat = new controller_heartbeat;
}

LiveData::~LiveData()
{
    delete heartbeat;
}

// callback function for received messages, not used as of now
void LiveData_resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

bool LiveData::update_param(uint8_t param_new, uint8_t &param_old, const string& param_name)
{
    if(param_new != param_old)
    {
        printf("\n%s changed from %i to %i\n", param_name.c_str(), param_old, param_new);
        param_old = param_new;
        return true;
    }
    return false;
}
bool LiveData::update_param(uint16_t param_new, uint16_t &param_old, const string& param_name)
{
    if(param_new != param_old)
    {
        printf("\n%s changed from %i to %i\n", param_name.c_str(), param_old, param_new);
        param_old = param_new;
        return true;
    }
    return false;
}

KU::StatusCode LiveData::update_heartbeat()
{
    while(true)
    {
        printf("Getting next page\n");
        CO_CANrxMsg_t* resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 100);
        if(ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEART_BEAT)
        {
            LOG_PRINT(("No Heart Beat\n"));
            return KU::NO_HEART_BEAT;
        }

        uint8_t page_num = resp->data[1];
        switch(page_num)
        {
            case 1:
            {
                printf("page %i\n", page_num);
                //update_param(resp->data[2], heartbeat->page1.vehicle_state, "Vehicle State");
                //update_param(resp->data[3], heartbeat->page1.error_value, "Error Value");
                //update_param(resp->data[4], (uint8_t)heartbeat->page1.traction_running, "Traction Running");

                break;
            }
            case 2:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 3:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 4:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 5:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 6:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 7:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 8:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 9:
            {
                printf("page %i\n", page_num);
                break;
            }
            case 10:
            {
                printf("page %i\n", page_num);
                break;
            }
        }
    }
}

