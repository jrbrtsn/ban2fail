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
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ez_libc.h"
#include "util.h"

#define MSGBUF_SZ 4096

static void
dflt_eprintf_line (const char *fmt, va_list ap)
/***************************************************************
 * default line vprintf()'ing function for eprintf() and friends.
 */
{
   vfprintf(stderr, fmt, ap);
   fputc ('\n', stderr);
   fflush (stderr);
}


/* Static data for all threads */
static struct {
   eprintf_line_f prt_f;
} S = {
   .prt_f= dflt_eprintf_line
};

eprintf_line_f
set_eprintf_line (eprintf_line_f newFunc)
/****************************************************
 * Set the global eprintf line vprintf()'ing function.
 */
{
   eprintf_line_f rtn= S.prt_f;
   S.prt_f= newFunc;
   return rtn;
}

/* Function to print out an error message. Usualy called by eprintf() macro. */
void _eprintf(
#ifdef DEBUG
      const char* filename,
      int lineno,
      const char *func,
#endif
      const char *fmt,
      ...
      )
/*********************************************************************************
 * Function to simplify printing error messages.
 */
{
   va_list args;
   char buf[MSGBUF_SZ];

#ifdef DEBUG
   /* Put the filename, lineno, and function name into a new format string */
   snprintf(buf, MSGBUF_SZ-1, "%s line %d, %s(): %s", filename, lineno, func, fmt);
#else
   snprintf(buf, MSGBUF_SZ-1, "%s", fmt);
#endif

   /* make sure buffer is null terminated */
   buf[MSGBUF_SZ-1]= '\0';


   /* Pass on doctored format string and varargs to vfprintf() */
   va_start(args, fmt);
   (* S.prt_f) (buf, args);
   va_end(args);
}

/* Function to print out an error message. Usualy called by sys_eprintf() macro. */
void _sys_eprintf(
      const char* (*strerror_f)(int errnum),
#ifdef DEBUG
      const char* filename,
      int lineno,
      const char *func,
#endif
      const char *fmt,
      ...
      )
/*********************************************************************************
 * Function to simplify printing error messages.
 */
{
   va_list args;
   char buf[MSGBUF_SZ];

   /* Put the filename, lineno, and function name into a new format string */
#ifdef DEBUG
   snprintf(buf, MSGBUF_SZ-1, "%s line %d, %s(): %s [ %s ]", filename, lineno, func, fmt, (*strerror_f)(errno));
#else
   snprintf(buf, MSGBUF_SZ-1, "%s [ %s ]", fmt, (*strerror_f)(errno));
#endif

   /* make sure buffer is null terminated */
   buf[MSGBUF_SZ-1]= '\0';


   /* Pass on doctored format string and varargs to vfprintf() */
   va_start(args, fmt);
   (* S.prt_f) (buf, args);
   va_end(args);
}

int64_t
timespec2ms(const struct timespec *ts)
/**********************************************************************
 * Convert a timespec structure to integer milliseconds.
 */ 
{
   return ts->tv_sec*1000 + ts->tv_nsec/1000000;
}

struct timespec*
ms2timespec(struct timespec *rtnBuf, int64_t ms)
/**********************************************************************
 * Load up a timespec struct given number of milliseconds.
 */ 
{
   rtnBuf->tv_sec= ms/1000;
   rtnBuf->tv_nsec= (ms%1000)*1000000;
   return rtnBuf;
}

const char*
bits2str(int64_t bits, const struct bitTuple *btArr)
/**********************************************************************
 * Returns  a null terminated buffer with OR'd set bits
 * printed out.
 */
{
   /* Rotating buffers so this can be used multiple times as arg to printf() */
#define N_BUFS 10
   static _Thread_local char bufArr[N_BUFS][1024];
   static _Thread_local unsigned count;
   const struct bitTuple *t;
   unsigned offset;
   char *buf= bufArr[++count%N_BUFS];

   /* Need this in case no bits are set */
   buf[0]= '\0';

   for(offset= 0, t= btArr; t->name; ++t) {
      if(!(bits & t->bit)) continue; 
      if(offset) offset += sprintf(buf+offset, "|");
      offset += sprintf(buf+offset, "%s", t->name);
   }
   return buf;
#undef N_BUFS
}

static const struct bitTuple*
findBitTuple(const char *symbol, const struct bitTuple *btArr)
/**********************************************************************
 * brute force search for matching symbol.
 */
{
   const struct bitTuple *bt;
   for(bt= btArr; bt->name; ++bt) {
      if(!strcmp(symbol, bt->name)) return bt;
   }

   return NULL;
}

int
str2bits(int64_t *rtnBuf, const char *str, const struct bitTuple *btArr)
/**********************************************************************
 * Convert all OR'd symbolic bits in str into the return value.
 *
 * RETURNS: 0 for success, -1 for error.
 */
{
   *rtnBuf= 0;
   if(!strlen(str)) return 0;

   int rtn= -1;
   char symbolArr[20][60];
   memset(symbolArr, 0, sizeof(symbolArr));

   int rc= sscanf(str,
         "%59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         "| %59[^|] "
         ,symbolArr[0]
         ,symbolArr[1]
         ,symbolArr[2]
         ,symbolArr[3]
         ,symbolArr[4]
         ,symbolArr[5]
         ,symbolArr[6]
         ,symbolArr[7]
         ,symbolArr[8]
         ,symbolArr[9]
         ,symbolArr[10]
         ,symbolArr[11]
         ,symbolArr[12]
         ,symbolArr[13]
         ,symbolArr[14]
         ,symbolArr[15]
         ,symbolArr[16]
         ,symbolArr[17]
         ,symbolArr[18]
         ,symbolArr[19]
         );

   if(-1 == rc) {
      sys_eprintf("ERROR: sscanf() failed");
      goto abort;
   }

   for(int i= 0; i < rc; ++i) {
      const struct bitTuple *bt= findBitTuple(symbolArr[i], btArr);
      if(!bt) {
         eprintf("ERROR: \"%s\" not found in bitTuple array.", symbolArr[i]);
         goto abort;
      }
      *rtnBuf |= bt->bit;
   }


   rtn= 0;

abort:
   return rtn;
}

void
printBuffer(FILE *fh, const char *buf)
/************************************************************************************
 * Print out the supplied buffer, replacing unprintable characters with the corresponding
 * hex value in pointy brackets, e.g. <0x01>
 */
{
   const char *pc;
   /* Print something for each character */
   for(pc= buf; pc && *pc; ++pc) {

      if(isprint(*pc)) { /* Character is printable */

         fputc(*pc, fh);

      } else { /* Character is NOT printable, show hex number instead */

         fprintf(fh, "<0x%02X>", *pc);

      }
   }
} 

int64_t
clock_gettime_ms(clockid_t whichClock)
/**********************************************************************
 * Returns current value of whichClock in milliseconds, avoiding the
 * need for struct timespec.
 * See man clock_gettime for more information.
 */
{
   struct timespec ts;
   if(-1 == clock_gettime(whichClock, &ts)) {
      sys_eprintf("\tclock_gettime() failed");
      abort();
   }
   return timespec2ms(&ts);
}

const char*
local_strftime (const time_t *pWhen, const char *fmt)
/***************************************************
 * Get local time in a static string buffer
 */
{
   /* Rotating buffers so this can be used multiple times as arg to printf() */
#define N_BUFS 5
#define BUF_SZ 64
   static _Thread_local char bufArr[N_BUFS][BUF_SZ];
   static _Thread_local unsigned count;
   char *buf= bufArr[++count%N_BUFS];

   /* Print the local time to a buffer */
   struct tm *tm= localtime (pWhen);
   if (!tm) {
      sys_eprintf ("localtime() failed");
      return NULL;
   }
   
   if (!strftime (buf, BUF_SZ-1, fmt, tm)) {
      sys_eprintf ("strftime(\"%s\") failed", fmt);
      return NULL;
   }
            
   return buf;
#undef BUF_SZ
#undef N_BUFS
}

const char*
strbits(int64_t bits, unsigned nBytes)
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
{
   /* Rotating buffers so this can be used multiple times as arg to printf() */
#define N_BUFS 8
#define BUF_SZ 65
   static _Thread_local char bufArr[N_BUFS][BUF_SZ];
   static _Thread_local unsigned count;
   char *buf= bufArr[++count%N_BUFS];
   unsigned pos= 0;

   /* order bits from MSbit -> LSbit */
   for(unsigned n= nBytes; n--;) {
      unsigned char byte= (bits >>(8*n)) << 56 >> 56;
      for(unsigned bit= 7; bit--;) {
         buf[pos]= byte & 1<<bit ? '1' : '0';
         ++pos;
      }
   }

   buf[pos]= '\0';
            
   return buf;
#undef BUF_SZ
#undef N_BUFS
}

int
fd_setNONBLOCK (int fd)
/***************************************************
 * Set a file descriptor to non-blocking mode.
 */
{
   int rtn= -1;

   /* Set for non-blocking I/O */
   int fcntlFlgs;
   /* This sets 'status' flags */
   if((fcntlFlgs= fcntl(fd, F_GETFL)) == -1 ||
      fcntl(fd, F_SETFL, fcntlFlgs | O_NONBLOCK) == -1 ||
      (fcntlFlgs= fcntl(fd, F_GETFD)) == -1 ||
      fcntl(fd, F_SETFD, fcntlFlgs | FD_CLOEXEC) == -1) {

     sys_eprintf("fcntl() failed");
     abort();
   }

  /* This sets 'descriptor' flags */
   fcntlFlgs= fcntl(fd, F_GETFD);
   if(fcntl(fd, F_SETFD, fcntlFlgs | FD_CLOEXEC) < 0) {
     sys_eprintf("fcntl() failed");
     abort();
   }

   rtn= 0;

abort:
   return rtn;

}

const struct enumTuple*
str2enum(const char *str, const struct enumTuple *etArr)
/**********************************************************************
 * Try to match str with an entry in the enumTupeArr.
 */
{
   const struct enumTuple *et;
   for(et= etArr; et->name; ++et) {
      if(!strcmp(str, et->name)) return et;
   }
   return NULL;
}

const char*
enum2str(int enumVal, const struct enumTuple *etArr)
/**********************************************************************
 * Return a the string representing enumVal, or NULL.
 */
{
   const struct enumTuple *et;
   for(et= etArr; et->name; ++et) {
      if(enumVal == et->enumVal) return et->name;
   }
   return NULL;
}

int
sleep_ms(unsigned msec)
/***************************************************
 * Sleep for the specified number of milliseconds.
 */
{
   struct timespec ts;
   ts.tv_sec= msec/1000;
   ts.tv_nsec= (msec%1000)*1000000;
   return nanosleep(&ts, NULL);
}

int
tcp_connect(const char *hostName, unsigned short port)
/***************************************************
 * Blocking TCP connect for convenience.
 */
{
   int rtn= -1, sock;
   struct hostent *hp;
   struct sockaddr_in sin;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(-1 == sock) {
     sys_eprintf("ERROR: socket() failed.");
     goto abort;
  }

  // Initialize socket address structure
  hp = gethostbyname(hostName);
  if(!hp) {
     sys_eprintf("ERROR: gethostbyname(\"%s\") failed.", hostName);
     goto abort;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&(sin.sin_addr.s_addr), hp->h_addr, hp->h_length);

  int err= connect(sock, (struct sockaddr *)&sin, sizeof(sin));
  if(-1 == err) {
     sys_eprintf("ERROR: connect(%s:%hu) failed.", hostName, port);
     goto abort;
  }

  rtn= sock;

abort:
  if(-1 == rtn) {
     if(-1 != sock) {
        ez_close(sock);
     }
  }
  return rtn;
}

int
tm_normalize(struct tm *buf)
/***************************************************
 * Normalize buf so member values are corrected.
 * Returns 0 for success, -1 for failure.
 */
{
   buf->tm_isdst= -1;
   time_t tt= mktime(buf);
   if(!localtime_r(&tt, buf)) {
      sys_eprintf ("localtime() failed");
      return -1;
   }
   return 0;
}

int
secs2tod(const time_t *pFrom, int tod_sec)
/***************************************************
 * Return the smallest number of seconds in the future
 * remaining between from and tod_sec time-of-day, without
 * respect to which day of the week/month/year.
 */
{
   assert(0 <= tod_sec && (23*3600+59*60+59) >= tod_sec);

   if(0 > tod_sec || (23*3600+59*60+59) < tod_sec) return -1;

   /* Get the local time breakdown in a buffer */
   struct tm tm_from;
   if(!localtime_r(pFrom, &tm_from)) {
      sys_eprintf ("localtime_r() failed");
      abort();
   }

   /* We'll look at today and tomorrow */
   for(int day= 0; day <= 1; ++day) {

      struct tm tm= tm_from;

      tm.tm_mday += day;
      if(-1 == tm_normalize(&tm)) assert(0);

      tm.tm_hour= tod_sec / 3600;
      tm.tm_min= (tod_sec % 3600) / 60;
      tm.tm_sec= (tod_sec % 3600) % 60;

      time_t dptime= timelocal(&tm);
      if(dptime > *pFrom) return (int)(dptime - *pFrom);
   }

   assert(0);

   /* Can't find a time (should be impossible to get here) */
   return -1;
}

const char*
indentStr(unsigned lvl, const char *pfix)
/***************************************************
 * Return a string with lvl concatenated pfix strings.
 */
{
#define N_BUFS 10
#define BUF_SZ 128
   static _Thread_local char bufArr[N_BUFS][BUF_SZ];
   static _Thread_local unsigned count;
   char *buf= bufArr[++count%N_BUFS];
   size_t len= strlen(pfix);

   buf[0]= '\0';
   for(unsigned i= 0; (i+1)*len < BUF_SZ && i < lvl; ++i) {
      strncpy(buf+i*len, pfix, len+1);
   }

   return buf;

#undef BUF_SZ
#undef N_BUFS
}

int
regex_compile(regex_t *preg, const char *pattern, int cflags)
/***************************************************
 * Regular expression compile with error reporting.
 */
{
   int rc, rtn= -1;

   rc= regcomp(preg, pattern, cflags);
   if(rc) {
#define BUF_SZ 1024
      char buf[BUF_SZ];
      regerror(rc, preg, buf, BUF_SZ);
      eprintf("ERROR: %s", buf);
      goto abort;
#undef BUF_SZ
   }

   rtn= 0;

abort:
   return rtn;
}

FILE*
pager_open(void)
/***************************************************
 * popen() the caller's pager.
 */
{
   char *cmd= getenv("PAGER");
   if(!cmd)
      cmd= "/bin/more";

   return ez_popen(cmd, "w");
}

const char*
prefix_home(const char *fname)
/***************************************************
 * return $HOME/fname in a static buffer.
 */
{
#define N_BUFS 4
#define BUF_SZ PATH_MAX+1
   static _Thread_local char bufArr[N_BUFS][BUF_SZ];
   static _Thread_local unsigned count;
   char *buf = bufArr[++count % N_BUFS];

   const char *home_env= getenv("HOME");

   if(home_env) {
      snprintf(buf, BUF_SZ-1, "%s/%s", home_env, fname);
   } else {
      snprintf(buf, BUF_SZ-1, "%s", fname);
   }

   return buf;

#undef BUF_SZ
#undef N_BUFS
}

const char*
pthread_t_str(pthread_t tid)
/***************************************************
 * return a hexidecimal representation of tid in
 * a rotating static buffer.
 */
{
#define N_BUFS 10
#define BUF_SZ (sizeof(tid)*2+3)
   static _Thread_local char bufArr[N_BUFS][BUF_SZ];
   static _Thread_local unsigned count;
   char *buf = bufArr[++count % N_BUFS];

   strcpy(buf, "0x");
   char *dst= buf+2;

   unsigned i, byte;
   size_t sz= sizeof(tid);
   for(i= 0; i < sz; ++i) {
      byte= *((unsigned char *)(&tid) + sz - i - 1);
      dst += sprintf(dst, "%02x", byte);
   }
   *dst= '\0';

   return buf;

#undef BUF_SZ
#undef N_BUFS
}


char*
skipspace(char *str)
/***************************************************
 * return first character in str which is not
 * whitespace, which could be the terminating null.
 */
{
   for(; *str && isspace(*str); ++str);
   return str;
}

const char*
skipspacec(const char *str)
/***************************************************
 * return first character in str which is not
 * whitespace, which could be the terminating null.
 */
{
   for(; *str && isspace(*str); ++str);
   return str;
}

char*
trimend(char *str)
/***************************************************
 * Trim space on end of string by replacing it with
 * null bytes.
 */
{
   size_t len= strlen(str);
   for(; len && isspace(str[len-1]); str[--len]= '\0');
   return str;
}

void
julian_2_gregorian(int *year, int *month, int *day, uint32_t julianDay)
/************************************************************************
 * Gregorian calendar starting from October 15, 1582
 * This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
 */
{
  uint64_t ell, n, i, j;

  ell = (uint64_t)julianDay + 68569;
  n = (4 * ell) / 146097;
  ell = ell - (146097 * n + 3) / 4;
  i = (4000 * (ell + 1)) / 1461001;
  ell = ell - (1461 * i) / 4 + 31;
  j = (80 * ell) / 2447;
  *day = ell - (2447 * j) / 80;
  ell = j / 11;
  *month = j + 2 - (12 * ell);
  *year = 100 * (n - 49) + i + ell;
}

int32_t
gregorian_2_julian(int year, int month, int day)
/* Gregorian calendar starting from October 15, 1582
 * Algorithm from Henry F. Fliegel and Thomas C. Van Flandern
 */
{
  return (1461 * (year + 4800 + (month - 14) / 12)) / 4
         + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
         - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
         + day - 32075;
}

const char*
bytes_2_hexStr(
      char *rtn_sbuf,
      unsigned rtn_sz,
      const unsigned char *src,
      unsigned src_len
      )
/***************************************************************
 * Convert bytes to hex characters, place in rtn_sbuf. 
 */
{
   if(src_len*2 >= rtn_sz)
      return NULL;

   unsigned i;
   for(i= 0; i < src_len; ++i) {
      snprintf(rtn_sbuf+i*2, rtn_sz-i*2, "%02x", (int)src[i]);
   }
   /* Null terminate the result */
   rtn_sbuf[i*2]= '\0';
   return rtn_sbuf;
}

int
rmdir_recursive(const char *path)
/***************************************************************
 * recursively remove a directory and all of it's contents.
 * RETURNS:
 * 0 for success
 * -1 for error
 */
{
   int rtn= -1;

   struct stat st;
   DIR *dir= ez_opendir(path);
   struct dirent *entry;
   STR sb;
   STR_constructor(&sb, PATH_MAX);

   while((entry= ez_readdir(dir))) {
      /* Skip uninteresting entries */
      if(!strcmp(".", entry->d_name) ||
         !strcmp("..", entry->d_name)) continue;

      STR_reset(&sb);
      STR_sprintf(&sb, "%s/%s", path, entry->d_name);

      ez_stat(STR_str(&sb), &st);
      if (S_ISDIR(st.st_mode)) {
         int rc= rmdir_recursive(STR_str(&sb));
         if(rc)
            goto abort;
      } else {
         ez_unlink(STR_str(&sb));
      }
   }

   rtn= 0;
   
abort:
   if(!rtn)
      ez_rmdir(path);

   STR_destructor(&sb);
   return rtn;
}

int _ez_rmdir_recursive (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname
      )
/***************************************************************
 * ez version of rmdir_recursive().
 */
{
   int rtn= rmdir_recursive (pathname);
   if (-1 == rtn) {
      _eprintf(fileName, lineNo, funcName, "rmdir_recursive(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/*============================================================*/
/*================ struct addrinfo ===========================*/
/*============================================================*/

const static struct bitTuple ai_flags_btArr[]= {
   {.name= "AI_ADDRCONFIG", .bit= AI_ADDRCONFIG},
   {.name= "AI_ALL", .bit= AI_ALL},
   {.name= "AI_CANONNAME", .bit= AI_CANONNAME},
   {.name= "AI_NUMERICHOST", .bit= AI_NUMERICHOST},
   {.name= "AI_NUMERICSERV", .bit= AI_NUMERICSERV},
   {.name= "AI_PASSIVE", .bit= AI_PASSIVE},
   {.name= "AI_V4MAPPED", .bit= AI_V4MAPPED},
   {}
};

const static struct enumTuple ai_family_etArr[]= {
   {.name= "AF_INET", .enumVal= AF_INET},
   {.name= "AF_INET6", .enumVal= AF_INET6},
   {.name= "AF_UNSPEC", .enumVal= AF_UNSPEC},
   {}
};

const static struct enumTuple ai_socktype_etArr[]= {
   {.name= "SOCK_DGRAM", .enumVal= SOCK_DGRAM},
   {.name= "SOCK_RAW", .enumVal= SOCK_RAW},
   {.name= "SOCK_STREAM", .enumVal= SOCK_STREAM},
   {}
};

const static struct enumTuple ai_protocol_etArr[]= {
   {.name= "IPPROTO_TCP", .enumVal= IPPROTO_TCP},
   {.name= "IPPROTO_UDP", .enumVal= IPPROTO_UDP},
   {}
};


int
addrinfo_print(struct addrinfo *ai, FILE *fh)
/*************************************************************
 * Print a legible rendition of all struct addrinfo in the
 * linked-list chain.
 *
 * ai:    pinter to a struct addrinfo
 * fh:    stream for output
 *
 * RETURNS:
 * 0 for success
 * -1 for error
 */
{
   for(; ai; ai= ai->ai_next) {
      const char *addr= addrinfo_2_addr(ai);
      ez_fprintf(fh,
"struct addressinfo {\n"
"\tai_flags= %s\n"
"\tai_family= %s\n"
"\tai_socktype= %s\n"
"\tai_protocol= %s\n"
"\tai_addrlen= %d\n"
"\tai_addr= %s\n"
"\tai_cannonname= %s\n"
"}\n"
      , bits2str(ai->ai_flags, ai_flags_btArr)
      , enum2str(ai->ai_family, ai_family_etArr)
      , enum2str(ai->ai_socktype, ai_socktype_etArr)
      , enum2str(ai->ai_protocol, ai_protocol_etArr)
      , (int)ai->ai_addrlen
      , addr ? addr : "NULL"
      , ai->ai_canonname ? ai->ai_canonname : "NULL"
      );

   }
   return 0;
}

int
addrinfo_is_match(const struct addrinfo *ai, const char *addr)
/***********************************************************************
 * Search all members in addrinfo linked list for a match.
 *
 * ai:    pinter to a struct addrinfo
 * addr:  NULL terminated numeric address for strcmp() comparison
 *
 * RETURNS:
 * 1  a match was found
 * 0  no match was found.
 */
{
   for(; ai; ai= ai->ai_next) {
      const char *this_addr= addrinfo_2_addr(ai);
      if(!strcmp(this_addr, addr))
         return 1;
   }
   return 0;
}

const char*
addrinfo_2_addr(const struct addrinfo *ai)
/***********************************************************************
 * Return in a static buffer a sting version of the numeric address found
 * in a single addrinfo struct.
 *
 * ai:    pinter to a struct addrinfo
 *
 * RETURNS:
 * NULL for failure
 * address of the static buffer containing address in null terminated string form.
 */
{
   /* Rotating buffers so this can be used multiple times as arg to printf() */
#define N_BUFS 5
#define BUF_SZ 43
   const char *rtn= NULL;
   if(!ai->ai_addr) goto abort;

   static _Thread_local char bufArr[N_BUFS][BUF_SZ];
   static _Thread_local unsigned count;
   char *buf= bufArr[++count%N_BUFS];

   memset(buf, 0, BUF_SZ);

   switch(ai->ai_family) {
      case AF_INET: {
            struct sockaddr_in *sin= (struct sockaddr_in*)ai->ai_addr;
            rtn= inet_ntop(AF_INET, &sin->sin_addr, buf, BUF_SZ-1);
         } break;

      case AF_INET6: {
            struct sockaddr_in6 *sin6= (struct sockaddr_in6*)ai->ai_addr;
            rtn= inet_ntop(AF_INET6, &sin6->sin6_addr, buf, BUF_SZ-1);
         } break;

      default:
         assert(0);

   }

abort:
   return rtn;
#undef BUF_SZ
#undef N_BUFS
}

