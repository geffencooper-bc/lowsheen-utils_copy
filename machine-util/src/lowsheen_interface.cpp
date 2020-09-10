

#include "lowsheen_interface.h"
#include "lowsheen_headers.h"
#include <libusb.h>


static const uint16_t USB_MULE_VID = 0x0483;
//static const uint16_t USB_MULE_PID = 0x5710;
static const uint16_t USB_MULE_PID = 0x0002;

class usb
{
    private:
        libusb_context *ctx; //a libusb session
        struct libusb_device_handle *dev_handle;
        int r;
        bool ready;
    public:
    usb(uint16_t vid, uint16_t pid)
    {
        ctx = nullptr;
        dev_handle = nullptr;
        ready = false;

        r = libusb_init(&ctx); //initialize a library session
        if(r < 0)
        {
            printf("Init Error: %d\r\n", r); //there was an error             
        }

        dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid); //these are vendorID and productID I found for my usb device

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
            return;
        }
        printf("Claimed Interface\r\n");

        ready = true;
    }

    ~usb()
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

    int32_t get_status()
    {
        return r;
    }

    int32_t write(uint8_t endpoint, uint8_t *data, int32_t data_size)
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

    int32_t read(uint8_t endpoint, uint8_t *data, int32_t data_size)
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

        printf("Writing Data...\r\n");
        r = libusb_bulk_transfer(dev_handle, (unsigned char )endpoint, (unsigned char *)data, data_size, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
        if(r == 0) 
            printf("Read Successful!\r\n");
        else
            printf("Read Error\r\n");

        return actual;
    }
};

namespace lowsheen
{

MuleInterface::MuleInterface(const char *filename)
{
    manifest.read(filename);
}

bool MuleInterface::enter_normal_mode()
{
    // detect mode
    // if xt_can, reset
    // detect mode
 
	usb mule_usb(USB_MULE_VID, USB_MULE_PID);

    return mule_usb.get_status() == 0;
}

header_t MuleInterface::get_header()
{
    uint8_t interface_data[1024];
    request_packet_t request;
    header_t header;
    int len;

    if(enter_normal_mode())
    {
        usb mule_usb(USB_MULE_VID, USB_MULE_PID);

        generate_request_packet(&request);

        mule_usb.write(0x03, (uint8_t *)&request, (int32_t)sizeof(request_packet_t));

        len = mule_usb.read(0x81, interface_data, (int32_t)sizeof(interface_data));

        interface_decode(&header, interface_data, len);

        return header;
    }
}

bool MuleInterface::enter_xt_can_mode()
{    
    header_t header = get_header();
    config_packet_t config_packet;

    if(header.safe_to_flash == 1 && header.estop_code == 0)
    {
        usb mule_usb(USB_MULE_VID, USB_MULE_PID);

        config_packet.magic_number = MAGIC_NUMBER;
        config_packet.mode = 2;
        config_packet.checksum = 0; // not used

        mule_usb.write(0x02, (uint8_t *)&config_packet, (int32_t)sizeof(config_packet));
        return true;
    }

    return false;
}

}
