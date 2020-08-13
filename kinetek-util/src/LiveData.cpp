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

// #define TLC "\033[50;0f"
// #define TLC2 "\033[50;40f"
// #define NEXT "\033[30D\033[1B" // goes back and down a line
// check if a parameter has changed since the last heartbeat
bool LiveData::update_param(uint8_t param_new, uint8_t param_old, const string& param_name)
{
    if(param_new != param_old)
    {
        printf("\n\033[1m%-30s\033[0mchanged from %i to %i", param_name.c_str(), param_old, param_new);
        return true;
    }
    else
    {
         //printf("\n%-30s: %i", param_name.c_str(), param_old);
    }
    return false;
}
bool LiveData::update_param(uint16_t param_new, uint16_t param_old, const string& param_name)
{
    if(param_new != param_old)
    {
        //printf("%s", NEXT);
        printf("\n\033[1m%-30s\033[0mchanged from %i to %i", param_name.c_str(), param_old, param_new);
        return true;
    }
    else
    {
        // printf("\n%s: %i", param_name.c_str(), param_old);
    }
    
    return false;
}


// updates the heartbeat struct every page
KU::StatusCode LiveData::update_heartbeat()
{
    // store a temp version to compare against
    controller_heartbeat temp;
    while(true)
    {
        // get the next page
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
                //printf("%s=== PAGE 1 ===\n", TLC);
                // copy the page data into the temp variable
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
                heartbeat->page1.vehicle_state = update_param(temp.page1.vehicle_state, heartbeat->page1.vehicle_state, "vehicle_state") ? temp.page1.vehicle_state: heartbeat->page1.vehicle_state;
                heartbeat->page1.error_value = update_param(temp.page1.error_value, heartbeat->page1.error_value, "error_value") ? temp.page1.error_value : heartbeat->page1.error_value;
               
                // input_flag0               
                heartbeat->page1.traction_running = update_param(temp.page1.traction_running, heartbeat->page1.traction_running, "traction_running") ? temp.page1.traction_running : heartbeat->page1.traction_running;
                heartbeat->page1.brush_running = update_param(temp.page1.brush_running, heartbeat->page1.brush_running, "brush_running") ? temp.page1.brush_running : heartbeat->page1.brush_running;
                heartbeat->page1.vacuum_running = update_param(temp.page1.vacuum_running, heartbeat->page1.vacuum_running, "vacuum_running") ? temp.page1.vacuum_running : heartbeat->page1.vacuum_running;
                heartbeat->page1.accel_sro = update_param(temp.page1.accel_sro, heartbeat->page1.accel_sro, "accel_sro") ? temp.page1.accel_sro : heartbeat->page1.accel_sro;
                heartbeat->page1.brake_open = update_param(temp.page1.brake_open, heartbeat->page1.brake_open, "brake_open") ? temp.page1.brake_open : heartbeat->page1.brake_open;
                heartbeat->page1.clean_water_state = update_param(temp.page1.clean_water_state, heartbeat->page1.clean_water_state, "clean_water_state") ? temp.page1.clean_water_state : heartbeat->page1.clean_water_state;
                heartbeat->page1.dirty_water_full = update_param(temp.page1.dirty_water_full, heartbeat->page1.dirty_water_full, "dirty_water_full") ? temp.page1.dirty_water_full : heartbeat->page1.dirty_water_full;
                heartbeat->page1.not_used = update_param(temp.page1.not_used, heartbeat->page1.not_used, "not_used") ? temp.page1.not_used : heartbeat->page1.not_used;

                // input_flag1              
                heartbeat->page1.seat_buf = update_param(temp.page1.seat_buf, heartbeat->page1.seat_buf, "seat_buf") ? temp.page1.seat_buf : heartbeat->page1.seat_buf;
                heartbeat->page1.ems_buf = update_param(temp.page1.ems_buf, heartbeat->page1.ems_buf, "ems_buf") ? temp.page1.ems_buf : heartbeat->page1.ems_buf;
                heartbeat->page1.start_buf = update_param(temp.page1.start_buf, heartbeat->page1.start_buf, "start_buf") ? temp.page1.start_buf : heartbeat->page1.start_buf;
                heartbeat->page1.aux1_buf = update_param(temp.page1.aux1_buf, heartbeat->page1.aux1_buf, "aux1_buf") ? temp.page1.aux1_buf : heartbeat->page1.aux1_buf;
                heartbeat->page1.aux2_buf = update_param(temp.page1.aux2_buf, heartbeat->page1.aux2_buf, "aux2_buf") ? temp.page1.aux2_buf : heartbeat->page1.aux2_buf;
                heartbeat->page1.aux3_buf = update_param(temp.page1.aux3_buf, heartbeat->page1.aux3_buf, "aux3_buf") ? temp.page1.aux3_buf : heartbeat->page1.aux3_buf;
                heartbeat->page1.brake_enable_buf = update_param(temp.page1.brake_enable_buf, heartbeat->page1.brake_enable_buf, "brake_enable_buf") ? temp.page1.brake_enable_buf : heartbeat->page1.brake_enable_buf;
                heartbeat->page1.step_brake_enable_buf = update_param(temp.page1.step_brake_enable_buf, heartbeat->page1.step_brake_enable_buf, "step_brake_enable_buf") ? temp.page1.step_brake_enable_buf : heartbeat->page1.step_brake_enable_buf;

                // input_flag2              
                heartbeat->page1.traction_open = update_param(temp.page1.traction_open, heartbeat->page1.traction_open, "traction_open") ? temp.page1.traction_open : heartbeat->page1.traction_open;
                heartbeat->page1.brush_open = update_param(temp.page1.brush_open, heartbeat->page1.brush_open, "brush_open") ? temp.page1.brush_open : heartbeat->page1.brush_open;
                heartbeat->page1.vacuum_open = update_param(temp.page1.vacuum_open, heartbeat->page1.vacuum_open, "vacuum_open") ? temp.page1.vacuum_open : heartbeat->page1.vacuum_open;
                heartbeat->page1.alarm_on = update_param(temp.page1.alarm_on, heartbeat->page1.alarm_on, "alarm_on") ? temp.page1.alarm_on : heartbeat->page1.alarm_on;
                heartbeat->page1.valve_on = update_param(temp.page1.valve_on, heartbeat->page1.valve_on, "valve_on") ? temp.page1.valve_on : heartbeat->page1.valve_on;
                heartbeat->page1.aux1_on2 = update_param(temp.page1.aux1_on2, heartbeat->page1.aux1_on2, "aux1_on2") ? temp.page1.aux1_on2 : heartbeat->page1.aux1_on2;
                heartbeat->page1.aux2_on2 = update_param(temp.page1.aux2_on2, heartbeat->page1.aux2_on2, "aux2_on2") ? temp.page1.aux2_on2 : heartbeat->page1.aux2_on2;
                heartbeat->page1.brake_on = update_param(temp.page1.brake_on, heartbeat->page1.brake_on, "brake_on") ? temp.page1.brake_on : heartbeat->page1.brake_on;
                
                // input_flag3
                heartbeat->page1.brush_on = update_param(temp.page1.brush_on, heartbeat->page1.brush_on, "brush_on") ? temp.page1.brush_on : heartbeat->page1.brush_on;
                heartbeat->page1.vacuum_on = update_param(temp.page1.vacuum_on, heartbeat->page1.vacuum_on, "vacuum_on") ? temp.page1.vacuum_on : heartbeat->page1.vacuum_on;
                heartbeat->page1.aux1_on3 = update_param(temp.page1.aux1_on3, heartbeat->page1.aux1_on3, "aux1_on3") ? temp.page1.aux1_on3 : heartbeat->page1.aux1_on3;
                heartbeat->page1.aux2_on3 = update_param(temp.page1.aux2_on3, heartbeat->page1.aux2_on3, "aux2_on3") ? temp.page1.aux2_on3 : heartbeat->page1.aux2_on3;
                heartbeat->page1.squeegee_up_limit = update_param(temp.page1.squeegee_up_limit, heartbeat->page1.squeegee_up_limit, "squeegee_up_limit") ? temp.page1.squeegee_up_limit : heartbeat->page1.squeegee_up_limit;
                heartbeat->page1.squeegee_down_limit = update_param(temp.page1.squeegee_down_limit, heartbeat->page1.squeegee_down_limit, "squeegee_down_limit") ? temp.page1.squeegee_down_limit : heartbeat->page1.squeegee_down_limit;
                heartbeat->page1.brush_deck_up_limit = update_param(temp.page1.brush_deck_up_limit, heartbeat->page1.brush_deck_up_limit, "brush_deck_up_limit") ? temp.page1.brush_deck_up_limit : heartbeat->page1.brush_deck_up_limit;
                heartbeat->page1.brush_deck_down_limit = update_param(temp.page1.brush_deck_down_limit, heartbeat->page1.brush_deck_down_limit, "brush_deck_down_limit") ? temp.page1.brush_deck_down_limit : heartbeat->page1.brush_deck_down_limit;
                break;
            }
            case 2:
            {
               // printf("%s=== PAGE 2 ===", TLC2);
                // input_flag4
                heartbeat->page2.clean_water_buf = update_param(temp.page2.clean_water_buf, heartbeat->page2.clean_water_buf, "clean_water_buf") ? temp.page2.clean_water_buf : heartbeat->page2.clean_water_buf;
                heartbeat->page2.dirty_water_buf = update_param(temp.page2.dirty_water_buf, heartbeat->page2.dirty_water_buf, "dirty_water_buf") ? temp.page2.dirty_water_buf : heartbeat->page2.dirty_water_buf;
                heartbeat->page2.valve_on = update_param(temp.page2.valve_on, heartbeat->page2.valve_on, "valve_on") ? temp.page2.valve_on : heartbeat->page2.valve_on;
                heartbeat->page2.alarm_on = update_param(temp.page2.alarm_on, heartbeat->page2.alarm_on, "alarm_on") ? temp.page2.alarm_on : heartbeat->page2.alarm_on;
                heartbeat->page2.forward_buf = update_param(temp.page2.forward_buf, heartbeat->page2.forward_buf, "forward_buf") ? temp.page2.forward_buf : heartbeat->page2.forward_buf;
                heartbeat->page2.reverse_buf = update_param(temp.page2.reverse_buf, heartbeat->page2.reverse_buf, "reverse_buf") ? temp.page2.reverse_buf : heartbeat->page2.reverse_buf;
                heartbeat->page2.battery_low = update_param(temp.page2.battery_low, heartbeat->page2.battery_low, "battery_low") ? temp.page2.battery_low : heartbeat->page2.battery_low;
                heartbeat->page2.lcd_watch_flag = update_param(temp.page2.lcd_watch_flag, heartbeat->page2.lcd_watch_flag, "lcd_watch_flag") ? temp.page2.lcd_watch_flag : heartbeat->page2.lcd_watch_flag;

                // input_flag5-6
                 // input_flag4
                heartbeat->page2.brush_unload_on = update_param(temp.page2.brush_unload_on, heartbeat->page2.brush_unload_on, "brush_unload_on") ? temp.page2.brush_unload_on : heartbeat->page2.brush_unload_on;
                heartbeat->page2.brush_unload_complete = update_param(temp.page2.brush_unload_complete, heartbeat->page2.brush_unload_complete, "brush_unload_complete") ? temp.page2.brush_unload_complete : heartbeat->page2.brush_unload_complete;
                heartbeat->page2.brush_load_on = update_param(temp.page2.brush_load_on, heartbeat->page2.brush_load_on, "brush_load_on") ? temp.page2.brush_load_on : heartbeat->page2.brush_load_on;
                heartbeat->page2.brush_load_complete = update_param(temp.page2.brush_load_complete, heartbeat->page2.brush_load_complete, "brush_load_complete") ? temp.page2.brush_load_complete : heartbeat->page2.brush_load_complete;
                heartbeat->page2.p3_f1 = update_param(temp.page2.p3_f1, heartbeat->page2.p3_f1, "p3_f1") ? temp.page2.p3_f1 : heartbeat->page2.p3_f1;
                heartbeat->page2.p3_f2 = update_param(temp.page2.p3_f2, heartbeat->page2.p3_f2, "p3_f2") ? temp.page2.p3_f2 : heartbeat->page2.p3_f2;
                heartbeat->page2.p3_f3 = update_param(temp.page2.p3_f3, heartbeat->page2.p3_f3, "p3_f3") ? temp.page2.p3_f3 : heartbeat->page2.p3_f3;
                heartbeat->page2.p3_f4 = update_param(temp.page2.p3_f4, heartbeat->page2.p3_f4, "p3_f4") ? temp.page2.p3_f4 : heartbeat->page2.p3_f4;
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

