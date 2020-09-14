
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
    {"interface", 'i', "NAME", 0, "Specify interface name, can0 by default"},
    {"read", 'r', "ARG", 0, "Read a parameter or file"},
    {"write", 'w', "ARG", 0, "Write a parameter or file\nParameter requires value argument"},
    {"value", 'v', "VAL", 0, "Value arg for write parameter"},
    {"cycle", 'c', 0, 0, "Reset xt can"},
    {"estop", 'e', "STATE", 0, "Toggle estop, 1 = trigger estop 2 = disable estop"},
    {"hearbeat", 'h', 0, 0, "Launch the live data output"},    
    {"details", 'd', 0, 0, "Print out build details"},    
    {0},
};

enum cmd_arg_type
{
    CMD_ARG_NONE = 0,
    CMD_ARG_READ,
    CMD_ARG_WRITE,
    CMD_ARG_ESTOP,
    CMD_ARG_CYCLE,
    CMD_ARG_HEARTBEAT,
};

struct cmd_args
{
    std::string manifest_file;
    std::string interface;
    std::string write;
    std::string read;    
    std::string value;
    std::string cycle;
    std::string estop;
    std::string heartbeat;
    cmd_arg_type type;
    bool has_value;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) 
{
    cmd_args *args = (cmd_args *)state->input;

    switch (key) {
    case 'i':
        args->interface = arg;
        break;
    case 'w': 
        if(args->type)
        {
            argp_usage (state);            
        }
        args->write = arg;
        args->type = CMD_ARG_WRITE;
        break;
    case 'r': 
        if(args->type)
        {
            argp_usage (state);  
        }
        args->read = arg;
        args->type = CMD_ARG_READ;
        break;
    case 'v':  
        args->value = arg;
        args->has_value = true;
        break;
    case 'c': 
        if(args->type)
        {
            argp_usage (state);  
        }    
        args->cycle = arg;
        args->type = CMD_ARG_CYCLE;
        break;
    case 'e': 
        if(args->type)
        {
            argp_usage (state);  
        }    
        args->estop = arg;
        args->type = CMD_ARG_ESTOP;
        break;
    case 'h':
        if(args->type)
        {
            argp_usage (state);  
        }    
        args->heartbeat = arg;
        args->type = CMD_ARG_HEARTBEAT;
        break;
    case 'd':  // version information
        printf("GIT Tag Name: %s\r\n", VERSION_GIT_TAG_NAME);
        printf("GIT Tag Hash: %s\r\n", VERSION_GIT_TAG_HASH);
        printf("GIT Tag Date: %s\r\n", VERSION_GIT_TAG_DATE);
        printf("Build Date: %s\r\n", VERSION_BUILD_DATE);
        exit(0);        
    case ARGP_KEY_END:
        if (state->arg_num < 1)
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
    std::string utility_name;
    std::string utility_args;
 
    argp_parse(&argp, argc, argv, 0, 0, &args);

    if(manifest.read(args.manifest_file.c_str()) == false)
    {
        std::cout << "Unable to read Manifest file" << std::endl;
        return -1;
    }

    if(interface.enter_normal_mode() == false)
    {
        std::cout << "Unable to find Lowsheen interface" << std::endl;
        return -1;
    }

    if(interface.get_header(&header) == false)
    {
        std::cout << "Failed to obtain Lowsheen Interface Information" << std::endl;
        return -1;
    }

    if(manifest.find(header.machine_id) == false)
    {
        std::cout << "machine name does not match manifest file list" << std::endl;
        return -1;
    }

    machine_id = header.machine_id;

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

    if(manifest.machines[machine_id].controller.controller_type == "kinetek")
    {
        utility_name = "kinetek-util";
    }
    else if(manifest.machines[machine_id].controller.controller_type == "vcis")
    {
        utility_name = "vcis-util";
    }

    switch(args.type)
    {
        default:
        case CMD_ARG_NONE:
            return 0;
        case CMD_ARG_READ:
            utility_args = "--read " + args.read;
            break;
        case CMD_ARG_WRITE:
            utility_args = "--write " + args.write;
            if(args.has_value)
            {
                utility_args += " --value " + args.value;
            }
            break;
        case CMD_ARG_ESTOP:
            utility_args = "--estop " + args.estop;
            break;
        case CMD_ARG_CYCLE:
            utility_args = "--cycle " + args.cycle;
            break;
        case CMD_ARG_HEARTBEAT:
            utility_args = "--hearbeat";
            break;
    }
 
    std::string u_args = utility_name + " " + args.manifest_file + " " + utility_args;
    std::cout << "Utility: " << u_args << std::endl;
    

    // revert back to mule if possible
    if(interface.enter_normal_mode() == false)
    {
        std::cout << "Unable to find Lowsheen interface" << std::endl;
        return -1;
    }

    return 0;
}
