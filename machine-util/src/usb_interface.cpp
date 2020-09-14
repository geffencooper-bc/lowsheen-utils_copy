
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

bool USBInterface::init()
{
    if(ctx != nullptr)
    {
        return true;
    }

    r = libusb_init(&ctx); //initialize a library session
    if(r < 0)
    {
        printf("Init Error: %d\r\n", r); //there was an error       
        return false;  
    }

    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation

    return true;
}

bool USBInterface::list(std::vector<USBDevice> *entries)
{
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config;
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices	
	ssize_t cnt; //holding number of devices in list

    if(init() == false)
    {
        return false;
    }

	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices

	if(cnt < 0) {
		printf("Get Device Error\r\n"); //there was an error
        return false;
	}

	//printf("%d Devices in list.\r\n", (int)cnt); //print total number of usb devices
		ssize_t i; //for iterating through the list

	for(int d = 0; d < cnt; d++) 
    {
		//printdev(devs[i]); //print specs of this device
        r = libusb_get_device_descriptor(devs[d], &desc);
        if (r < 0) {
            printf("failed to get device descriptor\r\n");
            return false;
        }
        // printf("Number of possible configurations: %d\r\n",(int)desc.bNumConfigurations);
        // printf("Device Class: %d\r\n", (int)desc.bDeviceClass);
        // printf("VendorID: %X\r\n", desc.idVendor);
        // printf("ProductID: %X\r\n", desc.idProduct);

        const struct libusb_interface *inter;
        const struct libusb_interface_descriptor *interdesc;
        const struct libusb_endpoint_descriptor *epdesc;

        USBDevice device;

        device.num_configurations = desc.bNumConfigurations;
        device.device_class = desc.bDeviceClass;
        device.vid = desc.idVendor;
        device.pid = desc.idProduct;

        entries->push_back(device);

	    // for(int i=0; i<(int)config->bNumInterfaces; i++) 
        // {
		//     inter = &config->interface[i];
		//     printf("Number of alternate settings: %d \r\n", inter->num_altsetting);
        //     for(int j=0; j<inter->num_altsetting; j++) 
        //     {
        //         interdesc = &inter->altsetting[j];
        //         printf("Interface Number: %d\r\n", (int)interdesc->bInterfaceNumber);
        //         printf("Number of endpoints: %d\r\n", (int)interdesc->bNumEndpoints);
        //         for(int k=0; k<(int)interdesc->bNumEndpoints; k++) 
        //         {
        //             epdesc = &interdesc->endpoint[k];
        //             printf("Descriptor Type: %d\r\n",(int)epdesc->bDescriptorType);
        //             printf("EP Address: %d\r\n", (int)epdesc->bEndpointAddress);
        //         }
        //     }
	    // }
	    //printf("\r\n");
	}

	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	    
    return true;
}

bool USBInterface::claim(uint16_t vid, uint16_t pid)
{
    if(init() == false)
    {
        return false;
    }

    if(dev_handle != nullptr)
    {
        return true;
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
    //printf("Claimed Interface\r\n");

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

            //printf("Released Interface\r\n");

            libusb_close(dev_handle); //close the device we opened
            dev_handle = nullptr;
        }

        libusb_exit(ctx); //close the session
        ctx = nullptr;

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

    //printf("Writing Data...\r\n");
    r = libusb_bulk_transfer(dev_handle, (unsigned char )endpoint, (unsigned char *)data, data_size, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
    
    if(r == 0 && actual == data_size) 
    {
        //printf("Writing Successful!\r\n");
    }
    else
    {
        printf("Write Error\r\n");
    }

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

    //printf("Reading Data...\r\n");
    r = libusb_bulk_transfer(dev_handle, (unsigned char )endpoint, (unsigned char *)data, data_size, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
    
    if(r == 0) 
    {
        //printf("Read Successful!\r\n");
    }
    else
    {
        printf("Read Error\r\n");
    }

    return actual;
}


}