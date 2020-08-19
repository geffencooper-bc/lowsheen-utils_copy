// This is the live data tool that is used to see if any values
// have changed from the last heartbeat

#ifndef LIVE_DATA_H
#define LIVE_DATA_H

#include "SocketCanHelper.h"
#include "KinetekUtilityCodes.h"
#include <string>
#include <chrono>
#include <vector>
#include <fstream>
#include "HexUtility.h"
#include <sys/ioctl.h>

using std::fstream;
using std::string;

// this class facilitates the live data tool
class LiveData
{
   public:
    // the Kinetek data parameters are divided into these categories
    enum ParamCategory
    {
        ERROR_STATE = 0,
        TRACTION_STATE,
        SCRUBBER_STATE,
        RECOVERY_STATE,
        META_STATE,
        MISC_STATE,
        UNKNOWN_STATE,
        BATTERY_STATE,
        TRACTION_ANALOG,
        SCRUBBER_ANALOG,
        RECOVERY_ANALOG,
        BATTERY_ANALOG,
        MISC_ANALOG
        //LATEST_CHANGES
    };

    // stores configuration information about each data section
    struct DataSection
    {
        DataSection(string name, ParamCategory category, uint8_t is_state, int num_params, int width, int x_pos, int y_pos, uint8_t selected_option, int param_index) :
                    name(name), category(category), is_state(is_state), num_params(num_params), width(width), x_pos(x_pos), y_pos(y_pos), selected_option(selected_option), param_index(param_index) {} 
        string name;
        ParamCategory category;
        uint8_t is_state; // state = 0, analog = 1
        int num_params; // height
        int width; // max param length
        int x_pos;
        int y_pos;
        uint8_t selected_option;
        int param_index;
        std::vector<string> params;
    };

    // This struct was taken from kinetek_kcca0237.cpp except the pages are not a union
    // because all the page data needs to be known from the last heart beat, not just the current page
    struct __attribute__((__packed__)) controller_heartbeat
    {
        struct __attribute__((__packed__))
        {
            uint8_t vehicle_state;
            uint8_t error_value;

            // input_flag0
            uint8_t traction_running : 1;
            uint8_t brush_running : 1;
            uint8_t vacuum_running : 1;
            uint8_t accel_sro : 1;
            uint8_t brake_open : 1;
            uint8_t clean_water_state : 1;
            uint8_t dirty_water_full : 1;
            uint8_t not_used : 1;

            // input_flag1
            uint8_t seat_buf : 1;
            uint8_t ems_buf : 1;
            uint8_t start_buf : 1;
            uint8_t aux1_buf : 1;
            uint8_t aux2_buf : 1;
            uint8_t aux3_buf : 1;
            uint8_t brake_enable_buf : 1;
            uint8_t step_brake_enable_buf : 1;

            // input_flag2
            uint8_t traction_open : 1;
            uint8_t brush_open : 1;
            uint8_t vacuum_open : 1;
            uint8_t alarm_on : 1;
            uint8_t valve_on : 1;
            uint8_t aux1_on2 : 1;
            uint8_t aux2_on2 : 1;
            uint8_t brake_on : 1;

            // input_flag3
            uint8_t brush_on : 1;
            uint8_t vacuum_on : 1;
            uint8_t aux1_on3 : 1;
            uint8_t aux2_on3 : 1;
            uint8_t squeegee_up_limit : 1;
            uint8_t squeegee_down_limit : 1;
            uint8_t brush_deck_up_limit : 1;
            uint8_t brush_deck_down_limit : 1;
        } page1;
        struct __attribute__((__packed__))
        {
            // input_flag4
            uint8_t clean_water_buf : 1;
            uint8_t dirty_water_buf : 1;
            uint8_t valve_on : 1;
            uint8_t alarm_on : 1;
            uint8_t forward_buf : 1;
            uint8_t reverse_buf : 1;
            uint8_t battery_low : 1;
            uint8_t lcd_watch_flag : 1;

            // input_flag5-6
            uint8_t brush_unload_on : 1;
            uint8_t brush_unload_complete : 1;
            uint8_t brush_load_on : 1;
            uint8_t brush_load_complete : 1;
            uint16_t p3_f1 : 3;
            uint16_t p3_f2 : 3;
            uint16_t p3_f3 : 3;
            uint16_t p3_f4 : 3;

            // input_flag7-9
            uint16_t p3_f5 : 3;
            uint16_t p3_f11 : 3;
            uint16_t p3_f12 : 3;
            uint16_t p3_f13 : 3;
            uint16_t p2_f6 : 3;
            uint16_t p2_f7 : 3;
            uint16_t p2_f13 : 3;
            uint16_t p2_f14 : 3;
        } page2;
        struct __attribute__((__packed__))
        {
            // error_flag
            uint8_t traction_error : 1;
            uint8_t brush_error : 1;
            uint8_t vacuum_error : 1;
            uint8_t brush_deck_error : 1;
            uint8_t squeegee_error : 1;
            uint8_t brake_error : 1;
            uint8_t no_brush_error : 1;
            uint8_t motor_state : 1;

            // error_flag1
            uint8_t aux1_error : 1;
            uint8_t aux2_error : 1;
            uint8_t alarm_error : 1;
            uint8_t valve_error : 1;
            uint8_t eeprom_error : 1;
            uint8_t brush_adjust_error : 1;
            uint8_t battery_charge_flag : 1;
            uint8_t batery_lockout : 1;

            uint8_t desired_direction;
            uint8_t actual_direction;
            uint8_t bat_volt_81_V_1;
            uint8_t bat_volt_81_V_2;
        } page3;
        struct __attribute__((__packed__))
        {
            uint8_t heatsink_volt_traction_81_V;
            uint8_t heatsink_volt_other_81_V;
            uint8_t tract_ddc;
            uint8_t tract_adc;
            uint8_t mosfet_temperature;
            uint8_t brush_current_A;
        } page4;
        struct __attribute__((__packed__))
        {
            uint8_t vacuum_current_A;
            uint8_t squeegee_current_A;
            uint8_t brush_deck_current_A;
            uint8_t desired_brush_deck_direction;
            uint8_t desired_squeegee_direction;
            uint8_t brush_auto_adjust_flag;
        } page5;
        struct __attribute__((__packed__))
        {
            uint8_t traction_rev_current_dir;
            uint8_t traction_rev_current_A;
            uint16_t traction_left_null_current;
            uint8_t traction_fwd_current_dir;
            uint8_t traction_fwd_current_A;
        } page6;
        struct __attribute__((__packed__))
        {
            uint16_t traction_right_null_current;
            uint16_t MCU_temp_raw;  // 1.412-temp*3.3/4096)/0.0043+25 in C
            uint8_t vacuum_ddc;
            uint8_t accelerator_raw;
        } page7;
        struct __attribute__((__packed__))
        {
            uint8_t traction_left_drain_voltage;
            uint8_t traction_right_drain_voltage;
            uint8_t brush_drain_voltage_16_81_V;
            uint8_t vacuum_drain_voltage_16_81_V;
            uint8_t brush_adc;
            uint8_t vacuum_adc;
        } page8;
        struct __attribute__((__packed__))
        {
            uint8_t customer_id;
            uint8_t firmware_major;
            uint8_t firmware_minor;
            uint8_t bat_type;
            uint8_t vacuum_off_delay_time;
            uint8_t brush_ddc;
        } page9;
        struct __attribute__((__packed__))
        {
            uint8_t aux1_drain_voltage;
            uint8_t aux2_drain_voltage;
            uint8_t line_coil_voltage;
            uint8_t valve_drain_voltage;
            uint8_t brake_drain_voltage;

            // input_flag16
            uint8_t squeegee_init_flag : 1;
            uint8_t brush_deck_init_flag : 1;
            uint8_t turn_buf : 1;
            uint8_t battery_very_low : 1;
            uint8_t accelerator_pressed : 1;
            uint8_t brush_deck_down_protection : 1;
            uint8_t aux4_buf : 1;
            uint8_t aux5_buf : 1;
        } page10;
    };

    // initializes objects
    LiveData(SocketCanHelper* sc, KU::CanDataList* ku_data);

    // deallocates memory
    ~LiveData();

    // parses INI file for live data options
    KU::StatusCode parse_ini(const string& file_path);

    // updates the heart beat struct every page
    KU::StatusCode update_heartbeat();

   private:
    controller_heartbeat* hb;
    SocketCanHelper* sc;
    KU::CanDataList* ku_data;

    string top_10;
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    
    // callback function for received messages, not used as of now
    friend void STU_resp_call_back(void* obj, const CO_CANrxMsg_t* can_msg);

    // check if a parameter has changed since the last heartbeat
    bool update_param_a(float new_value, float old_value, const string& log_name, ParamCategory type);
    bool update_param_s(uint8_t new_value, uint8_t old_value, const string& log_name, ParamCategory type);

    // keeps track of which categories are selected
    #define ABSENT_FLAG 0b00000000 // 00000000 --> don't show at all
    #define PRESENT_FLAG 0b00001111 // 00001111 --> show but not in the changes list
    #define ACTIVE_FLAG 0b11111111 // 11111111 --> show on screen and on changes list
    std::vector<DataSection*> sections;

    // the position, width, and height of the last section loaded
    int last_x = 0;
    int last_y = 0;
    int last_width = 0;
    int last_height = 0;

    bool finished_loading = false;
    bool refresh = true;

    winsize window_size;
};

#endif