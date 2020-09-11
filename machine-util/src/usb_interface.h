

#pragma once
#include <libusb.h>
#include <vector>

namespace lowsheen
{
struct USBDevice
{
    uint8_t num_configurations;
    uint8_t device_class;
    uint16_t vid;
    uint16_t pid;    
};


class USBInterface
{
    private:
        libusb_context *ctx; //a libusb session
        struct libusb_device_handle *dev_handle;
        int r;
        bool ready;
    public:
        USBInterface();
        ~USBInterface();    
        bool list(std::vector<USBDevice> *entries);     
        bool init();
        bool claim(uint16_t vid, uint16_t pid);        
        void release();
        int32_t get_status();

        int32_t write(uint8_t endpoint, uint8_t *data, int32_t data_size);
        int32_t read(uint8_t endpoint, uint8_t *data, int32_t data_size);
};

}
