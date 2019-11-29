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
#include <stdlib.h>

#include "ez_libz.h"
#include "util.h"

/***************************************************/
gzFile _ez_gzopen(
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *path,
      const char *mode
      )
{
   gzFile rtn= gzopen(path, mode);
   if(rtn) return rtn;

   _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzopen()");
   abort();
}

/***************************************************/
int _ez_gzclose(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file
      )
{
   int err= gzclose(file);
   if(Z_OK == err) return Z_OK;

   const char *msg="Unknow error";

   switch(err) {

      case Z_ERRNO:
         _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzclose()");
         abort();

      case Z_STREAM_ERROR:
         msg= "File not valid";
         break;

      case Z_MEM_ERROR:
         msg= "Out of memory";
         break;

      case Z_BUF_ERROR:
         msg= "Read ended in middle of a stream";
         break;

   }
   _eprintf(fileName, lineNo, funcName, "ERROR: gzclose() [ %s ]", msg);
   abort();
}

/***************************************************/
int _ez_gzwrite(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      voidpc buf,
      unsigned len
      )
{
   int n= gzwrite(file, buf, len);
   if(n == len) return n;

   int err;
   const char *str= gzerror(file, &err);
   if(Z_ERRNO == err) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzwrite()");
   } else {
      _eprintf(fileName, lineNo, funcName, "ERROR: gzwrite() [ %s ]", str);
   }
   abort();
}

/***************************************************/
int _ez_gzread(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      voidp buf,
      unsigned len
      )
{
   int n= gzread(file, buf, len);
   if(-1 != n) return n;

   int err;
   const char *str= gzerror(file, &err);
   if(Z_ERRNO == err) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzread()");
   } else {
      _eprintf(fileName, lineNo, funcName, "ERROR: gzread() [ %s ]", str);
   }
   abort();
}

/***************************************************/
int _ez_gzflush(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      int flush 
      )
{
   int err= gzflush(file, flush);
   if(Z_OK == err) return Z_OK;

   const char *str= gzerror(file, &err);
   if(Z_ERRNO == err) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzflush()");
   } else {
      _eprintf(fileName, lineNo, funcName, "ERROR: gzflush() [ %s ]", str);
   }
   abort();
}

/***************************************************/
z_off_t _ez_gzseek(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      z_off_t offset,
      int whence
      )
{
   z_off_t rtn= gzseek(file, offset, whence);
   if(-1 != rtn) return rtn;

   int err;
   const char *str= gzerror(file, &err);
   if(Z_ERRNO == err) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzseek()");
   } else {
      _eprintf(fileName, lineNo, funcName, "ERROR: gzseek() [ %s ]", str);
   }
   abort();
}

/***************************************************/
char* _ez_gzgets (
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      char *buf,
      int len
      )
{
   char *rtn= gzgets(file, buf, len);

   if(!rtn) {
      int err;
      const char *str= gzerror(file, &err);
      if(Z_OK != err && Z_STREAM_END != err) {
         if(Z_ERRNO == err) {
            _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "ERROR: gzgets()");
         } else {
            _eprintf(fileName, lineNo, funcName, "ERROR: gzgets() [ %s ]", str);
         }
         abort();
      }
   }

   return rtn;
}
