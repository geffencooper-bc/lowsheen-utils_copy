#include "LiveData.h"
#include <chrono>
#include <sstream>
#include <iomanip>

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
        LOG_PRINT(("Socket Can Helper is not initialized\n"));
        exit(EXIT_FAILURE);
    }
    this->sc = sc;

    if (ku_data == nullptr)
    {
        LOG_PRINT(("Kinetek Utility CanDataList is not initialized\n"));
        exit(EXIT_FAILURE);
    }
    this->ku_data = ku_data;
    heartbeat = new controller_heartbeat;
    begin = std::chrono::steady_clock::now();
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

#define TLC "\033[4;0f"
#define TLC2 "\033[4;40f"
#define TLC3 "\033[4;80f"
#define TLC4 "\033[4;120f"
#define TLC5 "\033[4;160f"
#define TLC6 "\033[44;0f"
#define TLC7 "\033[44;40f"
#define TLC8 "\033[44;80f"
#define TLC9 "\033[44;120f"
#define TLC10 "\033[44;160f"
#define SAVE "\033[s"
#define RESTORE "\033[u"
#define BACK "\033[10D"
#define DOWN   "\033[1B"
#define COORD0 "\033[16;120f"
#define COORD1 "\033[17;120f"
#define COORD2 "\033[18;120f"
#define COORD3 "\033[19;120f"
#define COORD4 "\033[20;120f"
#define BOLD_ON "\033[1m"
#define BOLD_OFF "\033[0m"
#define CLEAR "\033[2J"

#define MSG_LEN 70
// check if a parameter has changed since the last heartbeat
bool LiveData::update_param(uint8_t param_new, uint8_t param_old, const string& param_name, bool sig)
{
    end = std::chrono::steady_clock::now();
    std::stringstream stream;
    if(param_new != param_old && sig)
    {
        stream << BOLD_ON << std::setw(30) << param_name << BOLD_OFF << std::setw(5) << std::to_string(param_old) << "   to" << std::setw(5) << std::to_string(param_new) << " time: " << std::setw(10) << std::fixed << std::setprecision(5) << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000000.0;
        if(top_10.size() >= MSG_LEN*5)
        {
            top_10 = stream.str() + top_10.substr(0, top_10.size()-MSG_LEN);
        }
        else
        {
            top_10 += stream.str();
        }

        if(top_10.size() >= MSG_LEN)
        {
            LOG_PRINT(("%s1. %s", COORD0, top_10.substr(0, MSG_LEN).c_str()));
            if(top_10.size() >= MSG_LEN*2)
            {
                LOG_PRINT(("%s2. %s", COORD1, top_10.substr(MSG_LEN, MSG_LEN).c_str()));
                if(top_10.size() >= MSG_LEN*3)
                {
                    LOG_PRINT(("%s3. %s", COORD2, top_10.substr(MSG_LEN*2, MSG_LEN).c_str()));
                    if(top_10.size() >= MSG_LEN*4)
                    {
                        LOG_PRINT(("%s4. %s", COORD3, top_10.substr(MSG_LEN*3, MSG_LEN).c_str()));
                        if(top_10.size() >= MSG_LEN*5)
                        {
                            LOG_PRINT(("%s5. %s", COORD4, top_10.substr(MSG_LEN*4, MSG_LEN).c_str()));
                        }
                    }
                }
            }
        }
        //LOG_PRINT(("%s%s %i", COORD0,stream.str().c_str(), stream.str().size()));
        LOG_PRINT(("%s%s%s%-30s: %i", RESTORE, DOWN, SAVE, param_name.c_str(), param_old));
        return true;
    }
    LOG_PRINT(("%s%s%s%-30s: %i", RESTORE, DOWN, SAVE, param_name.c_str(), param_old));
    return false;
}
bool LiveData::update_param(uint16_t param_new, uint16_t param_old, const string& param_name, bool sig)
{
    end = std::chrono::steady_clock::now();
    std::stringstream stream;
    if(param_new != param_old && sig)
    {
        stream << BOLD_ON << std::setw(30) << param_name << BOLD_OFF << std::setw(5) << std::to_string(param_old) << "   to" << std::setw(5) << std::to_string(param_new) << " time: " << std::setw(10) << std::fixed << std::setprecision(5) << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000000.0;
        if(top_10.size() >= MSG_LEN*5)
        {
            top_10 = stream.str() + top_10.substr(0, top_10.size()-MSG_LEN);
        }
        else
        {
            top_10 += stream.str();
        }

        if(top_10.size() >= MSG_LEN)
        {
            LOG_PRINT(("%s1. %s", COORD0, top_10.substr(0, MSG_LEN).c_str()));
            if(top_10.size() >= MSG_LEN*2)
            {
                LOG_PRINT(("%s2. %s", COORD1, top_10.substr(MSG_LEN, MSG_LEN).c_str()));
                if(top_10.size() >= MSG_LEN*3)
                {
                    LOG_PRINT(("%s3. %s", COORD2, top_10.substr(MSG_LEN*2, MSG_LEN).c_str()));
                    if(top_10.size() >= MSG_LEN*4)
                    {
                        LOG_PRINT(("%s4. %s", COORD3, top_10.substr(MSG_LEN*3, MSG_LEN).c_str()));
                        if(top_10.size() >= MSG_LEN*5)
                        {
                            LOG_PRINT(("%s5. %s", COORD4, top_10.substr(MSG_LEN*4, MSG_LEN).c_str()));
                        }
                    }
                }
            }
        }
        //LOG_PRINT(("%s%s %i", COORD0,stream.str().c_str(), stream.str().size()));
        LOG_PRINT(("%s%s%s%-30s: %i", RESTORE, DOWN, SAVE, param_name.c_str(), param_old));
        return true;
    }
    LOG_PRINT(("%s%s%s%-30s: %i", RESTORE, DOWN, SAVE, param_name.c_str(), param_old));
    return false;
}


// updates the heartbeat struct every page
KU::StatusCode LiveData::update_heartbeat()
{
    printf("%s%s", CLEAR, TLC);
    // store a temp version to compare against
    controller_heartbeat temp;
    while(true)
    {
        // get the next page
        CO_CANrxMsg_t* resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 1000);
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
                LOG_PRINT(("%s%s%s", TLC, SAVE, "=== PAGE 1 ==="));
                // copy the page data into the temp variable
                memcpy(&temp.page1, resp->data+2, sizeof(temp.page1));
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
                LOG_PRINT(("%s%s%s", TLC2, SAVE, "=== PAGE 2 ==="));
                memcpy(&temp.page2, resp->data+2, sizeof(temp.page2));
    
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
                heartbeat->page2.brush_unload_on = update_param(temp.page2.brush_unload_on, heartbeat->page2.brush_unload_on, "brush_unload_on") ? temp.page2.brush_unload_on : heartbeat->page2.brush_unload_on;
                heartbeat->page2.brush_unload_complete = update_param(temp.page2.brush_unload_complete, heartbeat->page2.brush_unload_complete, "brush_unload_complete") ? temp.page2.brush_unload_complete : heartbeat->page2.brush_unload_complete;
                heartbeat->page2.brush_load_on = update_param(temp.page2.brush_load_on, heartbeat->page2.brush_load_on, "brush_load_on") ? temp.page2.brush_load_on : heartbeat->page2.brush_load_on;
                heartbeat->page2.brush_load_complete = update_param(temp.page2.brush_load_complete, heartbeat->page2.brush_load_complete, "brush_load_complete") ? temp.page2.brush_load_complete : heartbeat->page2.brush_load_complete;
                heartbeat->page2.p3_f1 = update_param(temp.page2.p3_f1, heartbeat->page2.p3_f1, "p3_f1") ? temp.page2.p3_f1 : heartbeat->page2.p3_f1;
                heartbeat->page2.p3_f2 = update_param(temp.page2.p3_f2, heartbeat->page2.p3_f2, "p3_f2") ? temp.page2.p3_f2 : heartbeat->page2.p3_f2;
                heartbeat->page2.p3_f3 = update_param(temp.page2.p3_f3, heartbeat->page2.p3_f3, "p3_f3") ? temp.page2.p3_f3 : heartbeat->page2.p3_f3;
                heartbeat->page2.p3_f4 = update_param(temp.page2.p3_f4, heartbeat->page2.p3_f4, "p3_f4") ? temp.page2.p3_f4 : heartbeat->page2.p3_f4;

                // input_flag7-9
                heartbeat->page2.p3_f5 = update_param(temp.page2.p3_f5, heartbeat->page2.p3_f5, "p3_f5") ? temp.page2.p3_f5 : heartbeat->page2.p3_f5;
                heartbeat->page2.p3_f11 = update_param(temp.page2.p3_f11, heartbeat->page2.p3_f11, "p3_f11") ? temp.page2.p3_f11 : heartbeat->page2.p3_f11;
                heartbeat->page2.p3_f12 = update_param(temp.page2.p3_f12, heartbeat->page2.p3_f12, "p3_f12") ? temp.page2.p3_f12 : heartbeat->page2.p3_f12;
                heartbeat->page2.p3_f13 = update_param(temp.page2.p3_f13, heartbeat->page2.p3_f13, "p3_f13") ? temp.page2.p3_f13 : heartbeat->page2.p3_f13;
                heartbeat->page2.p2_f6 = update_param(temp.page2.p2_f6, heartbeat->page2.p2_f6, "p3_f1") ? temp.page2.p2_f6 : heartbeat->page2.p2_f6;
                heartbeat->page2.p2_f7 = update_param(temp.page2.p2_f7, heartbeat->page2.p2_f7, "p2_f7") ? temp.page2.p2_f7 : heartbeat->page2.p2_f7;
                heartbeat->page2.p2_f13 = update_param(temp.page2.p2_f13, heartbeat->page2.p2_f13, "p2_f13") ? temp.page2.p2_f13 : heartbeat->page2.p2_f13;
                heartbeat->page2.p2_f14 = update_param(temp.page2.p2_f14, heartbeat->page2.p2_f14, "p2_f14") ? temp.page2.p2_f14 : heartbeat->page2.p2_f14;
                break;
            }
            case 3:
            {
                LOG_PRINT(("%s%s%s", TLC3, SAVE, "=== PAGE 3 ==="));
                memcpy(&temp.page3, resp->data+2, sizeof(temp.page3));
                // error_flag
                heartbeat->page3.traction_error = update_param(temp.page3.traction_error, heartbeat->page3.traction_error, "traction_error") ? temp.page3.traction_error : heartbeat->page3.traction_error;
                heartbeat->page3.brush_error = update_param(temp.page3.brush_error, heartbeat->page3.brush_error, "brush_error") ? temp.page3.brush_error : heartbeat->page3.brush_error;
                heartbeat->page3.vacuum_error = update_param(temp.page3.vacuum_error, heartbeat->page3.vacuum_error, "vacuum_error") ? temp.page3.vacuum_error : heartbeat->page3.vacuum_error;
                heartbeat->page3.brush_deck_error = update_param(temp.page3.brush_deck_error, heartbeat->page3.brush_deck_error, "brush_deck_error") ? temp.page3.brush_deck_error : heartbeat->page3.brush_deck_error;
                heartbeat->page3.squeegee_error = update_param(temp.page3.squeegee_error, heartbeat->page3.squeegee_error, "squeegee_error") ? temp.page3.squeegee_error : heartbeat->page3.squeegee_error;
                heartbeat->page3.brake_error = update_param(temp.page3.brake_error, heartbeat->page3.brake_error, "brake_error") ? temp.page3.brake_error : heartbeat->page3.brake_error;
                heartbeat->page3.no_brush_error = update_param(temp.page3.no_brush_error, heartbeat->page3.no_brush_error, "no_brush_error") ? temp.page3.no_brush_error : heartbeat->page3.no_brush_error;
                heartbeat->page3.motor_state = update_param(temp.page3.motor_state, heartbeat->page3.motor_state, "motor_state") ? temp.page3.motor_state : heartbeat->page3.motor_state;

                // error_flag1
                heartbeat->page3.aux1_error = update_param(temp.page3.aux1_error, heartbeat->page3.aux1_error, "aux1_error") ? temp.page3.aux1_error : heartbeat->page3.aux1_error;
                heartbeat->page3.aux2_error = update_param(temp.page3.aux2_error, heartbeat->page3.aux2_error, "aux2_error") ? temp.page3.aux2_error : heartbeat->page3.aux2_error;
                heartbeat->page3.alarm_error = update_param(temp.page3.alarm_error, heartbeat->page3.alarm_error, "alarm_error") ? temp.page3.alarm_error : heartbeat->page3.alarm_error;
                heartbeat->page3.valve_error = update_param(temp.page3.valve_error, heartbeat->page3.valve_error, "valve_error") ? temp.page3.valve_error : heartbeat->page3.valve_error;
                heartbeat->page3.eeprom_error = update_param(temp.page3.eeprom_error, heartbeat->page3.eeprom_error, "eeprom_error") ? temp.page3.eeprom_error : heartbeat->page3.eeprom_error;
                heartbeat->page3.brush_adjust_error = update_param(temp.page3.brush_adjust_error, heartbeat->page3.brush_adjust_error, "brush_adjust_error") ? temp.page3.brush_adjust_error : heartbeat->page3.brush_adjust_error;
                heartbeat->page3.battery_charge_flag = update_param(temp.page3.battery_charge_flag, heartbeat->page3.battery_charge_flag, "battery_charge_flag") ? temp.page3.battery_charge_flag : heartbeat->page3.battery_charge_flag;
                heartbeat->page3.batery_lockout = update_param(temp.page3.batery_lockout, heartbeat->page3.batery_lockout, "batery_lockout") ? temp.page3.batery_lockout : heartbeat->page3.batery_lockout;

                heartbeat->page3.desired_direction = update_param(temp.page3.desired_direction, heartbeat->page3.desired_direction, "desired_direction") ? temp.page3.desired_direction : heartbeat->page3.desired_direction;
                heartbeat->page3.actual_direction = update_param(temp.page3.actual_direction, heartbeat->page3.actual_direction, "actual_direction") ? temp.page3.actual_direction : heartbeat->page3.actual_direction;
                heartbeat->page3.bat_volt_81_V_1 = update_param(temp.page3.bat_volt_81_V_1, heartbeat->page3.bat_volt_81_V_1, "bat_volt_81_V_1", false) ? temp.page3.bat_volt_81_V_1 : heartbeat->page3.bat_volt_81_V_1;
                heartbeat->page3.bat_volt_81_V_2 = update_param(temp.page3.bat_volt_81_V_2, heartbeat->page3.bat_volt_81_V_2, "bat_volt_81_V_2", false) ? temp.page3.bat_volt_81_V_2 : heartbeat->page3.bat_volt_81_V_2;
                break;
            }
            case 4:
            {
                LOG_PRINT(("%s%s%s", TLC4, SAVE, "=== PAGE 4 ==="));
                memcpy(&temp.page4, resp->data+2, sizeof(temp.page4));
                heartbeat->page4.heatsink_volt_traction_81_V = update_param(temp.page4.heatsink_volt_traction_81_V, heartbeat->page4.heatsink_volt_traction_81_V, "heatsink_volt_traction_81_V", false) ? temp.page4.heatsink_volt_traction_81_V : heartbeat->page4.heatsink_volt_traction_81_V;
                heartbeat->page4.heatsink_volt_other_81_V = update_param(temp.page4.heatsink_volt_other_81_V, heartbeat->page4.heatsink_volt_other_81_V, "heatsink_volt_other_81_V", false) ? temp.page4.heatsink_volt_other_81_V : heartbeat->page4.heatsink_volt_other_81_V;
                heartbeat->page4.tract_ddc = update_param(temp.page4.tract_ddc, heartbeat->page4.tract_ddc, "tract_ddc") ? temp.page4.tract_ddc : heartbeat->page4.tract_ddc;
                heartbeat->page4.tract_adc = update_param(temp.page4.tract_adc, heartbeat->page4.tract_adc, "tract_adc") ? temp.page4.tract_adc : heartbeat->page4.tract_adc;
                heartbeat->page4.mosfet_temperature = update_param(temp.page4.mosfet_temperature, heartbeat->page4.mosfet_temperature, "mosfet_temperature", false) ? temp.page4.mosfet_temperature : heartbeat->page4.mosfet_temperature;
                heartbeat->page4.brush_current_A = update_param(temp.page4.brush_current_A, heartbeat->page4.brush_current_A, "brush_current_A") ? temp.page4.brush_current_A : heartbeat->page4.brush_current_A;
                break;
            }
            case 5:
            {
                LOG_PRINT(("%s%s%s", TLC5, SAVE, "=== PAGE 5 ==="));
                memcpy(&temp.page5, resp->data+2, sizeof(temp.page5));
                heartbeat->page5.vacuum_current_A = update_param(temp.page5.vacuum_current_A, heartbeat->page5.vacuum_current_A, "vacuum_current_A") ? temp.page5.vacuum_current_A : heartbeat->page5.vacuum_current_A;
                heartbeat->page5.squeegee_current_A = update_param(temp.page5.squeegee_current_A, heartbeat->page5.squeegee_current_A, "squeegee_current_A") ? temp.page5.squeegee_current_A : heartbeat->page5.squeegee_current_A;
                heartbeat->page5.brush_deck_current_A = update_param(temp.page5.brush_deck_current_A, heartbeat->page5.brush_deck_current_A, "brush_deck_current_A") ? temp.page5.brush_deck_current_A : heartbeat->page5.brush_deck_current_A;
                heartbeat->page5.desired_brush_deck_direction = update_param(temp.page5.desired_brush_deck_direction, heartbeat->page5.desired_brush_deck_direction, "desired_brush_deck_direction") ? temp.page5.desired_brush_deck_direction : heartbeat->page5.desired_brush_deck_direction;
                heartbeat->page5.desired_squeegee_direction = update_param(temp.page5.desired_squeegee_direction, heartbeat->page5.desired_squeegee_direction, "desired_squeegee_direction") ? temp.page5.desired_squeegee_direction : heartbeat->page5.desired_squeegee_direction;
                heartbeat->page5.brush_auto_adjust_flag = update_param(temp.page5.brush_auto_adjust_flag, heartbeat->page5.brush_auto_adjust_flag, "brush_auto_adjust_flag") ? temp.page5.brush_auto_adjust_flag : heartbeat->page5.brush_auto_adjust_flag;
                break;
            }
            case 6:
            {
                LOG_PRINT(("%s%s%s", TLC6, SAVE, "=== PAGE 6 ==="));
                memcpy(&temp.page6, resp->data+2, sizeof(temp.page6));
                heartbeat->page6.traction_rev_current_dir = update_param(temp.page6.traction_rev_current_dir, heartbeat->page6.traction_rev_current_dir, "traction_rev_current_dir") ? temp.page6.traction_rev_current_dir : heartbeat->page6.traction_rev_current_dir;
                heartbeat->page6.traction_rev_current_A = update_param(temp.page6.traction_rev_current_A, heartbeat->page6.traction_rev_current_A, "traction_rev_current_A") ? temp.page6.traction_rev_current_A : heartbeat->page6.traction_rev_current_A;
                heartbeat->page6.traction_left_null_current = update_param(temp.page6.traction_left_null_current, heartbeat->page6.traction_left_null_current, "traction_left_null_current") ? temp.page6.traction_left_null_current : heartbeat->page6.traction_left_null_current;
                heartbeat->page6.traction_fwd_current_dir = update_param(temp.page6.traction_fwd_current_dir, heartbeat->page6.traction_fwd_current_dir, "traction_fwd_current_dir") ? temp.page6.traction_fwd_current_dir : heartbeat->page6.traction_fwd_current_dir;
                heartbeat->page6.traction_fwd_current_A = update_param(temp.page6.traction_fwd_current_A, heartbeat->page6.traction_fwd_current_A, "traction_fwd_current_A") ? temp.page6.traction_fwd_current_A : heartbeat->page6.traction_fwd_current_A;
                break;
            }
            case 7:
            {
                LOG_PRINT(("%s%s%s", TLC7, SAVE, "=== PAGE 7 ==="));
                memcpy(&temp.page7, resp->data+2, sizeof(temp.page7));
                heartbeat->page7.traction_right_null_current = update_param(temp.page7.traction_right_null_current, heartbeat->page7.traction_right_null_current, "traction_right_null_current") ? temp.page7.traction_right_null_current : heartbeat->page7.traction_right_null_current;
                heartbeat->page7.MCU_temp_raw = update_param(temp.page7.MCU_temp_raw, heartbeat->page7.MCU_temp_raw, "MCU_temp_raw", false) ? temp.page7.MCU_temp_raw : heartbeat->page7.MCU_temp_raw;
                heartbeat->page7.vacuum_ddc = update_param(temp.page7.vacuum_ddc, heartbeat->page7.vacuum_ddc, "vacuum_ddc") ? temp.page7.vacuum_ddc : heartbeat->page7.vacuum_ddc;
                heartbeat->page7.accelerator_raw = update_param(temp.page7.accelerator_raw, heartbeat->page7.accelerator_raw, "accelerator_raw") ? temp.page7.accelerator_raw : heartbeat->page7.accelerator_raw;
                break;
            }
            case 8:
            {
                LOG_PRINT(("%s%s%s", TLC8, SAVE, "=== PAGE 8 ==="));
                memcpy(&temp.page8, resp->data+2, sizeof(temp.page8));
                heartbeat->page8.traction_left_drain_voltage = update_param(temp.page8.traction_left_drain_voltage, heartbeat->page8.traction_left_drain_voltage, "traction_left_drain_voltage") ? temp.page8.traction_left_drain_voltage : heartbeat->page8.traction_left_drain_voltage;
                heartbeat->page8.traction_right_drain_voltage = update_param(temp.page8.traction_right_drain_voltage, heartbeat->page8.traction_right_drain_voltage, "traction_right_drain_voltage") ? temp.page8.traction_right_drain_voltage : heartbeat->page8.traction_right_drain_voltage;
                heartbeat->page8.brush_drain_voltage_16_81_V = update_param(temp.page8.brush_drain_voltage_16_81_V, heartbeat->page8.brush_drain_voltage_16_81_V, "brush_drain_voltage_16_81_V", false) ? temp.page8.brush_drain_voltage_16_81_V : heartbeat->page8.brush_drain_voltage_16_81_V;
                heartbeat->page8.vacuum_drain_voltage_16_81_V = update_param(temp.page8.vacuum_drain_voltage_16_81_V, heartbeat->page8.vacuum_drain_voltage_16_81_V, "vacuum_drain_voltage_16_81_V", false) ? temp.page8.vacuum_drain_voltage_16_81_V : heartbeat->page8.vacuum_drain_voltage_16_81_V;
                heartbeat->page8.brush_adc = update_param(temp.page8.brush_adc, heartbeat->page8.brush_adc, "brush_adc") ? temp.page8.brush_adc : heartbeat->page8.brush_adc;
                heartbeat->page8.vacuum_adc = update_param(temp.page8.vacuum_adc, heartbeat->page8.vacuum_adc, "vacuum_adc") ? temp.page8.vacuum_adc : heartbeat->page8.vacuum_adc;
                break;
            }
            case 9:
            {
                LOG_PRINT(("%s%s%s", TLC9, SAVE, "=== PAGE 9 ==="));
                memcpy(&temp.page9, resp->data+2, sizeof(temp.page9));
                heartbeat->page9.customer_id = update_param(temp.page9.customer_id, heartbeat->page9.customer_id, "customer_id") ? temp.page9.customer_id : heartbeat->page9.customer_id;
                heartbeat->page9.firmware_major = update_param(temp.page9.firmware_major, heartbeat->page9.firmware_major, "firmware_major") ? temp.page9.firmware_major : heartbeat->page9.firmware_major;
                heartbeat->page9.firmware_minor = update_param(temp.page9.firmware_minor, heartbeat->page9.firmware_minor, "firmware_minor") ? temp.page9.firmware_minor : heartbeat->page9.firmware_minor;
                heartbeat->page9.bat_type = update_param(temp.page9.bat_type, heartbeat->page9.bat_type, "bat_type") ? temp.page9.bat_type : heartbeat->page9.bat_type;
                heartbeat->page9.vacuum_off_delay_time = update_param(temp.page9.vacuum_off_delay_time, heartbeat->page9.vacuum_off_delay_time, "vacuum_off_delay_time") ? temp.page9.vacuum_off_delay_time : heartbeat->page9.vacuum_off_delay_time;
                heartbeat->page9.brush_ddc = update_param(temp.page9.brush_ddc, heartbeat->page9.brush_ddc, "brush_ddc") ? temp.page9.brush_ddc : heartbeat->page9.brush_ddc;
                break;
            }
            case 10:
            {
                LOG_PRINT(("%s%s%s", TLC10, SAVE, "=== PAGE 10 ==="));
                memcpy(&temp.page10, resp->data+2, sizeof(temp.page10));
                heartbeat->page10.aux1_drain_voltage = update_param(temp.page10.aux1_drain_voltage, heartbeat->page10.aux1_drain_voltage, "aux1_drain_voltage") ? temp.page10.aux1_drain_voltage : heartbeat->page10.aux1_drain_voltage;
                heartbeat->page10.aux2_drain_voltage = update_param(temp.page10.aux2_drain_voltage, heartbeat->page10.aux2_drain_voltage, "aux2_drain_voltage") ? temp.page10.aux2_drain_voltage : heartbeat->page10.aux2_drain_voltage;
                heartbeat->page10.line_coil_voltage = update_param(temp.page10.line_coil_voltage, heartbeat->page10.line_coil_voltage, "line_coil_voltage") ? temp.page10.line_coil_voltage : heartbeat->page10.line_coil_voltage;
                heartbeat->page10.valve_drain_voltage = update_param(temp.page10.valve_drain_voltage, heartbeat->page10.valve_drain_voltage, "valve_drain_voltage") ? temp.page10.valve_drain_voltage : heartbeat->page10.valve_drain_voltage;
                heartbeat->page10.brake_drain_voltage = update_param(temp.page10.brake_drain_voltage, heartbeat->page10.brake_drain_voltage, "brake_drain_voltage", false) ? temp.page10.brake_drain_voltage : heartbeat->page10.brake_drain_voltage;
                
                // input_flag16
                heartbeat->page10.squeegee_init_flag = update_param(temp.page10.squeegee_init_flag, heartbeat->page10.squeegee_init_flag, "squeegee_init_flag") ? temp.page10.squeegee_init_flag : heartbeat->page10.squeegee_init_flag;
                heartbeat->page10.brush_deck_init_flag = update_param(temp.page10.brush_deck_init_flag, heartbeat->page10.brush_deck_init_flag, "brush_deck_init_flag") ? temp.page10.brush_deck_init_flag : heartbeat->page10.brush_deck_init_flag;
                heartbeat->page10.turn_buf = update_param(temp.page10.turn_buf, heartbeat->page10.turn_buf, "turn_buf") ? temp.page10.turn_buf : heartbeat->page10.turn_buf;
                heartbeat->page10.battery_very_low = update_param(temp.page10.battery_very_low, heartbeat->page10.battery_very_low, "battery_very_low") ? temp.page10.battery_very_low : heartbeat->page10.battery_very_low;
                heartbeat->page10.accelerator_pressed = update_param(temp.page10.accelerator_pressed, heartbeat->page10.accelerator_pressed, "accelerator_pressed") ? temp.page10.accelerator_pressed : heartbeat->page10.accelerator_pressed;
                heartbeat->page10.brush_deck_down_protection = update_param(temp.page10.brush_deck_down_protection, heartbeat->page10.brush_deck_down_protection, "brush_deck_down_protection") ? temp.page10.brush_deck_down_protection : heartbeat->page10.brush_deck_down_protection;
                heartbeat->page10.aux4_buf = update_param(temp.page10.aux4_buf, heartbeat->page10.aux4_buf, "aux4_buf") ? temp.page10.aux4_buf : heartbeat->page10.aux4_buf;
                heartbeat->page10.aux5_buf = update_param(temp.page10.aux5_buf, heartbeat->page10.aux5_buf, "aux5_buf") ? temp.page10.aux5_buf : heartbeat->page10.aux5_buf;
                break;
            }
        }
    }
}

