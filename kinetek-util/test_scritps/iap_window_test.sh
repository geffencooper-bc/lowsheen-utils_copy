#!/bin/bash

# This script searches for the 6ms window for
# sending the forced command. It starts by continuously 
# sending and receiving the request at a 1ms interval 
# until the IAP heartbeat is deected. 30ms is then subtracted
# from that time to get the estimated window location. An 8 ms
# range around that window is then tested and the results are
# sent to a .csv file

# start the timer
start=$SECONDS

OUTPUT_FILE=iap_window.csv

# send 150 messages (max) from time 0
RESULT=$(./kinetek-util_window_test 0 150)

# check if entered IAP mode
STATUS=${RESULT:0:1}
if [ $STATUS = "F" ]; then
    echo "COULD NOT ENTER IAP MODE"
    exit
fi

# suubtract 30ms from the time that receivded IAP heartbeat
TIME_VAL=${RESULT:2}
echo $RESULT
IAP_TIME=$((${TIME_VAL%%.*} - 30))
echo "REGION: " $IAP_TIME "ms"
sleep 1

# create an 8ms interval around that time
RANGE_START=$(($IAP_TIME -4))
RANGE_END=$(($IAP_TIME + 4))

touch $OUTPUT_FILE
echo "SEND_TIME,NUM_SUCCESSES" >> $OUTPUT_FILE

# get success rate for each point in the 8ms interval.
# For each point send 1 request and count successes out of 5
for i in {0..7}
do
   SUCCESS_COUNT=0
   for j in {0..4}
   do
      RESULT=$(./kinetek-util_window_test $(($RANGE_START + $i)) 1)
      echo "Time: " $(($i + $RANGE_START)) "Test: " $j "Result: " $RESULT
      if [ ${RESULT:0:1} = "S" ]; then
          SUCCESS_COUNT=$[$SUCCESS_COUNT + 1]
      fi
      sleep 2
   done
   echo $(($RANGE_START + $i)) "," $SUCCESS_COUNT >> $OUTPUT_FILE
done

duration=$(( SECONDS - start ))
#echo "Duration: " $duration "seconds" >> $OUTPUT_FILE

