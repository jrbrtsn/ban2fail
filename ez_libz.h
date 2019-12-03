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
#ifndef EZ_GZFILE_H
#define EZ_GZFILE_H

/* Simplified interface to libz file functions */
#define _GNU_SOURCE
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ez_gzopen(path, mode) \
   _ez_gzopen(__FILE__, __LINE__, __FUNCTION__, path, mode)
gzFile _ez_gzopen(
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *path,
      const char *mode
      );

#define ez_gzclose(file) \
   _ez_gzclose(__FILE__, __LINE__, __FUNCTION__, file)
int _ez_gzclose(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file
      );

#define ez_gzwrite(file, buf, len) \
   _ez_gzwrite(__FILE__, __LINE__, __FUNCTION__, file, buf, len)
int _ez_gzwrite(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      voidpc buf,
      unsigned len
      );

#define ez_gzread(file, buf, len) \
   _ez_gzread(__FILE__, __LINE__, __FUNCTION__, file, buf, len)
int _ez_gzread(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      voidp buf,
      unsigned len
      );

#define ez_gzflush(file, flush) \
   _ez_gzflush(__FILE__, __LINE__, __FUNCTION__, file, flush)
int _ez_gzflush(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      int flush 
      );

#define ez_gzseek(file, offset, whence) \
   _ez_gzseek(__FILE__, __LINE__, __FUNCTION__, file, offset, whence)
z_off_t _ez_gzseek(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      z_off_t offset,
      int whence
      );

#define ez_gztell(file) \
   _ez_gztell(__FILE__, __LINE__, __FUNCTION__, file)
z_off_t _ez_gztell(
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file
      );

#define ez_gzgets(file, buf, len) \
   _ez_gzgets(__FILE__, __LINE__, __FUNCTION__, file, buf, len)
char* _ez_gzgets (
      const char *fileName,
      int lineNo,
      const char *funcName,
      gzFile file,
      char *buf,
      int len
      );

#ifdef __cplusplus
}
#endif


#endif
