# ban2fail

(C) 2019 John D. Robertson <john@rrci.com>

**ban2fail** is a simple and efficient tool to coordinate log file scanning and
iptables filtering. As the name implies, *ban2fail* was inspired by the popular
*fail2ban* project (http://fail2ban.org).

*ban2fail* started with a few hours of frenzied C hacking after my mail server
was exploited to deliver spam for others who had cracked a user's SMTP send
password. After inspecting the log files I realized that crackers are now using
widely distributed attacks, and that I would need an app that could run several
times a minute on my rather modest Linode virtual server to have a chance of
stopping them. I hope you find this code useful.

## Configuration

*ban2fail* works from a configuration file found at
"/etc/ban2fail/ban2fail.cfg".  The overarching premise is that if any REGEX
appearing in a LOGTYPE clause matches a line in an associated log file, then by
default that IP will be blocked.


```
LOGTYPE auth {
   DIR= /var/log
   PREFIX= auth.log

   REGEX= imapd.*Login failed.*\[([0-9.]+)\]$

   REGEX= sshd.*Failed password.*from ([0-9.]+) port [0-9]+ ssh2$

   REGEX= Unable to negotiate with ([0-9.]+) port

   REGEX= in\.qpopper.*authentication failure.*tty=([0-9.]+)
}
```


Syntax in the config file is pretty much the same as the nftables syntax. All
keywords must be in upper case.  Any values in the key=value pairs have
whitespace stripped from the beginning and end of the line. Since there is no
escaping of characters going on, regular expressions are WYSIWYG.

Finding typos and so forth in the config file is easy; use the -v command flag
to print all unrecognized content (besides comments).

`ban2fail -v`

The only way to alter the default blocking behavior is with a MAX\_OFFENSES
clause. This clause allows you specify how many offenses are tolerated before an
IP is blocked. Offenses will naturally disappear as old logfiles are deleted by
*logrotate*.

```
# Take it easy on home boys
MAX_OFFENSES 5 {
   COUNTRY= US
}

# GeoIP doesn't know the location of every IP address
MAX_OFFENSES 3 {
   COUNTRY= unknown
}

# This is your whitelist: -1 means no limit.
MAX_OFFENSES -1 {

# me from home
   IP= 205.144.171.37

# Some user
   IP= 173.236.196.36
}
```

If you recieve a complaint about an address unjustly getting blocked, place it
in one of the MAX\_OFFENSES blocks, and the IP will be unblocked the next time
*ban2fail* runs in production mode.

## Working with *ban2fail*

There are two primary modes in which *ban2fail* is used:

* Production mode, where iptables rules are modified.

* Testing mode, where modifications to blocking rules are indicated.

### Production

In production mode it is expected that *ban2fail* is running from a cron job,
and no output is printed unless addresses are (un)blocked. It is also possible
to generate a listing of addresses, offense counts, and status with the -a
command flag. Likewise, a listing of countries and offense counts is available
with the -c flag.

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

+ *libcrypto* for md5 checksums

+ *libgeoip* to identify the country of origin for IP addresses

+ *libz* to read compressed log files

Build and install like so:

```
make release
sudo make install
```

The executable will be placed in "/usr/local/bin".



