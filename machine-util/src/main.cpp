
#include "lowsheen_interface.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <argp.h>
#include "version.h"
#include "manifest.h"

const char *argp_program_version = "programname programversion";
const char *argp_program_bug_address = "<your@email.address>";
static char doc[] = "MANIFEST_FILE MACHINE_NAME_OR_ID";
static char args_doc[] = "";
static struct argp_option options[] = { 
    {"update", 'u', "ARG", 0, "Update Program and Configuration Parameter"},
    {"read", 'r', "ARG", 0, "Read a parameter or file"},
    {"write", 'w', "ARG", 0, "Write a parameter or file\nParameter requires value argument"},
    {"interface", 'i', "NAME", 0, "Specify interface name, can0 by default"},
    {"cycle", 'c', 0, 0, "Reset xt can"},
    {"estop", 'e', "STATE", 0, "Toggle estop, 1 = trigger estop 2 = disable estop"},
    {"value", 'v', "VAL", 0, "Value arg for write parameter"},
    {"live", 'l', 0, 0, "Launch the live data output"},    
    {0},
};

struct cmd_args
{
    char *manifest_file;
    char *machine;
    bool update;
    bool *interface;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) 
{
    cmd_args *args = (cmd_args *)state->input;

    switch (key) {
    case 'p':
    
        break;
    case 'f': 
        break;
    case 'i': 
        break;
    case 'n':  
        break;
    case 'd': 
        break;
    case 'r': 
        break;
    case 's':
        break;
    case 'v':  // version information
        printf("GIT Tag Name: %s\r\n", VERSION_GIT_TAG_NAME);
        printf("GIT Tag Hash: %s\r\n", VERSION_GIT_TAG_HASH);
        printf("GIT Tag Date: %s\r\n", VERSION_GIT_TAG_DATE);
        printf("Build Date: %s\r\n", VERSION_BUILD_DATE);
        exit(0);        
    case 't': 
        break;       
    case 'e': 
        break;  
    case ARGP_KEY_END:
        if (state->arg_num < 2)
        {
            /* Not enough arguments. */
            argp_usage (state);
        }
        break;

    case ARGP_KEY_ARG: 
        // these are arguments passed without key
        if ( state->arg_num == 0 ) 
        {
            args->manifest_file = arg;
        } 
        else if ( state->arg_num == 1 ) 
        {
            args->machine = arg;
        } 
        else
        {
            /* wrong arguments*/
            argp_usage (state);
        }
        break;

    default: 
        return ARGP_ERR_UNKNOWN;
    }   

    return 0;
}
 
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
static cmd_args args;

int main(int argc, char *argv[])
{
    lowsheen::Manifest manifest;
    lowsheen::MuleInterface interface; 
    lowsheen::header_t header;
    int machine_id;
 
    argp_parse(&argp, argc, argv, 0, 0, &args);

    if(manifest.read(args.manifest_file) == false)
    {
        std::cout << "Unable to read Manifest file" << std::endl;
        return -1;
    }

    if(manifest.find(&machine_id, args.machine) == false)
    {
        std::cout << "machine name does not match manifest file list" << std::endl;
        return -1;
    }

    if(interface.get_header(&header) == false)
    {
        std::cout << "Failed to Find Lowsheen Interface" << std::endl;
        return -1;
    }

    if(header.safe_to_flash == 0)
    {
        std::cout << "Lowsheen indicated an unsafe state to flash" << std::endl;
        return -1;
    }

    if(header.estop_code != 0)
    {
        std::cout << "Lowsheen indicated an estop has been activated" << std::endl;
        return -1;
    }

    interface.enter_xt_can_mode();

    sleep(3);

    std::string utility_name;

    if(manifest.machines[machine_id].controller.controller_type == "kinetek")
    {
        utility_name = "kinetek-util";
    }
    else if(manifest.machines[machine_id].controller.controller_type == "vcis")
    {
        utility_name = "vcis-util";
    }

    std::cout << "This requires " << utility_name << std::endl;

    system("vcis-util --reset");

	return 0;
}
