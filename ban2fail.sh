#!/bin/bash
#
# JDR Wed 27 Nov 2019 01:30:29 PM EST
# The purpose of this script is to be run from a systemd service
# file, or sysvinit script.
#
# BE CAREFUL not to monitor the log file to which output from this
# script is written - you will have a feedback loop!
#

BAN2FAIL=/usr/local/bin/ban2fail
BAN2FAIL_CFG=/etc/ban2fail/ban2fail.cfg
INOTIFYWAIT=/usr/bin/inotifywait

# Uncomment this if you wish to see output from the time command
#TIME=time

# Do not run again for at least this many deciseconds, to
# avoid monopolizing CPU
MIN_PERIOD_DS=3

# Get period in nanoseconds for integer calculations
(( MIN_PERIOD_NS = MIN_PERIOD_DS * 100000000 ))

while true; do
   echo "Starting main loop"
   LOG_NAMES=$($BAN2FAIL --print-lfn | tr $'\n' ' ')
   LOG_NAMES="$LOG_NAMES $BAN2FAIL_CFG"

   # Always do initial check
   echo "Initial run for $BAN2FAIL"
   RAN_NS=$(date +%s%N)
   $TIME $BAN2FAIL

   echo "Monitoring: $LOG_NAMES"

   while read; do

      # if a file gets renamed, logrotate is doing it's thing.
      [[ "$REPLY" =~ MOVE_SELF ]] && break
      [[ "$REPLY" == $BAN2FAIL_CFG\ MODIFY ]] && break

      [[ "$REPLY" =~ MODIFY ]] || continue

# Uncomment this to see the inotifywait output which triggered this cycle
#echo "REPLY= '$REPLY'"

      NOW_NS=$(date +%s%N)
      (( SINCE_NS = NOW_NS - RAN_NS ))
#echo "RAN_NS= $RAN_NS, NOW_NS= $NOW_NS, SINCE_NS= $SINCE_NS, MIN_PERIOD_NS= $MIN_PERIOD_NS"
      if (( SINCE_NS < MIN_PERIOD_NS )); then

         (( REMAINING_NS = MIN_PERIOD_NS - SINCE_NS ))

         # break sleep time into seconds and nanosecond remainder components
         (( REMAINING_SEC = REMAINING_NS / 1000000000 ))
         (( REMAINING_NS_REM = REMAINING_NS % 1000000000 ))

#echo "REMAINING_NS= $REMAINING_NS, REMAINING_SEC= $REMAINING_SEC, REMAINING_NS_REM= $REMAINING_NS_REM"

         if (( REMAINING_SEC || REMAINING_NS_REM > 1000000 )); then

            # use printf command to format as floating point string
            remaining_sec_fp=$(printf '%d.%09d' $REMAINING_SEC $REMAINING_NS_REM)

#echo "sleeping for $remaining_sec_fp seconds"

            # sleep for floating point period of seconds
            sleep $remaining_sec_fp
         fi

      fi

      # Consume queued input to avoid running ban2fail more than necessary
      while read -t 0; do read; done

      echo "Running $BAN2FAIL"

      # Check for offenses
      # If ban2fail failed, then pause to avoid monopolizing CPU
      RAN_NS=$(date +%s%N)
      while ! $TIME $BAN2FAIL; do
         sleep .5
         RAN_NS=$(date +%s%N)
      done


   done < <(exec $INOTIFYWAIT -m $LOG_NAMES)

   echo ' Exiting main loop'

   sleep 1
done


exit 0
