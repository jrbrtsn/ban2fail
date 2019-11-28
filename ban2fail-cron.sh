#!/bin/bash -e
#
# JDR Wed 27 Nov 2019 08:16:00 PM EST
# The purpose of this script is to be run from a periodic
# cron job to send summary via email.
#

BAN2FAIL=/usr/local/bin/ban2fail


TRIES=

for (( TRIES= 0; TRIES < 10; ++TRIES )); do

# cron generates an entry in auth.log when it runs, which means
# ban2fail.sh will launch ban2fail because of the new log entry.
# Solution is to pause for a second before running ban2fail here.
   sleep 1

   $BAN2FAIL -s && break


done


exit 0
