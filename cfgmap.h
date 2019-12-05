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
#ifndef CFGMAP_H
#define CFGMAP_H
#include <stdio.h>
#include <stddef.h>

#include "map.h"
#include "ptrvec.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===================== CFGMAP_ENTRY ========================*/
typedef struct {
   char *symbol;
   PTRVEC value_lst; /* Possible multiple values for a given symbol */
   unsigned nLookups;/* How many times this symbol has been looked up */
   unsigned symLen;
} CFGMAP_ENTRY;

unsigned int
CFGMAP_ENTRY_numValues(const CFGMAP_ENTRY * self);
/******************************************************************
 * Return the number of values stored for a given entry.
 */

const char*
CFGMAP_ENTRY_symbol(const CFGMAP_ENTRY * self);
/******************************************************************
 * Return the string for a the symbol.
 */

const char*
CFGMAP_ENTRY_value(const CFGMAP_ENTRY * self, unsigned ndx);
/******************************************************************
 * Return the string for a the value indicated by ndx.
 */

const char*
CFGMAP_ENTRY_value_sn(const CFGMAP_ENTRY * self, unsigned *sn_buf, unsigned ndx);
/******************************************************************
 * Return the string *and* serial number for a the value indicated by ndx.
 */

void
CFGMAP_ENTRY_print(const CFGMAP_ENTRY * self, FILE * fh);
/************************************************************************
 * Print information for debugging.
 */

/*================== CFGMAP =================================*/
typedef struct {
   enum {
      CFGMAP_NTUPLES_KNOWN_FLG = 1 << 0
   } flags;
   unsigned recurs_lvl;
   MAP entry_map;	  /* Table of all entries in map */
   PTRVEC curlyBlock_lst; /* Stack of curly brace block symbols */
   unsigned serial_no_seq,/* For assigning serial number to values */
            nTuples;
} CFGMAP;

#define CFGMAP_file_create(p, fname) \
  (CFGMAP_file_constructor((p)=malloc(sizeof(CFGMAP)), fname) ? (p) : ( p ? realloc(CFGMAP_destructor(p),0) : 0 ))
CFGMAP*
CFGMAP_file_constructor(CFGMAP * self, const char *fname);
/******************************************************************
 * Create a map from a configuration file.
 */

#define CFGMAP_create(p) \
  ((p)= (CFGMAP_constructor((p)=malloc(sizeof(CFGMAP))) ? (p) : ( p ? realloc(CFGMAP_destructor(p),0) : 0 )))
CFGMAP*
CFGMAP_constructor(CFGMAP * self);
/******************************************************************
 * Create an empty map.
 */

#define CFGMAP_destroy(s) \
  {if(CFGMAP_destructor(s)) {free(s); s=0;}}
void*
CFGMAP_destructor(CFGMAP * self);
/******************************************************************
 * Free resources associated with configuration file map.
 */

int
CFGMAP_file_read(CFGMAP * self, const char *fname);
/**************************************************************************
 * Read a file to populate the map.
 */

void
CFGMAP_print(CFGMAP * self, FILE * fh);
/******************************************************************
 * Print to stream for debugging purposes
 */

int
CFGMAP_visitAllSymbols(CFGMAP * self, int (*func) (const char *symbol, void *ctxt), void *ctxt);
/******************************************************************
 * Call (*func)() for all symbols in the map.
 */

size_t
CFGMAP_numUnused_symbols(CFGMAP * self);
/******************************************************************
 * Print unused symbol count.
 */

void
CFGMAP_print_unused_symbols(CFGMAP * self, FILE * fh);
/******************************************************************
 * Print unused symbols for troubleshooting
 */

const CFGMAP_ENTRY*
CFGMAP_find(CFGMAP * self, const char *symbol);
/************************************************************************
 * Try to locate a matching CFGMAP_ENTRY
 */

struct CFGMAP_tuple {
   const char *key,
              *value;
   unsigned serial_no;
};

unsigned
CFGMAP_find_tuples(CFGMAP * self, struct CFGMAP_tuple rtn_arr[], const char *symbol);
/************************************************************************
 * Find all tuples with a key matching symbol
 * returns the number of tuples which were populated.
 */

int
CFGMAP_regex_find(CFGMAP * self, const CFGMAP_ENTRY * rtn_arr[], const char *patternStr);
/************************************************************************
 * Find all instances of symbol which match patternStr.
 * Returns: the number of matches, or -1 for error.
 */

const char*
CFGMAP_find_single_value(CFGMAP * self, const char *symbol);
/************************************************************************
 * Find exactly one value for a symbol, or return NULL.
 */

const char*
CFGMAP_find_last_value(CFGMAP * self, const char *symbol);
/************************************************************************
 * Find the last value for a symbol, or return NULL.
 */

int
CFGMAP_append(CFGMAP * self, const char *symbol, unsigned int symLen, const char *value);
/**************************************************************************
 * Append a value to a config entry, creating a new entry if necessary.
 */

int
CFGMAP_query_uint(CFGMAP * self, unsigned int *pRtn, unsigned int dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience query function.
 */

int
CFGMAP_query_last_enum(
      CFGMAP * self,
      int *pRtn,
      int dfltVal,
      const char *symbol,
      const struct enumTuple et_arr[]
      );
/**********************************************************************************
 * Convenience query function for enums.
 */

int
CFGMAP_query_last_flags(
      CFGMAP * self,
      int *pRtn,
      int dfltVal,
      const char *symbol,
      const struct bitTuple bt_arr[]
      );
/**********************************************************************************
 * Convenience query function for possibly OR'd flags.
 */

int
CFGMAP_query_last_bool(CFGMAP * self, int *pRtn, int dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience function.
 */

int
CFGMAP_query_last_int(CFGMAP * self, int *pRtn, int dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience function.
 */

int
CFGMAP_query_last_uint(CFGMAP * self, unsigned *pRtn, unsigned dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience function.
 */

int
CFGMAP_query_last_dbl(CFGMAP * self, double *pRtn, double dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience function.
 */

int
CFGMAP_query_last_string(CFGMAP * self, char **pRtn, const char *dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience function.
 * Note: string assigned to *pRtn was allocated by strdup().
 */

int
CFGMAP_query_last_time_of_day(CFGMAP * self, int *pRtn, int dfltVal, const char *symbol);
/**********************************************************************************
 * Convenience function to convert hh:mm to a number of seconds.
 */

unsigned int
CFGMAP_numEntries(CFGMAP * self);
/******************************************************************
 * Return the number of values stored for a given entry.
 */

unsigned int
CFGMAP_numTuples(CFGMAP * self);
/******************************************************************
 * Return the number of tuples stored in the configuration map.
 */

int
CFGMAP_obtain_prefix(char *rtnBuf, size_t buf_sz, const char *path);
/******************************************************************
 * Given a full path string, place the prefix in rtnBuf.
 */

#ifdef __cplusplus
}
#endif
#endif
