/***************************************************************************
 *   Copyright (C) 2018 by John D. Robertson                               *
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
/***************************************************************************
                          util.h  -  description                              
Common utility routines needed by most c and c++ applications.

                             -------------------
    begin                : Fri Oct 19 10:09:38 EDT 2018
    email                : john@rrci.com                                     
 ***************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "str.h"

#ifndef MAX
#       define MAX(a,b) \
   ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#       define MIN(a,b) \
   ((a) < (b) ? (a) : (b))
#endif

#ifndef stringify
#       define _stringify(a) #a
#       define stringify(a)  _stringify(a)
#endif

#ifndef member_size
#       define member_size(type,member) \
         sizeof(((type*)0)->member)
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#       define ERRNO_CHECK \
         if(errno) { \
            sys_eprintf("ERRNO= %d", errno);\
            fflush(NULL);\
            abort();\
         }
#else
#       define ERRNO_CHECK
#endif

/* For readability */
typedef void (*eprintf_line_f) (const char*, va_list);

eprintf_line_f
set_eprintf_line (eprintf_line_f newFunc);
/****************************************************
 * Set the global eprintf line vprintf()'ing function.
 * This is useful for sending error messages to places
 * other than stderr.
 *
 * RETURNS: the previous global eprintf line vprintf()'ing function.
 */


/* Macro to conveniently create error messages which contain source filename, lineno, and funcname. */
#ifdef DEBUG
#       define eprintf(fmt, ...) \
         _eprintf(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#       define eprintf(fmt, ...) \
         _eprintf(fmt, ##__VA_ARGS__)
#endif

/* Function to print out an error message. Usually called by eprintf() macro. */
void _eprintf(
#ifdef DEBUG
      const char* filename,
      int lineno,
      const char *func,
#endif
      const char *fmt,
      ...
      );


/* Macro to conveniently create error messages which contain source filename, lineno, and funcname. */
#ifdef DEBUG
#       define sys_eprintf(fmt, ...) \
         _sys_eprintf((const char*(*)(int))strerror, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#       define sys_eprintf(fmt, ...) \
         _sys_eprintf((const char*(*)(int))strerror, fmt, ##__VA_ARGS__)
#endif

/* Function to print out an error message and strerror(errno). Usualy called by sys_eprintf() macro. */
void _sys_eprintf(
      const char* (*strerror_f)(int errnum), /* pass in which strerror() to use, for ZeroMQ support */
#ifdef DEBUG
      const char* filename,
      int lineno,
      const char *func,
#endif
      const char *fmt,
      ...
      );

#if 0
/* Special verion for working with ZeroMQ */
#ifdef DEBUG
#       define zys_eprintf(fmt, ...) \
         _sys_eprintf(zmq_strerror, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#       define zys_eprintf(fmt, ...) \
         _sys_eprintf(zmq_strerror, fmt, ##__VA_ARGS__)
#endif
#endif


/* Forward declaration */
struct timespec;

int64_t
timespec2ms(const struct timespec *ts);
/**********************************************************************
 * Convert a timespec structure to integer milliseconds.
 */ 

struct timespec*
ms2timespec(struct timespec *rtnBuf, int64_t ms);
/**********************************************************************
 * Load up a timespec struct given number of milliseconds.
 */ 

int64_t
clock_gettime_ms(clockid_t whichClock);
/**********************************************************************
 * Returns current value of whichClock in milliseconds, avoiding the
 * need for struct timespec.
 * See man clock_gettime for more information.
 */

/* Need to fill out an array of these to use bitsString() */
struct bitTuple {
   const char *name; // Make this NULL to terminate your array
   int64_t bit; /* Not bit position; one and only one bit set */
};

const char*
bits2str(int64_t bits, const struct bitTuple *btArr);
/**********************************************************************
 * Convert a bit field into a readable string.
 * Uses rotating per-thread static buffer for return, so it is safe
 * to call multiple times in a single printf()ish invocation.
 *
 * bits: bit field of interest.
 * btArr: array of struct bitTuple to map bits to strings.
 *
 * RETURNS:
 * null terminated buffer with a string representing the OR'd set bits
 */

int
str2bits(int64_t *rtnBuf, const char *str, const struct bitTuple *btArr);
/**********************************************************************
 * Convert all OR'd symbolic bits in str into the return value.
 *
 * RETURNS: 0 for success, -1 for error.
 */


/* Need to fill out an array of these to use str2enum() */
struct enumTuple {
   const char *name; // Make this NULL to terminate your array
   int enumVal; /* enum corresponding to name */
};

const struct enumTuple*
str2enum(const char *str, const struct enumTuple *etArr);
/**********************************************************************
 * Try to match str with an entry in the enumTupeArr.
 *
 * RETURNS: address of match struct enumTuple, or NULL if none are found.
 */

const char*
enum2str(int enumVal, const struct enumTuple *etArr);
/**********************************************************************
 * Return a the string representing enumVal, or NULL.
 */

void
printBuffer(FILE *fh, const char *buf);
/************************************************************************************
 * Print out the supplied buffer, replacing unprintable characters with the corresponding
 * hex value in pointy brackets, e.g. <0x01>
 */

const char*
local_strftime (const time_t *pWhen, const char *fmt);
/***************************************************
 * Get local time in a static string buffer. Format
 * string is passed to strftime().
 */

int
fd_setNONBLOCK (int fd);
/***************************************************
 * Set a file descriptor to non-blocking mode.
 */

int
sleep_ms(unsigned msec);
/***************************************************
 * Sleep for the specified number of milliseconds.
 */

int
tcp_connect(const char *hostName, unsigned short port);
/***************************************************
 * Blocking TCP connect for convenience.
 */

int
tm_normalize(struct tm *buf);
/***************************************************
 * Normalize buf so member values are corrected.
 * Returns 0 for success, -1 for failure.
 */

int
secs2tod(const time_t *pFrom, int tod_sec);
/***************************************************
 * Return the smallest number of seconds in the future
 * remaining between from and tod_sec time-of-day, without
 * respect to which day of the week/month/year.
 * RETURN -1 for error, or seconds until tod_secs.
 */

const char*
indentStr(unsigned lvl, const char *pfix);
/***************************************************
 * Return a string with lvl concatenated pfix strings.
 */

int
regex_compile(regex_t *preg, const char *pattern, int cflags);
/***************************************************
 * Regular expression compile with error reporting.
 */

FILE*
pager_open(void);
/***************************************************
 * popen() the caller's $PAGER. Calling pclose()
 * will then wait for pager to finish, and subsequently
 * close the stream.
 */

const char*
prefix_home(const char *fname);
/***************************************************
 * return $HOME/fname in a static buffer.
 */

char*
skipspace(char *str);
/***************************************************
 * return first character in str which is not
 * whitespace, which could be the terminating null.
 */

const char*
skipspacec(const char *str);
/*************************************************
 * same as skipspace(), except using const types.
 */


char*
trimend(char *str);
/***************************************************
 * Trim space on end of string by replacing it with
 * null bytes.
 */

#define trim(str) \
   skipspace(trimend(str))

const char*
pthread_t_str(pthread_t tid);
/***************************************************
 * return a hexidecimal representation of tid in
 * a rotating static buffer.
 */

void
julian_2_gregorian(int *year, int *month, int *day, uint32_t julianDay);
/************************************************************************
 * Gregorian calendar starting from October 15, 1582
 * This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
 */

int32_t
gregorian_2_julian(int year, int month, int day);
/* Gregorian calendar starting from October 15, 1582
 * Algorithm from Henry F. Fliegel and Thomas C. Van Flandern
 */

const char*
strbits(int64_t bits, unsigned nBytes);
/**********************************************************************
 * Convert a bits to a null terminated string of '1' and '0' characters.
 * Uses rotating per-thread static buffer for return, so it is safe
 * to call multiple times in a single printf()ish invocation.
 *
 * bits: bit field of interest.
 * nBytes: Number of bytes to consider.
 *
 * RETURNS:
 * null terminated buffer with a string representing bits as '0' or '1'
 */

const char*
bytes_2_hexStr(
      char *rtn_sbuf,
      unsigned rtn_sz,
      const unsigned char *src,
      unsigned src_len
      );
/***************************************************************
 * Convert bytes to hex characters, place null terminated string
 * in rtn_sbuf. 
 */

int
rmdir_recursive(const char *path);
/***************************************************************
 * recursively remove a directory and all of it's contents.
 * RETURNS:
 * 0 for success
 * -1 for error
 */

#define ez_rmdir_recursive(path) \
         _ez_rmdir_recursive(__FILE__, __LINE__, __FUNCTION__, path)
int _ez_rmdir_recursive (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *path
      );

#ifdef __cplusplus
}
#endif

#endif
