
#include "usb_interface.h"
#include <cstdio>

namespace lowsheen
{

USBInterface::USBInterface()
{
    ctx = nullptr;
    dev_handle = nullptr;
    r = 0;
    ready = false;
}

USBInterface::~USBInterface()
{        
    release();
}

bool USBInterface::claim(uint16_t vid, uint16_t pid)
{
    ctx = nullptr;
    dev_handle = nullptr;
    ready = false;

    r = libusb_init(&ctx); //initialize a library session
    if(r < 0)
    {
        printf("Init Error: %d\r\n", r); //there was an error     
        return false;        
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid); //these are vendorID and productID I found for my usb device

    if(dev_handle == nullptr)
    {
        printf("Cannot Obtain Interface. VID:%04X PID:%04X\r\n", vid, pid);
        return false;
    }

    if(libusb_kernel_driver_active(dev_handle, 0) == 1)  //find out if kernel driver is attached
    {
        printf("Kernel Driver Active\r\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
        {
            printf("Kernel Driver Detached!\r\n");
        }
    }

    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)

    if(r < 0) 
    {
        printf("Cannot Claim Interface\r\n");
        return false;
    }
    printf("Claimed Interface\r\n");

    ready = true;
    return true;
}

void USBInterface::release()
{
    if(ctx != nullptr)
    {
        if(dev_handle != nullptr)
        {
            r = libusb_release_interface(dev_handle, 0); //release the claimed interface
            
            if(r != 0) 
            {
                printf("Cannot Release Interface\r\n");
                return;
            }

            printf("Released Interface\r\n");

            libusb_close(dev_handle); //close the device we opened
        }

        libusb_exit(ctx); //close the session
    }
}

int32_t USBInterface::get_status()
{
    return r;
}

int32_t USBInterface::write(uint8_t endpoint, uint8_t *data, int32_t data_size)
{
    int actual;

    if(!ready)
    {
        return -1;
    }

    printf("Writing Data...\r\n");
    r = libusb_bulk_transfer(dev_handle, (unsigned char )endpoint, (unsigned char *)data, data_size, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
    if(r == 0 && actual == data_size) 
        printf("Writing Successful!\r\n");
    else
        printf("Write Error\r\n");

    return actual;
}

int32_t USBInterface::read(uint8_t endpoint, uint8_t *data, int32_t data_size)
{
    if(!ready)
    {
        return -1;
    }

    int actual;
    
    if(!ready)
    {
        return -1;
    }

    printf("Reading Data...\r\n");
    r = libusb_bulk_transfer(dev_handle, (unsigned char )endpoint, (unsigned char *)data, data_size, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
    if(r == 0) 
        printf("Read Successful!\r\n");
    else
        printf("Read Error\r\n");

    return actual;
}


}