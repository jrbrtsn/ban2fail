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

# Make sure directories exist with correct ownership & mode
[[ -e $OTHERDIR ]] || sudo mkdir $OTHERDIR
sudo chown .adm $OTHERDIR
sudo chmod 2755 $OTHERDIR

[[ -e $CONFDIR ]] || sudo mkdir $CONFDIR
sudo chown .adm $CONFDIR
sudo chmod 2750 $CONFDIR
[[ -e $CONFDIR/$CONF ]] || sudo cp $CONF $CONFDIR

[[ -e $BINDIR ]] || sudo mkdir $BINDIR

# No need for debugging symbols in binary
strip release/$BIN
sudo cp -f release/$BIN $BINDIR/
# Need suid bit so non-root can run iptables
sudo chmod 4750 $BINDIR/$BIN

chmod +x $SVCSH
sudo cp -f $SVCSH $OTHERDIR/

systemctl status ban2fail &>/dev/null && sudo systemctl stop ban2fail
sudo /bin/cp $SVC $SVCDIR/
sudo systemctl daemon-reload
sudo systemctl start ban2fail
sudo systemctl enable ban2fail

echo "Installation completed successfully"

exit 0
