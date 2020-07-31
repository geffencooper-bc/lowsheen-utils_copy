#!/bin/sh
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
# helper script to bring up the socket can interfaces manually

ip link set down can0
ip link set down can2

ip link set can0 type can bitrate 250000
ip link set can2 type can bitrate 250000

ip link set up can0
ip link set up can2


