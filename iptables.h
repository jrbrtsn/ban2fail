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
#ifndef IPTABLES_H
#define IPTABLES_H

#include "ptrvec.h"

#ifdef __cplusplus
extern "C" {
#endif

int
IPTABLES_is_currently_blocked(const char *addr);
/********************************************************
 * This provides an efficient lookup of addresses blocked
 * by iptables in the filter table, INPUT chain.
 * 
 * RETURN:
 * 1 if the supplied addr is blocked by iptables.
 * 0 otherwise.
 */

int
IPTABLES_block_addresses(PTRVEC *h_vec, unsigned batch_sz);
/**************************************************************
 * Block addresses in batches of batch_sz.
 */

int
IPTABLES_unblock_addresses(PTRVEC *h_vec, unsigned batch_sz);
/**************************************************************
 * Unblock addresses in batches of batch_sz.
 */

int
IPTABLES_fill_in_missing(MAP *h_rtn_map);
/**************************************************************
 * Fill in all blocked IP's which are not already in *h_map.
 */

#ifdef __cplusplus
}
#endif

#endif

