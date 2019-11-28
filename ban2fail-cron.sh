#!/bin/bash -e
#
# JDR Wed 27 Nov 2019 08:16:00 PM EST
# The purpose of this script is to be run from a periodic
# cron job to send summary via email.
#

BAN2FAIL=/usr/local/bin/ban2fail


TRIES=

for (( TRIES= 0; TRIES < 10; ++TRIES )); do

   $BAN2FAIL -s && break

   sleep 1

done


exit 0
