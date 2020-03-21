#!/bin/sh

sudo ip link set down can0
sudo ip link set down can1

sudo ip link set can0 type can bitrate 250000
sudo ip link set can1 type can bitrate 250000

sudo ip link set up can0
sudo ip link set up can1
