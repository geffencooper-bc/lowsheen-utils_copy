

#include "lowsheen_interface.h"
#include "lowsheen_headers.h"
#include "usb_interface.h"

static const uint16_t USB_MULE_VID = 0x0483;
//static const uint16_t USB_MULE_PID = 0x5710;
static const uint16_t USB_MULE_PID = 0x0002;


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
 
	USBInterface usb_interface;
    
    return usb_interface.claim(USB_MULE_VID, USB_MULE_PID);
}

header_t MuleInterface::get_header()
{
    uint8_t interface_data[1024];
    request_packet_t request;
    header_t header;
    int len;

    USBInterface usb_interface;

    if(enter_normal_mode())
    {
        usb_interface.claim(USB_MULE_VID, USB_MULE_PID);

        generate_request_packet(&request);

        usb_interface.write(0x03, (uint8_t *)&request, (int32_t)sizeof(request_packet_t));

        len = usb_interface.read(0x81, interface_data, (int32_t)sizeof(interface_data));

        interface_decode(&header, interface_data, len);

        return header;
    }
}

bool MuleInterface::enter_xt_can_mode()
{    
    header_t header = get_header();
    config_packet_t config_packet;
    USBInterface usb_interface;

    if(header.safe_to_flash == 1 && header.estop_code == 0)
    {
        usb_interface.claim(USB_MULE_VID, USB_MULE_PID);

        config_packet.magic_number = MAGIC_NUMBER;
        config_packet.mode = 2;
        config_packet.checksum = 0; // not used

        usb_interface.write(0x02, (uint8_t *)&config_packet, (int32_t)sizeof(config_packet));
        return true;
    }

    return false;
}

}
