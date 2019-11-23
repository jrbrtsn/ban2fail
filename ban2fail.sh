#!/bin/bash
#
# JDR Wed 20 Nov 2019 10:48:14 PM EST
# The purpose of this script is to be run from a minutely cron job,
# running the job several times a minute,
# making reasonably sure there is no overlap.
#

LOGFILE=/var/log/ban2fail.log

WHEN=$(date)

echo -n "$WHEN" >>$LOGFILE

BEGIN_SEC=$(date +%s)

count=0
while true; do

  (( ++count ))

  NOW_SEC=$(date +%s)

  (( NOW_SEC - BEGIN_SEC > 45 )) && break


  /usr/local/bin/ban2fail

  echo -n " $count" >>$LOGFILE
  FINISHED_SEC=$(date +%s)
  (( SLEEP = 10 - FINISHED_SEC + NOW_SEC ))

  (( SLEEP < 1 )) && continue
  sleep $SLEEP

done

echo  >>$LOGFILE

exit 0
