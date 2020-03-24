from __future__ import print_function

import can 



# Using specific buses works similar:
# bus = can.interface.Bus(bustype='socketcan', channel='vcan0', bitrate=250000)
# bus = can.interface.Bus(bustype='pcan', channel='PCAN_USBBUS1', bitrate=250000)
# bus = can.interface.Bus(bustype='ixxat', channel=0, bitrate=250000)
# bus = can.interface.Bus(bustype='vector', app_name='CANalyzer', channel=0, bitrate=250000)
# ...
#bus = can.interface.Bus(bustype='pcan', channel='PCAN_USBBUS1', bitrate=250000)
bus1 = can.interface.Bus(bustype='socketcan', channel='can1')
#bus2 = can.interface.Bus(bustype='socketcan', channel='can1')


msg1 = can.Message(arbitration_id=0x100,
                    data=[1, 2, 3, 4, 5, 6, 7, 8],
                    is_extended_id=False)


msg2 = can.Message(arbitration_id=0x200,
                    data=[1, 2, 3, 4, 5, 6, 7, 8],
                    is_extended_id=False)


try:
    bus1.send(msg1)
    print("Message 1 sent on {}".format(bus1.channel_info))    
  #  bus2.send(msg2)
   # print("Message 2 sent on {}".format(bus2.channel_info))
except can.CanError:
    print("Message NOT sent")


