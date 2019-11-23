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
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "util.h"

int
STR_sinit(STR *self, size_t sz_hint)
/**********************************************************************************
 * Initialization to be called for static instances of STR each use, but
 * actual initialization only occurs once.
 */
{
  int rtn= 1;
  if(!self->buf) {
    if(!STR_constructor(self, sz_hint)) goto abort;
  } else {
    STR_reset(self);
  }

  rtn= 0;
abort:
  return rtn;
}


STR*
STR_constructor(STR *self, size_t sz_hint)
/**********************************************************************************
 * Prepare a STR for use with initial size of sz_hint.
 */
{
  STR *rtn= NULL;

  assert(sz_hint);

  self->sz= sz_hint;
  if(!(self->buf= malloc(self->sz))) goto abort;
  STR_reset(self);

  rtn= self;

abort:
  return rtn;
}

void*
STR_destructor(STR *self)
/**********************************************************************************
 * Free resources associated with STR.
 */
{
  if(self->buf) free(self->buf);
  return self;
}

static int
growbuf(STR *self)
/**********************************************************************************
 * Attempt to increase the size buffer initially trying to double it, then
 * backing off 10% at a time.
 * Returns non-zero for error.
 */
{
  int rtn= 1;
  size_t i;
  /* Initially try do double memory. If that fails, back off 10% at a time */
  for(i= 20; i > 10; i--) {
    char *p;
    size_t new_sz= self->sz * i / 10;
    /* Try to reallocate the memory */
    if(!(p= realloc(self->buf, new_sz))) continue;
    /* Try vsnprintf() again */
    self->buf= p;
    self->sz = new_sz;
    break;
  }

  if(i != 10) rtn= 0; /* Not success if we grew the buffer */

abort:
  return rtn;
}

int
STR_sprintf(STR *self, const char *fmt, ...)
/**********************************************************************************
 * Same as sprintf, except you don't have to worry about buffer overflows.
 * Returns non-zero for error.
 */
{
  int is_done, rtn= -1;
  va_list arglist;

  /* Catch empty strings */
  if(!strlen(fmt)) return 0;

  /* vsprintf the fmt string */
  for(is_done= 0; !is_done;) {
    int rc;
    va_start (arglist, fmt);
    rc= vsnprintf(self->buf+self->len, self->sz - self->len, fmt, arglist);
    if(rc >= (self->sz - self->len)) { /* Buffer isn't large enough */
      if(growbuf(self)) is_done= 1;
    } else {
      if(rc != -1) { /* Successful return */
        rtn= rc;
        self->len += rc;
      }
      is_done= 1;
    }
    va_end (arglist);
  }

  return rtn;
}

int
STR_vsprintf(STR *self, const char *fmt, va_list ap)
/**********************************************************************************
 * Same as vsprintf, except you don't have to worry about buffer overflows.
 * Returns non-zero for error.
 */
{
  int is_done, rtn= -1;

  /* Catch empty strings */
  if(!strlen(fmt)) return 0;

  /* vsprintf the fmt string */
  for(is_done= 0; !is_done;) {
    int rc;
    va_list arglist;
#if __GNUC__ == 2
    arglist= ap;
#else
    va_copy(arglist, ap);
#endif
    rc= vsnprintf(self->buf+self->len, self->sz - self->len, fmt, arglist);
    if(rc >= (self->sz - self->len)) { /* Buffer isn't large enough */
      if(growbuf(self)) is_done= 1;
    } else {
      if(rc != -1) { /* Successful return */
        rtn= rc;
        self->len += rc;
      }
      is_done= 1;
    }
    va_end (arglist);
  }

  return rtn;
}

int
STR_append(STR *self, const char *str, size_t str_len)
/**********************************************************************************
 * Append string str to the end of the current buffer.
 * Returns -1 for error.
 */
{
  if(str_len == -1) str_len= strlen(str);

  /* Make sure there is enough space to store the string */
  while(self->len + str_len + 1 >= self->sz) {
    if(growbuf(self)) return 1;
  }

  /* Copy the string into place */
  memcpy(self->buf + self->len, str, str_len);
  /* Update length */
  self->len += str_len;
  /* Null terminate string */
  self->buf[self->len] = '\0';
  return 0;
}

int
STR_putc(STR *self, int c)
/**********************************************************************************
 * Append a single character to the end of the current buffer.
 * Returns -1 for error.
 */
{
  /* Make sure there is enough space to store the string */
  while(self->len + 1 >= self->sz) {
    if(growbuf(self)) return 1;
  }

  /* Copy the character into place */
  self->buf[self->len]= c;
  /* Update length */
  ++self->len;
  /* Null terminate string */
  self->buf[self->len] = '\0';
  return 0;
}

const char*
STR_tolower(STR *self)
/**********************************************************************************
 * Convert all characters in buffer to lower case.
 */
{
  char *c;
  for(c= self->buf; *c; ++c) {
    *c= tolower(*c);
  }
  return self->buf;
}

const char*
STR_XMLencode(STR *self, const char *src)
/**************************************************************
 * encode the src string for XML into self. Return self's buffer.
 * NOTE: self does not get reset!
 */
{
   const char *c;
   if(!src) return "";

   for(c= src; *c; ++c) {

      const char *str= NULL;
      unsigned len= 0;


      switch(*c) {

#define doit(litstr) \
      len= strlen(litstr); str= litstr

         case '"': doit("&quot;"); break;
         case '\'': doit("&apos;"); break;
         case '<': doit("&lt;"); break;
         case '>': doit("&gt;"); break;
         case '&': doit("&amp;"); break;
#undef doit
      }

      if(str) { // Special string was assigned
         STR_append(self, str, len);
      } else {
         STR_putc(self, *c);
      }
   }
   return STR_str(self);
}

const char*
STR_URLencode(STR *self, const char *src)
/**************************************************************
 * encode the src string for URL into self. Return self's buffer.
 * NOTE: self does not get reset!
 */
{
   const char *c;
   if(!src) return "";

   for(c= src; *c; ++c) {

      const char *str= NULL;
      switch(*c) {
         case '!': str= "%21"; break;
         case '#': str= "%23"; break;
         case '$': str= "%24"; break;
         case '&': str= "%26"; break;
         case '\'': str= "%27"; break;
         case '(': str= "%28"; break;
         case ')': str= "%29"; break;
         case '*': str= "%2A"; break;
         case '+': str= "%2B"; break;
         case ',': str= "%2C"; break;
//         case '/': str= "%2F"; break;
         case ':': str= "%3A"; break;
         case ';': str= "%3B"; break;
         case '=': str= "%3D"; break;
         case '?': str= "%3F"; break;
         case '@': str= "%40"; break;
         case '[': str= "%5B"; break;
         case ']': str= "%5D"; break;
      }

      if(str) { // Special string was assigned
         STR_append(self, str, 3);
      } else {
         STR_putc(self, *c);
      }
   }

   return STR_str(self);
}

const char*
STR_utf8toHTML(STR *self, const char *src)
/**************************************************************
 * place the HTML representation of the utf-8 src string into self.
 * Return self's buffer.
 * NOTE: self does not get reset!
 */
{
   const unsigned char *c;
   unsigned code= '?';
   if(!src) return "";

   for(c= (const unsigned char*)src; *c; ++c) {

      /******* Map UTF-8 to a code point **********/

      if(0xF0 == (*c & 0xF8)) { // first of four bytes

         /* Make sure the string doesn't end too soon */
         assert(c[1] && c[2] && c[3]);

         code= (*c & ~0xF8) << 18;
         ++c;
         code |= (*c & ~0xC0) << 12;
         ++c;
         code |= (*c & ~0xC0) << 6;
         ++c;
         code |= (*c & ~0xC0);

      } else if(0xE0 == (*c & 0xF0)) { // first of three bytes

         /* Make sure the string doesn't end too soon */
         assert(c[1] && c[2]);

         code= (*c & ~0xF0) << 12;
         ++c;
         code |= (*c & ~0xC0) << 6;
         ++c;
         code |= (*c & ~0xC0);

      } else if(0xC0 == (*c & 0xE0)) { // first of two bytes

         /* Make sure the string doesn't end too soon */
         assert(c[1]);
         code= (*c & ~0xE0) << 6;
         ++c;
         code |= (*c & ~0xC0);

      } else if(0 == (*c & 0x80)) {// first of one byte

         code= *c;

      }

      /************** Assign HTML special string if one is defined **********/
      const char *str= NULL;
      unsigned len= 0;

      switch(code) {

#define doit(litstr) \
      len= strlen(litstr); str= litstr

         case '"': doit("&quot;"); break;
         case '<': doit("&lt;"); break;
         case '>': doit("&gt;"); break;
         case '&': doit("&amp;"); break;
         case 160: doit("&nbsp;"); break;
         case 161: doit("&iexcl;"); break;
         case 162: doit("&cent;"); break;
         case 163: doit("&pound;"); break;
         case 164: doit("&curren;"); break;
         case 165: doit("&yen;"); break;
         case 166: doit("&brvbar;"); break;
         case 167: doit("&sect;"); break;
         case 168: doit("&uml;"); break;
         case 169: doit("&copy;"); break;
         case 170: doit("&ordf;"); break;
         case 171: doit("&laquo;"); break;
         case 172: doit("&not;"); break;
         case 173: doit("&shy;"); break;
         case 174: doit("&reg;"); break;
         case 175: doit("&macr;"); break;
         case 176: doit("&deg;"); break;
         case 177: doit("&plusmn;"); break;
         case 178: doit("&sup2;"); break;
         case 179: doit("&sup3;"); break;
         case 180: doit("&acute;"); break;
         case 181: doit("&micro;"); break;
         case 182: doit("&para;"); break;
         case 183: doit("&middot;"); break;
         case 184: doit("&cedil;"); break;
         case 185: doit("&sup1;"); break;
         case 186: doit("&ordm;"); break;
         case 187: doit("&raquo;"); break;
         case 188: doit("&frac14;"); break;
         case 189: doit("&frac12;"); break;
         case 190: doit("&frac34;"); break;
         case 191: doit("&iquest;"); break;
         case 192: doit("&Agrave;"); break;
         case 193: doit("&Aacute;"); break;
         case 194: doit("&Acirc;"); break;
         case 195: doit("&Atilde;"); break;
         case 196: doit("&Auml;"); break;
         case 197: doit("&Aring;"); break;
         case 198: doit("&AElig;"); break;
         case 199: doit("&Ccedil;"); break;
         case 200: doit("&Egrave;"); break;
         case 201: doit("&Eacute;"); break;
         case 202: doit("&Ecirc;"); break;
         case 203: doit("&Euml;"); break;
         case 204: doit("&Igrave;"); break;
         case 205: doit("&Iacute;"); break;
         case 206: doit("&Icirc;"); break;
         case 207: doit("&Iuml;"); break;
         case 208: doit("&ETH;"); break;
         case 209: doit("&Ntilde;"); break;
         case 210: doit("&Ograve;"); break;
         case 211: doit("&Oacute;"); break;
         case 212: doit("&Ocirc;"); break;
         case 213: doit("&Otilde;"); break;
         case 214: doit("&Ouml;"); break;
         case 215: doit("&times;"); break;
         case 216: doit("&Oslash;"); break;
         case 217: doit("&Ugrave;"); break;
         case 218: doit("&Uacute;"); break;
         case 219: doit("&Ucirc;"); break;
         case 220: doit("&Uuml;"); break;
         case 221: doit("&Yacute;"); break;
         case 222: doit("&THORN;"); break;
         case 223: doit("&szlig;"); break;
         case 224: doit("&agrave;"); break;
         case 225: doit("&aacute;"); break;
         case 226: doit("&acirc"); break;
         case 227: doit("&atilde;"); break;
         case 228: doit("&auml;"); break;
         case 229: doit("&aring;"); break;
         case 230: doit("&aelig;"); break;
         case 231: doit("&ccedil;"); break;
         case 232: doit("&egrave;"); break;
         case 233: doit("&eacute;"); break;
         case 234: doit("&ecirc;"); break;
         case 235: doit("&euml;"); break;
         case 236: doit("&igrave;"); break;
         case 237: doit("&iacute;"); break;
         case 238: doit("&icirc;"); break;
         case 239: doit("&iuml;"); break;
         case 240: doit("&eth;"); break;
         case 241: doit("&ntilde;"); break;
         case 242: doit("&ograve;"); break;
         case 243: doit("&oacute;"); break;
         case 244: doit("&ocirc;"); break;
         case 245: doit("&otilde;"); break;
         case 246: doit("&ouml;"); break;
         case 247: doit("&divide;"); break;
         case 248: doit("&oslash;"); break;
         case 249: doit("&ugrave;"); break;
         case 250: doit("&uacute;"); break;
         case 251: doit("&ucirc;"); break;
         case 252: doit("&uuml;"); break;
         case 253: doit("&yacute;"); break;
         case 254: doit("&thorn;"); break;
         case 255: doit("&yuml;"); break;
         case 8364: doit("&euro;"); break;
#undef doit

      }

      /****** Place final representation into our string buffer ******/
      if(str) { // Special string was assigned

         STR_append(self, str, len);

      } else if(code < 128 && isprint(code)) { // Normal ASCII character

         STR_putc(self, code);

      } else if(65533 == code) {

         // This is the placeholder for unrecognized characters

      } else { // All others

         STR_sprintf(self, "&#%u;", code);
      }

   }

   return STR_str(self);
}

const char*
STR_escapeJSONstr(STR *self, const char *src)
/**************************************************************
 * Escape any characters such the src can be used in a JSON
 * string.
 * Return self's buffer.
 * NOTE: self does not get reset!
 */
{
   const char *pc;
   for(pc= src; *pc; ++pc) {
      switch(*pc) {
         case '\\':
            STR_sprintf(self, "\\\\");
            break;

         case '"':
            STR_sprintf(self, "\\\"");
            break;

         default:
            if(iscntrl(*pc)) {
               STR_sprintf(self, "\\u%4X", (int)(*pc));
            } else {
               STR_putc(self, *pc);
            }
      }
   }
   return STR_str(self);
}

