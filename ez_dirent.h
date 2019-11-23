
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
                          ez_dirent.h  -  description                              
dirent calls with boilerplate error handling.

                             -------------------
    begin                : Tue Nov 13 19:42:23 EST 2018
    email                : john@rrci.com                                     
 ***************************************************************************/
#ifndef EZ_DIRENT_H
#define EZ_DIRENT_H

#include <sys/types.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif


#define ez_opendir(name) \
   _ez_opendir(__FILE__, __LINE__, __FUNCTION__, name)
DIR* _ez_opendir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *name
      );

#define ez_closedir(dirp) \
   _ez_closedir(__FILE__, __LINE__, __FUNCTION__, dirp)
int _ez_closedir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      DIR *dirp
      );

#define ez_readdir(dirp) \
         _ez_readdir(__FILE__, __LINE__, __FUNCTION__, dirp)
struct dirent* _ez_readdir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      DIR *dirp
      );

#ifdef __cplusplus
}
#endif

#endif
