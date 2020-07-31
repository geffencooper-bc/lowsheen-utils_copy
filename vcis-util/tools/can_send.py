# ============================================================================
# Copyright 2020 Brain Corporation. All rights reserved. Brain
# Corporation proprietary and confidential.
# ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
# WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
# Portions of this Source Code and its related modules/libraries
# may be governed by one or more third party licenses, additional
# information of which can be found at:
# https://info.braincorp.com/open-source-attributions
# ============================================================================
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


