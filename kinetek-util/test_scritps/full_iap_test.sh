#!/bin/bash

# this script runs the entire iap process several times
# and outputs the results to a log file

# start the timer
start=$SECONDS

# specify the number of iterations of the test here
NUM_TESTS=2

i=0
NUM_SUCCESSES=0

EXEC_NAME=kinetek-util_full_iap_test
OUTPUT_FILE=full_iap_test.txt

touch $OUTPUT_FILE

while [ $i -lt $NUM_TESTS ]
do
  echo "Running download: $i"
  echo "Test $i" >> $OUTPUT_FILE

  # call the download program
  RESULT=$(./$EXEC_NAME -w 2.27.hex)

  if [ "$RESULT" = "+" ]; then
      NUM_SUCCESSES=$[$NUM_SUCCESSES+1]
      echo "SUCCESS" >> $OUTPUT_FILE
  else
      echo "FAIL" >> $OUTPUT_FILE
  fi
  echo " " >> $OUTPUT_FILE

  i=$[$i+1]
  sleep 2
done

# output the final stats
echo "Score: " $NUM_SUCCESSES "/" $NUM_TESTS >> $OUTPUT_FILE
duration=$(( SECONDS - start ))
echo "Duration: " $duration "seconds" >> $OUTPUT_FILE


