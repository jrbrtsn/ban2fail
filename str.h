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
#ifndef STR_H
#define STR_H

#include <stdarg.h>
#include <sys/types.h>

/* STR is a dynamically sized null terminated string buffer which is always
 * appended until STR_reset() is called. It is particularly useful for
 * things like creating complex SQL queries.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cb {
  size_t sz,
         len;

  char *buf;
} STR;

#define STR_str(self) \
  ((const char*)(self)->buf)
/**********************************************************************************
 * Return the pointer to the buffer.
 */

#define STR_len(self) \
  ((const size_t)(self)->len)
/**********************************************************************************
 * Return the current length of the string in the buffer
 */

#define STR_reset(self) \
  ((self)->buf[((self)->len= 0)]= '\0')
/**********************************************************************************
 * Reset the buffer so that the length is zero.
 */

int
STR_sinit(STR *self, size_t sz_hint);
/**********************************************************************************
 * Initialization to be called for static instances of STR each use, but
 * actual initialization only occurs once.
 */

#define STR_create(p)\
   ((p)= (STR_constructor((p)=malloc(sizeof(STR))) ? (p) : ( p ?  realloc(STR_destructor(p),0): 0)))
STR*
STR_constructor(STR *self, size_t sz_hint);
/**********************************************************************************
 * Prepare a STR for use with initial size of sz_hint.
 */


#define STR_destroy(self)\
  if(STR_destructor(self)){free(self);(self)=NULL;} 
void*
STR_destructor(STR *self);
/**********************************************************************************
 * Free resources associated with STR.
 */

int
STR_sprintf(STR *self, const char *fmt, ...);
/**********************************************************************************
 * Same as sprintf, except you don't have to worry about buffer overflows.
 * Returns -1 for error.
 */

int
STR_vsprintf(STR *self, const char *fmt, va_list ap);
/**********************************************************************************
 * Same as vsprintf, except you don't have to worry about buffer overflows.
 * Returns -1 for error.
 */

int
STR_append(STR *self, const char *str, size_t str_len);
/**********************************************************************************
 * Append string str to the end of the current buffer.
 * Returns -1 for error.
 */

int
STR_putc(STR *self, int c);
/**********************************************************************************
 * Append a single character to the end of the current buffer.
 * Returns -1 for error.
 */

const char*
STR_tolower(STR *self);
/**********************************************************************************
 * Convert all characters in buffer to lower case.
 */

const char*
STR_XMLencode(STR *self, const char *src);
/**************************************************************
 * encode the src string for XML into self. Return self's buffer.
 * NOTE: self does not get reset!
 */

const char*
STR_URLencode(STR *self, const char *src);
/**************************************************************
 * encode the src string for URL into self. Return self's buffer.
 * NOTE: self does not get reset!
 */

const char*
STR_utf8toHTML(STR *self, const char *src);
/**************************************************************
 * place the HTML representation of the utf-8 src string into self.
 * Return self's buffer.
 * NOTE: self does not get reset!
 */

const char*
STR_escapeJSONstr(STR *self, const char *src);
/**************************************************************
 * Escape any characters such the src can be used in a JSON
 * string.
 * Return self's buffer.
 * NOTE: self does not get reset!
 */

#ifdef __cplusplus
}

// C++ wrapper

class StrBuf {
   public:
      StrBuf(size_t sz_hint)
         {STR_constructor(&obj, sz_hint);}

      ~StrBuf()
         {STR_destructor(&obj);}

      inline const char *str() const
         {return STR_str(&obj);}

      inline size_t len() const
         {return STR_len(&obj);}

      inline void reset()
         {STR_reset(&obj);}

      inline int sprintf(const char *fmt, ...)
         {
            va_list ap;
            va_start (ap, fmt);
            int rtn= STR_vsprintf(&obj, fmt, ap);
            va_end (ap);
            return rtn;
         }

      inline int vsprintf(const char *fmt, va_list ap)
         {
            return STR_vsprintf(&obj, fmt, ap);
         }

      inline int append(const char *str, size_t str_len)
         {return STR_append(&obj, str, str_len);}

      inline int putc(int c)
         {return STR_putc(&obj, c);}

      inline const char* tolower()
         {return STR_tolower(&obj);}

      inline const char* XMLencode(const char *src)
         {return STR_XMLencode(&obj, src);}

      inline const char* URLencode(const char *src)
         {return STR_URLencode(&obj, src);}

      inline const char* utf8toHTML(const char *src)
         {return STR_utf8toHTML(&obj, src);}

   private:
      STR obj;
};
#endif

#endif
