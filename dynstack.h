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

#ifndef DS_H
#define DS_H

typedef struct _DS {
   unsigned size,
            count,
            elsize;
   char *stk_ptr;
} DS;


#define DS_currBuf_sz(s) \
  ((const unsigned)(s)->size)

#define DS_numItems(s)\
  ((const unsigned)(s)->count)
/* Return the number of items currently on the stack */

#define DS_ptr(s)\
  ((void*)(s)->stk_ptr)
/* Return the stack pointer */

#define DS_qsort(s, cmp_func)\
  qsort((s)->stk_ptr, (s)->count, (s)->elsize, cmp_func)
/* Sort the stack using qsort() */

#define DS_reset(s) \
  ((s)->count= 0)
/* Reset the stack so that no items are contained */


#ifdef __cplusplus
extern "C" {
#endif

#define DS_create(p, elmtSize, numElmts) \
  ((p)= (DS_constructor((p)=malloc(sizeof(DS)), elmtSize, numElmts) ? (p) : ( p ? realloc(DS_destructor(p),0) : 0 )))
DS*
DS_constructor(DS *self, unsigned int elmtSize, unsigned int numElmts);
/* Initialize the dynamic stack object.  You must provide the
 * size of the elements (in bytes) you wish to use the stack for, and
 * the approximate max number of element you think you will use.
 */

int
DS_sinit(DS *self, unsigned int elmtSize, unsigned int numElmts);
/* Initializion for static instance of DS. Initializes once per
 * process, and reset() all other times.
 */

#define DS_destroy(s) \
 {if(DS_destructor(s)) {free(s);s=0;}}
void*
DS_destructor(DS *self);
/* Close the dynamic stack, free all memory, etc. */


int
DS_top(DS *self, void *dest_ptr);
/* Copy the element at the top of the stack to the dest_ptr buffer
 * area.
 */

int
DS_pop(DS *self, void *dest_ptr);
/* Pop the top element off the stack and copy it to the dest_ptr
 * buffer area
 */

int
DS_push(DS *self, const void *src_ptr);
/* Push a new element on the top of the stack. */

int
DS_entry_exists(const DS *self, const void *target_ptr);
/* Return true if the item exists in the stack */

int
DS_visitAllEntries(DS *self, int (* func)(void *item_ptr, void *data), void *data);
/******************************************************************************
 * Visit all entries in the stack. if (*func) returns nonzero, then
 * the process stops and DS_visitAllEntries will return non-zero.
 */

#ifdef __cplusplus
}
#endif
#endif /* DS_H */
