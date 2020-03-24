
#include <argp.h>
#include <stdbool.h>
#include "CU.h"
#include <string.h>
#include <stdlib.h> 

const char *argp_program_version = "programname programversion";
const char *argp_program_bug_address = "<your@email.address>";
static char doc[] = "Your program description.";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] = { 
    { "program", 'p', 0, 0, "Select program to Download"},
    { "file", 'f', 0, 0, "File to program device"},
    { "interface", 'i', 0, 0, "socketcan interface"},
    { "node", 'n', 0, 0, "selected canopen node"},
    { "details", 'd', 0, 0, "print canopen node information"},
    { "reset", 'r', 0, 0, "reset adapter"},
    { 0 } 
};

struct arguments {
    enum { CHARACTER_MODE, WORD_MODE, LINE_MODE } mode;
    bool isCaseInsensitive;
};


static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    CU_TaskDetails *cmd = state->input;
    switch (key) {
    case 'p':
        if((strlen(arg) < 3) || (strcmp(arg, "0x") != 0))
        {
            cmd->programID = strtol(&arg[2], NULL, 10);
        }
        else
        {
            cmd->programID = strtol(&arg[2], NULL, 16);
        }   
        // add program task
        CU_TASK_addTask("program", 0);
        break;
    case 'f': cmd->programFilename = arg; break;
    case 'i': cmd->interfaceName = arg; break;
    case 'n': 
        if((strlen(arg) < 3) || (strcmp(arg, "0x") != 0))
        {
            cmd->nodeID = strtol(&arg[2], NULL, 10);
        }
        else
        {
            cmd->nodeID = strtol(&arg[2], NULL, 16);
        }   
        break;
    case 'd': 
        // add task info task
        CU_TASK_addTask("info", 0);
        break;
    case 'r': 
        // add task reset
        CU_TASK_addTask("reset", 0);
        break;
    case ARGP_KEY_ARG: return 0;
    default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

void CU_COMMAND_parseArgs(CU_TaskDetails *cmd, int argc, char *argv[])
{
    argp_parse(&argp, argc, argv, 0, 0, cmd);
}