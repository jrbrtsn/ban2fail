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
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include "dynstack.h"

#define elAddress(s, ndx)\
  ((s)->stk_ptr + (ndx) * (s)->elsize)


DS*
DS_constructor(DS *self, unsigned int elmtSize, unsigned int numElmts )
/******************************************************************
 * Open a dynamic stack, and get a key with which to interact with the stack.
 */
{
  if(!self) return NULL;
  memset(self, 0, sizeof(*self));

  if(!( self->stk_ptr = malloc( elmtSize*numElmts ) )) return NULL;

  self->elsize = elmtSize;
  self->size = numElmts;

  return self;
}

int
DS_sinit(DS *self, unsigned int elmtSize, unsigned int numElmts)
/****************************************************************
 * Initializion for static instance of DS. Initializes once per
 * process, and reset() all other times.
 */
{
  int rtn= 1;
  if(!self->stk_ptr) {
    if(!DS_constructor(self, elmtSize, numElmts)) goto abort;
  } else {
    DS_reset(self);
  }
  rtn= 0;
abort:
  return rtn;
}

void*
DS_destructor(DS *self )
/******************************************************************
 * Close the dynamic stack and free all memory.
 */
{
   if(self->stk_ptr) free( self->stk_ptr );
   return self;
}

static void*
top( DS *self )
/********************* top() ***********************************
 * return a pointer to the top of the stack.
 */
{
   if(!self->count) return NULL;
   return (void *)elAddress( self, self->count - 1);
}

#define copy(s, dst, src)\
  memcpy(dst, src, (s)->elsize)

int
DS_top( DS *self, void *dest_ptr )
/******************** DS_top() ************************************
 * Pop the item off the top of the stack.
 */
{
  void *src_ptr;

  if(!self->count) return -1;

  src_ptr= (void*)elAddress(self, self->count - 1);

  /* Get a copy of the stack item */
  copy(self, dest_ptr, src_ptr);

  return 0;
}

int
DS_pop( DS *self, void *dest_ptr )
/******************** DS_pop() ************************************
 * Pop the item off the top of the stack.
 */
{

   /* Try to get a copy of the top stack item */
   if(DS_top(self, dest_ptr)) return -1;

   /* decrement the stack pointer */
   --self->count;

   return 0;
}

int
DS_push( DS *self, const void *src_ptr )
/******************** DS_push() ***********************************
 * Push an item on to the dynamic stack.
 */
{
  char *dest_ptr;

   /* If more memory is needed, get it */
   if(self->count == self->size) {
     unsigned new_sz= self->size*2;
     char *ptr;

     if(!(ptr = realloc( self->stk_ptr, self->elsize * new_sz ))) return -1;

     self->stk_ptr= ptr;
     self->size= new_sz;
   }

   /* Increment the stack pointer */
   ++self->count;

   /* Get the address to the destination memory */
   dest_ptr = elAddress( self, self->count - 1);

   /* Make a copy of this item */
   copy(self, dest_ptr, src_ptr);

   return 0;
}

int
DS_entry_exists(const DS *self, const void *target_ptr)
/******************************************************************
 * Return true if the item exists in the stack 
 */
{
unsigned i;
char *p;

  for(i= 0; i < self->count; i++) {
    p= self->stk_ptr + i * self->elsize;
    if(!memcmp(p, target_ptr, self->elsize)) return 1;
  }
  return 0;
}

int
DS_visitAllEntries(DS *self, int (* func)(void *item_ptr, void *data), void *data)
/******************************************************************************/
{
   for(unsigned i= 0; i < self->count; ++i) {
      void *item_ptr= self->stk_ptr + i * self->elsize;
      int rc= (*func)(item_ptr, data);
      if(rc) return rc;
   }
   return 0;
}

