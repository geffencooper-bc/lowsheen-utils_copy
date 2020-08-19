#include "LiveData.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

// escape sequences to adjust cursor location and text format
#define TLC "\033[0;0f"
// #define TLC_TRACTION_V "\033[23;0f"
// #define TLC_SCRUBBER_V "\033[23;40f"
// #define TLC_RECOVERY_V "\033[23;80f"
// #define TLC_MISC_V "\033[23;117f"
// #define TLC_BATTERY_V "\033[23;155f"
#define TLC_ERROR "\033[38;0f"
#define TLC_ERROR_S1 "\033[40;0f"
#define TLC_ERROR_S2 "\033[40;40f"
// #define TLC_TRACTION_S "\033[0;0f"
// #define TLC_SCRUBBER_S "\033[0;40f"
// #define TLC_RECOVERY_S "\033[0;80f"
// #define TLC_MISC_S "\033[0;117f"
// #define TLC_UNKNOWN_S "\033[0;185f"
// #define TLC_BATTERY_S "\033[0;155f"
// #define TLC_META "\033[27;155f"
#define SAVE "\033[s"
#define RESTORE "\033[u"
#define BACK "\033[1D"
#define DOWN "\033[1B"
#define UP "\033[1A"
#define COORD0 "\033[39;80f"
#define BOLD_ON "\033[1m"
#define YELLOW_TITLE "\033[1;33m"
#define RED_TITLE "\033[1;31m"
#define ATTRIB_OFF "\033[0m"
#define CLEAR "\033[2J"
#define PADDING 3
#define RIGHT_EDGE 180
#define BOTTOM_EDGE 23

// number of parameters in each category
#define TRAC_S_SIZE 15
#define SCRUB_S_SIZE 19
#define RECOV_S_SIZE 7
#define MISC_S_SIZE 15
#define BATT_S_SIZE 3
#define UNKNOWN_S_SIZE 13
#define META_S_SIZE 5
#define TRAC_V_SIZE 9
#define SCRUB_V_SIZE 5
#define RECOV_V_SIZE 5
#define MISC_V_SIZE 8
#define BATT_V_SIZE 1
#define ERROR_S1_size 8
#define ERROR_S2_size 8

// width of each category
#define TRAC_S_WIDTH 33
#define SCRUB_S_WIDTH 33
#define RECOV_S_WIDTH 33
#define MISC_S_WIDTH 33
#define BATT_S_WIDTH 23
#define UNKNOWN_S_WIDTH 20
#define META_S_WIDTH 25
#define TRAC_V_WIDTH 40
#define SCRUB_V_WIDTH 40
#define RECOV_V_WIDTH 40
#define MISC_V_WIDTH 40
#define BATT_V_WIDTH 30
#define ERROR_S1_WIDTH 35
#define ERROR_S2_WIDTH 35

// initialize objects
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
    hb = new controller_heartbeat;

    // start the clock on launch
    begin = std::chrono::steady_clock::now();
}

// deallocate memory
LiveData::~LiveData()
{
    delete hb;
}

// callback function for received messages, not used as of now
void LiveData_resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

// clear the screen upon exit
void my_handler(int s)
{
    printf("%s%s", CLEAR, TLC);
    exit(1);
}
// updates the heartbeat struct every page
KU::StatusCode LiveData::update_heartbeat()
{
    parse_ini("live_data_options.ini");
    signal (SIGINT,my_handler);
    // the first run of the while loop
    static bool is_first = true;

    // clear the screen
    printf("%s%s", CLEAR, "\033[0;0f");

    // store a tmp version to compare against to see if values changed
    controller_heartbeat tmp;
    CO_CANrxMsg_t* resp;
    uint8_t page_num = 0;
    uint8_t last_page_num = 0;
    while (true)
    {
        // get the next page
        resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 500);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEART_BEAT)
        {
            LOG_PRINT(("No Heart Beat\n"));
            return KU::NO_HEART_BEAT;
        }

        last_page_num = page_num;
        page_num = resp->data[1];
        if (is_first)
        {
            while (page_num != 1)
            {
                resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 500);
                page_num = resp->data[1];
            }
            is_first = false;
        }
        else
        {
            if(last_page_num+1 == 11)
            {
                last_page_num = 0;
            }
            while (page_num != last_page_num+1)
            {
                //printf("%s%s", "\033[20;0f", "MISSED A PAGE");
                resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 500);
                page_num = resp->data[1];
            } 
        }
        
        switch (page_num)
        {
            case 1:
            {
                // copy the page data into the tmp variable
                memcpy(&tmp.page1, resp->data + 2, sizeof(tmp.page1));
                hb->page1.vehicle_state =
                    update_param_s(tmp.page1.vehicle_state, hb->page1.vehicle_state, "vehicle_state", TRACTION_STATE)
                        ? tmp.page1.vehicle_state
                        : hb->page1.vehicle_state;
                hb->page1.error_value =
                    update_param_s(tmp.page1.error_value, hb->page1.error_value, "error_value", ERROR_STATE)
                        ? tmp.page1.error_value
                        : hb->page1.error_value;

                // input_flag0
                hb->page1.traction_running = update_param_s(tmp.page1.traction_running, hb->page1.traction_running,
                                                            "traction_running", TRACTION_STATE)
                                                 ? tmp.page1.traction_running
                                                 : hb->page1.traction_running;
                hb->page1.brush_running =
                    update_param_s(tmp.page1.brush_running, hb->page1.brush_running, "brush_running", SCRUBBER_STATE)
                        ? tmp.page1.brush_running
                        : hb->page1.brush_running;
                hb->page1.vacuum_running =
                    update_param_s(tmp.page1.vacuum_running, hb->page1.vacuum_running, "vacuum_running", RECOVERY_STATE)
                        ? tmp.page1.vacuum_running
                        : hb->page1.vacuum_running;
                hb->page1.accel_sro =
                    update_param_s(tmp.page1.accel_sro, hb->page1.accel_sro, "accel_sro", TRACTION_STATE)
                        ? tmp.page1.accel_sro
                        : hb->page1.accel_sro;
                hb->page1.brake_open =
                    update_param_s(tmp.page1.brake_open, hb->page1.brake_open, "brake_open", TRACTION_STATE)
                        ? tmp.page1.brake_open
                        : hb->page1.brake_open;
                hb->page1.clean_water_state = update_param_s(tmp.page1.clean_water_state, hb->page1.clean_water_state,
                                                             "clean_water_state", SCRUBBER_STATE)
                                                  ? tmp.page1.clean_water_state
                                                  : hb->page1.clean_water_state;
                hb->page1.dirty_water_full = update_param_s(tmp.page1.dirty_water_full, hb->page1.dirty_water_full,
                                                            "dirty_water_full", SCRUBBER_STATE)
                                                 ? tmp.page1.dirty_water_full
                                                 : hb->page1.dirty_water_full;
                hb->page1.not_used = update_param_s(tmp.page1.not_used, hb->page1.not_used, "not_used", UNKNOWN_STATE)
                                         ? tmp.page1.not_used
                                         : hb->page1.not_used;

                // input_flag1
                hb->page1.seat_buf = update_param_s(tmp.page1.seat_buf, hb->page1.seat_buf, "seat_buf", MISC_STATE)
                                         ? tmp.page1.seat_buf
                                         : hb->page1.seat_buf;
                hb->page1.ems_buf = update_param_s(tmp.page1.ems_buf, hb->page1.ems_buf, "ems_buf", MISC_STATE)
                                        ? tmp.page1.ems_buf
                                        : hb->page1.ems_buf;
                hb->page1.start_buf = update_param_s(tmp.page1.start_buf, hb->page1.start_buf, "start_buf", MISC_STATE)
                                          ? tmp.page1.start_buf
                                          : hb->page1.start_buf;
                hb->page1.aux1_buf = update_param_s(tmp.page1.aux1_buf, hb->page1.aux1_buf, "aux1_buf", MISC_STATE)
                                         ? tmp.page1.aux1_buf
                                         : hb->page1.aux1_buf;
                hb->page1.aux2_buf = update_param_s(tmp.page1.aux2_buf, hb->page1.aux2_buf, "aux2_buf", MISC_STATE)
                                         ? tmp.page1.aux2_buf
                                         : hb->page1.aux2_buf;
                hb->page1.aux3_buf = update_param_s(tmp.page1.aux3_buf, hb->page1.aux3_buf, "aux3_buf", MISC_STATE)
                                         ? tmp.page1.aux3_buf
                                         : hb->page1.aux3_buf;
                hb->page1.brake_enable_buf = update_param_s(tmp.page1.brake_enable_buf, hb->page1.brake_enable_buf,
                                                            "brake_enable_buf", TRACTION_STATE)
                                                 ? tmp.page1.brake_enable_buf
                                                 : hb->page1.brake_enable_buf;
                hb->page1.step_brake_enable_buf =
                    update_param_s(tmp.page1.step_brake_enable_buf, hb->page1.step_brake_enable_buf,
                                   "step_brake_enable_buf", TRACTION_STATE)
                        ? tmp.page1.step_brake_enable_buf
                        : hb->page1.step_brake_enable_buf;

                // input_flag2
                hb->page1.traction_open =
                    update_param_s(tmp.page1.traction_open, hb->page1.traction_open, "traction_open", TRACTION_STATE)
                        ? tmp.page1.traction_open
                        : hb->page1.traction_open;
                hb->page1.brush_open =
                    update_param_s(tmp.page1.brush_open, hb->page1.brush_open, "brush_open", SCRUBBER_STATE)
                        ? tmp.page1.brush_open
                        : hb->page1.brush_open;
                hb->page1.vacuum_open =
                    update_param_s(tmp.page1.vacuum_open, hb->page1.vacuum_open, "vacuum_open", RECOVERY_STATE)
                        ? tmp.page1.vacuum_open
                        : hb->page1.vacuum_open;
                hb->page1.alarm_on = update_param_s(tmp.page1.alarm_on, hb->page1.alarm_on, "alarm_on", MISC_STATE)
                                         ? tmp.page1.alarm_on
                                         : hb->page1.alarm_on;
                hb->page1.valve_on = update_param_s(tmp.page1.valve_on, hb->page1.valve_on, "valve_on", SCRUBBER_STATE)
                                         ? tmp.page1.valve_on
                                         : hb->page1.valve_on;
                hb->page1.aux1_on2 = update_param_s(tmp.page1.aux1_on2, hb->page1.aux1_on2, "aux1_on2", MISC_STATE)
                                         ? tmp.page1.aux1_on2
                                         : hb->page1.aux1_on2;
                hb->page1.aux2_on2 = update_param_s(tmp.page1.aux2_on2, hb->page1.aux2_on2, "aux2_on2", MISC_STATE)
                                         ? tmp.page1.aux2_on2
                                         : hb->page1.aux2_on2;
                hb->page1.brake_on = update_param_s(tmp.page1.brake_on, hb->page1.brake_on, "brake_on", TRACTION_STATE)
                                         ? tmp.page1.brake_on
                                         : hb->page1.brake_on;

                // input_flag3
                hb->page1.brush_on = update_param_s(tmp.page1.brush_on, hb->page1.brush_on, "brush_on", SCRUBBER_STATE)
                                         ? tmp.page1.brush_on
                                         : hb->page1.brush_on;
                hb->page1.vacuum_on =
                    update_param_s(tmp.page1.vacuum_on, hb->page1.vacuum_on, "vacuum_on", RECOVERY_STATE)
                        ? tmp.page1.vacuum_on
                        : hb->page1.vacuum_on;
                hb->page1.aux1_on3 = update_param_s(tmp.page1.aux1_on3, hb->page1.aux1_on3, "aux1_on3", MISC_STATE)
                                         ? tmp.page1.aux1_on3
                                         : hb->page1.aux1_on3;
                hb->page1.aux2_on3 = update_param_s(tmp.page1.aux2_on3, hb->page1.aux2_on3, "aux2_on3", MISC_STATE)
                                         ? tmp.page1.aux2_on3
                                         : hb->page1.aux2_on3;
                hb->page1.squeegee_up_limit = update_param_s(tmp.page1.squeegee_up_limit, hb->page1.squeegee_up_limit,
                                                             "squeegee_up_limit", RECOVERY_STATE)
                                                  ? tmp.page1.squeegee_up_limit
                                                  : hb->page1.squeegee_up_limit;
                hb->page1.squeegee_down_limit =
                    update_param_s(tmp.page1.squeegee_down_limit, hb->page1.squeegee_down_limit, "squeegee_down_limit",
                                   RECOVERY_STATE)
                        ? tmp.page1.squeegee_down_limit
                        : hb->page1.squeegee_down_limit;
                hb->page1.brush_deck_up_limit =
                    update_param_s(tmp.page1.brush_deck_up_limit, hb->page1.brush_deck_up_limit, "brush_deck_up_limit",
                                   SCRUBBER_STATE)
                        ? tmp.page1.brush_deck_up_limit
                        : hb->page1.brush_deck_up_limit;
                hb->page1.brush_deck_down_limit =
                    update_param_s(tmp.page1.brush_deck_down_limit, hb->page1.brush_deck_down_limit,
                                   "brush_deck_down_limit", SCRUBBER_STATE)
                        ? tmp.page1.brush_deck_down_limit
                        : hb->page1.brush_deck_down_limit;
                break;
            }
            case 2:
            {
                memcpy(&tmp.page2, resp->data + 2, sizeof(tmp.page2));

                // input_flag4
                hb->page2.clean_water_buf = update_param_s(tmp.page2.clean_water_buf, hb->page2.clean_water_buf,
                                                           "clean_water_buf", SCRUBBER_STATE)
                                                ? tmp.page2.clean_water_buf
                                                : hb->page2.clean_water_buf;
                hb->page2.dirty_water_buf = update_param_s(tmp.page2.dirty_water_buf, hb->page2.dirty_water_buf,
                                                           "dirty_water_buf", SCRUBBER_STATE)
                                                ? tmp.page2.dirty_water_buf
                                                : hb->page2.dirty_water_buf;
                hb->page2.valve_on = update_param_s(tmp.page2.valve_on, hb->page2.valve_on, "valve_on", SCRUBBER_STATE)
                                         ? tmp.page2.valve_on
                                         : hb->page2.valve_on;
                hb->page2.alarm_on = update_param_s(tmp.page2.alarm_on, hb->page2.alarm_on, "alarm_on", MISC_STATE)
                                         ? tmp.page2.alarm_on
                                         : hb->page2.alarm_on;
                hb->page2.forward_buf =
                    update_param_s(tmp.page2.forward_buf, hb->page2.forward_buf, "forward_buf", TRACTION_STATE)
                        ? tmp.page2.forward_buf
                        : hb->page2.forward_buf;
                hb->page2.reverse_buf =
                    update_param_s(tmp.page2.reverse_buf, hb->page2.reverse_buf, "reverse_buf", TRACTION_STATE)
                        ? tmp.page2.reverse_buf
                        : hb->page2.reverse_buf;
                hb->page2.battery_low =
                    update_param_s(tmp.page2.battery_low, hb->page2.battery_low, "battery_low", BATTERY_STATE)
                        ? tmp.page2.battery_low
                        : hb->page2.battery_low;
                hb->page2.lcd_watch_flag =
                    update_param_s(tmp.page2.lcd_watch_flag, hb->page2.lcd_watch_flag, "lcd_watch_flag", BATTERY_STATE)
                        ? tmp.page2.lcd_watch_flag
                        : hb->page2.lcd_watch_flag;

                // input_flag5-6
                hb->page2.brush_unload_on = update_param_s(tmp.page2.brush_unload_on, hb->page2.brush_unload_on,
                                                           "brush_unload_on", SCRUBBER_STATE)
                                                ? tmp.page2.brush_unload_on
                                                : hb->page2.brush_unload_on;
                hb->page2.brush_unload_complete =
                    update_param_s(tmp.page2.brush_unload_complete, hb->page2.brush_unload_complete,
                                   "brush_unload_complete", SCRUBBER_STATE)
                        ? tmp.page2.brush_unload_complete
                        : hb->page2.brush_unload_complete;
                hb->page2.brush_load_on =
                    update_param_s(tmp.page2.brush_load_on, hb->page2.brush_load_on, "brush_load_on", SCRUBBER_STATE)
                        ? tmp.page2.brush_load_on
                        : hb->page2.brush_load_on;
                hb->page2.brush_load_complete =
                    update_param_s(tmp.page2.brush_load_complete, hb->page2.brush_load_complete, "brush_load_complete",
                                   SCRUBBER_STATE)
                        ? tmp.page2.brush_load_complete
                        : hb->page2.brush_load_complete;
                hb->page2.p3_f1 = update_param_s(tmp.page2.p3_f1, hb->page2.p3_f1, "p3_f1", UNKNOWN_STATE)
                                      ? tmp.page2.p3_f1
                                      : hb->page2.p3_f1;
                hb->page2.p3_f2 = update_param_s(tmp.page2.p3_f2, hb->page2.p3_f2, "p3_f2", UNKNOWN_STATE)
                                      ? tmp.page2.p3_f2
                                      : hb->page2.p3_f2;
                hb->page2.p3_f3 = update_param_s(tmp.page2.p3_f3, hb->page2.p3_f3, "p3_f3", UNKNOWN_STATE)
                                      ? tmp.page2.p3_f3
                                      : hb->page2.p3_f3;
                hb->page2.p3_f4 = update_param_s(tmp.page2.p3_f4, hb->page2.p3_f4, "p3_f4", UNKNOWN_STATE)
                                      ? tmp.page2.p3_f4
                                      : hb->page2.p3_f4;

                // input_flag7-9
                hb->page2.p3_f5 = update_param_s(tmp.page2.p3_f5, hb->page2.p3_f5, "p3_f5", UNKNOWN_STATE)
                                      ? tmp.page2.p3_f5
                                      : hb->page2.p3_f5;
                hb->page2.p3_f11 = update_param_s(tmp.page2.p3_f11, hb->page2.p3_f11, "p3_f11", UNKNOWN_STATE)
                                       ? tmp.page2.p3_f11
                                       : hb->page2.p3_f11;
                hb->page2.p3_f12 = update_param_s(tmp.page2.p3_f12, hb->page2.p3_f12, "p3_f12", UNKNOWN_STATE)
                                       ? tmp.page2.p3_f12
                                       : hb->page2.p3_f12;
                hb->page2.p3_f13 = update_param_s(tmp.page2.p3_f13, hb->page2.p3_f13, "p3_f13", UNKNOWN_STATE)
                                       ? tmp.page2.p3_f13
                                       : hb->page2.p3_f13;
                hb->page2.p2_f6 = update_param_s(tmp.page2.p2_f6, hb->page2.p2_f6, "p3_f1", UNKNOWN_STATE)
                                      ? tmp.page2.p2_f6
                                      : hb->page2.p2_f6;
                hb->page2.p2_f7 = update_param_s(tmp.page2.p2_f7, hb->page2.p2_f7, "p2_f7", UNKNOWN_STATE)
                                      ? tmp.page2.p2_f7
                                      : hb->page2.p2_f7;
                hb->page2.p2_f13 = update_param_s(tmp.page2.p2_f13, hb->page2.p2_f13, "p2_f13", UNKNOWN_STATE)
                                       ? tmp.page2.p2_f13
                                       : hb->page2.p2_f13;
                hb->page2.p2_f14 = update_param_s(tmp.page2.p2_f14, hb->page2.p2_f14, "p2_f14", UNKNOWN_STATE)
                                       ? tmp.page2.p2_f14
                                       : hb->page2.p2_f14;
                break;
            }
            case 3:
            {
                memcpy(&tmp.page3, resp->data + 2, sizeof(tmp.page3));
                // error_flag
                hb->page3.traction_error =
                    update_param_s(tmp.page3.traction_error, hb->page3.traction_error, "traction_error", ERROR_STATE)
                        ? tmp.page3.traction_error
                        : hb->page3.traction_error;
                hb->page3.brush_error =
                    update_param_s(tmp.page3.brush_error, hb->page3.brush_error, "brush_error", ERROR_STATE)
                        ? tmp.page3.brush_error
                        : hb->page3.brush_error;
                hb->page3.vacuum_error =
                    update_param_s(tmp.page3.vacuum_error, hb->page3.vacuum_error, "vacuum_error", ERROR_STATE)
                        ? tmp.page3.vacuum_error
                        : hb->page3.vacuum_error;
                hb->page3.brush_deck_error = update_param_s(tmp.page3.brush_deck_error, hb->page3.brush_deck_error,
                                                            "brush_deck_error", ERROR_STATE)
                                                 ? tmp.page3.brush_deck_error
                                                 : hb->page3.brush_deck_error;
                hb->page3.squeegee_error =
                    update_param_s(tmp.page3.squeegee_error, hb->page3.squeegee_error, "squeegee_error", ERROR_STATE)
                        ? tmp.page3.squeegee_error
                        : hb->page3.squeegee_error;
                hb->page3.brake_error =
                    update_param_s(tmp.page3.brake_error, hb->page3.brake_error, "brake_error", ERROR_STATE)
                        ? tmp.page3.brake_error
                        : hb->page3.brake_error;
                hb->page3.no_brush_error =
                    update_param_s(tmp.page3.no_brush_error, hb->page3.no_brush_error, "no_brush_error", ERROR_STATE)
                        ? tmp.page3.no_brush_error
                        : hb->page3.no_brush_error;
                hb->page3.motor_state =
                    update_param_s(tmp.page3.motor_state, hb->page3.motor_state, "motor_state", ERROR_STATE)
                        ? tmp.page3.motor_state
                        : hb->page3.motor_state;

                // error_flag1
                hb->page3.aux1_error =
                    update_param_s(tmp.page3.aux1_error, hb->page3.aux1_error, "aux1_error", ERROR_STATE)
                        ? tmp.page3.aux1_error
                        : hb->page3.aux1_error;
                hb->page3.aux2_error =
                    update_param_s(tmp.page3.aux2_error, hb->page3.aux2_error, "aux2_error", ERROR_STATE)
                        ? tmp.page3.aux2_error
                        : hb->page3.aux2_error;
                hb->page3.alarm_error =
                    update_param_s(tmp.page3.alarm_error, hb->page3.alarm_error, "alarm_error", ERROR_STATE)
                        ? tmp.page3.alarm_error
                        : hb->page3.alarm_error;
                hb->page3.valve_error =
                    update_param_s(tmp.page3.valve_error, hb->page3.valve_error, "valve_error", ERROR_STATE)
                        ? tmp.page3.valve_error
                        : hb->page3.valve_error;
                hb->page3.eeprom_error =
                    update_param_s(tmp.page3.eeprom_error, hb->page3.eeprom_error, "eeprom_error", ERROR_STATE)
                        ? tmp.page3.eeprom_error
                        : hb->page3.eeprom_error;
                hb->page3.brush_adjust_error =
                    update_param_s(tmp.page3.brush_adjust_error, hb->page3.brush_adjust_error, "brush_adjust_error",
                                   ERROR_STATE)
                        ? tmp.page3.brush_adjust_error
                        : hb->page3.brush_adjust_error;
                hb->page3.battery_charge_flag =
                    update_param_s(tmp.page3.battery_charge_flag, hb->page3.battery_charge_flag, "battery_charge_flag",
                                   ERROR_STATE)
                        ? tmp.page3.battery_charge_flag
                        : hb->page3.battery_charge_flag;
                hb->page3.batery_lockout =
                    update_param_s(tmp.page3.batery_lockout, hb->page3.batery_lockout, "batery_lockout", ERROR_STATE)
                        ? tmp.page3.batery_lockout
                        : hb->page3.batery_lockout;

                hb->page3.desired_direction = update_param_s(tmp.page3.desired_direction, hb->page3.desired_direction,
                                                             "desired_direction", TRACTION_STATE)
                                                  ? tmp.page3.desired_direction
                                                  : hb->page3.desired_direction;
                hb->page3.actual_direction = update_param_s(tmp.page3.actual_direction, hb->page3.actual_direction,
                                                            "actual_direction", TRACTION_STATE)
                                                 ? tmp.page3.actual_direction
                                                 : hb->page3.actual_direction;
                hb->page3.bat_volt_81_V_1 = tmp.page3.bat_volt_81_V_1;
                hb->page3.bat_volt_81_V_2 = tmp.page3.bat_volt_81_V_2;
                static float last_bat_volt = (hb->page3.bat_volt_81_V_1 << 8 | hb->page3.bat_volt_81_V_2) / 81;
                static float curr_bat_volt;
                curr_bat_volt = (float)(hb->page3.bat_volt_81_V_1 << 8 | hb->page3.bat_volt_81_V_2) / 81;
                update_param_a(curr_bat_volt, last_bat_volt, "battery_voltage_81", BATTERY_VALUE);
                last_bat_volt = curr_bat_volt;
                break;
            }
            case 4:
            {
                memcpy(&tmp.page4, resp->data + 2, sizeof(tmp.page4));
                hb->page4.heatsink_volt_traction_81_V =
                    update_param_a((float)tmp.page4.heatsink_volt_traction_81_V * 16 / 81,
                                   (float)hb->page4.heatsink_volt_traction_81_V * 16 / 81,
                                   "heatsink_volt_traction_81_V", MISC_VALUE)
                        ? tmp.page4.heatsink_volt_traction_81_V
                        : hb->page4.heatsink_volt_traction_81_V;
                hb->page4.heatsink_volt_other_81_V = update_param_a((float)tmp.page4.heatsink_volt_other_81_V * 16 / 81,
                                                                    (float)hb->page4.heatsink_volt_other_81_V * 16 / 81,
                                                                    "heatsink_volt_other_81_V", MISC_VALUE)
                                                         ? tmp.page4.heatsink_volt_other_81_V
                                                         : hb->page4.heatsink_volt_other_81_V;
                hb->page4.tract_ddc =
                    update_param_a(tmp.page4.tract_ddc, hb->page4.tract_ddc, "tract_ddc", TRACTION_VALUE)
                        ? tmp.page4.tract_ddc
                        : hb->page4.tract_ddc;
                hb->page4.tract_adc =
                    update_param_a(tmp.page4.tract_adc, hb->page4.tract_adc, "tract_adc", TRACTION_VALUE)
                        ? tmp.page4.tract_adc
                        : hb->page4.tract_adc;
                hb->page4.mosfet_temperature =
                    update_param_a(tmp.page4.mosfet_temperature, hb->page4.mosfet_temperature, "mosfet_temperature",
                                   MISC_VALUE)
                        ? tmp.page4.mosfet_temperature
                        : hb->page4.mosfet_temperature;
                hb->page4.brush_current_A = update_param_a(tmp.page4.brush_current_A, hb->page4.brush_current_A,
                                                           "brush_current_A", SCRUBBER_VALUE)
                                                ? tmp.page4.brush_current_A
                                                : hb->page4.brush_current_A;
                break;
            }
            case 5:
            {
                memcpy(&tmp.page5, resp->data + 2, sizeof(tmp.page5));
                hb->page5.vacuum_current_A = update_param_a(tmp.page5.vacuum_current_A, hb->page5.vacuum_current_A,
                                                            "vacuum_current_A", RECOVERY_VALUE)
                                                 ? tmp.page5.vacuum_current_A
                                                 : hb->page5.vacuum_current_A;
                hb->page5.squeegee_current_A =
                    update_param_a(tmp.page5.squeegee_current_A, hb->page5.squeegee_current_A, "squeegee_current_A",
                                   RECOVERY_VALUE)
                        ? tmp.page5.squeegee_current_A
                        : hb->page5.squeegee_current_A;
                hb->page5.brush_deck_current_A =
                    update_param_a(tmp.page5.brush_deck_current_A, hb->page5.brush_deck_current_A,
                                   "brush_deck_current_A", SCRUBBER_VALUE)
                        ? tmp.page5.brush_deck_current_A
                        : hb->page5.brush_deck_current_A;
                hb->page5.desired_brush_deck_direction =
                    update_param_s(tmp.page5.desired_brush_deck_direction, hb->page5.desired_brush_deck_direction,
                                   "desired_brush_deck_direction", SCRUBBER_STATE)
                        ? tmp.page5.desired_brush_deck_direction
                        : hb->page5.desired_brush_deck_direction;
                hb->page5.desired_squeegee_direction =
                    update_param_s(tmp.page5.desired_squeegee_direction, hb->page5.desired_squeegee_direction,
                                   "desired_squeegee_direction", RECOVERY_STATE)
                        ? tmp.page5.desired_squeegee_direction
                        : hb->page5.desired_squeegee_direction;
                hb->page5.brush_auto_adjust_flag =
                    update_param_s(tmp.page5.brush_auto_adjust_flag, hb->page5.brush_auto_adjust_flag,
                                   "brush_auto_adjust_flag", SCRUBBER_STATE)
                        ? tmp.page5.brush_auto_adjust_flag
                        : hb->page5.brush_auto_adjust_flag;
                break;
            }
            case 6:
            {
                memcpy(&tmp.page6, resp->data + 2, sizeof(tmp.page6));
                hb->page6.traction_rev_current_dir =
                    update_param_s(tmp.page6.traction_rev_current_dir, hb->page6.traction_rev_current_dir,
                                   "traction_rev_current_dir", TRACTION_STATE)
                        ? tmp.page6.traction_rev_current_dir
                        : hb->page6.traction_rev_current_dir;
                hb->page6.traction_rev_current_A =
                    update_param_a(tmp.page6.traction_rev_current_A, hb->page6.traction_rev_current_A,
                                   "traction_rev_current_A", TRACTION_VALUE)
                        ? tmp.page6.traction_rev_current_A
                        : hb->page6.traction_rev_current_A;
                hb->page6.traction_left_null_current = update_param_a(__bswap_16(tmp.page6.traction_left_null_current),
                                                                      __bswap_16(hb->page6.traction_left_null_current),
                                                                      "traction_left_null_current", TRACTION_VALUE)
                                                           ? tmp.page6.traction_left_null_current
                                                           : hb->page6.traction_left_null_current;
                hb->page6.traction_fwd_current_dir =
                    update_param_s(tmp.page6.traction_fwd_current_dir, hb->page6.traction_fwd_current_dir,
                                   "traction_fwd_current_dir", TRACTION_STATE)
                        ? tmp.page6.traction_fwd_current_dir
                        : hb->page6.traction_fwd_current_dir;
                hb->page6.traction_fwd_current_A =
                    update_param_a(tmp.page6.traction_fwd_current_A, hb->page6.traction_fwd_current_A,
                                   "traction_fwd_current_A", TRACTION_VALUE)
                        ? tmp.page6.traction_fwd_current_A
                        : hb->page6.traction_fwd_current_A;
                break;
            }
            case 7:
            {
                memcpy(&tmp.page7, resp->data + 2, sizeof(tmp.page7));
                hb->page7.traction_right_null_current =
                    update_param_a(__bswap_16(tmp.page7.traction_right_null_current),
                                   __bswap_16(hb->page7.traction_right_null_current), "traction_right_null_current",
                                   TRACTION_VALUE)
                        ? tmp.page7.traction_right_null_current
                        : hb->page7.traction_right_null_current;
                hb->page7.MCU_temp_raw =
                    update_param_a((1.412 - ((float)__bswap_16(tmp.page7.MCU_temp_raw) * 3.3 / 4096)) / 0.0043 + 25,
                                   (1.412 - ((float)__bswap_16(tmp.page7.MCU_temp_raw) * 3.3 / 4096)) / 0.0043 + 25,
                                   "MCU_tmp_raw", MISC_VALUE)
                        ? tmp.page7.MCU_temp_raw
                        : hb->page7.MCU_temp_raw;
                hb->page7.vacuum_ddc =
                    update_param_a(tmp.page7.vacuum_ddc, hb->page7.vacuum_ddc, "vacuum_ddc", RECOVERY_VALUE)
                        ? tmp.page7.vacuum_ddc
                        : hb->page7.vacuum_ddc;
                hb->page7.accelerator_raw =
                    update_param_a((float)tmp.page7.accelerator_raw * 32 * 3.3 / 4096,
                                   (float)hb->page7.accelerator_raw * 32 * 3.3 / 4096, "accelerator_raw", MISC_VALUE)
                        ? tmp.page7.accelerator_raw
                        : hb->page7.accelerator_raw;
                break;
            }
            case 8:
            {
                memcpy(&tmp.page8, resp->data + 2, sizeof(tmp.page8));
                hb->page8.traction_left_drain_voltage =
                    update_param_a((float)tmp.page8.traction_left_drain_voltage * 16 / 81,
                                   (float)hb->page8.traction_left_drain_voltage * 16 / 81,
                                   "traction_left_drain_voltage", TRACTION_VALUE)
                        ? tmp.page8.traction_left_drain_voltage
                        : hb->page8.traction_left_drain_voltage;
                hb->page8.traction_right_drain_voltage =
                    update_param_a((float)tmp.page8.traction_right_drain_voltage * 16 / 81,
                                   (float)hb->page8.traction_right_drain_voltage * 16 / 81,
                                   "traction_right_drain_voltage", TRACTION_VALUE)
                        ? tmp.page8.traction_right_drain_voltage
                        : hb->page8.traction_right_drain_voltage;
                hb->page8.brush_drain_voltage_16_81_V =
                    update_param_a((float)tmp.page8.brush_drain_voltage_16_81_V * 16 / 81,
                                   (float)hb->page8.brush_drain_voltage_16_81_V * 16 / 81,
                                   "brush_drain_voltage_16_81_V", SCRUBBER_VALUE)
                        ? tmp.page8.brush_drain_voltage_16_81_V
                        : hb->page8.brush_drain_voltage_16_81_V;
                hb->page8.vacuum_drain_voltage_16_81_V =
                    update_param_a((float)tmp.page8.vacuum_drain_voltage_16_81_V * 16 / 81,
                                   (float)hb->page8.vacuum_drain_voltage_16_81_V * 16 / 81,
                                   "vacuum_drain_voltage_16_81_V", RECOVERY_VALUE)
                        ? tmp.page8.vacuum_drain_voltage_16_81_V
                        : hb->page8.vacuum_drain_voltage_16_81_V;
                hb->page8.brush_adc =
                    update_param_a(tmp.page8.brush_adc, hb->page8.brush_adc, "brush_adc", SCRUBBER_VALUE)
                        ? tmp.page8.brush_adc
                        : hb->page8.brush_adc;
                hb->page8.vacuum_adc =
                    update_param_a(tmp.page8.vacuum_adc, hb->page8.vacuum_adc, "vacuum_adc", RECOVERY_VALUE)
                        ? tmp.page8.vacuum_adc
                        : hb->page8.vacuum_adc;
                break;
            }
            case 9:
            {
                memcpy(&tmp.page9, resp->data + 2, sizeof(tmp.page9));
                hb->page9.customer_id =
                    update_param_s(tmp.page9.customer_id, hb->page9.customer_id, "customer_id", META_STATE)
                        ? tmp.page9.customer_id
                        : hb->page9.customer_id;
                hb->page9.firmware_major =
                    update_param_s(tmp.page9.firmware_major, hb->page9.firmware_major, "firmware_major", META_STATE)
                        ? tmp.page9.firmware_major
                        : hb->page9.firmware_major;
                hb->page9.firmware_minor =
                    update_param_s(tmp.page9.firmware_minor, hb->page9.firmware_minor, "firmware_minor", META_STATE)
                        ? tmp.page9.firmware_minor
                        : hb->page9.firmware_minor;
                hb->page9.bat_type = update_param_s(tmp.page9.bat_type, hb->page9.bat_type, "bat_type", META_STATE)
                                         ? tmp.page9.bat_type
                                         : hb->page9.bat_type;
                hb->page9.vacuum_off_delay_time =
                    update_param_s(tmp.page9.vacuum_off_delay_time, hb->page9.vacuum_off_delay_time,
                                   "vacuum_off_delay_time", META_STATE)
                        ? tmp.page9.vacuum_off_delay_time
                        : hb->page9.vacuum_off_delay_time;
                hb->page9.brush_ddc =
                    update_param_s(tmp.page9.brush_ddc, hb->page9.brush_ddc, "brush_ddc", SCRUBBER_VALUE)
                        ? tmp.page9.brush_ddc
                        : hb->page9.brush_ddc;
                break;
            }
            case 10:
            {
                memcpy(&tmp.page10, resp->data + 2, sizeof(tmp.page10));
                hb->page10.aux1_drain_voltage =
                    update_param_a((float)tmp.page10.aux1_drain_voltage * 16 / 81,
                                   (float)hb->page10.aux1_drain_voltage * 16 / 81, "aux1_drain_voltage", MISC_VALUE)
                        ? tmp.page10.aux1_drain_voltage
                        : hb->page10.aux1_drain_voltage;
                hb->page10.aux2_drain_voltage =
                    update_param_a((float)tmp.page10.aux2_drain_voltage * 16 / 81,
                                   (float)hb->page10.aux2_drain_voltage * 16 / 81, "aux2_drain_voltage", MISC_VALUE)
                        ? tmp.page10.aux2_drain_voltage
                        : hb->page10.aux2_drain_voltage;
                hb->page10.line_coil_voltage =
                    update_param_a((float)tmp.page10.line_coil_voltage * 16 / 81,
                                   (float)hb->page10.line_coil_voltage * 16 / 81, "line_coil_voltage", MISC_VALUE)
                        ? tmp.page10.line_coil_voltage
                        : hb->page10.line_coil_voltage;
                hb->page10.valve_drain_voltage = update_param_a((float)tmp.page10.valve_drain_voltage * 16 / 81,
                                                                (float)hb->page10.valve_drain_voltage * 16 / 81,
                                                                "valve_drain_voltage", SCRUBBER_VALUE)
                                                     ? tmp.page10.valve_drain_voltage
                                                     : hb->page10.valve_drain_voltage;
                hb->page10.brake_drain_voltage = update_param_a((float)tmp.page10.brake_drain_voltage * 16 / 81,
                                                                (float)hb->page10.brake_drain_voltage * 16 / 81,
                                                                "brake_drain_voltage", TRACTION_VALUE)
                                                     ? tmp.page10.brake_drain_voltage
                                                     : hb->page10.brake_drain_voltage;

                // input_flag16
                hb->page10.squeegee_init_flag =
                    update_param_s(tmp.page10.squeegee_init_flag, hb->page10.squeegee_init_flag, "squeegee_init_flag",
                                   RECOVERY_STATE)
                        ? tmp.page10.squeegee_init_flag
                        : hb->page10.squeegee_init_flag;
                hb->page10.brush_deck_init_flag =
                    update_param_s(tmp.page10.brush_deck_init_flag, hb->page10.brush_deck_init_flag,
                                   "brush_deck_init_flag", SCRUBBER_STATE)
                        ? tmp.page10.brush_deck_init_flag
                        : hb->page10.brush_deck_init_flag;
                hb->page10.turn_buf =
                    update_param_s(tmp.page10.turn_buf, hb->page10.turn_buf, "turn_buf", TRACTION_STATE)
                        ? tmp.page10.turn_buf
                        : hb->page10.turn_buf;
                hb->page10.battery_very_low = update_param_s(tmp.page10.battery_very_low, hb->page10.battery_very_low,
                                                             "battery_very_low", BATTERY_STATE)
                                                  ? tmp.page10.battery_very_low
                                                  : hb->page10.battery_very_low;
                hb->page10.accelerator_pressed =
                    update_param_s(tmp.page10.accelerator_pressed, hb->page10.accelerator_pressed,
                                   "accelerator_pressed", MISC_STATE)
                        ? tmp.page10.accelerator_pressed
                        : hb->page10.accelerator_pressed;
                hb->page10.brush_deck_down_protection =
                    update_param_s(tmp.page10.brush_deck_down_protection, hb->page10.brush_deck_down_protection,
                                   "brush_deck_down_protection", SCRUBBER_STATE)
                        ? tmp.page10.brush_deck_down_protection
                        : hb->page10.brush_deck_down_protection;
                hb->page10.aux4_buf = update_param_s(tmp.page10.aux4_buf, hb->page10.aux4_buf, "aux4_buf", MISC_STATE)
                                          ? tmp.page10.aux4_buf
                                          : hb->page10.aux4_buf;
                hb->page10.aux5_buf = update_param_s(tmp.page10.aux5_buf, hb->page10.aux5_buf, "aux5_buf", MISC_STATE)
                                          ? tmp.page10.aux5_buf
                                          : hb->page10.aux5_buf;
                break;
            }
        }
    }
}

bool LiveData::is_selected(ParamCategory type)
{
    switch(type)
    {
        case TRACTION_STATE:
        {
            return traction_state_selected == ACTIVE_FLAG;
        }
        case SCRUBBER_STATE:
        {
            return scrubber_state_selected == ACTIVE_FLAG;
        }
        case RECOVERY_STATE:
        {
            return recovery_state_selected == ACTIVE_FLAG;
        }
        case ERROR_STATE:
        {
            return error_state_selected == ACTIVE_FLAG;
        }
        case META_STATE:
        {
            return meta_state_selected == ACTIVE_FLAG;
        }
        case BATTERY_STATE:
        {
            return battery_state_selected == ACTIVE_FLAG;
        }
        case MISC_STATE:
        {
            return misc_state_selected == ACTIVE_FLAG;
        }
        case UNKNOWN_STATE:
        {
            return unknown_state_selected == ACTIVE_FLAG;
        }
        case TRACTION_VALUE:
        {
            return traction_value_selected == ACTIVE_FLAG;
        }
        case RECOVERY_VALUE:
        {
            return recovery_value_selected == ACTIVE_FLAG;
        }
        case SCRUBBER_VALUE:
        {
            return scrubber_value_selected == ACTIVE_FLAG;
        }
        case MISC_VALUE:
        {
            return misc_value_selected == ACTIVE_FLAG;
        }
        case BATTERY_VALUE:
        {
            return battery_value_selected == ACTIVE_FLAG;
        }
    }
}

bool LiveData::update_param_a(float new_value, float old_value, const string& log_name, ParamCategory type)
{
    static int traction_a_count = -1;
    static int scrubber_a_count = -1;
    static int recovery_a_count = -1;
    static int battery_a_count = -1;
    static int misc_a_count = -1;
    static int unknown_a_count = -1;

    end = std::chrono::steady_clock::now();
    std::stringstream stream;
    if (new_value != old_value && is_selected(type))
    {
        LOG_PRINT(("%s%s%sLAST 10 CHANGES%s", COORD0, UP, RED_TITLE, ATTRIB_OFF));
        static int last_size;
        stream << BOLD_ON << std::setw(30) << log_name << ATTRIB_OFF << std::setw(5) << std::to_string(old_value)
               << " -->" << std::setw(5) << std::to_string(new_value) << "   time: " << std::setw(15) << std::fixed
               << std::setprecision(10)
               << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
        last_size = stream.str().size();

        if (top_10.size() >= last_size * 10)
        {
            top_10 = stream.str() + top_10.substr(0, top_10.size() - last_size);
        }
        else
        {
            top_10 += stream.str();
        }
        for (int i = 0; i < top_10.size() / last_size; i++)
        {
            LOG_PRINT(("%s", COORD0));
            for (int j = 0; j < i; j++)
            {
                LOG_PRINT(("%s", DOWN));
            }
            LOG_PRINT(("%i. %s", i, top_10.substr(last_size * (i), last_size).c_str()));
        }
        LOG_PRINT(("%s", COORD0));
        for(int i = 0; i < 11; i++)
        {
            LOG_PRINT(("%s", DOWN));
        }
        //LOG_PRINT(("size: %i", last_size));
    }

    switch (type)
    {
        case TRACTION_VALUE:
        {
            static string POSITION;
            if(traction_value_selected & PRESENT_FLAG)
            {
                if(traction_a_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    traction_a_count++;
                    last_x = x;
                    last_y = y;
                    last_width = TRAC_V_WIDTH;
                }
                if (traction_a_count == TRAC_V_SIZE)
                {
                    traction_a_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sTRACTION VALUE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < traction_a_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%.1f", log_name.c_str(), new_value));
                traction_a_count++;
            }
            break;
        }
        case SCRUBBER_VALUE:
        {
            if(scrubber_value_selected & PRESENT_FLAG)
            {
                static string POSITION;
                if(scrubber_a_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    scrubber_a_count++;
                    last_x = x;
                    last_y = y;
                    last_width = SCRUB_V_WIDTH;
                }
                if (scrubber_a_count == SCRUB_V_SIZE)
                {
                    scrubber_a_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sSCRUBBER VALUE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < scrubber_a_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%.1f", log_name.c_str(), new_value));
                scrubber_a_count++;
            }
            break;
        }
        case RECOVERY_VALUE:
        {
            if(RECOVERY_VALUE & PRESENT_FLAG)
            {
                static string POSITION;
                if(recovery_a_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    recovery_a_count++;
                    last_x = x;
                    last_y = y;
                    last_width = RECOV_V_WIDTH;
                }
                if (recovery_a_count == RECOV_V_SIZE)
                {
                    recovery_a_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sRECOVERY VALUE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < recovery_a_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%.1f", log_name.c_str(), new_value));
                recovery_a_count++;
            }
            break;
        }
        case BATTERY_VALUE:
        {
            if(battery_value_selected & PRESENT_FLAG)
            {
                static string POSITION;
                if(battery_a_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    battery_a_count++;
                    last_x = x;
                    last_y = y;
                    last_width = BATT_V_WIDTH;
                }
                if (battery_a_count == BATT_V_SIZE)
                {
                    battery_a_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sBATTERY VALUE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < battery_a_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-20s:%.2f", log_name.c_str(), new_value));
                battery_a_count++;
            }
            break;
        }
        case MISC_VALUE:
        {
            if(misc_value_selected & PRESENT_FLAG)
            {
                static string POSITION;
                if(misc_a_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    misc_a_count++;
                    last_x = x;
                    last_y = y;
                    last_width = MISC_V_WIDTH;
                }
                if (misc_a_count == MISC_V_SIZE)
                {
                    misc_a_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sMISC VALUE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < misc_a_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%.1f", log_name.c_str(), new_value));
                misc_a_count++;
            }
            break;
        }
    }
    if (new_value != old_value)
    {
        return true;
    }
    return false;
}

bool LiveData::update_param_s(uint8_t new_value, uint8_t old_value, const string& log_name, ParamCategory type)
{
    static int traction_s_count = -1;
    static int scrubber_s_count = -1;
    static int recovery_s_count = -1;
    static int battery_s_count = -1;
    static int misc_s_count = -1;
    static int unknown_s_count = -1;
    static int error_s_count = 0;
    static int meta_s_count = -1;
    end = std::chrono::steady_clock::now();
    std::stringstream stream;
    if (new_value != old_value && is_selected(type))
    {
        LOG_PRINT(("%s%s%sLAST 10 CHANGES%s", COORD0, UP, RED_TITLE, ATTRIB_OFF));
        static int last_size;
        stream << BOLD_ON << std::setw(30) << log_name << ATTRIB_OFF << std::setw(5) << std::to_string(old_value)
               << " -->" << std::setw(5) << std::to_string(new_value) << "   time: " << std::setw(15) << std::fixed
               << std::setprecision(10)
               << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
        last_size = stream.str().size();

        if (top_10.size() >= last_size * 10)
        {
            top_10 = stream.str() + top_10.substr(0, top_10.size() - last_size);
        }
        else
        {
            top_10 += stream.str();
        }
        for (int i = 0; i < top_10.size() / last_size; i++)
        {
            LOG_PRINT(("%s", COORD0));
            for (int j = 0; j < i; j++)
            {
                LOG_PRINT(("%s", DOWN));
            }
            LOG_PRINT(("%i. %s", i, top_10.substr(last_size * (i), last_size).c_str()));
        }
        LOG_PRINT(("%s", COORD0));
        for(int i = 0; i < 11; i++)
        {
            LOG_PRINT(("%s", DOWN));
        }
        //LOG_PRINT(("size: %i", last_size));
    }

    static int last_size = stream.str().size();
    switch (type)
    {
        case TRACTION_STATE:
        {
            static string POSITION;
            if(traction_state_selected & PRESENT_FLAG)
            {
                if(traction_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ";" + std::to_string(x) + "f";
                    traction_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = TRAC_S_WIDTH;
                }
                if (traction_s_count == TRAC_S_SIZE)
                {
                    traction_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sTRACTION STATE%s%s%s", POSITION.c_str(), YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(), DOWN));
                for (int i = 0; i < traction_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                traction_s_count++;
            }
            break;
        }
        case SCRUBBER_STATE:
        {
            static string POSITION;
            if(scrubber_state_selected & PRESENT_FLAG)
            {
                if(scrubber_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    scrubber_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = SCRUB_S_WIDTH;
                }
                if (scrubber_s_count == SCRUB_S_SIZE)
                {
                    scrubber_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sSCRUBBER STATE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < scrubber_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                scrubber_s_count++;
            }
            break;
        }
        case RECOVERY_STATE:
        {
            static string POSITION;
            if(recovery_state_selected & PRESENT_FLAG)
            {
                if(recovery_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    recovery_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = RECOV_S_WIDTH;
                }
                if (recovery_s_count == RECOV_S_SIZE)
                {
                    recovery_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sRECOVERY STATE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < recovery_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                recovery_s_count++;
            }
            break;
        }
        case BATTERY_STATE:
        {
            static string POSITION;
            if(battery_state_selected & PRESENT_FLAG)
            {
                if(battery_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    battery_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = BATT_S_WIDTH;
                }
                if (battery_s_count == BATT_S_SIZE)
                {
                    battery_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sBATTERY STATE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < battery_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-20s:%i", log_name.c_str(), new_value));
                battery_s_count++;
            }
            break;
        }
        case MISC_STATE:
        {
            static string POSITION;
            if(misc_state_selected & PRESENT_FLAG)
            {
                if(misc_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    misc_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = MISC_S_WIDTH;
                }
                if (misc_s_count == MISC_S_SIZE)
                {
                    misc_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sMISC STATE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < misc_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                misc_s_count++;
            }
            break;
        }
        case UNKNOWN_STATE:
        {
            static string POSITION;
            if(unknown_state_selected & PRESENT_FLAG)
            {
                if(unknown_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    unknown_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = UNKNOWN_S_WIDTH;
                }
                if (unknown_s_count == UNKNOWN_S_SIZE)
                {
                    unknown_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sUNKNOWN STATE%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < unknown_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-14s:%i", log_name.c_str(), new_value));
                unknown_s_count++;
            }
            break;
        }
        case ERROR_STATE:
        {
            if(error_state_selected & PRESENT_FLAG)
            {
                if(error_s_count == ERROR_S1_size + ERROR_S2_size  + 1)
                {
                    error_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sERROR STATE%s%s%s", TLC_ERROR, RED_TITLE, ATTRIB_OFF, TLC_ERROR, DOWN));
                if (error_s_count == 0)
                {
                    LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                    error_s_count++;
                }
                else
                {
                    if (error_s_count <= ERROR_S1_size)
                    {
                        LOG_PRINT(("%s", TLC_ERROR_S1));
                        for (int i = 0; i < error_s_count; i++)
                        {
                            LOG_PRINT(("%s", DOWN));
                        }
                        LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                        error_s_count++;
                    }
                    else
                    {
                        LOG_PRINT(("%s", TLC_ERROR_S2));
                        for (int i = 0; i < error_s_count - 8; i++)
                        {
                            LOG_PRINT(("%s", DOWN));
                        }
                        LOG_PRINT(("%-28s:%i", log_name.c_str(), new_value));
                        error_s_count++;
                    }
                }
            }
            break;
        }
        case META_STATE:
        {
            static string POSITION;
            if(meta_state_selected & PRESENT_FLAG)
            {
                if(meta_s_count < 0)
                {
                    int x;
                    int y;
                    if(last_x >= RIGHT_EDGE)
                    {
                        x = 0;
                        y = BOTTOM_EDGE;
                    }
                    else
                    {
                        x = last_x + last_width + PADDING;
                        y = last_y;
                    }
                    POSITION = "\033[" + std::to_string(y) + ';' + std::to_string(x) + "f";
                    meta_s_count++;
                    last_x = x;
                    last_y = y;
                    last_width = META_S_WIDTH;
                }
                if (meta_s_count == META_S_SIZE)
                {
                    meta_s_count = 0;
                }
                // print the location according to the count and then the variable
                LOG_PRINT(("%s%sMETADATA%s%s%s", POSITION.c_str(),YELLOW_TITLE, ATTRIB_OFF, POSITION.c_str(),DOWN));
                for (int i = 0; i < meta_s_count; i++)
                {
                    LOG_PRINT(("%s", DOWN));
                }
                LOG_PRINT(("%-22s:%i", log_name.c_str(), new_value));
                meta_s_count++;
            }
            break;
        }
    }
    if (new_value != old_value)
    {
        return true;
    }
    return false;
}

KU::StatusCode LiveData::parse_ini(const string& file_path)
{
    fstream stu_file;
    stu_file.open(file_path, std::ios::in);
    stu_file.close();
    if (stu_file.fail())
    {
        stu_file.open(file_path, std::ios::out);
        stu_file << "[STATE]\ntraction_state = 1\nscrubber_state = 1\nrecovery_state = 1\nmisc_state = 1\nbattery_state = 1\nunknown_state = 0\n"
                 << "error_state = 1\nmeta_state = 1\n[ANALOG]\ntraction_value = 1\nscrubber_value = 1\nrecovery_value = 1\nmisc_value = 1\n"
                 << "battery_value = 1\n[LATEST_CHANGES]\ntraction_state = 1\nscrubber_state = 1\nrecovery_state = 1\nmisc_state = 1\n"
                 << "battery_state = 1\nunknown_state = 0\nerror_state = 1\nmeta_state = 0\ntraction_value = 0\nscrubber_value = 0\nrecovery_value = 0\n"
                 << "misc_value = 0\nbattery_value = 0"; 
        stu_file.close();  
    }
    stu_file.open(file_path, std::ios::in);
    string curr_line = "";
    getline(stu_file, curr_line);
    if(curr_line == "[STATE]")
    {
        while(curr_line != "[ANALOG]")
        {
            getline(stu_file, curr_line);
            if(curr_line.substr(0, sizeof("traction_state")-1) == "traction_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                traction_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("scrubber_state")-1) == "scrubber_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                scrubber_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("recovery_state")-1) == "recovery_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                recovery_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("misc_state")-1) == "misc_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                misc_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("battery_state")-1) == "battery_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                battery_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("unknown_state")-1) == "unknown_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                unknown_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("error_state")-1) == "error_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                error_state_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("meta_state")-1) == "meta_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                meta_state_selected |= PRESENT_FLAG;
            }
        }
    }
    if (curr_line == "[ANALOG]")
    {
        while(curr_line != "[LATEST_CHANGES]")
        {
            getline(stu_file, curr_line);
            if(curr_line.substr(0, sizeof("traction_value")-1) == "traction_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                traction_value_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("scrubber_value")-1) == "scrubber_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                scrubber_value_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("recovery_value")-1) == "recovery_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                recovery_value_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("misc_value")-1) == "misc_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                misc_value_selected |= PRESENT_FLAG;
            }
            else if(curr_line.substr(0, sizeof("battery_value")-1) == "battery_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                battery_value_selected |= PRESENT_FLAG;
            }
        }
    }
    if(curr_line == "[LATEST_CHANGES]")
    {
        while(getline(stu_file, curr_line))
        {
            if(curr_line.substr(0, sizeof("traction_state")-1) == "traction_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                traction_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("scrubber_state")-1) == "scrubber_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                scrubber_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("recovery_state")-1) == "recovery_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                recovery_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("misc_state")-1) == "misc_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                misc_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("battery_state")-1) == "battery_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                battery_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("unknown_state")-1) == "unknown_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                unknown_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("error_state")-1) == "error_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                error_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("meta_state")-1) == "meta_state" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                meta_state_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("traction_value")-1) == "traction_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                traction_value_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("scrubber_value")-1) == "scrubber_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                scrubber_value_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("recovery_value")-1) == "recovery_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                recovery_value_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("misc_value")-1) == "misc_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                misc_value_selected |= ACTIVE_FLAG;
            }
            else if(curr_line.substr(0, sizeof("battery_value")-1) == "battery_value" && curr_line.substr(curr_line.size()-1,1) == "1")
            {
                battery_value_selected |= ACTIVE_FLAG;
            }
        }
        return KU::NO_ERROR;
    }
}