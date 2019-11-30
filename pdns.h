/***************************************************************************
 *   Copyright (C) 2019 by John D. Robertson                               *
 *   john@rrci.com                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*
 * JDR Sat 30 Nov 2019 08:27:23 AM EST
 * Performs DNS reverse + fwd lookups in parallel. Having to use a bunch of threads
 * to overcome the serialization inherent in blocking calls is a travesty, but
 * I couldn't find a non-blocking version of getnameinfo().
 *
 * Oh yeah, and I'm not in the mood to write it myself ;-)
 */

#ifndef PARALLEL_DNS_H
#define PARALLEL_DNS_H


/* Number of threads to use in parallel */
#define PDNS_MAX_THREADS        200
#define PDNS_MGR_INBOX_SZ       PDNS_MAX_THREADS*2
#define PDNS_WORKER_INBOX_SZ    1
/* Give worker threads a chance to join */
#define PDNS_SHUTDOWN_PAUSE_MS  500

enum PDNS_flags {
   PDNS_SERVFAIL_FLG=   1<<0,
   PDNS_NXDOMAIN_FLG=   1<<1,
   PDNS_REV_DNS_FLG=    1<<2,
   PDNS_FWD_DNS_FLG=    1<<3,
   PDNS_FWD_FAIL_FLG=   1<<4,
   PDNS_FWD_NONE_FLG=   1<<5,
   PDNS_DONE_MASK= PDNS_SERVFAIL_FLG|PDNS_NXDOMAIN_FLG|PDNS_FWD_DNS_FLG
};

#define _GNU_SOURCE
#include "logEntry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fix recursive #include dependency */
struct _LOGENTRY;

int
PDNS_lookup(struct _LOGENTRY *lePtrArr[], unsigned nItems, unsigned timeout_ms);
/**************************************************************
 * Perform parallel DNS reverse lookups on all LOGENTRY objects
 * referenced in lePtrArr until finished, or timeout_ms has lapsed.
 *
 * lePtrArr:   array of pointers to LOGENTRY objects
 * nItems:  length of array
 * timeout_ms: maximum amount of time to spend performing lookups.
 *
 * RETURNS
 * suceess - number of lookups completed
 * -1    Failures
 */

#ifdef __cplusplus
}
#endif

#endif
