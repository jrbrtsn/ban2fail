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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ptrvec.h"


static int
grow (PTRVEC * self)
{
  if (PTRVEC_resize (self, self->maxItems * 2))
    {
      if (PTRVEC_resize (self, self->maxItems + self->maxItems / 2))
	{
	  if (PTRVEC_resize (self, self->maxItems + self->maxItems / 4))
	    return 1;
	}
    }
  return 0;
}

int
PTRVEC_find (PTRVEC * self, unsigned int *ndxBuf, void *item)
{
  unsigned int i;
  void *ptr;

  PTRVEC_loopFwd (self, i, ptr)
  {
    if (ptr == item)
      {
        if(ndxBuf) *ndxBuf = (self->head + i) % self->maxItems;
	return 1;
      }
  }
  return 0;
}

int
PTRVEC_sinit(PTRVEC *self, unsigned initMaxItems)
/***********************************************
 * Static initialization call.
 */
{
  int rtn= 1;

  if(!(self->ptrArr)) {
    if(!(PTRVEC_constructor(self, initMaxItems))) goto abort;
  } else {
    PTRVEC_reset(self);
  }

  rtn= 0;
abort:
  return rtn;
}

PTRVEC *
PTRVEC_constructor (PTRVEC * self, unsigned int initMaxItems)
{
  if (!self) return NULL;
  memset (self, 0, sizeof (*self));
  if (!(self->ptrArr = malloc (initMaxItems * sizeof (void*)))) return NULL;

  self->maxItems = initMaxItems;
  return self;
}

void *
PTRVEC_destructor (PTRVEC * self)
{
  if (self->ptrArr) free (self->ptrArr);
  if(self->sortBuf) free(self->sortBuf);
  return self;
}

int
PTRVEC_sort (PTRVEC * self, int (*cmp) (const void *const*, const void *const*))
{
  void *ptr;
  unsigned int i, sz;
  unsigned int numItems = self->numItems;;

  /* Nothing to sort */
  if(!numItems) return 0;

  /* Compute how large the sort buffer needs to be */
  sz= numItems * sizeof (void *);

  /* If there is no sort buffer, or it isn't big enough, allocate more */
  if(sz > self->sortBuf_sz) {
    if(self->sortBuf) free(self->sortBuf);
    self->sortBuf_sz= 0;
    if(!(self->sortBuf= malloc(sz))) return 1;
    self->sortBuf_sz= sz;
  }
//  if (!(block = malloc (numItems * sizeof (void *)))) return 1;


  PTRVEC_loopFwd (self, i, ptr) self->sortBuf[i] = ptr;
  PTRVEC_reset (self);

  qsort(self->sortBuf, numItems, sizeof (void *), (int(*)(const void*, const void*))cmp);
  for (i = 0; i < numItems; i++) {
    PTRVEC_addTail (self, self->sortBuf[i]);
  }

  return 0;
}


int
PTRVEC_resize (PTRVEC * self, unsigned int maxItems)
{
  unsigned int headSize;
  void *tmp;

  if (!maxItems)
    return 0;
  if (maxItems < self->numItems)
    return 1;

  if (!(tmp = realloc (self->ptrArr, maxItems * sizeof (void*))))
    return 1;
  self->ptrArr = tmp;

  if (self->head > self->tail)
    {
      headSize = self->maxItems - self->head;
      memmove (self->ptrArr + maxItems - headSize,
	       self->ptrArr + self->maxItems - headSize,
	       headSize * sizeof (*self->ptrArr));
      self->head = maxItems - headSize;
    }

  self->maxItems = maxItems;

  return 0;
}


void *
PTRVEC_addHead (PTRVEC * self, void *ptr)
{
  if (self->numItems == self->maxItems && grow (self))
    return NULL;

  if (self->numItems)
    self->head = (self->head + self->maxItems - 1) % self->maxItems;

  self->ptrArr[self->head] = ptr;
  self->numItems++;
  return ptr;
}

void *
PTRVEC_remHead (PTRVEC * self)
{
  void *tmp;

  if (!self->numItems)
    return NULL;
  self->numItems--;
  tmp = self->ptrArr[self->head];
  if (self->numItems)
    self->head = (self->head + 1) % self->maxItems;

  return tmp;
}

void *
PTRVEC_addTail (PTRVEC * self, void *ptr)
{
  if (self->numItems == self->maxItems && grow (self))
    return NULL;

  if (self->numItems)
    self->tail = (self->tail + 1) % self->maxItems;

  self->ptrArr[self->tail] = ptr;
  self->numItems++;
  return ptr;
}

void *
PTRVEC_remTail (PTRVEC * self)
{
  void *tmp;

  if (!self->numItems)
    return NULL;
  self->numItems--;
  tmp = self->ptrArr[self->tail];

  if (self->numItems)
    self->tail = (self->tail + self->maxItems - 1) % self->maxItems;
  return tmp;
}

void *
PTRVEC_remove (PTRVEC * self, void *item)
{
  unsigned int blockLen;
  unsigned int ndx;

  if (!PTRVEC_find (self, &ndx, item))
    return NULL;

  if (self->numItems == 1)
    {
      self->head = self->tail = 0;
    }
  else if (ndx == self->head)
    {
      self->head = (self->head + 1) % self->maxItems;
    }
  else if (ndx == self->tail)
    {
      self->tail = (self->tail + self->maxItems - 1) % self->maxItems;
    }
  else if (ndx < self->tail)	/* Non contiguous, top block */
    {
      blockLen = self->tail - ndx;
      memmove (self->ptrArr + ndx,
	       self->ptrArr + ndx + 1, blockLen * sizeof (*self->ptrArr));
      self->tail--;
    }
  else				/* Contiguous or non contiguous bottom block */
    {
      blockLen = ndx - self->head;
      memmove (self->ptrArr + self->head + 1,
	       self->ptrArr + self->head, blockLen * sizeof (*self->ptrArr));
      self->head++;
    }

  self->numItems--;
  return item;
}

#ifdef DEBUG
void
PTRVEC_assert_integrity(PTRVEC *self)
{
char *p, val;
unsigned i;
  PTRVEC_loopFwd(self, i, p) {
    val= *p;
  }
}
#endif
