# ban2fail

(C) 2019 John D. Robertson <john@rrci.com>

**ban2fail** is a simple and efficient tool to coordinate log file scanning,
reporting, and iptables filtering. As the name implies, *ban2fail* was
inspired by the popular *fail2ban* project (http://fail2ban.org). The main
technical advantages *ban2fail* provides over *fail2ban* are:

+ All relevant logfiles on disk are scanned, not just the current log files.

+ A unique and transparent caching scheme is employed to make this process at
least 100x as fast as doing the same thing with, say, *grep*.

+ Instantaneously and conveniently produces on command all offending logfile
entries which exist somewhere in the logfile history for given address(es).

+ Easily handles hundreds of thousands of blocked IP addresses.

+ Directly calls iptables, and handles filtering rules in batches of 100 per
call.

+ Provides integrated reporting with reverse and forward DNS information.

+ When reporting, DNS lookups are performed in parallel with 200 simultaneous
lookups.

+ Written in pure C, with less than 15,000 lines of source code.

+ Efficient enough to run every 0.4 seconds without monopolizing a CPU core on a
modest server.


*ban2fail* started with a few hours of frenzied C hacking after my mail server
was exploited to deliver spam for others who had cracked a user's SMTP send
password. After inspecting the log files I realized that crackers are now using
widely distributed attacks, and that I would need an extremely efficient tool
that could scan my entire log file history in a fraction of a second on my
rather modest Linode virtual server to have a chance of stopping them. Here are
the timing results for a typical scan on my server:

```
real    0m0.325s
user    0m0.186s
sys     0m0.150s
```

Currently I am running *ban2fail* from a *systemd* service file which triggers
*ban2fail* whenever a watched log file is modified. This gives attackers at
most a 0.4 second window to do their worst. I hope you find this code useful.

## Configuration

*ban2fail* works from a configuration file found at
"/etc/ban2fail/ban2fail.cfg".  The overarching premise is that if any REGEX
appearing in a LOGTYPE clause matches a line in an associated log file, then by
default that IP will be blocked.


```
LOGTYPE auth {

   # Where to find the log files
   DIR= /var/log
   PREFIX= auth.log

   # How to read the timestamp
   TIMESTAMP auth_ts {
      # isolates the timestamp from a line matched by a TARGET
      REGEX= ^(.*) srv
      # Passed to strptime() to intrepret the timestamp string
      STRPTIME= %b %d %T
      # These stamps do not include the year, so it is implied.
      FLAGS= GUESS_YEAR
   }

   TARGET imap {
      # Pattern to search for, isolates the IP address
      REGEX= imapd.*Login failed.*\[([0-9.a-f:]+)\]$
      # Assign this as the severity of the offense.
      SEVERITY= 3
   }

   TARGET ssh {
      SEVERITY= 4
      REGEX= sshd.*Failed password.*from ([0-9.a-f:]+) port [0-9]+ ssh2$
      REGEX= sshd.*Invalid user.*from ([0-9.a-f:]+) port
   }

   TARGET negotiate_fail {
      SEVERITY= 2
      REGEX= Unable to negotiate with ([0-9.a-f:]+) port
   }

   TARGET dovecot {
      SEVERITY= 3
      REGEX= dovecot.*authentication failure.*rhost=([0-9.a-f:]+)
   }
}
```


Syntax in the config file is pretty much the same as the nftables syntax. All
keywords must be in upper case.  Any values in the key=value pairs have
whitespace stripped from the beginning and end of the line. Since there is
little escaping of characters going on, regular expressions are mostly WYSIWYG.
If you have a hash symbol '#' or a double quote '"' in your pattern (which are
special characters for the config file parser), you will need to escape
them like so:

```
# Nov 27 02:03:03 srv named[764]: client @0x7fe6a0053420 1.192.90.183#27388 (www.ipplus360.com): query (cache) 'www.ipplus360.com/A/IN' denied
   REGEX= named.*client.* ([0-9.a-f:]+)\#.*denied$
```

Finding typos and so forth in the config file is easy; use the -v command flag
to print all unrecognized content (besides comments).

`ban2fail -v`

The only way to alter the default blocking behavior is with a MAX\_OFFENSES
clause. This clause allows you specify how many offenses are tolerated before an
IP is blocked. Offenses will naturally disappear as old logfiles are deleted by
*logrotate*.

```
# Whitelist ourself
MAX_OFFENSES -1 {
# Put your server's IP addresses here
#   IP= 1.2.3.4
   IP= 127.0.0.1
#   IP= dead:beef::20::32a
   IP= ::1
}

# Allegedly legit servers
MAX_OFFENSES 50 {

# Google Ireland
   IP= 2a00:1450:4864:20::32a
   IP= 2a00:1450:4864:20::336

# Google EU
# Attempted to break in
#   IP= 35.205.240.168

# Google US
   IP= 09.85.216.42
# Attempted to break in
#   IP= 130.211.246.128
   IP= 209.85.166.194
   IP= 209.85.166.195
   IP= 209.85.208.67
   IP= 209.85.214.194
   IP= 209.85.215.173
   IP= 209.85.215.175
   IP= 209.85.215.193
   IP= 209.85.216.42
   IP= 2607:f8b0:4864:20::1034
   IP= 2607:f8b0:4864:20::a46

# Yahoo
   IP= 106.10.244.139

# Outlook
   IP= 40.92.4.30
   IP= 40.107.73.61
   IP= 40.107.74.48
   IP= 40.107.74.72 
   IP= 40.107.76.74
   IP= 40.107.79.52
   IP= 40.107.79.59
   IP= 40.107.80.40
   IP= 40.107.80.53
   IP= 40.107.80.78
   IP= 40.107.82.75
   IP= 52.101.129.30
   IP= 52.101.132.108
   IP= 52.101.136.79
   IP= 52.101.140.230
}

# "trusted" addresses
MAX_OFFENSES 200 {

# me from home
#   IP= 1.2.3.4/20

# Customer
#   IP= 5.6.7.8/24
}
```

If you recieve a complaint about an address unjustly getting blocked, place it
in one of the MAX\_OFFENSES blocks, and the IP will be unblocked the next time
*ban2fail* runs in production mode.

## Working with *ban2fail*

There are two primary modes in which *ban2fail* is used:

* Production mode, where iptables rules are modified.

* Testing mode, where modifications to blocking rules are merely indicated.

### Production

In production mode it is expected that *ban2fail* is running non-interactively,
and no output is printed unless addresses are (un)blocked. It is also possible
to generate a listing of addresses, offense counts, and status with the -a
command flag. Likewise, a listing of countries and offense counts is available
with the *-c* flag. In order to get DNS information for the *-a* flag, follow
with a plus for all DNS info *-a+*, or a minus for only legit (backward &
forward match) info *-a-*. In the list, DNS issues are presented like so:

```
# DNS is good
0 Dec 06 08:31      1/0    offenses AR [BLK] 200.71.237.244     host244.200-71-237.telecom.net.ar

# Reverse lookup failed with DNS server
0 Dec 05 19:43      1/0    offenses GB [BLK] 185.217.230.146     SERVFAIL

# Reverse lookup is a non-existent domain
2 Dec 05 21:11      1/0    offenses US [BLK] 67.205.153.94       NXDOMAIN

# Forward lookup does not match reverse lookup
0 Dec 06 08:40      1/0    offenses LU [BLK] 92.38.132.54       ibocke43.monster !

# Forward DNS record does not exist
0 Dec 06 08:37      1/0    offenses US [BLK] 63.81.90.135       63-81-90-135.nca.lanset.com !!

# DNS is inconclusive due to lack of response from a DNS server
0 Dec 05 22:04      1/0    offenses RU [BLK] 77.221.144.107     news5.burningcoalsa.com ~
```

If you want to see the offending log lines for specific address(es), supply
them on the command line like so:

```
john@srv:~$  ban2fail 68.183.105.52
====== Report for 68.183.105.52 ======
------- /var/log/auth.log -------------
Dec  5 17:50:47 srv sshd[22326]: Invalid user cron from 68.183.105.52 port 41874
Dec  5 17:50:48 srv sshd[22326]: Failed password for invalid user cron from 68.183.105.52 port 41874 ssh2

```

### Testing

In test mode (-t flag) the presumption is that you are testing a modified
configuration which is not yet in place, and that you don't want to disturb the
production setup. This is how you might do that:

`ban2fail -t myNew.cfg -a`

No iptables rules will be modified. You will be shown in the listing which
addresses would be (un)blocked if the contents of "myNew.cfg" was in place, and
*ban2fail* was running in production mode.

When you are happy with the new configuration, copy it into place, and the the
iptable rule changes will be realized the next time *ban2fail* runs in
production mode.

## Building the Project

I've tested *ban2fail* only on Debian Buster, but it should compile on just
about any modern Linux distro. It uses the following libraries:

+ *libcrypto* from the libssl package, for md5 checksums

+ *libGeoIP* to identify the country of origin for IP addresses

+ *libz* to read compressed log files

+ *libpthread* for parallel DNS lookups (200 simultaneous)

+ *libdb* caching of offense location and size in log files

Build and install like so:

```
make release
make install
```

The make *install* target calls *install.sh*, which does a bunch of stuff
including setting up and enabling a systemd service, so you might want have a
look before pulling the trigger.

*ban2fail.service* points to *ban2fail.sh*, which can be tested from the command line for debugging.  Remember to make sure the service is disabled:
```
systemctl stop ban2fail
```

In order to run *ban2fail* as non-root user, the user must belong to group
'adm'. This is so in order to run iptables, which is accomplished via setuid(0)
at the appropriate time.

