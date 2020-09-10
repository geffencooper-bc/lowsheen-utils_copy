
#include <string>
#include <map>
#include <vector>

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
        bool read(std::string filename);
        bool write(std::string filename);
    };
}