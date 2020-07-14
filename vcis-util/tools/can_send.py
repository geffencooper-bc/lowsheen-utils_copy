from __future__ import print_function

import can 

bus = can.interface.Bus(bustype='socketcan', channel='can0')

msg = can.Message(arbitration_id=0x100,
                    data=[1, 2, 3, 4, 5, 6, 7, 8],
                    is_extended_id=False)

try:
    bus.send(msg)
    print("Message sent on {}".format(bus.channel_info))   
    msg = bus.recv(timeout=1000)
    print(msg) 
except can.CanError:
    print("Message NOT sent")


