#!/bin/sh


ip link set down can0
ip link set down can1

ip link set can0 type can bitrate 250000
ip link set can1 type can bitrate 250000

ip link set up can0
ip link set up can1


