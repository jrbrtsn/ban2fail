#!/bin/bash
#
# Add new addresses to ban2fail

set -efu

WD=~/projects/ban2fail
PROTO_CFG=$WD/real-ban2fail.cfg
ACTUAL_CFG=/etc/ban2fail/ban2fail.cfg
BAN2FAIL=/usr/local/bin/ban2fail
B2F_GREP=/usr/local/bin/b2f-grep
MD5SUM=
NEW_MD5SUM=
JUNK=
SEP=
IP=
PRTL_IP=
LINE=

trap "echo && $BAN2FAIL -s" 0

while true; do

   read MD5SUM JUNK < <(md5sum $ACTUAL_CFG)

   read -p 'Next address(es) please? ' IP

   # Only consider editing PROTO_CFG is IP is not empty.
   if [[ -n "$IP" ]]; then

      $BAN2FAIL $IP

      echo '--------------------------------'

      read -p 'Edit config file? '
      case ${REPLY:0:1} in
         y|Y) true;;
         *) continue;;
      esac
         
      # Edit all addresses we got from b2f-grep
      for LINE in $(2>/dev/null $B2F_GREP $IP <$PROTO_CFG); do
         $EDITOR +${LINE} $PROTO_CFG
      done
   fi

   # See if PROTO_CFG was changed.
   read NEW_MD5SUM JUNK < <(md5sum $PROTO_CFG)
   [[ $NEW_MD5SUM == $MD5SUM ]] && continue


   $BAN2FAIL -t $PROTO_CFG
   read -p 'Make this permanent? '
   case ${REPLY:0:1} in
      y|Y) true;;
      *) continue;;
    esac
   sudo /bin/cp $PROTO_CFG $ACTUAL_CFG
   $BAN2FAIL -s
done



