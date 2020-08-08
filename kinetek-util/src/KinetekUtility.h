// This is the top level class that facilitates the Kinetek Utility
// by parsing command line arguments and running the according tool.

#ifndef KINETEK_UTILITY_H
#define KINETEK_UTILITY_H

#include "IAP.h"
#include "STUparam.h"
#include "KinetekUtilityCodes.h"

// This class is the interface and entry point into the kinetek tools
// and can be used in two ways:

// 1. If the Kinetek Utility is being used by running the executable in the shell
// then parse_args can be called with the command line arguments. parse_args will
// then call the according functions to run the desried tool.

// 2. If the Kinetek Utility is being used as a "library", then init_can and the 
// the desired function under "tools" can be called directly.
class KinetekUtility
{
    public:
    // init objects
    KinetekUtility();

    ~KinetekUtility();

    // parses the command line input and runs the correct function
    void parse_args(int argc, char** argv);

    // sets up socket can helper object and connects to can interface, channel name is usually "can0"
    KU::StatusCode init_can(const char* channel_name);

    // translates a status code into a human readable string
    string translate_status_code(KU::StatusCode status);

    // tools
    KU::StatusCode run_iap(const string& file_path, bool iap_mode);
    KU::StatusCode read_stu_to_file(const string& file_path);
    KU::StatusCode write_stu_from_file(const string& file_path);
    KU::StatusCode read_stu_param(uint8_t param_num);
    KU::StatusCode write_stu_param(uint8_t param_num);

    private:
    SocketCanHelper* sc;
    KU::CanDataList* ku_data; // holds all can data used by the utilities
    IAP* iap;
    STUparam* stu;

    bool can_initialized; // keeps track of whether init_can has been called
};

#endif