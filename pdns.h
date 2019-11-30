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
#ifndef PARALLEL_DNS_H
#define PARALLEL_DNS_H

#include "logEntry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Number of threads to use in parallel */
#define PDNS_MAX_THREADS 100
#define PDNS_INBOX_SZ (PDNS_MAX_THREADS*3)
#define PDNS_CHILD_INBOX_SZ 1
#define PDNS_SHUTDOWN_PAUSE_MS 500

int
PDNS_lookup(LOGENTRY *lePtrArr[], unsigned nItems, unsigned timeout_ms);
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
