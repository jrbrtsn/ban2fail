#!/bin/bash
#
# JDR Wed 27 Nov 2019 01:30:29 PM EST
# The purpose of this script is to be run from a systemd service
# file, or sysvinit script.
#
# BE CAREFUL not to monitor the log file to which output from this
# script is written - you will have a feedback loop!
#

BAN2FAIL_CFG=/etc/ban2fail/ban2fail.cfg
INOTIFYWAIT=/usr/bin/inotifywait
BAN2FAIL=/usr/local/bin/ban2fail

# For testing only
#BAN2FAIL="/usr/local/bin/ban2fail -t $BAN2FAIL_CFG"

# Uncomment this if you wish to see output from the time command
#TIME=time

# Do not run again for at least this many deciseconds, to
# avoid monopolizing CPU
MIN_PERIOD_DS=3

# Get period in nanoseconds for integer calculations
(( MIN_PERIOD_NS = MIN_PERIOD_DS * 100000000 ))

while true; do
   echo "Starting main loop"
   MON_FNAMES=$($BAN2FAIL --print-lfn | tr $'\n' ' ')
   MON_FNAMES="$MON_FNAMES $BAN2FAIL_CFG"

   # Always do initial check
   echo "Initial run for $BAN2FAIL"
   RAN_NS=$(date +%s%N)
   $TIME $BAN2FAIL

   echo "Monitoring: $MON_FNAMES"

   while read FILE OPS; do

      case "$OPS" in
         MOVE_SELF) break;;

         MODIFY) [[ "$FILE" == $BAN2FAIL_CFG ]] && break;;

         *) continue;;
      esac

# Uncomment this to see the inotifywait output which triggered this cycle
#echo "FILE= '$FILE', OPS= '$OPS'"

      NOW_NS=$(date +%s%N)
      (( SINCE_NS = NOW_NS - RAN_NS ))

      if (( SINCE_NS < MIN_PERIOD_NS )); then

         (( REMAINING_NS = MIN_PERIOD_NS - SINCE_NS ))

         # 'sleep' command wants a string representation of floating point number of seconds,
         # so we need to break sleep time into seconds and nanosecond remainder components
         (( REMAINING_SEC = REMAINING_NS / 1000000000 ))
         (( REMAINING_NS_REM = REMAINING_NS % 1000000000 ))

         if (( REMAINING_SEC || REMAINING_NS_REM > 1000000 )); then

            # use printf command to format as floating point string
            REMAINING_SEC_FP=$(printf '%d.%09d' $REMAINING_SEC $REMAINING_NS_REM)

            # sleep for floating point period of seconds
            sleep $REMAINING_SEC_FP
         fi
      fi

      echo "Running $BAN2FAIL"

      # Here is where we check for offenses.
      # If ban2fail failes it is probably because logrotated
      # is managing the log files, so bail out...
      RAN_NS=$(date +%s%N)
      $TIME $BAN2FAIL || break

   done < <($INOTIFYWAIT -m $MON_FNAMES)

   echo 'Exiting main loop'
   # Pause to let things settle down
   sleep 1

done


exit 0
