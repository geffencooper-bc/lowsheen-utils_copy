#!/bin/bash

# this script tests the robustness of entering IAP
# mode using the forced method and selective method
# by trying them several times. The mode can be
# specified as an argument into the script ("s" or "f").
# After each attempt, a string is printed and
# gets redirected to an output .txt file

start=$SECONDS
EXEC_NAME=""
OUTPUT_FILE=""

NUM_SUCCESS=0
NUM_RETRIES=0
NUM_TESTS=3

# First parse the command line option
if [ "$1" = "s" ]; then
    echo "Running Selective Mode Test..."
    OUTPUT_FILE=selective_stats.txt
    EXEC_NAME=kinetek-util_s
    # selective mode will simulate a forced failure but do not want to count this as an attempt
    NUM_RETRIES=$(($NUM_RETRIES + -1*$NUM_TESTS))
elif [ "$1" = "f" ]; then
    echo "Running Forced Mode Test..."
    OUTPUT_FILE=forced_stats.txt
    EXEC_NAME=kinetek-util_f
fi

touch $OUTPUT_FILE

# run the test 100 times
i=0
while [ $i -lt $NUM_TESTS ]
do
  # launch xt_can and set up can interface
  #python shining_software/src/catkin_ws/src/lowsheen/src/lowsheen_lib/lowsheen_stm_interface.py --xt_can
  #sleep 1.5
  #sudo ip link set can0 up type can bitrate 250000
  #sudo ip link set up can0

  # call the download program
  echo "Running Test: $i"
  echo "Result $i" >> $OUTPUT_FILE
  RESULT=$(./$EXEC_NAME -w 2.27.hex)

  if [ ${RESULT:0:1} = "S" ]; then
      NUM_SUCCESS=$[$NUM_SUCCESS+1]
  fi

NUM_RETRIES=$[$NUM_RETRIES+${RESULT:2:1}+${RESULT:4:1}]

  echo $RESULT >> $OUTPUT_FILE
  echo " " >> $OUTPUT_FILE

  # stop xt_can
  #./stop_app.sh
  i=$[$i+1]
  sleep 1
done

echo "Score: " $NUM_SUCCESS "/" $NUM_TESTS >> $OUTPUT_FILE
echo "Retries Required: " $NUM_RETRIES >> $OUTPUT_FILE
duration=$(( SECONDS - start ))
echo "Duration: " $duration "seconds" >> $OUTPUT_FILE
