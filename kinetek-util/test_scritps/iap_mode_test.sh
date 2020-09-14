#!/bin/bash

# this script tests the robustness of entering IAP
# mode using the forced method and selective method
# by trying them several times. The mode can be
# specified as an argument into the script ("s", "f", "n").
# The "n" option means to execute the overall state machine,
# not just the forced or selective portion.


# start the timer
start=$SECONDS

# specify the number of test iterations 
NUM_TESTS=100

# separate executables for each test
EXEC_NAME=""

# log the results
OUTPUT_FILE=""

# stats
NUM_SUCCESS=0
TOTAL_NUM_RETRIES=0

# First parse the command line option to run the according mode
if [ "$1" = "s" ]; then
    echo "Running Selective Mode Test..."
    OUTPUT_FILE=selective_test_stats.txt
    EXEC_NAME=kinetek-util_selective_test
    # selective mode will simulate a forced failure but do not want to count this as an attempt
    TOTAL_NUM_RETRIES=$(($TOTAL_NUM_RETRIES - $NUM_TESTS))
elif [ "$1" = "f" ]; then
    echo "Running Forced Mode Test..."
    OUTPUT_FILE=forced_test_stats.txt
    EXEC_NAME=kinetek-util_forced_test
elif [ "$1" = "n" ]; then
    echo "Running Normal IAP Mode Test..."
    OUTPUT_FILE=normal_iap_mode_test_stats.txt
    EXEC_NAME=kinetek-util_normal_iap_mode_test
fi

touch $OUTPUT_FILE

# run the test
i=0
while [ $i -lt $NUM_TESTS ]
do
  echo "Running Test: $i"
  echo "Test $i" >> $OUTPUT_FILE

  # call the download program
  RESULT=$(./$EXEC_NAME -w 2.27.hex)

  if [ ${RESULT:0:1} = "+" ]; then
      NUM_SUCCESS=$[$NUM_SUCCESS+1]
      echo "Result: Success" >> $OUTPUT_FILE
  elif [ ${RESULT:0:1} = "-" ]; then
      echo "Result: Fail" >> $OUTPUT_FILE
  fi

  TOTAL_NUM_RETRIES=$[$TOTAL_NUM_RETRIES+${RESULT:2:1}+${RESULT:4:1}]
  IAP_MODE_RETRIES=${RESULT:2:1}

  FORCED_RETRIES=${RESULT:4:1}

  # selective mode simulates a forced failure in the beggining, don't want to count that as an attempt
  if [ "$1" = "s" ]; then
     FORCED_RETRIES=$[$FORCED_RETRIES - 1]
  fi

  MODE_USED=${RESULT:6:1}

  echo "Number of Forced Mode retries: " $FORCED_RETRIES >> $OUTPUT_FILE
  echo "Number of IAP Mode retries: " $IAP_MODE_RETRIES >> $OUTPUT_FILE
  
  if [ "$MODE_USED" = "S" ]; then
      echo "Mode Used: Selective" >> $OUTPUT_FILE
  elif [ "$MODE_USED" = "F" ]; then
      echo "Mode Used: Forced" >> $OUTPUT_FILE
  fi

  echo " " >> $OUTPUT_FILE

  i=$[$i+1]
  sleep 1
done

# output the final stats
echo "Score: " $NUM_SUCCESS "/" $NUM_TESTS >> $OUTPUT_FILE
echo "Retries Required: " $TOTAL_NUM_RETRIES >> $OUTPUT_FILE
duration=$(( SECONDS - start ))
echo "Duration: " $duration "seconds" >> $OUTPUT_FILE

