#include <libusb.h>
#include <stdio.h>
#include "lowsheen_interface.h"
#include <iostream>

void printdev(libusb_device *dev); //prototype of the function

// int attempt_xt_can(libusb_context *ctx)
// {
//     struct libusb_device_handle *dev_handle;
//     int r;
//     int actual;
//     lowsheen::config_packet_t config_packet = { MAGIC_NUMBER, 0x02, 0x00};

// 	dev_handle = libusb_open_device_with_vid_pid(ctx, 0x0483, 0x0002); //these are vendorID and productID I found for my usb device

// 	if(dev_handle == NULL)
//     {
// 		printf("Cannot open device\r\n");
//         return 1;
//     }
 
//     if(libusb_kernel_driver_active(dev_handle, 0) == 1)  //find out if kernel driver is attached
//     {
//         printf("Kernel Driver Active\r\n");
//         if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
//         {
//             printf("Kernel Driver Detached!\r\n");
//         }
//     }

//     r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)

// 	if(r < 0) 
//     {
// 		printf("Cannot Claim Interface\r\n");
// 		return 1;
// 	}
// 	printf("Claimed Interface\r\n");
	
//  	printf("Writing Data...\r\n");
// 	r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), (unsigned char *)&config_packet, sizeof(config_packet), &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
// 	if(r == 0 && actual == sizeof(config_packet)) 
// 		printf("Writing Successful!\r\n");
// 	else
// 		printf("Write Error\r\n");
	
// 	r = libusb_release_interface(dev_handle, 0); //release the claimed interface
// 	if(r!=0) {
// 		printf("Cannot Release Interface\r\n");
// 		return 1;
// 	}
// 	printf("Released Interface\r\n");

// 	libusb_close(dev_handle); //close the device we opened

//     return 0;
// }

void list_devices()
{
	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_context *ctx = NULL; //a libusb session
	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	r = libusb_init(&ctx); //initialize a library session
	if(r < 0)
    {
		printf("Init Error: %d\r\n", r); //there was an error
		return;
	}
	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if(cnt < 0) {
		printf("Get Device Error\r\n"); //there was an error
        return;
	}
	printf("%d Devices in list.\r\n", (int)cnt); //print total number of usb devices
		ssize_t i; //for iterating through the list

	for(i = 0; i < cnt; i++) 
    {
		printdev(devs[i]); //print specs of this device
	}
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	    
    libusb_exit(ctx); //close the session
}

int main(int argc, char *argv[])
{

    lowsheen::MuleInterface interface(argv[1]);

    lowsheen::header_t header;
    
    if(interface.get_header(&header))
    {
        std::cout << "Found header: " << header.header_protocol_version << std::endl;

        interface.enter_xt_can_mode();
    }
    else
    {
        std::cout << "failed to communicate with lowsheen device" << std::endl;
    }

	return 0;
}


void printdev(libusb_device *dev) {
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		printf("failed to get device descriptor\r\n");
		return;
	}
	printf("Number of possible configurations: %d\r\n",(int)desc.bNumConfigurations);
	printf("Device Class: %d\r\n", (int)desc.bDeviceClass);
	printf("VendorID: %X\r\n", desc.idVendor);
	printf("ProductID: %X\r\n", desc.idProduct);

	struct libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	printf("Interfaces: %d\r\n", (int)config->bNumInterfaces);

	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;

	for(int i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		printf("Number of alternate settings: %d \r\n", inter->num_altsetting);
		for(int j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			printf("Interface Number: %d\r\n", (int)interdesc->bInterfaceNumber);
			printf("Number of endpoints: %d\r\n", (int)interdesc->bNumEndpoints);
			for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				printf("Descriptor Type: %d\r\n",(int)epdesc->bDescriptorType);
				printf("EP Address: %d\r\n", (int)epdesc->bEndpointAddress);
			}
		}
	}
	printf("\r\n\r\n\r\n");
	libusb_free_config_descriptor(config);
}