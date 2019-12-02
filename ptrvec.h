/***************************************************************************
 *   Copyright (C) 2008 by John D. Robertson                               *
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
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PTRVEC_H
#define PTRVEC_H

typedef struct
{
  void **ptrArr;
  unsigned int maxItems, numItems, head, tail;

  void **sortBuf;
  unsigned sortBuf_sz;
}
PTRVEC;

#ifdef __cplusplus
extern "C"
{
#endif

#define PTRVEC_arr(self) \
  ((self)->ptrArr)

#define PTRVEC_is_init(self) \
  PTRVEC_arr(self)

int
PTRVEC_sinit(PTRVEC *self, unsigned initMaxItems);
/***********************************************
 * Static initialization call.
 */

PTRVEC *PTRVEC_constructor (PTRVEC * self, unsigned int initMaxItems);
/***********************************************
 * Construct a PTRVEC.
 *
 * initMaxItems - a guess at how many items it will need to hold.
 * returns - pointer to the object, or NULL for failure.
 */

#define PTRVEC_create(p, initMaxItems) \
  ((p)=(PTRVEC_constructor((p)=malloc(sizeof(PTRVEC)), initMaxItems) ? (p) : ( p ? realloc(PTRVEC_destructor(p),0) : 0 )))

void *PTRVEC_destructor (PTRVEC * self);
/***********************************************
 * Destruct a PTRVEC.
 */

#define PTRVEC_destroy(self) \
  {if(PTRVEC_destructor(self)) {free(self);}}

void *PTRVEC_addHead (PTRVEC * self, void *ptr);
/***********************************************
 * Add an item to the head of the 'list'.
 *
 * ptr - pointer to the item to add.
 * returns - pointer to the node added, or NULL for failure.
 */

void *PTRVEC_remHead (PTRVEC * self);
/************************************************
 * Remove an item from the head of the 'list'.
 *
 * returns - pointer to the item, or NULL if the 'list' is emtpy.
 */

void *PTRVEC_addTail (PTRVEC * self, void *ptr);
/***********************************************
 * Add an item to the tail of the 'list'.
 *
 * ptr - pointer to the item to add.
 * returns - pointer to the node added, or NULL for failure.
 */

void *PTRVEC_remTail (PTRVEC * self);
/************************************************
 * Remove an item from the tail of the 'list'.
 *
 * returns - pointer to the item, or NULL if the 'list' is emtpy.
 */

int PTRVEC_resize (PTRVEC * self, unsigned int maxItems);
/************************************************
 * Resize the vector.  This is normally done automatically
 * for you.
 *
 * maxItems - the new number of items to use in the list.
 * returns - nonzero if there is not enough memory, or if maxItems < self->numItems.
 */

int PTRVEC_sort (PTRVEC * self, int (*cmp) (const void *const*, const void *const*));
/************************************************
 * Use qsort() to sort the items in the vector according to the cmp function.
 *
 * cmp - function pointer to use for comparison.
 * returns - nonzero for failure (not enough memory for temporary block)
 */

int PTRVEC_find (PTRVEC * self, unsigned int *ndxBuf, void *item);
/************************************************
 * Searches for an item.  If found, returns 0 and writes index number of
 * item into ndxBuf. if ndxBuf is not NULL.
 * returns 1 if found, 0 if not found.
 */

void *PTRVEC_remove (PTRVEC * self, void *item);
/***************************************************
 * Remove an item from the PTRVEC.
 */

#ifdef DEBUG
void
PTRVEC_assert_integrity(PTRVEC *self);
/***************************************************
 * Perform internal integrity checks for corruption.
 */
#endif

#define PTRVEC_reset(self) \
   (self)->numItems= (self)->head= (self)->tail= 0
/************************************************
 * void PTRVEC_reset(PTRVEC *self);
 * Reset the 'list' to contain no items.
 */

#define PTRVEC_numItems(self) \
   (self)->numItems
/************************************************
 * unsigned int PTRVEC_numItems(PTRVEC *self);
 *
 * Return the number of items in the 'list'.
 */

#define PTRVEC_ndxPtr(self, ndx) \
   (ndx>=(self)->numItems?0:(self)->ptrArr[((self)->head+ndx)%(self)->maxItems])
/*************************************************
 * void *PTRVEC_ndxPtr(PTRVEC *self, unsigned int ndx);
 *
 * get a pointer given it's position in the 'array'.
 *
 * ndx - index of the position in the 'array'
 * returns - pointer, or NULL if the ndx is invalid
 */

#define PTRVEC_first(self) \
   ((self)->numItems?(self)->ptrArr[(self)->head]:0)
/*******************************************************
 * void *PTRVEC_first(PTRVEC *self);
 * gets the first pointer in the vector.
 *
 * returns - pointer to the first item, or NULL if the vector is emtpy.
 */

#define PTRVEC_last(self) \
   ((self)->numItems?(self)->ptrArr[(self)->tail]:0)
/*******************************************************
 * void *PTRVEC_last(PTRVEC *self);
 * gets the last pointer in the vector.
 *
 * returns - pointer to the last item, or NULL if the vector is emtpy.
 */


#ifdef __cplusplus

/* Macros for traversing vector in list like fashion */
#define PTRVEC_loopFwd(self, i, ptr) \
   for(i=0, ptr= (decltype(ptr))(self)->ptrArr[(self)->head];\
       i < (self)->numItems;\
       ++i, ptr= (decltype(ptr))(self)->ptrArr[((self)->head+i)%(self)->maxItems])

#define PTRVEC_loopBkwd(self, i, ptr) \
   for(i=0, ptr= (decltype(ptr))(self)->ptrArr[(self)->tail];\
       i < (self)->numItems;\
       ++i, ptr= (decltype(ptr))(self)->ptrArr[((self)->tail+(self)->maxItems-i)%(self)->maxItems])
#else

/* Macros for traversing vector in list like fashion */
#define PTRVEC_loopFwd(self, i, ptr) \
   for(i=0, ptr= (typeof(ptr))(self)->ptrArr[(self)->head];\
       i < (self)->numItems;\
       ++i, ptr= (typeof(ptr))(self)->ptrArr[((self)->head+i)%(self)->maxItems])

#define PTRVEC_loopBkwd(self, i, ptr) \
   for(i=0, ptr= (typeof(ptr))(self)->ptrArr[(self)->tail];\
       i < (self)->numItems;\
       ++i, ptr= (typeof(ptr))(self)->ptrArr[((self)->tail+(self)->maxItems-i)%(self)->maxItems])

#endif

#ifdef __cplusplus
}
#endif

#endif
