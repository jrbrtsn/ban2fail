#!/bin/bash -e
#
# JDR Wed 27 Nov 2019 01:30:29 PM EST
# The purpose of this script is to be run from a systemd service
# file, or sysvinit script.
#

BAN2FAIL=/usr/local/bin/ban2fail
BAN2FAIL_CFG=/etc/ban2fail/ban2fail.cfg
INOTIFYWAIT=/usr/bin/inotifywait

# Uncomment this if you wish to see output from the time command
#TIME=time

# Always do initial check
echo "Initial run for $BAN2FAIL"
$TIME $BAN2FAIL

while true; do
   echo "Starting main loop"
   LOG_NAMES=$($BAN2FAIL --print-lfn | tr $'\n' ' ')
   LOG_NAMES="$LOG_NAMES $BAN2FAIL_CFG"

   echo "Monitoring: $LOG_NAMES"

   while read; do
      # if a file gets renamed, logrotate is doing it's thing.
      [[ "$REPLY" =~ MOVE_SELF ]] && break
      [[ "$REPLY" == $BAN2FAIL_CFG\ MODIFY ]] && break

      [[ "$REPLY" =~ MODIFY ]] || continue

# Uncomment this to see the inotifywait output which triggered this cycle
#echo "REPLY= '$REPLY'"

      # Avoid running ban2fail multiple times if possible
      while read -t 0; do
         read
      done

      echo "Running $BAN2FAIL"
      # Check for offenses
      # If ban2fail failed, then pause to avoid DOS on CPU
      $TIME $BAN2FAIL || sleep 1

   done < <(exec $INOTIFYWAIT -m $LOG_NAMES)

   echo ' Exiting main loop'

   sleep 1
done


exit 0
