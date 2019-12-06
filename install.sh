#!/bin/bash -e

set -u

if [[ $(id -u) != 0 ]]; then
   echo 'This script must be run as root!'
   exit 1
fi

BINDIR=/usr/local/bin
CONFDIR=/etc/ban2fail
OTHERDIR=/usr/local/share/ban2fail
SVCDIR=/etc/systemd/system

BIN=ban2fail
CONF=ban2fail.cfg
SVC=ban2fail.service
SVCSH=ban2fail.sh
CACHEDIR=/var/cache/ban2fail
TEST_CACHEDIR=${CACHEDIR}-test
LOCKDIR=/run/lock/ban2fail
TEST_LOCKDIR=${LOCKDIR}-test

# Make sure directories exist with correct ownership & mode
[[ -e $OTHERDIR ]] || mkdir $OTHERDIR
chmod 2755 $OTHERDIR
chown .adm $OTHERDIR

[[ -e $CONFDIR ]] || mkdir $CONFDIR
chmod 2750 $CONFDIR
chown .adm $CONFDIR
# TODO: substitute correct hostname in config
[[ -e $CONFDIR/$CONF ]] || cp $CONF $CONFDIR

[[ -e $BINDIR ]] || mkdir $BINDIR

# No need for debugging symbols in binary
strip release/$BIN
cp -f release/$BIN $BINDIR/
chown .adm $BINDIR/$BIN
# Need suid bit so non-root can run iptables
chmod 4755 $BINDIR/$BIN

chmod +x $SVCSH
cp -f $SVCSH $OTHERDIR/

systemctl status ban2fail &>/dev/null && systemctl stop ban2fail
# Clean out cache and lockfiles
[[ -e $CACHEDIR ]] && rm -r $CACHEDIR
[[ -e $TEST_CACHEDIR ]] && rm -r $TEST_CACHEDIR
[[ -e $LOCKDIR ]] && rm -r $LOCKDIR
[[ -e $TEST_LOCKDIR ]] && rm -r $TEST_LOCKDIR
cp $SVC $SVCDIR/
systemctl daemon-reload
systemctl start ban2fail
systemctl enable ban2fail

echo "Installation completed successfully"
systemctl status ban2fail
exit 0
