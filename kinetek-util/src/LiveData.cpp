//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions


// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//==================================================================

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

// escape sequences to adjust cursor location and text format
#define TLC "\033[0;0f"
#define SAVE "\033[s"
#define RESTORE "\033[u"
#define BACK "\033[1D"
#define DOWN "\033[1B"
#define UP "\033[1A"
#define BOLD_ON "\033[1m"
#define YELLOW_TITLE "\033[1;33m"
#define RED_TITLE "\033[1;31m"
#define ATTRIB_OFF "\033[0m"
#define CLEAR "\033[2J"
#define PADDING 3

// keeps track of which categories are selected
#define ABSENT_FLAG 0b00000000   // 00000000 --> don't show at all
#define PRESENT_FLAG 0b00001111  // 00001111 --> show but not in the changes list
#define ACTIVE_FLAG 0b11110000   // 11110000 --> show on changes list

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

    // create all the sections and place into vector
    sections.push_back(new DataSection("TRACTION_STATE", TRACTION_STATE, 1, -1, -1, -1, -1, 0, -1));
    sections.push_back(new DataSection("SCRUBBER_STATE", SCRUBBER_STATE, 1, -1, -1, -1, -1, 0, -1));
    sections.push_back(new DataSection("RECOVERY_STATE", RECOVERY_STATE, 1, -1, -1, -1, -1, 0, -1));
    sections.push_back(new DataSection("BATTERY_STATE", BATTERY_STATE, 1, -1, -1, -1, -1, 0, -1));
    //sections.push_back(new DataSection("METADATA", META_STATE, 1, -1, -1, -1, -1, 0, -1));
     sections.push_back(new DataSection("ERROR_STATE", ERROR_STATE, 1, -1, -1, -1, -1, 0, -1)); // comment this and it works?
    // sections.push_back(new DataSection("MISC_STATE", MISC_STATE, 1, -1, -1, -1, -1, 0, -1));
    // sections.push_back(new DataSection("UNKNOWN", UNKNOWN_STATE, 1, -1, -1, -1, -1, 0, -1));
    // sections.push_back(new DataSection("TRACTION_ANALOG", TRACTION_ANALOG, 0, -1, -1, -1, -1, 0, -1));
    // sections.push_back(new DataSection("SCRUBBER_ANALOG", SCRUBBER_ANALOG, 0, -1, -1, -1, -1, 0, -1));
    // sections.push_back(new DataSection("RECOVERY_ANALOG", RECOVERY_ANALOG, 0, -1, -1, -1, -1, 0, -1));
    // sections.push_back(new DataSection("MISC_ANALOG", MISC_ANALOG, 0, -1, -1, -1, -1, 0, -1));
    // sections.push_back(new DataSection("BATTERY_ANALOG", BATTERY_ANALOG, 0, -1, -1, -1, -1, 0, -1));
    // changes = new DataSection("LATEST_CHANGES", LATEST_CHANGES, 0, -1, -1, -1, -1, 0, -1);
}

// deallocate memory
LiveData::~LiveData()
{
    for (int i = 0; i < sections.size(); i++)
    {
        delete sections[i];
    }
    delete hb;
}

// callback function for received messages, not used as of now
void LiveData_resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg)
{
    // nothing needed
}

// move cursor back to bottom left corner on CTRL + C
void on_quit(int s)
{
    winsize tmp_win;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &tmp_win);
    string pos = "\033[" + std::to_string(tmp_win.ws_row) + ";0f";
    printf("%s", pos.c_str());
    exit(1);
}

// loop that updates the heartbeat struct every page
KU::StatusCode LiveData::update_heartbeat()
{
    // start the clock on launch
    begin = std::chrono::steady_clock::now();

    // check section configuration settings
    parse_ini("live_data_options.ini");
    signal(SIGINT, on_quit); // function called on CTRL + C

    // set this to cycle the loop until reach page 1
    bool get_first_page = true;

    // clear the screen
    printf("%s%s", CLEAR, "\033[0;0f");

    // store a tmp version to compare against to see if values changed
    controller_heartbeat tmp_hb;

    CO_CANrxMsg_t* resp;
    uint8_t page_num = 0;
    uint8_t last_page_num = 0;
    winsize tmp_win;

    // keep reading Kinetek heartbeat pages until user quits
    while (true)
    {
        // if the window size has changed, then reset the sections and screen
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &tmp_win);
        if (tmp_win.ws_col != window_size.ws_col || tmp_win.ws_row != window_size.ws_row)
        {
            printf("%s", CLEAR);
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size);
            for (int i = 0; i < sections.size(); i++)
            {
                sections[i]->param_index = -1;
                sections[i]->x_pos = -1;
                sections[i]->y_pos = -1;
                last_x = 0;
                last_y = 0;
                last_width = 0;
                last_height = 0;
                get_first_page = true;
                refresh = true;
            }
        }

        // get the next page
        resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 500);
        if (ku_data->get_response_type(resp->ident, resp->data, resp->DLC) != KU::HEART_BEAT)
        {
            LOG_PRINT(("No Heart Beat\n"));
            return KU::NO_HEART_BEAT;
        }

        last_page_num = page_num;
        page_num = resp->data[1];
        // wait until start at page 1
        if (get_first_page)
        {
            while (page_num != 1)
            {
                resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 500);
                page_num = resp->data[1];
            }
            get_first_page = false;
        }
        else
        {
            if (last_page_num + 1 == 11)
            {
                last_page_num = 0;
            }
            // if a page was missed, wait until the missed page is received again
            while (page_num != last_page_num + 1)
            {
                resp = sc->get_frame(KU::HEART_BEAT_ID, this, LiveData_resp_call_back, 500);
                page_num = resp->data[1];
            }
        }

        // update all the parameters according to the page
        switch (page_num)
        {
            case 1:
            {
                memcpy(&tmp_hb.page1, resp->data + 2, sizeof(tmp_hb.page1));
                hb->page1.vehicle_state =
                    update_param_s(tmp_hb.page1.vehicle_state, hb->page1.vehicle_state, "vehicle_state", TRACTION_STATE)
                        ? tmp_hb.page1.vehicle_state
                        : hb->page1.vehicle_state;
                hb->page1.error_value =
                    update_param_s(tmp_hb.page1.error_value, hb->page1.error_value, "error_value", ERROR_STATE)
                        ? tmp_hb.page1.error_value
                        : hb->page1.error_value;

                // input_flag0
                hb->page1.traction_running = update_param_s(tmp_hb.page1.traction_running, hb->page1.traction_running,
                                                            "traction_running", TRACTION_STATE)
                                                 ? tmp_hb.page1.traction_running
                                                 : hb->page1.traction_running;
                hb->page1.brush_running =
                    update_param_s(tmp_hb.page1.brush_running, hb->page1.brush_running, "brush_running", SCRUBBER_STATE)
                        ? tmp_hb.page1.brush_running
                        : hb->page1.brush_running;
                hb->page1.vacuum_running =
                    update_param_s(tmp_hb.page1.vacuum_running, hb->page1.vacuum_running, "vacuum_running", RECOVERY_STATE)
                        ? tmp_hb.page1.vacuum_running
                        : hb->page1.vacuum_running;
                hb->page1.accel_sro =
                    update_param_s(tmp_hb.page1.accel_sro, hb->page1.accel_sro, "accel_sro", TRACTION_STATE)
                        ? tmp_hb.page1.accel_sro
                        : hb->page1.accel_sro;
                hb->page1.brake_open =
                    update_param_s(tmp_hb.page1.brake_open, hb->page1.brake_open, "brake_open", TRACTION_STATE)
                        ? tmp_hb.page1.brake_open
                        : hb->page1.brake_open;
                hb->page1.clean_water_state = update_param_s(tmp_hb.page1.clean_water_state, hb->page1.clean_water_state,
                                                             "clean_water_state", SCRUBBER_STATE)
                                                  ? tmp_hb.page1.clean_water_state
                                                  : hb->page1.clean_water_state;
                hb->page1.dirty_water_full = update_param_s(tmp_hb.page1.dirty_water_full, hb->page1.dirty_water_full,
                                                            "dirty_water_full", SCRUBBER_STATE)
                                                 ? tmp_hb.page1.dirty_water_full
                                                 : hb->page1.dirty_water_full;
                hb->page1.not_used = update_param_s(tmp_hb.page1.not_used, hb->page1.not_used, "not_used", UNKNOWN_STATE)
                                         ? tmp_hb.page1.not_used
                                         : hb->page1.not_used;

                // input_flag1
                hb->page1.seat_buf = update_param_s(tmp_hb.page1.seat_buf, hb->page1.seat_buf, "seat_buf", MISC_STATE)
                                         ? tmp_hb.page1.seat_buf
                                         : hb->page1.seat_buf;
                hb->page1.ems_buf = update_param_s(tmp_hb.page1.ems_buf, hb->page1.ems_buf, "ems_buf", MISC_STATE)
                                        ? tmp_hb.page1.ems_buf
                                        : hb->page1.ems_buf;
                hb->page1.start_buf = update_param_s(tmp_hb.page1.start_buf, hb->page1.start_buf, "start_buf", MISC_STATE)
                                          ? tmp_hb.page1.start_buf
                                          : hb->page1.start_buf;
                hb->page1.aux1_buf = update_param_s(tmp_hb.page1.aux1_buf, hb->page1.aux1_buf, "aux1_buf", MISC_STATE)
                                         ? tmp_hb.page1.aux1_buf
                                         : hb->page1.aux1_buf;
                hb->page1.aux2_buf = update_param_s(tmp_hb.page1.aux2_buf, hb->page1.aux2_buf, "aux2_buf", MISC_STATE)
                                         ? tmp_hb.page1.aux2_buf
                                         : hb->page1.aux2_buf;
                hb->page1.aux3_buf = update_param_s(tmp_hb.page1.aux3_buf, hb->page1.aux3_buf, "aux3_buf", MISC_STATE)
                                         ? tmp_hb.page1.aux3_buf
                                         : hb->page1.aux3_buf;
                hb->page1.brake_enable_buf = update_param_s(tmp_hb.page1.brake_enable_buf, hb->page1.brake_enable_buf,
                                                            "brake_enable_buf", TRACTION_STATE)
                                                 ? tmp_hb.page1.brake_enable_buf
                                                 : hb->page1.brake_enable_buf;
                hb->page1.step_brake_enable_buf =
                    update_param_s(tmp_hb.page1.step_brake_enable_buf, hb->page1.step_brake_enable_buf,
                                   "step_brake_enable_buf", TRACTION_STATE)
                        ? tmp_hb.page1.step_brake_enable_buf
                        : hb->page1.step_brake_enable_buf;

                // input_flag2
                hb->page1.traction_open =
                    update_param_s(tmp_hb.page1.traction_open, hb->page1.traction_open, "traction_open", TRACTION_STATE)
                        ? tmp_hb.page1.traction_open
                        : hb->page1.traction_open;
                hb->page1.brush_open =
                    update_param_s(tmp_hb.page1.brush_open, hb->page1.brush_open, "brush_open", SCRUBBER_STATE)
                        ? tmp_hb.page1.brush_open
                        : hb->page1.brush_open;
                hb->page1.vacuum_open =
                    update_param_s(tmp_hb.page1.vacuum_open, hb->page1.vacuum_open, "vacuum_open", RECOVERY_STATE)
                        ? tmp_hb.page1.vacuum_open
                        : hb->page1.vacuum_open;
                hb->page1.alarm_on = update_param_s(tmp_hb.page1.alarm_on, hb->page1.alarm_on, "alarm_on", MISC_STATE)
                                         ? tmp_hb.page1.alarm_on
                                         : hb->page1.alarm_on;
                hb->page1.valve_on = update_param_s(tmp_hb.page1.valve_on, hb->page1.valve_on, "valve_on", SCRUBBER_STATE)
                                         ? tmp_hb.page1.valve_on
                                         : hb->page1.valve_on;
                hb->page1.aux1_on2 = update_param_s(tmp_hb.page1.aux1_on2, hb->page1.aux1_on2, "aux1_on2", MISC_STATE)
                                         ? tmp_hb.page1.aux1_on2
                                         : hb->page1.aux1_on2;
                hb->page1.aux2_on2 = update_param_s(tmp_hb.page1.aux2_on2, hb->page1.aux2_on2, "aux2_on2", MISC_STATE)
                                         ? tmp_hb.page1.aux2_on2
                                         : hb->page1.aux2_on2;
                hb->page1.brake_on = update_param_s(tmp_hb.page1.brake_on, hb->page1.brake_on, "brake_on", TRACTION_STATE)
                                         ? tmp_hb.page1.brake_on
                                         : hb->page1.brake_on;

                // input_flag3
                hb->page1.brush_on = update_param_s(tmp_hb.page1.brush_on, hb->page1.brush_on, "brush_on", SCRUBBER_STATE)
                                         ? tmp_hb.page1.brush_on
                                         : hb->page1.brush_on;
                hb->page1.vacuum_on =
                    update_param_s(tmp_hb.page1.vacuum_on, hb->page1.vacuum_on, "vacuum_on", RECOVERY_STATE)
                        ? tmp_hb.page1.vacuum_on
                        : hb->page1.vacuum_on;
                hb->page1.aux1_on3 = update_param_s(tmp_hb.page1.aux1_on3, hb->page1.aux1_on3, "aux1_on3", MISC_STATE)
                                         ? tmp_hb.page1.aux1_on3
                                         : hb->page1.aux1_on3;
                hb->page1.aux2_on3 = update_param_s(tmp_hb.page1.aux2_on3, hb->page1.aux2_on3, "aux2_on3", MISC_STATE)
                                         ? tmp_hb.page1.aux2_on3
                                         : hb->page1.aux2_on3;
                hb->page1.squeegee_up_limit = update_param_s(tmp_hb.page1.squeegee_up_limit, hb->page1.squeegee_up_limit,
                                                             "squeegee_up_limit", RECOVERY_STATE)
                                                  ? tmp_hb.page1.squeegee_up_limit
                                                  : hb->page1.squeegee_up_limit;
                hb->page1.squeegee_down_limit =
                    update_param_s(tmp_hb.page1.squeegee_down_limit, hb->page1.squeegee_down_limit, "squeegee_down_limit",
                                   RECOVERY_STATE)
                        ? tmp_hb.page1.squeegee_down_limit
                        : hb->page1.squeegee_down_limit;
                hb->page1.brush_deck_up_limit =
                    update_param_s(tmp_hb.page1.brush_deck_up_limit, hb->page1.brush_deck_up_limit, "brush_deck_up_limit",
                                   SCRUBBER_STATE)
                        ? tmp_hb.page1.brush_deck_up_limit
                        : hb->page1.brush_deck_up_limit;
                hb->page1.brush_deck_down_limit =
                    update_param_s(tmp_hb.page1.brush_deck_down_limit, hb->page1.brush_deck_down_limit,
                                   "brush_deck_down_limit", SCRUBBER_STATE)
                        ? tmp_hb.page1.brush_deck_down_limit
                        : hb->page1.brush_deck_down_limit;
                break;
            }
            case 2:
            {
                memcpy(&tmp_hb.page2, resp->data + 2, sizeof(tmp_hb.page2));
                // input_flag4
                hb->page2.clean_water_buf = update_param_s(tmp_hb.page2.clean_water_buf, hb->page2.clean_water_buf,
                                                           "clean_water_buf", SCRUBBER_STATE)
                                                ? tmp_hb.page2.clean_water_buf
                                                : hb->page2.clean_water_buf;
                hb->page2.dirty_water_buf = update_param_s(tmp_hb.page2.dirty_water_buf, hb->page2.dirty_water_buf,
                                                           "dirty_water_buf", SCRUBBER_STATE)
                                                ? tmp_hb.page2.dirty_water_buf
                                                : hb->page2.dirty_water_buf;
                hb->page2.valve_on = update_param_s(tmp_hb.page2.valve_on, hb->page2.valve_on, "valve_on", SCRUBBER_STATE)
                                         ? tmp_hb.page2.valve_on
                                         : hb->page2.valve_on;
                hb->page2.alarm_on = update_param_s(tmp_hb.page2.alarm_on, hb->page2.alarm_on, "alarm_on", MISC_STATE)
                                         ? tmp_hb.page2.alarm_on
                                         : hb->page2.alarm_on;
                hb->page2.forward_buf =
                    update_param_s(tmp_hb.page2.forward_buf, hb->page2.forward_buf, "forward_buf", TRACTION_STATE)
                        ? tmp_hb.page2.forward_buf
                        : hb->page2.forward_buf;
                hb->page2.reverse_buf =
                    update_param_s(tmp_hb.page2.reverse_buf, hb->page2.reverse_buf, "reverse_buf", TRACTION_STATE)
                        ? tmp_hb.page2.reverse_buf
                        : hb->page2.reverse_buf;
                hb->page2.battery_low =
                    update_param_s(tmp_hb.page2.battery_low, hb->page2.battery_low, "battery_low", BATTERY_STATE)
                        ? tmp_hb.page2.battery_low
                        : hb->page2.battery_low;
                hb->page2.lcd_watch_flag =
                    update_param_s(tmp_hb.page2.lcd_watch_flag, hb->page2.lcd_watch_flag, "lcd_watch_flag", BATTERY_STATE)
                        ? tmp_hb.page2.lcd_watch_flag
                        : hb->page2.lcd_watch_flag;

                // input_flag5-6
                hb->page2.brush_unload_on = update_param_s(tmp_hb.page2.brush_unload_on, hb->page2.brush_unload_on,
                                                           "brush_unload_on", SCRUBBER_STATE)
                                                ? tmp_hb.page2.brush_unload_on
                                                : hb->page2.brush_unload_on;
                hb->page2.brush_unload_complete =
                    update_param_s(tmp_hb.page2.brush_unload_complete, hb->page2.brush_unload_complete,
                                   "brush_unload_complete", SCRUBBER_STATE)
                        ? tmp_hb.page2.brush_unload_complete
                        : hb->page2.brush_unload_complete;
                hb->page2.brush_load_on =
                    update_param_s(tmp_hb.page2.brush_load_on, hb->page2.brush_load_on, "brush_load_on", SCRUBBER_STATE)
                        ? tmp_hb.page2.brush_load_on
                        : hb->page2.brush_load_on;
                hb->page2.brush_load_complete =
                    update_param_s(tmp_hb.page2.brush_load_complete, hb->page2.brush_load_complete, "brush_load_complete",
                                   SCRUBBER_STATE)
                        ? tmp_hb.page2.brush_load_complete
                        : hb->page2.brush_load_complete;
                hb->page2.p3_f1 = update_param_s(tmp_hb.page2.p3_f1, hb->page2.p3_f1, "p3_f1", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p3_f1
                                      : hb->page2.p3_f1;
                hb->page2.p3_f2 = update_param_s(tmp_hb.page2.p3_f2, hb->page2.p3_f2, "p3_f2", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p3_f2
                                      : hb->page2.p3_f2;
                hb->page2.p3_f3 = update_param_s(tmp_hb.page2.p3_f3, hb->page2.p3_f3, "p3_f3", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p3_f3
                                      : hb->page2.p3_f3;
                hb->page2.p3_f4 = update_param_s(tmp_hb.page2.p3_f4, hb->page2.p3_f4, "p3_f4", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p3_f4
                                      : hb->page2.p3_f4;

                // input_flag7-9
                hb->page2.p3_f5 = update_param_s(tmp_hb.page2.p3_f5, hb->page2.p3_f5, "p3_f5", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p3_f5
                                      : hb->page2.p3_f5;
                hb->page2.p3_f11 = update_param_s(tmp_hb.page2.p3_f11, hb->page2.p3_f11, "p3_f11", UNKNOWN_STATE)
                                       ? tmp_hb.page2.p3_f11
                                       : hb->page2.p3_f11;
                hb->page2.p3_f12 = update_param_s(tmp_hb.page2.p3_f12, hb->page2.p3_f12, "p3_f12", UNKNOWN_STATE)
                                       ? tmp_hb.page2.p3_f12
                                       : hb->page2.p3_f12;
                hb->page2.p3_f13 = update_param_s(tmp_hb.page2.p3_f13, hb->page2.p3_f13, "p3_f13", UNKNOWN_STATE)
                                       ? tmp_hb.page2.p3_f13
                                       : hb->page2.p3_f13;
                hb->page2.p2_f6 = update_param_s(tmp_hb.page2.p2_f6, hb->page2.p2_f6, "p3_f1", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p2_f6
                                      : hb->page2.p2_f6;
                hb->page2.p2_f7 = update_param_s(tmp_hb.page2.p2_f7, hb->page2.p2_f7, "p2_f7", UNKNOWN_STATE)
                                      ? tmp_hb.page2.p2_f7
                                      : hb->page2.p2_f7;
                hb->page2.p2_f13 = update_param_s(tmp_hb.page2.p2_f13, hb->page2.p2_f13, "p2_f13", UNKNOWN_STATE)
                                       ? tmp_hb.page2.p2_f13
                                       : hb->page2.p2_f13;
                hb->page2.p2_f14 = update_param_s(tmp_hb.page2.p2_f14, hb->page2.p2_f14, "p2_f14", UNKNOWN_STATE)
                                       ? tmp_hb.page2.p2_f14
                                       : hb->page2.p2_f14;
                break;
            }
            case 3:
            {
                memcpy(&tmp_hb.page3, resp->data + 2, sizeof(tmp_hb.page3));
                // error_flag
                hb->page3.traction_error =
                    update_param_s(tmp_hb.page3.traction_error, hb->page3.traction_error, "traction_error", ERROR_STATE)
                        ? tmp_hb.page3.traction_error
                        : hb->page3.traction_error;
                hb->page3.brush_error =
                    update_param_s(tmp_hb.page3.brush_error, hb->page3.brush_error, "brush_error", ERROR_STATE)
                        ? tmp_hb.page3.brush_error
                        : hb->page3.brush_error;
                hb->page3.vacuum_error =
                    update_param_s(tmp_hb.page3.vacuum_error, hb->page3.vacuum_error, "vacuum_error", ERROR_STATE)
                        ? tmp_hb.page3.vacuum_error
                        : hb->page3.vacuum_error;
                hb->page3.brush_deck_error = update_param_s(tmp_hb.page3.brush_deck_error, hb->page3.brush_deck_error,
                                                            "brush_deck_error", ERROR_STATE)
                                                 ? tmp_hb.page3.brush_deck_error
                                                 : hb->page3.brush_deck_error;
                hb->page3.squeegee_error =
                    update_param_s(tmp_hb.page3.squeegee_error, hb->page3.squeegee_error, "squeegee_error", ERROR_STATE)
                        ? tmp_hb.page3.squeegee_error
                        : hb->page3.squeegee_error;
                hb->page3.brake_error =
                    update_param_s(tmp_hb.page3.brake_error, hb->page3.brake_error, "brake_error", ERROR_STATE)
                        ? tmp_hb.page3.brake_error
                        : hb->page3.brake_error;
                hb->page3.no_brush_error =
                    update_param_s(tmp_hb.page3.no_brush_error, hb->page3.no_brush_error, "no_brush_error", ERROR_STATE)
                        ? tmp_hb.page3.no_brush_error
                        : hb->page3.no_brush_error;
                hb->page3.motor_state =
                    update_param_s(tmp_hb.page3.motor_state, hb->page3.motor_state, "motor_state", ERROR_STATE)
                        ? tmp_hb.page3.motor_state
                        : hb->page3.motor_state;

                // error_flag1
                hb->page3.aux1_error =
                    update_param_s(tmp_hb.page3.aux1_error, hb->page3.aux1_error, "aux1_error", ERROR_STATE)
                        ? tmp_hb.page3.aux1_error
                        : hb->page3.aux1_error;
                hb->page3.aux2_error =
                    update_param_s(tmp_hb.page3.aux2_error, hb->page3.aux2_error, "aux2_error", ERROR_STATE)
                        ? tmp_hb.page3.aux2_error
                        : hb->page3.aux2_error;
                hb->page3.alarm_error =
                    update_param_s(tmp_hb.page3.alarm_error, hb->page3.alarm_error, "alarm_error", ERROR_STATE)
                        ? tmp_hb.page3.alarm_error
                        : hb->page3.alarm_error;
                hb->page3.valve_error =
                    update_param_s(tmp_hb.page3.valve_error, hb->page3.valve_error, "valve_error", ERROR_STATE)
                        ? tmp_hb.page3.valve_error
                        : hb->page3.valve_error;
                hb->page3.eeprom_error =
                    update_param_s(tmp_hb.page3.eeprom_error, hb->page3.eeprom_error, "eeprom_error", ERROR_STATE)
                        ? tmp_hb.page3.eeprom_error
                        : hb->page3.eeprom_error;
                hb->page3.brush_adjust_error =
                    update_param_s(tmp_hb.page3.brush_adjust_error, hb->page3.brush_adjust_error, "brush_adjust_error",
                                   ERROR_STATE)
                        ? tmp_hb.page3.brush_adjust_error
                        : hb->page3.brush_adjust_error;
                hb->page3.battery_charge_flag =
                    update_param_s(tmp_hb.page3.battery_charge_flag, hb->page3.battery_charge_flag, "battery_charge_flag",
                                   BATTERY_STATE)
                        ? tmp_hb.page3.battery_charge_flag
                        : hb->page3.battery_charge_flag;
                hb->page3.batery_lockout =
                    update_param_s(tmp_hb.page3.batery_lockout, hb->page3.batery_lockout, "batery_lockout", BATTERY_STATE)
                        ? tmp_hb.page3.batery_lockout
                        : hb->page3.batery_lockout;

                hb->page3.desired_direction = update_param_s(tmp_hb.page3.desired_direction, hb->page3.desired_direction,
                                                             "desired_direction", TRACTION_STATE)
                                                  ? tmp_hb.page3.desired_direction
                                                  : hb->page3.desired_direction;
                hb->page3.actual_direction = update_param_s(tmp_hb.page3.actual_direction, hb->page3.actual_direction,
                                                            "actual_direction", TRACTION_STATE)
                                                 ? tmp_hb.page3.actual_direction
                                                 : hb->page3.actual_direction;
                hb->page3.bat_volt_81_V_1 = tmp_hb.page3.bat_volt_81_V_1;
                hb->page3.bat_volt_81_V_2 = tmp_hb.page3.bat_volt_81_V_2;
                static float last_bat_volt = (hb->page3.bat_volt_81_V_1 << 8 | hb->page3.bat_volt_81_V_2) / 81;
                static float curr_bat_volt;
                curr_bat_volt = (float)(hb->page3.bat_volt_81_V_1 << 8 | hb->page3.bat_volt_81_V_2) / 81;
                update_param_a(curr_bat_volt, last_bat_volt, "battery_voltage_81", BATTERY_ANALOG);
                last_bat_volt = curr_bat_volt;
                break;
            }
            case 4:
            {
                memcpy(&tmp_hb.page4, resp->data + 2, sizeof(tmp_hb.page4));
                hb->page4.heatsink_volt_traction_81_V =
                    update_param_a((float)tmp_hb.page4.heatsink_volt_traction_81_V * 16 / 81,
                                   (float)hb->page4.heatsink_volt_traction_81_V * 16 / 81,
                                   "heatsink_volt_traction_81_V", MISC_ANALOG)
                        ? tmp_hb.page4.heatsink_volt_traction_81_V
                        : hb->page4.heatsink_volt_traction_81_V;
                hb->page4.heatsink_volt_other_81_V = update_param_a((float)tmp_hb.page4.heatsink_volt_other_81_V * 16 / 81,
                                                                    (float)hb->page4.heatsink_volt_other_81_V * 16 / 81,
                                                                    "heatsink_volt_other_81_V", MISC_ANALOG)
                                                         ? tmp_hb.page4.heatsink_volt_other_81_V
                                                         : hb->page4.heatsink_volt_other_81_V;
                hb->page4.tract_ddc =
                    update_param_a(tmp_hb.page4.tract_ddc, hb->page4.tract_ddc, "tract_ddc", TRACTION_ANALOG)
                        ? tmp_hb.page4.tract_ddc
                        : hb->page4.tract_ddc;
                hb->page4.tract_adc =
                    update_param_a(tmp_hb.page4.tract_adc, hb->page4.tract_adc, "tract_adc", TRACTION_ANALOG)
                        ? tmp_hb.page4.tract_adc
                        : hb->page4.tract_adc;
                hb->page4.mosfet_temperature =
                    update_param_a(tmp_hb.page4.mosfet_temperature, hb->page4.mosfet_temperature, "mosfet_temperature",
                                   MISC_ANALOG)
                        ? tmp_hb.page4.mosfet_temperature
                        : hb->page4.mosfet_temperature;
                hb->page4.brush_current_A = update_param_a(tmp_hb.page4.brush_current_A, hb->page4.brush_current_A,
                                                           "brush_current_A", SCRUBBER_ANALOG)
                                                ? tmp_hb.page4.brush_current_A
                                                : hb->page4.brush_current_A;
                break;
            }
            case 5:
            {
                memcpy(&tmp_hb.page5, resp->data + 2, sizeof(tmp_hb.page5));
                hb->page5.vacuum_current_A = update_param_a(tmp_hb.page5.vacuum_current_A, hb->page5.vacuum_current_A,
                                                            "vacuum_current_A", RECOVERY_ANALOG)
                                                 ? tmp_hb.page5.vacuum_current_A
                                                 : hb->page5.vacuum_current_A;
                hb->page5.squeegee_current_A =
                    update_param_a(tmp_hb.page5.squeegee_current_A, hb->page5.squeegee_current_A, "squeegee_current_A",
                                   RECOVERY_ANALOG)
                        ? tmp_hb.page5.squeegee_current_A
                        : hb->page5.squeegee_current_A;
                hb->page5.brush_deck_current_A =
                    update_param_a(tmp_hb.page5.brush_deck_current_A, hb->page5.brush_deck_current_A,
                                   "brush_deck_current_A", SCRUBBER_ANALOG)
                        ? tmp_hb.page5.brush_deck_current_A
                        : hb->page5.brush_deck_current_A;
                hb->page5.desired_brush_deck_direction =
                    update_param_s(tmp_hb.page5.desired_brush_deck_direction, hb->page5.desired_brush_deck_direction,
                                   "desired_brush_deck_direction", SCRUBBER_STATE)
                        ? tmp_hb.page5.desired_brush_deck_direction
                        : hb->page5.desired_brush_deck_direction;
                hb->page5.desired_squeegee_direction =
                    update_param_s(tmp_hb.page5.desired_squeegee_direction, hb->page5.desired_squeegee_direction,
                                   "desired_squeegee_direction", RECOVERY_STATE)
                        ? tmp_hb.page5.desired_squeegee_direction
                        : hb->page5.desired_squeegee_direction;
                hb->page5.brush_auto_adjust_flag =
                    update_param_s(tmp_hb.page5.brush_auto_adjust_flag, hb->page5.brush_auto_adjust_flag,
                                   "brush_auto_adjust_flag", SCRUBBER_STATE)
                        ? tmp_hb.page5.brush_auto_adjust_flag
                        : hb->page5.brush_auto_adjust_flag;
                break;
            }
            case 6:
            {
                memcpy(&tmp_hb.page6, resp->data + 2, sizeof(tmp_hb.page6));
                hb->page6.traction_rev_current_dir =
                    update_param_s(tmp_hb.page6.traction_rev_current_dir, hb->page6.traction_rev_current_dir,
                                   "traction_rev_current_dir", TRACTION_STATE)
                        ? tmp_hb.page6.traction_rev_current_dir
                        : hb->page6.traction_rev_current_dir;
                hb->page6.traction_rev_current_A =
                    update_param_a(tmp_hb.page6.traction_rev_current_A, hb->page6.traction_rev_current_A,
                                   "traction_rev_current_A", TRACTION_ANALOG)
                        ? tmp_hb.page6.traction_rev_current_A
                        : hb->page6.traction_rev_current_A;
                hb->page6.traction_left_null_current = update_param_a(__bswap_16(tmp_hb.page6.traction_left_null_current),
                                                                      __bswap_16(hb->page6.traction_left_null_current),
                                                                      "traction_left_null_current", TRACTION_ANALOG)
                                                           ? tmp_hb.page6.traction_left_null_current
                                                           : hb->page6.traction_left_null_current;
                hb->page6.traction_fwd_current_dir =
                    update_param_s(tmp_hb.page6.traction_fwd_current_dir, hb->page6.traction_fwd_current_dir,
                                   "traction_fwd_current_dir", TRACTION_STATE)
                        ? tmp_hb.page6.traction_fwd_current_dir
                        : hb->page6.traction_fwd_current_dir;
                hb->page6.traction_fwd_current_A =
                    update_param_a(tmp_hb.page6.traction_fwd_current_A, hb->page6.traction_fwd_current_A,
                                   "traction_fwd_current_A", TRACTION_ANALOG)
                        ? tmp_hb.page6.traction_fwd_current_A
                        : hb->page6.traction_fwd_current_A;
                break;
            }
            case 7:
            {
                memcpy(&tmp_hb.page7, resp->data + 2, sizeof(tmp_hb.page7));
                hb->page7.traction_right_null_current =
                    update_param_a(__bswap_16(tmp_hb.page7.traction_right_null_current),
                                   __bswap_16(hb->page7.traction_right_null_current), "traction_right_null_current",
                                   TRACTION_ANALOG)
                        ? tmp_hb.page7.traction_right_null_current
                        : hb->page7.traction_right_null_current;
                hb->page7.MCU_temp_raw =
                    update_param_a((1.412 - ((float)__bswap_16(tmp_hb.page7.MCU_temp_raw) * 3.3 / 4096)) / 0.0043 + 25,
                                   (1.412 - ((float)__bswap_16(tmp_hb.page7.MCU_temp_raw) * 3.3 / 4096)) / 0.0043 + 25,
                                   "MCU_tmp_raw", MISC_ANALOG)
                        ? tmp_hb.page7.MCU_temp_raw
                        : hb->page7.MCU_temp_raw;
                hb->page7.vacuum_ddc =
                    update_param_a(tmp_hb.page7.vacuum_ddc, hb->page7.vacuum_ddc, "vacuum_ddc", RECOVERY_ANALOG)
                        ? tmp_hb.page7.vacuum_ddc
                        : hb->page7.vacuum_ddc;
                hb->page7.accelerator_raw =
                    update_param_a((float)tmp_hb.page7.accelerator_raw * 32 * 3.3 / 4096,
                                   (float)hb->page7.accelerator_raw * 32 * 3.3 / 4096, "accelerator_raw", MISC_ANALOG)
                        ? tmp_hb.page7.accelerator_raw
                        : hb->page7.accelerator_raw;
                break;
            }
            case 8:
            {
                memcpy(&tmp_hb.page8, resp->data + 2, sizeof(tmp_hb.page8));
                hb->page8.traction_left_drain_voltage =
                    update_param_a((float)tmp_hb.page8.traction_left_drain_voltage * 16 / 81,
                                   (float)hb->page8.traction_left_drain_voltage * 16 / 81,
                                   "traction_left_drain_voltage", TRACTION_ANALOG)
                        ? tmp_hb.page8.traction_left_drain_voltage
                        : hb->page8.traction_left_drain_voltage;
                hb->page8.traction_right_drain_voltage =
                    update_param_a((float)tmp_hb.page8.traction_right_drain_voltage * 16 / 81,
                                   (float)hb->page8.traction_right_drain_voltage * 16 / 81,
                                   "traction_right_drain_voltage", TRACTION_ANALOG)
                        ? tmp_hb.page8.traction_right_drain_voltage
                        : hb->page8.traction_right_drain_voltage;
                hb->page8.brush_drain_voltage_16_81_V =
                    update_param_a((float)tmp_hb.page8.brush_drain_voltage_16_81_V * 16 / 81,
                                   (float)hb->page8.brush_drain_voltage_16_81_V * 16 / 81,
                                   "brush_drain_voltage_16_81_V", SCRUBBER_ANALOG)
                        ? tmp_hb.page8.brush_drain_voltage_16_81_V
                        : hb->page8.brush_drain_voltage_16_81_V;
                hb->page8.vacuum_drain_voltage_16_81_V =
                    update_param_a((float)tmp_hb.page8.vacuum_drain_voltage_16_81_V * 16 / 81,
                                   (float)hb->page8.vacuum_drain_voltage_16_81_V * 16 / 81,
                                   "vacuum_drain_voltage_16_81_V", RECOVERY_ANALOG)
                        ? tmp_hb.page8.vacuum_drain_voltage_16_81_V
                        : hb->page8.vacuum_drain_voltage_16_81_V;
                hb->page8.brush_adc =
                    update_param_a(tmp_hb.page8.brush_adc, hb->page8.brush_adc, "brush_adc", SCRUBBER_ANALOG)
                        ? tmp_hb.page8.brush_adc
                        : hb->page8.brush_adc;
                hb->page8.vacuum_adc =
                    update_param_a(tmp_hb.page8.vacuum_adc, hb->page8.vacuum_adc, "vacuum_adc", RECOVERY_ANALOG)
                        ? tmp_hb.page8.vacuum_adc
                        : hb->page8.vacuum_adc;
                break;
            }
            case 9:
            {
                memcpy(&tmp_hb.page9, resp->data + 2, sizeof(tmp_hb.page9));
                hb->page9.customer_id =
                    update_param_s(tmp_hb.page9.customer_id, hb->page9.customer_id, "customer_id", META_STATE)
                        ? tmp_hb.page9.customer_id
                        : hb->page9.customer_id;
                hb->page9.firmware_major =
                    update_param_s(tmp_hb.page9.firmware_major, hb->page9.firmware_major, "firmware_major", META_STATE)
                        ? tmp_hb.page9.firmware_major
                        : hb->page9.firmware_major;
                hb->page9.firmware_minor =
                    update_param_s(tmp_hb.page9.firmware_minor, hb->page9.firmware_minor, "firmware_minor", META_STATE)
                        ? tmp_hb.page9.firmware_minor
                        : hb->page9.firmware_minor;
                hb->page9.bat_type = update_param_s(tmp_hb.page9.bat_type, hb->page9.bat_type, "bat_type", META_STATE)
                                         ? tmp_hb.page9.bat_type
                                         : hb->page9.bat_type;
                hb->page9.vacuum_off_delay_time =
                    update_param_s(tmp_hb.page9.vacuum_off_delay_time, hb->page9.vacuum_off_delay_time,
                                   "vacuum_off_delay_time", META_STATE)
                        ? tmp_hb.page9.vacuum_off_delay_time
                        : hb->page9.vacuum_off_delay_time;
                hb->page9.brush_ddc =
                    update_param_s(tmp_hb.page9.brush_ddc, hb->page9.brush_ddc, "brush_ddc", SCRUBBER_ANALOG)
                        ? tmp_hb.page9.brush_ddc
                        : hb->page9.brush_ddc;
                break;
            }
            case 10:
            {
                memcpy(&tmp_hb.page10, resp->data + 2, sizeof(tmp_hb.page10));
                hb->page10.aux1_drain_voltage =
                    update_param_a((float)tmp_hb.page10.aux1_drain_voltage * 16 / 81,
                                   (float)hb->page10.aux1_drain_voltage * 16 / 81, "aux1_drain_voltage", MISC_ANALOG)
                        ? tmp_hb.page10.aux1_drain_voltage
                        : hb->page10.aux1_drain_voltage;
                hb->page10.aux2_drain_voltage =
                    update_param_a((float)tmp_hb.page10.aux2_drain_voltage * 16 / 81,
                                   (float)hb->page10.aux2_drain_voltage * 16 / 81, "aux2_drain_voltage", MISC_ANALOG)
                        ? tmp_hb.page10.aux2_drain_voltage
                        : hb->page10.aux2_drain_voltage;
                hb->page10.line_coil_voltage =
                    update_param_a((float)tmp_hb.page10.line_coil_voltage * 16 / 81,
                                   (float)hb->page10.line_coil_voltage * 16 / 81, "line_coil_voltage", MISC_ANALOG)
                        ? tmp_hb.page10.line_coil_voltage
                        : hb->page10.line_coil_voltage;
                hb->page10.valve_drain_voltage = update_param_a((float)tmp_hb.page10.valve_drain_voltage * 16 / 81,
                                                                (float)hb->page10.valve_drain_voltage * 16 / 81,
                                                                "valve_drain_voltage", SCRUBBER_ANALOG)
                                                     ? tmp_hb.page10.valve_drain_voltage
                                                     : hb->page10.valve_drain_voltage;
                hb->page10.brake_drain_voltage = update_param_a((float)tmp_hb.page10.brake_drain_voltage * 16 / 81,
                                                                (float)hb->page10.brake_drain_voltage * 16 / 81,
                                                                "brake_drain_voltage", TRACTION_ANALOG)
                                                     ? tmp_hb.page10.brake_drain_voltage
                                                     : hb->page10.brake_drain_voltage;

                // input_flag16
                hb->page10.squeegee_init_flag =
                    update_param_s(tmp_hb.page10.squeegee_init_flag, hb->page10.squeegee_init_flag, "squeegee_init_flag",
                                   RECOVERY_STATE)
                        ? tmp_hb.page10.squeegee_init_flag
                        : hb->page10.squeegee_init_flag;
                hb->page10.brush_deck_init_flag =
                    update_param_s(tmp_hb.page10.brush_deck_init_flag, hb->page10.brush_deck_init_flag,
                                   "brush_deck_init_flag", SCRUBBER_STATE)
                        ? tmp_hb.page10.brush_deck_init_flag
                        : hb->page10.brush_deck_init_flag;
                hb->page10.turn_buf =
                    update_param_s(tmp_hb.page10.turn_buf, hb->page10.turn_buf, "turn_buf", TRACTION_STATE)
                        ? tmp_hb.page10.turn_buf
                        : hb->page10.turn_buf;
                hb->page10.battery_very_low = update_param_s(tmp_hb.page10.battery_very_low, hb->page10.battery_very_low,
                                                             "battery_very_low", BATTERY_STATE)
                                                  ? tmp_hb.page10.battery_very_low
                                                  : hb->page10.battery_very_low;
                hb->page10.accelerator_pressed =
                    update_param_s(tmp_hb.page10.accelerator_pressed, hb->page10.accelerator_pressed,
                                   "accelerator_pressed", MISC_STATE)
                        ? tmp_hb.page10.accelerator_pressed
                        : hb->page10.accelerator_pressed;
                hb->page10.brush_deck_down_protection =
                    update_param_s(tmp_hb.page10.brush_deck_down_protection, hb->page10.brush_deck_down_protection,
                                   "brush_deck_down_protection", SCRUBBER_STATE)
                        ? tmp_hb.page10.brush_deck_down_protection
                        : hb->page10.brush_deck_down_protection;
                hb->page10.aux4_buf = update_param_s(tmp_hb.page10.aux4_buf, hb->page10.aux4_buf, "aux4_buf", MISC_STATE)
                                          ? tmp_hb.page10.aux4_buf
                                          : hb->page10.aux4_buf;
                hb->page10.aux5_buf = update_param_s(tmp_hb.page10.aux5_buf, hb->page10.aux5_buf, "aux5_buf", MISC_STATE)
                                          ? tmp_hb.page10.aux5_buf
                                          : hb->page10.aux5_buf;
                finished_loading = true;
                break;
            }
        }
    }
}

// finds the desired section in the vector
LiveData::DataSection* LiveData::get_section(ParamCategory type)
{
    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->category == type)
        {
            return sections[i];
        }
    }
}

// updates the last 10 changes section
void LiveData::update_changes(DataSection* section, std::stringstream& latest_change, bool just_refresh)
{
    // the changes are posted in the bottom left corner
    changes->num_params = 10;
    string pos = "\033[" + std::to_string(window_size.ws_row - changes->num_params) + ";0f";

    end = std::chrono::steady_clock::now();
    LOG_PRINT(("%s%s%sLAST 10 CHANGES%s", pos.c_str(), UP, RED_TITLE, ATTRIB_OFF));

    // configure the section parameters
    changes->width = latest_change.str().size();
    changes->x_pos = 0;
    changes->y_pos = window_size.ws_row - changes->num_params;
    // LOG_PRINT(("\033[60;60f %i", changes->width));

    // if the value hasn't changed but the window size has, reprint the section
    if (just_refresh)
    {
        for (int i = 0; i < top_10.size() / changes->width; i++)
        {
            LOG_PRINT(("%s", pos.c_str()));
            for (int j = 0; j < i; j++)
            {
                LOG_PRINT(("%s", DOWN));
            }
            LOG_PRINT(("%i. %s", i, top_10.substr(changes->width * (i), changes->width).c_str()));
        }
        refresh = false;
    }
    // when a new value changes edit the changes list
    else
    {
        // if there are 10 changes in the list, grab top 9 and insert lastest change
        if (top_10.size() >= changes->width * 10)
        {
            top_10 = latest_change.str() + top_10.substr(0, top_10.size() - changes->width);
        }
        // otherwise just insert at the front
        else 
        {
            string temp = top_10;
            top_10 = latest_change.str();
            top_10 += temp;
        }
        changes->params.clear();
        changes->params.push_back(top_10);

        // print out the changes
        for (int i = 0; i < top_10.size() / changes->width; i++)
        {
            LOG_PRINT(("%s", pos.c_str()));
            for (int j = 0; j < i; j++)
            {
                LOG_PRINT(("%s", DOWN));
            }
            LOG_PRINT(("%i. %s", i, top_10.substr(changes->width * (i), changes->width).c_str()));
        }
    }
}

// update section position and parameters, returns true if position has changed
bool LiveData::update_section(DataSection* section, const string& log_name, int param_size)
{
    // store copies of last height and width in case changes are invalid
    int tmp_last_height = last_height;
    int tmp_last_width = last_width;

    // if not finished loading sections for the first time, add current parameter to the section
    if (!finished_loading)
    {
        section->params.push_back(log_name);

        // find the section width and height
        for (int i = 0; i < section->params.size(); i++)
        {
            int new_width = section->params[i].size();
            int curr_width = section->width;
            if (new_width > curr_width)
            {
                section->width = section->params[i].size();
            }
        }
        section->num_params = section->params.size();
        return false;
    }
    // update the sections to display when need to refresh or it is the first time publishing
    if ((section->param_index < 0) && finished_loading && (section->selected_option & PRESENT_FLAG))
    {
        if (section->param_index < 0)
        {
            section->param_index = 0;
        }
        int cols_left = window_size.ws_col - (last_x + last_width);
        int rows_left = window_size.ws_row - (last_y + last_height);
        // if the section can fit in the remaining width and height space then display it
        if (cols_left > section->width + PADDING && window_size.ws_row - last_y > section->num_params)
        {
            // check if first section to display
            if (last_width == 0)
            {
                section->x_pos = 0;
                section->y_pos = 0;
            }
            else
            {
                // set position to next available column
                section->x_pos = last_x + last_width + PADDING;
                section->y_pos = last_y;
            }
        }
        // if the section can fit in the remaining height of the next section row
        else if (rows_left > section->num_params + PADDING)
        {
            section->x_pos = 0;
            section->y_pos = last_y + last_height + PADDING;

            // store the last height and width for this new row
            last_height = section->num_params;
            last_width = section->width;
        }
        // if there is no room, don't display these sections
        else
        {
            section->x_pos = -1;
            section->y_pos = -1;
        }
        // if placed section within bounds of changes section, move the section or don't display it
        if (section->x_pos < (changes->x_pos + changes->width) &&
            (section->x_pos + section->width) > changes->x_pos &&
            section->y_pos < (changes->y_pos + changes->num_params) &&
            (section->y_pos + section->num_params + PADDING) > changes->y_pos)
        {
            if (window_size.ws_col - (changes->x_pos + changes->width) > section->width + PADDING)
            {
                section->x_pos = changes->x_pos + changes->width;
            }
            else
            {
                section->x_pos = -1;
                section->y_pos = -1;
                // revert back to original values
                last_height = tmp_last_height;
                last_width = tmp_last_width;
            }
        }
        // update last section values if displayed the section
        if (section->x_pos != -1)
        {
            last_x = section->x_pos;
            last_y = section->y_pos;
            last_width = section->width + 1 + param_size; // 1 accounts for the colon
            last_height = (section->num_params > last_height) ? section->num_params : last_height;
        }
        return true;
    }
    return true;
}

// update an analog parameter value
bool LiveData::update_param_a(float new_value, float old_value, const string& log_name, ParamCategory type)
{
    // get the according section
    DataSection* section = get_section(type);

    // display changes if parameter is active
    std::stringstream stream;
    stream << BOLD_ON << std::setw(30) << log_name << ATTRIB_OFF << std::setw(10) << old_value << " -->"
               << std::setw(10) << new_value << "   time: " << std::setw(10) << std::fixed << std::setprecision(5)
               << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
    
    // update changes sections if window size changed or parameter value changed
    if (refresh)
    {
        update_changes(section, stream, true);
    }      
    else if(new_value != old_value && (section->selected_option & ACTIVE_FLAG) == ACTIVE_FLAG)
    {
        update_changes(section, stream, false);
    }

    bool republish = update_section(section, log_name, std::to_string(new_value).size());

    // display the header if on display
    if (section->x_pos >= 0 && section->y_pos >= 0 && republish)
    {
        if (section->param_index == section->num_params)
        {
            section->param_index = 0;
        }
        string section_position = "\033[" + std::to_string(section->y_pos) + ';' + std::to_string(section->x_pos) + "f";
        LOG_PRINT(("%s%s%s%s%s%s", section_position.c_str(), YELLOW_TITLE, section->name.c_str(), ATTRIB_OFF,
                   section_position.c_str(), DOWN));
        for (int i = 0; i < section->param_index; i++)
        {
            LOG_PRINT(("%s", DOWN));
        }
        // pad the section params based on the width
        LOG_PRINT(("%-*s:%0.1f", section->width, section->params[section->param_index].c_str(), new_value));

        section->param_index++;
    }
    if (new_value != old_value)
    {
        return true;
    }
    return false;
}

// update a state parameter value
bool LiveData::update_param_s(uint8_t new_value, uint8_t old_value, const string& log_name, ParamCategory type)
{
    // get the according section
    DataSection* section = get_section(type);

    std::stringstream stream;
    stream << BOLD_ON << std::setw(30) << log_name << ATTRIB_OFF << std::setw(10) << std::to_string(old_value)
               << " -->" << std::setw(10) << std::to_string(new_value) << "   time: " << std::setw(10) << std::fixed
               << std::setprecision(5) << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
    // update changes sections if window size changed or parameter value changed
    if (refresh)
    {
        update_changes(section, stream, true);
    }      
    else if(new_value != old_value && (section->selected_option & ACTIVE_FLAG) == ACTIVE_FLAG)
    {
        update_changes(section, stream, false);
    }

    bool republish = update_section(section, log_name, std::to_string(new_value).size());

    // display the header if on display
    if (section->x_pos >= 0 && section->y_pos >= 0 && republish)
    {
        if (section->param_index == section->num_params)
        {
            section->param_index = 0;
        }
        string section_position = "\033[" + std::to_string(section->y_pos) + ';' + std::to_string(section->x_pos) + "f";
        LOG_PRINT(("%s%s%s%s%s%s", section_position.c_str(), YELLOW_TITLE, section->name.c_str(), ATTRIB_OFF,
                   section_position.c_str(), DOWN));
        for (int i = 0; i < section->param_index; i++)
        {
            LOG_PRINT(("%s", DOWN));
        }
        // pad the section params based on the width
        LOG_PRINT(("%-*s:%i", section->width, section->params[section->param_index].c_str(), new_value));

        section->param_index++;
    }
    if (new_value != old_value)
    {
        return true;
    }
    return false;
}

KU::StatusCode LiveData::parse_ini(const string& file_path)
{
#define DISPLAY_OFF "=0"
#define DISPLAY_ON "=1"
#define STATE_SECTION "[STATE]"
#define ANALOG_SECTION "[ANALOG]"
#define CHANGES_SECTION "[CHANGES]"

    // first try to read the file, if DNE create it and output the following default options
    fstream config_file;
    config_file.open(file_path, std::ios::in);
    if (config_file.fail())
    {
        config_file.open(file_path, std::ios::out);

        // turn on error state, meta data, and traction state by default
        config_file << STATE_SECTION << "\n";
        for (int i = 0; i < sections.size(); i++)
        {
            if (sections[i]->is_state && (sections[i]->category != ERROR_STATE && sections[i]->category != META_STATE &&
                                          sections[i]->category != TRACTION_STATE))
            {
                config_file << sections[i]->name << DISPLAY_OFF << "\n";
            }
            else if (sections[i]->is_state &&
                     (sections[i]->category == ERROR_STATE || sections[i]->category == META_STATE ||
                      sections[i]->category == TRACTION_STATE))
            {
                config_file << sections[i]->name << DISPLAY_ON << "\n";
            }
        }
        // no analog values by default
        config_file << ANALOG_SECTION << "\n";
        for (int i = 0; i < sections.size(); i++)
        {
            if (!sections[i]->is_state)
            {
                config_file << sections[i]->name << DISPLAY_OFF << "\n";
            }
        }
        // only error values and traction state show up in last changes by default
        config_file << CHANGES_SECTION << "\n";
        for (int i = 0; i < sections.size(); i++)
        {
            if (sections[i]->category != ERROR_STATE && sections[i]->category != TRACTION_STATE)
            {
                config_file << sections[i]->name << DISPLAY_OFF << "\n";
            }
            else
            {
                config_file << sections[i]->name << DISPLAY_ON << "\n";
            }
        }
        config_file.close();
    }

    // parse the ini file and initialize the options accordingly
    if (!config_file.is_open())
    {
        config_file.open(file_path, std::ios::in);
    }
    string curr_line = "";
    string parameter = "";
    string status = "";
    hu_getline(config_file, curr_line);
    if (curr_line == STATE_SECTION)
    {
        // for items in state section, determine if on/off
        while (curr_line != ANALOG_SECTION)
        {
            hu_getline(config_file, curr_line);
            parameter = curr_line.substr(0, curr_line.find('='));
            for (int i = 0; i < sections.size(); i++)
            {
                if (sections[i]->name == parameter)
                {
                    if (curr_line.substr(curr_line.find('=') + 1, 1) == "1")
                    {
                        sections[i]->selected_option |= PRESENT_FLAG;
                    }
                    break;
                }
            }
        }
    }
    if (curr_line == ANALOG_SECTION)
    {
        // for items in analog section, determine if on/off
        while (curr_line != CHANGES_SECTION)
        {
            hu_getline(config_file, curr_line);
            parameter = curr_line.substr(0, curr_line.find('='));
            for (int i = 0; i < sections.size(); i++)
            {
                if (sections[i]->name == parameter)
                {
                    if (curr_line.substr(curr_line.find('=') + 1, 1) == "1")
                    {
                        sections[i]->selected_option |= PRESENT_FLAG;
                    }
                    break;
                }
            }
        }
    }
    if (curr_line == CHANGES_SECTION)
    {
        // for items in changes section, determine if on/off
        while (hu_getline(config_file, curr_line))
        {
            parameter = curr_line.substr(0, curr_line.find('='));
            for (int i = 0; i < sections.size(); i++)
            {
                if (sections[i]->name == parameter)
                {
                    if (curr_line.substr(curr_line.find('=') + 1, 1) == "1")
                    {
                        sections[i]->selected_option |= ACTIVE_FLAG;
                    }
                    break;
                }
            }
        }
    }
    return KU::NO_ERROR;
}