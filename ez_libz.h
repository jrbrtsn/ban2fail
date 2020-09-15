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
#ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#endif
#include <zlib.h>

#include "ez.h"

#ifdef __cplusplus
extern "C" {
#endif

ez_proto (gzFile, gzopen,
      const char *path,
      const char *mode);
#ifdef DEBUG
#       define ez_gzopen(...) \
   _ez_gzopen(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzopen(...) \
   _ez_gzopen(__VA_ARGS__)
#endif

ez_proto (int, gzclose,
      gzFile file);
#ifdef DEBUG
#       define ez_gzclose(...) \
   _ez_gzclose(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzclose(...) \
   _ez_gzclose(__VA_ARGS__)
#endif

ez_proto (int, gzwrite,
      gzFile file,
      voidpc buf,
      unsigned len);
#ifdef DEBUG
#       define ez_gzwrite(...) \
   _ez_gzwrite(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzwrite(...) \
   _ez_gzwrite(__VA_ARGS__)
#endif

ez_proto (int, gzread,
      gzFile file,
      voidp buf,
      unsigned len);
#ifdef DEBUG
#       define ez_gzread(...) \
   _ez_gzread(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzread(...) \
   _ez_gzread(__VA_ARGS__)
#endif

ez_proto (int, gzflush,
      gzFile file,
      int flush);
#ifdef DEBUG
#       define ez_gzflush(...) \
   _ez_gzflush(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzflush(...) \
   _ez_gzflush(__VA_ARGS__)
#endif

ez_proto (z_off_t, gzseek,
      gzFile file,
      z_off_t offset,
      int whence);
#ifdef DEBUG
#       define ez_gzseek(...) \
   _ez_gzseek(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzseek(...) \
   _ez_gzseek(__VA_ARGS__)
#endif

ez_proto (z_off_t, _ez_gztell,
      gzFile file);
#ifdef DEBUG
#       define ez_gztell(...) \
   _ez_gztell(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gztell(...) \
   _ez_gztell(__VA_ARGS__)
#endif

ez_proto (char*, gzgets, 
      gzFile file,
      char *buf,
      int len);
#ifdef DEBUG
#       define ez_gzgets(...) \
   _ez_gzgets(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gzgets(...) \
   _ez_gzgets(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif


#endif
