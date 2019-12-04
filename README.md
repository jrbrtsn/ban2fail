# ban2fail

(C) 2019 John D. Robertson <john@rrci.com>

**ban2fail** is a simple and efficient tool to coordinate log file scanning,
reporting, and iptables filtering. As the name implies, *ban2fail* was
inspired by the popular *fail2ban* project (http://fail2ban.org).

*ban2fail* started with a few hours of frenzied C hacking after my mail server
was exploited to deliver spam for others who had cracked a user's SMTP send
password. After inspecting the log files I realized that crackers are now using
widely distributed attacks, and that I would need an extremely efficient tool
that could run in a fraction of a second on my rather modest Linode virtual
server to have a chance of stopping them. Here are the timing results for a
typical scan on my server:

```
real    0m0.269s
user    0m0.108s
sys     0m0.134s
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
      REGEX= dovecot.*authentication failure.*rhost=([0-9.]+)
   }

}
```


Syntax in the config file is pretty much the same as the nftables syntax. All
keywords must be in upper case.  Any values in the key=value pairs have
whitespace stripped from the beginning and end of the line. Since there is
little escaping of characters going on, regular expressions are mostly WYSIWYG.
If you have a hash symbol '#' in your pattern (which is the comment character
for the config file parser), you will need to escape it like so:

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
0 Dec 04 11:04  185.31.204.22       1/0    offenses GB [BLK] mail.damianbasel.audise.com 

# Reverse lookup does not match forward lookup
0 Dec 04 08:47  103.238.80.23       2/0    offenses VN [BLK] example.com !

# Forward DNS is unavailable
4 Dec 04 10:54  106.51.230.190      2/0    offenses IN [BLK] broadband.actcorp.in !!

# DNS is inconclusive due to lack of response from DNS servers
0 Dec 04 04:13  87.120.246.53       1/0    offenses BG [BLK] client.playtime.bg ~
```

If you want to see the offending log lines for specific address(es), supply
them on the command line like so:

```
user@srv:~$ ban2fail 208.187.162.100
====== Report for 208.187.162.100 ======
------- /var/log/exim4/mainlog -------------
2019-12-04 12:08:15 H=(mail.spika.stream) [208.187.162.100] F=<first.class.turmeric.cbd-mgregory=robertsonoptical.com@spika.stream> rejected RCPT <mgregory@robertsonoptical.com>: 208.187.162.100 is listed at zen.spamhaus.org (127.0.0.3: https://www.spamhaus.org/sbl/query/SBLCSS)

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

+ *libpthread* for parallel DNS lookups (200 simulataneous)

+ *libdb* caching of offense location and size in log files

Build and install like so:

```
make release
make install
```

The executable will be placed in "/usr/local/bin".

In order to run *ban2fail* as a systemd service which actively monitors log
files, put the service file *ban2fail.service* in place as well as placing
*ban2fail.sh* in '/usr/local/share/ban2fail/'.

*ban2fail.sh* can also be tested from the command line.  The user must belong to
group 'adm' in order to run iptables, which is accomplished via setuid() at the
appropriate time.

