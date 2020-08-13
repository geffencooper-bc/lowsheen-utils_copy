#include "LiveData.h"
#include <chrono>

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

bool LiveData::update_param(uint8_t param_new, uint8_t param_old, const string& param_name)
{
    if(param_new != param_old)
    {
        printf("\n%s changed from %i to %i\n", param_name.c_str(), param_old, param_new);
        return true;
    }
    return false;
}
bool LiveData::update_param(uint16_t param_new, uint16_t param_old, const string& param_name)
{
    if(param_new != param_old)
    {
        printf("\n%s changed from %i to %i\n", param_name.c_str(), param_old, param_new);
        return true;
    }
    return false;
}

KU::StatusCode LiveData::update_heartbeat()
{
    controller_heartbeat temp;
    while(true)
    {
        CO_CANrxMsg_t* resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 200);
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
                memcpy(&temp.page1, resp->data+2, sizeof(temp.page1));
                // std::chrono::steady_clock::time_point begin;
                // std::chrono::steady_clock::time_point end;
                // if(reset)
                // {
                //     begin = std::chrono::steady_clock::now();
                //     reset = false;
                // }
                // else
                // {
                //     end = std::chrono::steady_clock::now();
                //     reset = true;
                //     printf("Time: %f\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000000.0);
                // }
                // heartbeat->page1.vehicle_state = update_param(temp.page1.vehicle_state, heartbeat->page1.vehicle_state, "vehicle_state") ? temp.page1.vehicle_state: heartbeat->page1.vehicle_state;
                // heartbeat->page1.error_value = update_param(temp.page1.error_value, heartbeat->page1.error_value, "error_value") ? temp.page1.error_value : heartbeat->page1.error_value;
                // heartbeat->page1.traction_running = update_param(temp.page1.traction_running, heartbeat->page1.traction_running, "traction_running") ? temp.page1.traction_running : heartbeat->page1.traction_running;
                // heartbeat->page1.brush_running = update_param(temp.page1.brush_running, heartbeat->page1.brush_running, "brush_running") ? temp.page1.brush_running : heartbeat->page1.brush_running;
                // heartbeat->page1.vacuum_running = update_param(temp.page1.vacuum_running, heartbeat->page1.vacuum_running, "vacuum_running") ? temp.page1.vacuum_running : heartbeat->page1.vacuum_running;
                // heartbeat->page1.accel_sro = update_param(temp.page1.accel_sro, heartbeat->page1.accel_sro, "accel_sro") ? temp.page1.accel_sro : heartbeat->page1.accel_sro;
                // heartbeat->page1.brake_open = update_param(temp.page1.brake_open, heartbeat->page1.brake_open, "brake_open") ? temp.page1.brake_open : heartbeat->page1.brake_open;
                // heartbeat->page1.clean_water_state = update_param(temp.page1.clean_water_state, heartbeat->page1.clean_water_state, "clean_water_state") ? temp.page1.clean_water_state : heartbeat->page1.clean_water_state;
                // heartbeat->page1.dirty_water_full = update_param(temp.page1.dirty_water_full, heartbeat->page1.dirty_water_full, "dirty_water_full") ? temp.page1.dirty_water_full : heartbeat->page1.dirty_water_full;
                // heartbeat->page1.not_used = update_param(temp.page1.not_used, heartbeat->page1.not_used, "not_used") ? temp.page1.not_used : heartbeat->page1.not_used;

                heartbeat->page1.brush_on = update_param(temp.page1.brush_on, heartbeat->page1.brush_on, "brush_on") ? temp.page1.brush_on : heartbeat->page1.brush_on;

                break;
            }
            case 2:
            {
                break;
            }
            case 3:
            {
                break;
            }
            case 4:
            {
                break;
            }
            case 5:
            {
                break;
            }
            case 6:
            {
                break;
            }
            case 7:
            {
                break;
            }
            case 8:
            {
                break;
            }
            case 9:
            {
                break;
            }
            case 10:
            {
                break;
            }
        }
    }
}

