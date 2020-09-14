
#include <string>
#include <map>
#include <vector>

#pragma once

namespace lowsheen
{
    class ControllerEntry
    {
    public:
        std::string controller_type;
        std::map<int, std::string> programs;
        std::map<int, std::string> params;
    };

    class MachineEntry
    {
    public:    
        std::string machine_name;
        ControllerEntry controller;
    };

    class Manifest
    {
    public:
        std::map<int, MachineEntry> machines;
        std::vector<std::string> paths;
        bool read(const char * filename);
        bool write(const char * filename);
        bool find(int *id, const char *id_or_name);
        bool find(int id);
    };
}