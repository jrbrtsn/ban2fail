#!/bin/bash -e

set -u

BAN2FAIL=/usr/local/bin/ban2fail
TMPFILE=/tmp/$$.cron-rpt.sh

trap "[[ -e $TMPFILE ]] && rm $TMPFILE" 0

$BAN2FAIL -a- >$TMPFILE

while read; do
   MATCH=$(<$TMPFILE egrep "$REPLY\>")
   [[ "$MATCH" =~ BLK ]] || continue
   echo "$MATCH"
done <<\_EOF_
google\.com
outbound\.protection\.outlook\.com
mail-mail\.facebook\.com
yahoo\.com
_EOF_


exit 0
