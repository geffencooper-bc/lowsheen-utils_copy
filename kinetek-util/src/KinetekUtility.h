// This is the top level class that facilitates the Kinetek Utility
// by parsing command line arguments and running the according tool.

#ifndef KINETEK_UTILITY_H
#define KINETEK_UTILITY_H

#include "IAP.h"
#include "STUparam.h"
#include "KinetekUtilityCodes.h"

// This class is the interface and entry point into the kinetek tools.
// This class can be used in two ways:
// If the Kinetek Utility is being used by running the executable in the shell
// then parse_args can be called with the command line arguments.
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

    void run_iap();
    void run_stu();

    private:
    SocketCanHelper* sc;
    KU::CanDataList* ku_data; // holds all can data used by the utilities
    IAP* iap;
    STUparam* stu;

    bool can_initialized; // keeps track of whether init_can has been called
};

#endif