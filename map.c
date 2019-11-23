/***************************************************************************
                          map.c  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 2001 by John D. Robertson
    email                : john@rrci.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "map.h"

struct MAP_node {
   unsigned int len;
   void *item_ptr;
   /* Key goes here */
};

/* Store the variable length key after the 'item_ptr' member of the MAP_node by
 * malloc()'ing extra memory. This avoids indirection and memory fragmentation.
 */

/* Macro to access the address of the variable length key */
#define KEY_PTR(ht_node_ptr)\
   ((void *)(((char *)(ht_node_ptr)) + sizeof(struct MAP_node)))

static unsigned int
hash(const unsigned char *key, unsigned int len)
/********************************************************************
 * Hashing function lifted from http://burtleburtle.net/bob/hash/hashfaq.html
 */
{
unsigned int i, h;

   for(h=0, i=0; i<len; ++i) {
      h += key[i];
      h += (h<<10);
      h ^= (h>>6);
   }

   h += (h<<3);
   h ^= (h>>11);
   h += (h<<15);

   return h;
}


MAP *
MAP_constructor(MAP *self,
                    unsigned int numBuckets,
                    unsigned int slotsPerBucket)
/****************************************************************
 * Construct a hash table with numBuckets buckets. slotsPerBucket provides
 * an initial sizing of the buckets, but they can grow dynamically.
 */
{
unsigned int i;

  if(!self) return NULL;
   memset(self, 0, sizeof(*self));

   self->numBuckets= numBuckets;

   /* Get the array of buckets */
   if(!(self->bucketArr= malloc(numBuckets*sizeof(PTRVEC)))) goto abort;

   /* Initialize each bucket */
   for(i= 0; i < numBuckets; ++i) {
      if(!PTRVEC_constructor(self->bucketArr+i, slotsPerBucket)) goto abort;
   }

   return self;
abort:
   return NULL;
}

int
MAP_sinit(MAP *self,
              unsigned int numBuckets,
              unsigned int slotsPerBucket)
/***********************************************************
 * Initialize or clear() for a static instance.
 */
{
  int rtn= 1;
  if(!self->bucketArr) {
    if(!MAP_constructor(self, numBuckets, slotsPerBucket)) goto abort;
  } else {
    MAP_clear(self);
  }

  rtn= 0;
abort:
  return rtn;
}

void*
MAP_destructor(MAP *self)
/****************************************************************/
{
unsigned int i;
void *ptr;

   for(i= 0; i < self->numBuckets; ++i) {
      while((ptr= PTRVEC_remHead(self->bucketArr+i))) free(ptr);
      if(!PTRVEC_destructor(self->bucketArr+i)) return NULL;
   }
   free(self->bucketArr);
   return self;
}

int
MAP_clearAndDestroy(MAP *self, void *(* destructor)(void *self))
/***********************************************************************/
{
int rtn= 0;
unsigned int ndx;
struct MAP_node *n_ptr;

   /* Loop through the buckets */
   for(ndx= 0; ndx < self->numBuckets; ++ndx) {
      /* For each node in the bucket... */
      while((n_ptr= PTRVEC_remHead(self->bucketArr+ndx))) {

         /* Call the supplied destructor */
         if(destructor) {
           if(!(*destructor)(n_ptr->item_ptr)) rtn= 1;
           /* Free the item */
           free(n_ptr->item_ptr);
         }

         /* And the node */
         free(n_ptr);
      }
   }
   return rtn;
}


int
MAP_visitAllEntries(MAP *self, int (* func)(void *item_ptr, void *data), void *data)
/******************************************************************************/
{
unsigned ndx, i;
struct MAP_node *n_ptr;

   /* Loop through the buckets */
   for(ndx= 0; ndx < self->numBuckets; ++ndx) {
      /* For each node in the bucket... */
      PTRVEC_loopFwd(self->bucketArr+ndx, i, n_ptr) {
         /* Call the supplied function */
         if((*func)(n_ptr->item_ptr, data)) return 1;
      }
   }
   return 0;
}

unsigned
MAP_numItems(MAP *self)
/******************************************************************************
 * Return a count of the items indexed in the hash table.
 */
{
unsigned ndx, rtn= 0;

   /* Loop through the buckets */
   for(ndx= 0; ndx < self->numBuckets; ++ndx) {
     rtn += PTRVEC_numItems(self->bucketArr+ndx);
   }
   return rtn;
}

int
MAP_addKey(MAP *self,
               const void *key_ptr,
               unsigned int keyLen,
               void *item_ptr)
/***************************************************************************/
{
struct MAP_node *n_ptr;
unsigned int ndx;

   /* Figure out in which bucket to dump it */
   ndx= hash(key_ptr, keyLen) % self->numBuckets;

   /* malloc() the mode and add it to the node list */
   if(!(n_ptr= malloc(sizeof(*n_ptr) + keyLen)) ||
      !PTRVEC_addTail(self->bucketArr+ndx, n_ptr)) return 1;

   /* store pertinant information in the node */
   n_ptr->len= keyLen;
   n_ptr->item_ptr= item_ptr;
   memcpy(KEY_PTR(n_ptr), key_ptr, keyLen);
   return 0;
}

int
MAP_findItems(MAP *self,
                  void* rtnArr[],
                  unsigned int rtnArrSize,
                  const void *key_ptr,
                  unsigned int keyLen)
/**********************************************************/
{
unsigned int ndx, i;
int count= 0;
struct MAP_node *n_ptr;

   /* Figure out which bucket to search in */
   ndx= hash(key_ptr, keyLen) % self->numBuckets;

   /* Loop through the bucket looking for a matching key */
   PTRVEC_loopFwd(self->bucketArr + ndx, i, n_ptr) {
      /* Compare the keys */
      if(keyLen == n_ptr->len &&
         !memcmp(KEY_PTR(n_ptr), key_ptr, keyLen)) {
         /* Store item_ptr in the return array */
         if(count == rtnArrSize) return -1;
         rtnArr[count]= n_ptr->item_ptr;
         ++count;
      }
   }
   return count;
}

static struct MAP_node *
_MAP_findNode(MAP *self,
                  unsigned int *rtnBucketNo_ptr,
                  const void *key_ptr,
                  unsigned int keyLen)
/**********************************************************************
 * Find the node that matches the supplied key.
 * Return it's pointer, or NULL if it cannot be found.
 */
{
unsigned int ndx, i;
struct MAP_node *n_ptr;

   /* Figure out which bucket to search in */
   ndx= hash(key_ptr, keyLen) % self->numBuckets;

   /* Loop through the bucket looking for a matching key */
   PTRVEC_loopFwd(self->bucketArr + ndx, i, n_ptr) {
      /* Compare the keys */
      if(keyLen == n_ptr->len &&
         !memcmp(KEY_PTR(n_ptr), key_ptr, keyLen)) {
         *rtnBucketNo_ptr= ndx;
         return n_ptr;
      }
   }
   return NULL;
}

void*
MAP_findItem(MAP *self,
                 const void *key_ptr,
                 unsigned int keyLen)
/*************************************************************************/
{
struct MAP_node *n_ptr;
unsigned int bucket;

   if(!(n_ptr= _MAP_findNode(self, &bucket, key_ptr, keyLen))) return NULL;
   return n_ptr->item_ptr;
}

void*
MAP_removeSpecificItem(MAP *self,
                           const void *key_ptr,
                           unsigned int keyLen,
                           void *pItem)
/******************************************************************************
 * Find the the first matching key and remove it from the hash table.
 * pItem is the address of the specific item to be removed.
 * Returns:
 * NULL           Not found
 * item_ptr       first one found
 */
{
  unsigned int ndx, i;
  struct MAP_node *n_ptr;
  void *rtn= NULL;

   /* Figure out which bucket to search in */
   ndx= hash(key_ptr, keyLen) % self->numBuckets;

   /* Loop through the bucket looking for a matching key */
   PTRVEC_loopFwd(self->bucketArr + ndx, i, n_ptr) {
      /* Compare the keys */
      if(keyLen == n_ptr->len &&
         !memcmp(KEY_PTR(n_ptr), key_ptr, keyLen) &&
         n_ptr->item_ptr == pItem) { /* And compare the item pointer */

        rtn= n_ptr->item_ptr; /* Remember this for return value */
        free(PTRVEC_remove(self->bucketArr + ndx, n_ptr)); /* Remove entry from this bucket */
        break;
      }
   }
   return rtn;
}

void*
MAP_removeItem(MAP *self,
                   const void *key_ptr,
                   unsigned int keyLen)
/******************************************************************************/
{
struct MAP_node *n_ptr;
unsigned int bucket;
void *item_ptr;

   if(!(n_ptr= _MAP_findNode(self, &bucket, key_ptr, keyLen))) return NULL;
   item_ptr= n_ptr->item_ptr;
   free(PTRVEC_remove(self->bucketArr + bucket, n_ptr));

   return item_ptr;
}

static int
load_arr(void *item_ptr, void *data)
/******************************************************************
 * lambda function to load all CFG_DICT_ENTRY's into an array.
 */
{
  void ***ppp= (void***)data;
  **ppp= item_ptr;
  ++(*ppp);
  return 0;
}

void
MAP_fetchAllItems(MAP *self, void **rtn_arr)
/******************************************************************************
 * Place the itme pointers into the supplied array.
 */
{
  MAP_visitAllEntries(self, load_arr, &rtn_arr);
}
