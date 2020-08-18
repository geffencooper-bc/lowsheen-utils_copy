# ============================================================================
# Copyright 2020 Brain Corporation. All rights reserved. Brain
# Corporation proprietary and confidential.
# ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
# WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
# Portions of this Source Code and its related modules/libraries
# may be governed by one or more third party licenses, additional
# information of which can be found at:
# https://info.braincorp.com/open-source-attributions

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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


