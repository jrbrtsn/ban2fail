#!/bin/bash
#
# JDR Wed 20 Nov 2019 10:48:14 PM EST
# The purpose of this script is to be run from a minutely cron job,
# running the job several times a minute,
# making reasonably sure there is no overlap.
#

BAN2FAIL=/usr/local/bin/ban2fail
LOGFILE=/var/log/ban2fail.log
#LOGFILE=/dev/pts/2
PERIOD_SEC=5

WHEN=$(date)

echo -n "$WHEN" >>$LOGFILE

BEGIN_SEC=$(date +%s)

count=0
while true; do

  (( ++count ))

  NOW_SEC=$(date +%s)

  (( MAX_SEC= 60 - PERIOD_SEC - 1 ))

  (( NOW_SEC - BEGIN_SEC > MAX_SEC )) && break

  $BAN2FAIL

  echo -n " $count" >>$LOGFILE
  FINISHED_SEC=$(date +%s)
  (( SLEEP = 5 - FINISHED_SEC + NOW_SEC ))

  (( SLEEP < 1 )) && continue
  sleep $SLEEP

done

echo  >>$LOGFILE

exit 0
