#!/bin/bash -e

set -u

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
[[ -e $OTHERDIR ]] || sudo mkdir $OTHERDIR
sudo chmod 2755 $OTHERDIR
sudo chown .adm $OTHERDIR

[[ -e $CONFDIR ]] || sudo mkdir $CONFDIR
sudo chmod 2750 $CONFDIR
sudo chown .adm $CONFDIR
# TODO: substitute correct hostname in config
[[ -e $CONFDIR/$CONF ]] || sudo cp $CONF $CONFDIR

[[ -e $BINDIR ]] || sudo mkdir $BINDIR

# No need for debugging symbols in binary
strip release/$BIN
sudo cp -f release/$BIN $BINDIR/
sudo chown .adm $BINDIR/$BIN
# Need suid bit so non-root can run iptables
sudo chmod 4755 $BINDIR/$BIN

chmod +x $SVCSH
sudo cp -f $SVCSH $OTHERDIR/

systemctl status ban2fail &>/dev/null && sudo systemctl stop ban2fail
# Clean out cache and lockfiles
[[ -e $CACHEDIR ]] && sudo rm -r $CACHEDIR
[[ -e $TEST_CACHEDIR ]] && sudo rm -r $TEST_CACHEDIR
[[ -e $LOCKDIR ]] && sudo rm -r $LOCKDIR
[[ -e $TEST_LOCKDIR ]] && sudo rm -r $TEST_LOCKDIR
sudo cp $SVC $SVCDIR/
sudo systemctl daemon-reload
sudo systemctl start ban2fail
sudo systemctl enable ban2fail

echo "Installation completed successfully"
systemctl status ban2fail
exit 0
