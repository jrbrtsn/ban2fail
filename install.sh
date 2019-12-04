#!/bin/bash -e

BINDIR=/usr/local/bin
B2F=ban2fail
DST=$BINDIR/$B2F

echo $0 $@
env | grep INSTALL
strip release/$B2F
sudo /bin/cp -f release/$B2F $DST
sudo chown .adm $DST
sudo chmod 4750 $DST

# TODO: create user, group, yadda yadda


exit 0
