#!/bin/bash

# this script tests the robustness of entering IAP
# mode using the forced methodand selective method
# by trying them 100 times. The mode can be
# specified as an argument into the script
# After each attempt, a csv string is printed and
# get redirected to an output .txt file

start=$SECONDS
EXEC_NAME=""
OUTPUT_FILE=""

# First parse the command line option
if [ "$1" = "s" ]; then
    echo "Running Selective Mode Test..."
    OUTPUT_FILE=selective_stats.txt
    EXEC_NAME=kinetek-util_s
elif [ "$1" = "f" ]; then
    echo "Running Forced Mode Test..."
    OUTPUT_FILE=forced_stats.txt
    EXEC_NAME=kinetek-util_f
fi

touch $OUTPUT_FILE

# run the test 100 times
i=0
while [ $i -lt 3 ]
do
  # launch xt_can and set up can interface
  #python shining_software/src/catkin_ws/src/lowsheen/src/lowsheen_lib/lowsheen_stm_interface.py --xt_can
  #sleep 1.5
  #sudo ip link set can0 up type can bitrate 250000
  #sudo ip link set up can0

  # call the download program
  echo "Running Test: $i"
  echo "Result $i" >> $OUTPUT_FILE
  ./$EXEC_NAME -w 2.27.hex >> $OUTPUT_FILE
  echo " " >> $OUTPUT_FILE

  # stop xt_can
  #./stop_app.sh
  i=$[$i+1]
  sleep 1
done

duration=$(( SECONDS - start ))
echo "Duration: " $duration "seconds" >> $OUTPUT_FILE
