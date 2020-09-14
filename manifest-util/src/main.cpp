

#include <iostream>
#include "manifest.h"

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cout << "Incorrect Arguments" << std::endl;
    }

    lowsheen::Manifest manifest;

    if(manifest.read(argv[1]))
    {
        std::cout << "Success: " << manifest.paths[0] << std::endl;
    }

    return 0;
}




