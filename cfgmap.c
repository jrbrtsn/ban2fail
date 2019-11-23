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
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <libgen.h>
#include <assert.h>

#include "cfgmap.h"
#include "ptrvec.h"
#include "util.h"

/* Internal classes */
typedef struct _VALUE {
   char *str;
   unsigned serial_no;
} VALUE;

/* Forward declarations */
static CFGMAP_ENTRY* CFGMAP_ENTRY_constructor(CFGMAP_ENTRY * self,
						const char *symbol,
						unsigned symLen);
static int _CFGMAP_fh_read(CFGMAP * self, FILE * fh, const char *fname);
static int cmp_symbol(const void *v1, const void *v2);
static int count_tuples(void *item_ptr, void *data);
static int str_extract_string(char **rtnVal, const char **pStr);
static int unused_load_arr(void *item_ptr, void *data);
static VALUE* CFGMAP_ENTRY_append_value(CFGMAP_ENTRY * self,
					 const char *val_str,
					 unsigned serial_no);
static VALUE* VALUE_constructor(VALUE * self, const char *str, unsigned serial_no);
static void* CFGMAP_ENTRY_destructor(CFGMAP_ENTRY * self);
static void* VALUE_destructor(VALUE * self);

/* Internal macros */
#define VALUE_create(p, str, serial_no) \
  ((p)= (VALUE_constructor((p)=malloc(sizeof(VALUE)), str, serial_no) ? (p) : ( p ? realloc(VALUE_destructor(p),0) : 0 )))

#define VALUE_destroy(s) \
  {if(VALUE_destructor(s)) {free(s); s=NULL;}}

#define CFGMAP_ENTRY_create(p, sym, symLen) \
  (CFGMAP_ENTRY_constructor((p)=malloc(sizeof(CFGMAP_ENTRY)), sym, symLen) ? (p) : ( p ? realloc(CFGMAP_ENTRY_destructor(p),0) : 0 ))

#define CFGMAP_ENTRY_destroy(s) \
  if(CFGMAP_ENTRY_destructor(s)) {free(s);}

/*===========================================================================================*/
/*================================== CFGMAP ===============================================*/
/*===========================================================================================*/

const CFGMAP_ENTRY*
CFGMAP_find(CFGMAP * self, const char *symbol)
/************************************************************************
 * Find entry for the given null terminated symbol.
 */
{
   CFGMAP_ENTRY *rtn =
       MAP_findItem(&self->entry_tbl, symbol, strlen(symbol));
   if (rtn)
      ++rtn->nLookups;
   return rtn;
}

const char*
CFGMAP_find_single_value(CFGMAP * self, const char *symbol)
/************************************************************************
 * Find exactly one value for a symbol, or return NULL.
 */
{
   const CFGMAP_ENTRY *pEntry;
   unsigned numVals;

   if (!(pEntry = CFGMAP_find(self, symbol)))
      return NULL;

   if ((numVals = CFGMAP_ENTRY_numValues(pEntry)) != 1) {
      eprintf("ERROR: \"%s\" has %u values instead of 1.", symbol, numVals);
      return NULL;
   }

   return CFGMAP_ENTRY_value(pEntry, 0);
}

const char*
CFGMAP_find_last_value(CFGMAP * self, const char *symbol)
/************************************************************************
 * Find the last value for a symbol, or return NULL.
 */
{
   const CFGMAP_ENTRY *pEntry;
   unsigned numVals;

   if (!(pEntry = CFGMAP_find(self, symbol)))
      return NULL;

   numVals = CFGMAP_ENTRY_numValues(pEntry);

   return CFGMAP_ENTRY_value(pEntry, numVals - 1);
}

int
CFGMAP_append(CFGMAP * self, const char *symbol, unsigned int symLen,
	       const char *value)
/**************************************************************************
 * Append a value to a config entry, creating a new entry if necessary.
 */
{
   CFGMAP_ENTRY *pEntry;
   VALUE *v;

   /* Create a new CFGMAP_ENTRY if no match is found. */
   if (!(pEntry = MAP_findItem(&self->entry_tbl, symbol, symLen))) {
      if (!CFGMAP_ENTRY_create(pEntry, symbol, symLen))
	 return 1;
      if (MAP_addKey
	  (&self->entry_tbl, pEntry->symbol, pEntry->symLen, pEntry))
	 assert(0);
   }

   /* Return now if there is no value to remember. */
   if (!strlen(value))
      return 0;

   /* Must recount tuples */
   self->flags &= ~CFGMAP_NTUPLES_KNOWN_FLG;

   /* Otherwise, add the value to the list */
   if (!CFGMAP_ENTRY_append_value(pEntry, value, ++self->serial_no_seq))
      assert(0);

   return 0;

}

CFGMAP*
CFGMAP_file_constructor(CFGMAP * self, const char *fname)
/******************************************************************
 * Create a map populated from the supplied file.
 */
{

   if (!(CFGMAP_constructor(self)))
      return NULL;

   if (CFGMAP_file_read(self, fname))
      return NULL;

   return self;
}

CFGMAP*
CFGMAP_constructor(CFGMAP * self)
/******************************************************************
 * Create an empty map.
 */
{

   if (!self)
      return NULL;
   memset(self, 0, sizeof(*self));

   /* Initialize data structures */
   if (!MAP_constructor(&self->entry_tbl, 30, 100) ||
       !PTRVEC_constructor(&self->curlyBlock_lst, 10))
      return NULL;

   return self;
}

void*
CFGMAP_destructor(CFGMAP * self)
/******************************************************************
 * Free resources associated with map.
 */
{

   /* Remove configuration map entries */
   MAP_clearAndDestroy(&self->entry_tbl,
			   (void *(*)(void *))CFGMAP_ENTRY_destructor);
   MAP_destructor(&self->entry_tbl);

   {				/* Remove curly brace block symbols list */
      char *str;
      while ((str = PTRVEC_remHead(&self->curlyBlock_lst))) {
	 free(str);
      }
   }
   PTRVEC_destructor(&self->curlyBlock_lst);

   return self;
}

static int
_CFGMAP_fh_read(CFGMAP * self, FILE * fh, const char *fname)
/**************************************************************************
 * Read a file to populate the map.
 */
{
   int rtn = 1;
   unsigned lineNo;
   char cwd[PATH_MAX];

   /* Note the change in recursion level */
   ++self->recurs_lvl;

   {				/* get a copy of the current directory */
      char tmp_fname[PATH_MAX];

      strncpy(tmp_fname, fname, sizeof(tmp_fname) - 1);
      tmp_fname[sizeof(tmp_fname) - 1] = '\0';
      strncpy(cwd, dirname(tmp_fname), sizeof(cwd) - 1);
      cwd[sizeof(cwd) - 1] = '\0';
   }

#ifdef DDEBUG
   eprintf("INFO: %s(\"%s\")", __FUNCTION__, fname);
#endif

   { /* Parse the file, build a map */
      char lineBuf[1024];
      unsigned int charNo, currLen;
      enum {
	 COMMENT_STATE = 1 << 0,
	 STRING_STATE = 1 << 1,
	 CURLY_BEGIN_STATE = 1 << 2,
	 REPLACE_STATE = 1 << 3,
	 NDX_STATE = 1 << 4,
	 ESCAPED_STATE = 1 << 5
      } stateFlags;

      for (lineNo = 1, charNo = 1, currLen = 0, stateFlags = 0;; ++charNo) {
	 char *str;
	 int c;

	 /* Check for buffer overflow */
	 if (currLen + 1 == sizeof(lineBuf)) {
	    eprintf("ERROR: line buffer overflow at line %u.", lineNo);
	    goto abort;
	 }

	 /* Grab the next character from the file */
	 c = getc(fh);

	 /* End of current line? */
	 if (c == '\n' || c == EOF) {
	    char substBuf[sizeof(lineBuf)];

	    // TODO: here is where a continuation backslash could be checked for

	    if (stateFlags & STRING_STATE) {
	       eprintf("ERROR: Unterminated string in \"%s\" at line %u.",
		       fname, lineNo);
	       goto abort;
	    }

	    /* Null terminate buffer */
	    lineBuf[currLen] = '\0';

	    /* Skip leading whitespace */
	    /* Get rid of trailing whitespace */
            str= trim(lineBuf);

	    { /*--- Perform substitutions ---*/
	       unsigned nCopied;

	       /* Null terminate substituted string. */
	       substBuf[sizeof(substBuf) - 1] = '\0';

	       /* Perform substitutions */
	       for (nCopied = 0; *str && nCopied + 1 < sizeof(substBuf);) {

		  if (!strncmp(str, "$CWD", 4)) {
		     strncpy(substBuf + nCopied, cwd,
			     sizeof(substBuf) - nCopied - 1);
		     str += 4;
		     nCopied += strlen(substBuf + nCopied);
		  } else {
		     substBuf[nCopied] = *str;
		     ++str;
		     ++nCopied;
		  }
	       }
	       /* Make sure the substBuf is null terminated */
	       substBuf[nCopied] = '\0';
	       /* Further work done with contents of substitution buffer */
	       currLen = nCopied;
	       str = substBuf;
	    }

	    /* Process lines with non-zero length */
	    if (*str) {

	       assert(currLen);

	       /* Last character issues */
	       switch (str[currLen - 1]) {
	       case '{':
		  stateFlags |= CURLY_BEGIN_STATE;
		  /* Get rid of trailing whitespace */
		  str[--currLen] = '\0';
                  trimend(str);
		  break;

	       case '}':
		  {
		     char *entry = PTRVEC_remTail(&self->curlyBlock_lst);
		     if (!entry) {
			eprintf
			    ("ERROR: Unmatched '}' found in \"%s\" at line %u.",
			     fname, lineNo);
			goto abort;
		     }
		     free(entry);
		  }
		  if (currLen == 1)
		     goto doneProcessing;
		  break;
	       }

	       if (!strncmp(str, ".include", 8)) {
		  char *fname;

		  str += 8;
#pragma GCC diagnostic ignored "-Wcast-qual"
		  if (str_extract_string(&fname, (const char **)(&str)))
		     assert(0);

		  /* Call our self to continue */
		  if (CFGMAP_file_read(self, fname)) {
		     eprintf("ERROR: failed at line %u.", lineNo);
		     goto abort;
		  }

		  free(fname);
		  goto doneProcessing;

	       } else if (!strncmp(str, ".shell", 6)) {
		  char *cmd;
		  FILE *pofh = NULL;

//		  while (*cmd && isspace(*cmd)) ++cmd;
		  cmd = skipspace(str + 6);
                  

		  if (!(pofh = popen(cmd, "r")) ||
		      _CFGMAP_fh_read(self, pofh, fname)) {
		     eprintf("ERROR: \".shell %s\" failed at line %u.", cmd,
			     lineNo);
		     goto abort;
		  }

		  if (pofh)
		     pclose(pofh);

		  goto doneProcessing;
	       } else if (*str == '@') {

		  /* Skip leading whitespace */
		  for (++str, --currLen; *str && isspace(*str); ++str, --currLen) ;

		  stateFlags |= REPLACE_STATE;

	       } else if (*str == '}') {	/* Ending of curly brace block */

		  char *entry = PTRVEC_remTail(&self->curlyBlock_lst);
		  if (!entry) {
		     eprintf("ERROR: Unmatched '}' found in \"%s\" at line %u.",
			     fname, lineNo);
		     goto abort;
		  }
		  free(entry);
		  goto doneProcessing;

	       }

	       {		/* At this point we hope we have a symbol+value pair */
		  size_t symLen;
		  unsigned ndx;
		  char *valStr, *ndxStr, symBuf[sizeof(lineBuf) * 2];

		  /* Find the length of the symbol */
		  symLen = strcspn(str, " \t=[");

		  /* Default to end of symbol */
		  valStr = str + symLen;

		  /* If we have a value, find the beginning */
		  if (*valStr) {
		     /* Skip whitespace */
//		     while (*valStr && isspace(*valStr)) ++valStr;
                     valStr= skipspace(valStr);

		     if (*valStr == '[') {
			int n;
			if (sscanf(valStr, "[ %u ]%n", &ndx, &n) < 1) {
			   eprintf
			       ("ERROR: array notation in .replace without a value in \"%s\" at line %u.",
				fname, lineNo);
			   goto abort;
			}
			valStr += n;
			stateFlags |= NDX_STATE;

			/* Skip whitespace */
//			while (*valStr && isspace(*valStr)) ++valStr;
                        valStr= skipspace(valStr);
		     }

		     if (*valStr == '=')
			++valStr;

		     /* Skip remaining whitespace */
//		     while (*valStr && isspace(*valStr)) ++valStr;
                     valStr= skipspace(valStr);
		  }

		  /* Generate symbol prefix, if necessary */
		  if (PTRVEC_numItems(&self->curlyBlock_lst)) {
		     size_t len;
		     symBuf[0] = '\0';

		     {		/* Prefix segments from ancestor curly brace blocks */
			unsigned i;
			const char *pfix;
			PTRVEC_loopFwd(&self->curlyBlock_lst, i, pfix) {
			   strncat(symBuf, "\\", sizeof(symBuf) - 1);
			   strncat(symBuf, pfix, sizeof(symBuf) - 1);
			}
		     }

		     /* Leading period before current symbol */
		     strncat(symBuf, "\\", sizeof(symBuf) - 1);

		     /* Tag on the current symbol */
		     len = strlen(symBuf);
		     if (len + symLen >= sizeof(symBuf)) {
			eprintf
			    ("ERROR: symbol buffer overrun in \"%s\" at line %u.",
			     fname, lineNo);
			goto abort;
		     }
		     strncat(symBuf + len, str, symLen);
		     symLen += len;

		  } else {
		     assert(symLen + 1 < sizeof(symBuf));

		     /* Copy into symBuf. */
		     symBuf[0] = '\\';
		     memcpy(symBuf + 1, str, symLen);
		     ++symLen;
		  }
		  /* Null terminate symBuf */
		  symBuf[symLen] = '\0';

		  if (stateFlags & REPLACE_STATE) {	/* Need to replace an existing entry */
		     unsigned nVals;
		     /* We have to skip over the default leading '\' in symBuf, because it was supplied in the config file. */
		     CFGMAP_ENTRY *pEntry =
			 MAP_findItem(&self->entry_tbl, symBuf + 1,
					  symLen - 1);

		     if (!pEntry) {
			eprintf
			    ("ERROR: No symbol \"%s\" to replace in \"%s\" at line %u.",
			     symBuf + 1, fname, lineNo);
			goto abort;
		     }

		     nVals = CFGMAP_ENTRY_numValues(pEntry);

		     if (stateFlags & NDX_STATE) {	/* Entry is indexed */
			unsigned i;
			VALUE *val, *valArr[nVals];

			if (ndx >= nVals) {
			   eprintf
			       ("ERROR: .replace ndx= %u out of range in \"%s\" at line %u.",
				ndx, fname, lineNo);
			   goto abort;
			}

			/* Load value VALUES into temporary array */
			PTRVEC_loopFwd(&pEntry->value_lst, i, val) {
			   if (ndx == i) {	/* Replace this value */
			      /* Preserve the serial number */
			      unsigned sn = val->serial_no;
			      VALUE_destroy(val);
			      VALUE_create(valArr[i], valStr, sn);
			      assert(valArr[i]);
			   } else {	/* Copy existing value */
			      valArr[i] = val;
			   }
			}

			/* Put temporary array back into PTRVEC */
			PTRVEC_reset(&pEntry->value_lst);
			for (i = 0; i < nVals; ++i) {
			   PTRVEC_addTail(&pEntry->value_lst, valArr[i]);
			}

		     } else {	/* Must be only one entry */
			if (nVals != 1) {
			   eprintf
			       ("ERROR: Non-indexed .replace with ambiguous %u value target in \"%s\" at line %u.",
				nVals, fname, lineNo);
			   goto abort;
			}

			free(PTRVEC_remHead(&pEntry->value_lst));
			PTRVEC_addTail(&pEntry->value_lst, strdup(valStr));

		     }
		  } else {
		     assert(!(stateFlags & NDX_STATE));
		     /* Add the symbol+value pair to the memory based map */
		     if (CFGMAP_append(self, symBuf, symLen, valStr)) {
			assert(0);
		     }
		     /* Handle the case where a curly brace block begins */
		     if (stateFlags & CURLY_BEGIN_STATE) {
			PTRVEC_addTail(&self->curlyBlock_lst, strdup(valStr));
		     }
		  }
	       }		/* symbol + value pair */
 doneProcessing:
	       ;
	    }

	    /* Non-empty string */
	    /* All done? */
	    if (c == EOF)
	       break;

	    /* Get ready for the next line */
	    ++lineNo;
	    currLen = 0;
	    charNo = 0;
	    stateFlags = 0;
	 }

	 /* EOL or EOF */
	 /* Ignore other non-printing characters */
	 if (!isprint(c)) {
	    continue;
	 }

	 /* Handle escaped characters */
	 if (c == '\\' &&
	     !(stateFlags & COMMENT_STATE) && !(stateFlags & ESCAPED_STATE)) {

	    stateFlags |= ESCAPED_STATE;
	    continue;
	 }

	 /* Handle the different states of the "state machine" */
	 if (stateFlags & COMMENT_STATE) {	/* In the middle of a comment */
	    /* Ignore character */
	    continue;

	 } else if (stateFlags & STRING_STATE) {	/* In the middle of a quoted string */

	    switch (c) {
	    case '"':
	       if (!(stateFlags & ESCAPED_STATE)) {
		  stateFlags &= ~STRING_STATE;
	       }
	       break;
	    }

	    /* valid character */
	    lineBuf[currLen] = c;
	    ++currLen;

	 } else {		/* Currently no special state */

	    switch (c) {
	    case '#':		/* Ignore rest of line */
	       if (!(stateFlags & ESCAPED_STATE)) {
		  stateFlags |= COMMENT_STATE;
		  continue;
	       }
	       break;

	    case '"':		/* This is a string which may contain a # character */
	       if (!(stateFlags & ESCAPED_STATE)) {
		  stateFlags |= STRING_STATE;
	       }
	       break;

	    default:
	       if (stateFlags & ESCAPED_STATE) {
		  lineBuf[currLen] = '\\';
		  ++currLen;
	       }
	    }
	    /* This is a valid character */
	    lineBuf[currLen] = c;
	    ++currLen;

	 }			/*  Currently no special state */
	 stateFlags &= ~ESCAPED_STATE;
      }				/* character by character for loop */
   }				/* Parse file scope */

   /* Note the change in recursion level */
   --self->recurs_lvl;

   /* Check for unmatched curly braces in the outermost file */
   if (!self->recurs_lvl && PTRVEC_numItems(&self->curlyBlock_lst)) {
      eprintf("ERROR: Unterminated curly brace in \"%s\" at line %u.", fname,
	      lineNo);
      goto abort;
   }

   rtn = 0;

 abort:
   return rtn;
}

int
CFGMAP_file_read(CFGMAP * self, const char *fname)
/**************************************************************************
 * Read a file to populate the map.
 */
{
   int rtn = 1;
   FILE *fh = NULL;

#ifdef DDEBUG
   eprintf("INFO: %s(\"%s\")", __FUNCTION__, fname);
#endif

   /* Open the supplied file for reading */
   if (!(fh = fopen(fname, "r"))) {
      eprintf("ERROR: cannot open \"%s\"", fname);
      goto abort;
   }

   if (_CFGMAP_fh_read(self, fh, fname))
      goto abort;

   rtn = 0;

 abort:
   if (fh)
      fclose(fh);
   return rtn;
}

int
CFGMAP_query_uint(CFGMAP * self, unsigned int *pRtn, unsigned int dfltVal, const char *symbol)
/**********************************************************************************
 * Convenience query function.
 */
{
   const char *valStr;

   /* Initialize with default value */
   *pRtn = dfltVal;

   if (!(valStr = CFGMAP_find_single_value(self, symbol)))
      return 0;

   if (sscanf(valStr, "%u", pRtn) != 1) {
      eprintf("ERROR: \"%s = %s\" is not a valid unsigned integer specifier.",
	      symbol, valStr);
      return 1;
   }

   return 0;
}

int
CFGMAP_query_last_flags(
      CFGMAP * self,
      int *pRtn,
      int dfltVal,
      const char *symbol,
      const struct bitTuple bt_arr[]
      )
/**********************************************************************************
 * Convenience query function for possibly OR'd flags. bt_arr[] is terminated with a NULL value
 * in enumStr.
 */
{
   const char *valStr, *str;
   size_t flagLen;

   // TODO: implement all bit operators (e.g. '|', '~', '<<', '>>')

   /* Initialize with default value */
   *pRtn = dfltVal;

   /* See if any corresponding entries exist in the map */
   if (!(valStr = CFGMAP_find_last_value(self, symbol)))
      return 0;

   int64_t rtnBuf;
   if(-1 == str2bits(&rtnBuf, valStr, bt_arr)) {
      eprintf("ERROR: \"%s\" is not a valid flag.", symbol, valStr);
      return 1;
   }

   *pRtn= rtnBuf;


#if 0
   /* Go through the OR'd flags, one at a time */
   for (str = valStr; (flagLen = strcspn(str, " \t|"));
	str += flagLen, str += strspn(str, " \t|")) {
      char flagBuf[flagLen + 1];
      const struct bitTuple *es;

      memcpy(flagBuf, str, flagLen);
      flagBuf[flagLen] = '\0';

      for (es = es_arr; es->name; ++es) {
	 if (strcmp(es->name, flagBuf))
	    continue;
	 *pRtn |= es->bit;
	 break;
      }

      if (!es->name) {
	 eprintf("ERROR: \"%s\" is not a valid flag.", symbol, valStr);
	 return 1;
      }
   }
#endif

   return 0;
}

int
CFGMAP_query_last_enum(
      CFGMAP * self,
      int *pRtn,
      int dfltVal,
      const char *symbol,
      const struct enumTuple et_arr[]
      )
/**********************************************************************************
 * Convenience query function for enums. et_arr[] is terminated with a NULL value
 * in enumStr.
 */
{
   const char *valStr;
   unsigned i;

   /* Initialize with default value */
   *pRtn = dfltVal;

   /* See if any corresponding entries exist in the map */
   if (!(valStr = CFGMAP_find_last_value(self, symbol)))
      return 0;

   const struct enumTuple *et= str2enum(valStr, et_arr);

   /* Assign enum value, if found */
   if(et) {
      *pRtn= et->enumVal;
      return 0;
   }

#if 0

   /* Look for an enum value match */
   for (i = 0; es_arr[i].name; ++i) {

      /* If no match, continue */
      if (strcmp(valStr, es_arr[i].name))
	 continue;

      /* Assign value, return */
      *pRtn = es_arr[i].enumVal;
      return 0;
   }
#endif
   return 0;
}

int
CFGMAP_query_last_bool(CFGMAP * self,
			int *pRtn, int dfltVal, const char *symbol)
/**********************************************************************************
 * Convenience function.
 */
{
   const char *val;

   *pRtn = dfltVal;

   if (!(val = CFGMAP_find_last_value(self, symbol)))
      return 0;

   if (!strcmp(val, "yes") || !strcmp(val, "true") || !strcmp(val, "1")) {
      *pRtn = 1;
      return 0;
   }

   if (!strcmp(val, "no") || !strcmp(val, "false") || !strcmp(val, "0")) {
      *pRtn = 0;
      return 0;
   }

   eprintf("ERROR: \"%s = %s\" is not a valid boolean specifier.", symbol, val);
   return 1;
}

int
CFGMAP_query_last_int(CFGMAP * self, int *pRtn, int dfltVal,
		       const char *symbol)
/**********************************************************************************
 * Convenience function.
 */
{
   const char *val;

   *pRtn = dfltVal;

   if (!(val = CFGMAP_find_last_value(self, symbol)))
      return 0;

   if (sscanf(val, "%d", pRtn) != 1) {
      eprintf("ERROR: \"%s = %s\" is not a valid integer specification.",
	      symbol, val);
      return 1;
   }

   return 0;
}

int
CFGMAP_query_last_uint(CFGMAP * self, unsigned *pRtn, unsigned dfltVal,
			const char *symbol)
/**********************************************************************************
 * Convenience function.
 */
{
   const char *val;

   *pRtn = dfltVal;

   if (!(val = CFGMAP_find_last_value(self, symbol)))
      return 0;

   if (sscanf(val, "%u", pRtn) != 1) {
      eprintf
	  ("ERROR: \"%s = %s\" is not a valid unsigned integer specification.",
	   symbol, val);
      return 1;
   }

   return 0;
}

int
CFGMAP_query_last_dbl(CFGMAP * self, double *pRtn, double dfltVal,
		       const char *symbol)
/**********************************************************************************
 * Convenience function.
 */
{
   const char *val;

   *pRtn = dfltVal;

   if (!(val = CFGMAP_find_last_value(self, symbol)))
      return 0;

   if (sscanf(val, "%lf", pRtn) != 1) {
      eprintf("ERROR: \"%s = %s\" is not a valid floating point specification.",
	      symbol, val);
      return 1;
   }

   return 0;
}

int
CFGMAP_query_last_string(CFGMAP * self, char **pRtn, const char *dfltVal,
			  const char *symbol)
/**********************************************************************************
 * Convenience function.
 * Note: string assigned to *pRtn was allocated by strdup().
 */
{
   const char *val;

   if (!(val = CFGMAP_find_last_value(self, symbol))) {

      if (dfltVal) {
	 *pRtn = strdup(dfltVal);
	 assert(*pRtn);
      } else {
	 /* NULL is a valid dfltVal */
	 *pRtn = NULL;
      }

      return 0;
   }

   return str_extract_string(pRtn, &val);

}

int
CFGMAP_query_last_time_of_day(CFGMAP * self, int *pRtn, int dfltVal,
			       const char *symbol)
/**********************************************************************************
 * Convenience function to convert hh:mm to a number of seconds.
 */
{
   const char *val;
   int hr, min;

   *pRtn = dfltVal;

   if (!(val = CFGMAP_find_last_value(self, symbol)))
      return 0;

   if (sscanf(val, "%u:%u", &hr, &min) != 2) {
      eprintf("ERROR: \"%s = %s\" is not a valid time of day specification.",
	      symbol, val);
      return 1;
   }

   /* Convert hours and minutes into seconds */
   *pRtn = 3600 * hr + 60 * min;

   return 0;
}

static int
cmp_symbol(const void *v1, const void *v2)
/******************************************************************
 * lambda function for qsort() puts entries in case insensitive,
 * alphabetical order.
 */
{
   const CFGMAP_ENTRY *const *ppE1 = (const CFGMAP_ENTRY * const *)v1,
       *const *ppE2 = (const CFGMAP_ENTRY * const *)v2;

   return strcasecmp((*ppE1)->symbol, (*ppE2)->symbol);
}

void
CFGMAP_print(CFGMAP * self, FILE * fh)
/******************************************************************
 * Print to stream for debugging purposes
 */
{
   size_t i, count = MAP_numItems(&self->entry_tbl);
   CFGMAP_ENTRY *entryPtr_arr[count];

   /* Load entry pointers into an array */
   MAP_fetchAllItems(&self->entry_tbl, (void **)entryPtr_arr);

   /* Sort the pointer array */
   qsort(entryPtr_arr, count, sizeof(CFGMAP_ENTRY *), cmp_symbol);

   /* Print the entries in sorted order */
   for (i = 0; i < count; ++i) {
      CFGMAP_ENTRY_print(entryPtr_arr[i], fh);
   }
}

static int
unused_load_arr(void *item_ptr, void *data)
/******************************************************************
 * lambda function to load unused CFGMAP_ENTRY's into an array.
 */
{
   CFGMAP_ENTRY *e = (CFGMAP_ENTRY *) item_ptr;

   /* If it was not ever looked up, it was unused */
   if (!e->nLookups) {
      void ***ppp = (void ***)data;
      **ppp = item_ptr;
      ++(*ppp);
   }
   return 0;
}

size_t
CFGMAP_numUnused_symbols(CFGMAP * self)
/******************************************************************
 * Print unused symbol count.
 */
{
   size_t count = MAP_numItems(&self->entry_tbl);
   CFGMAP_ENTRY *entryPtr_arr[count + 1], **ppEntry;

   /* Zero out the array so we can find the end */
   memset(entryPtr_arr, 0, (count + 1) * sizeof(CFGMAP_ENTRY *));

   /* Load entry pointers into an array */
   ppEntry = entryPtr_arr;
   MAP_visitAllEntries(&self->entry_tbl, unused_load_arr, &ppEntry);

   /* Find out how many entrys are unreferenced */
   for (count = 0; entryPtr_arr[count]; ++count) ;

   return count;
}

void
CFGMAP_print_unused_symbols(CFGMAP * self, FILE * fh)
/******************************************************************
 * Print unused symbols for troubleshooting
 */
{
   size_t count = MAP_numItems(&self->entry_tbl);
   CFGMAP_ENTRY *entryPtr_arr[count + 1], **ppEntry;

   /* Zero out the array so we can find the end */
   memset(entryPtr_arr, 0, (count + 1) * sizeof(CFGMAP_ENTRY *));

   /* Load entry pointers into an array */
   ppEntry = entryPtr_arr;
   MAP_visitAllEntries(&self->entry_tbl, unused_load_arr, &ppEntry);

   /* Find out how many entrys are unreferenced */
   for (count = 0; entryPtr_arr[count]; ++count) ;

   /* Sort the pointer array */
   qsort(entryPtr_arr, count, sizeof(CFGMAP_ENTRY *), cmp_symbol);

   /* Print the entries in sorted order, stopping when null pointer is encountered */
   for (ppEntry = entryPtr_arr; *ppEntry; ++ppEntry) {
      CFGMAP_ENTRY_print(*ppEntry, fh);
   }
}

unsigned int
CFGMAP_numEntries(CFGMAP * self)
/******************************************************************
 * Return the number of entries stored in the configuration map.
 */
{
   return MAP_numItems(&self->entry_tbl);
}

static int count_tuples(void *item_ptr, void *data)
/******************************************************************
 * lambda function to count tuples
 */
{
   const CFGMAP_ENTRY *e = (const CFGMAP_ENTRY *)item_ptr;
   unsigned *pCount = (unsigned *)data;

   *pCount += CFGMAP_ENTRY_numValues(e);

   return 0;
}

unsigned int
CFGMAP_numTuples(CFGMAP * self)
/******************************************************************
 * Return the number of tuples stored in the configuration map.
 */
{
   /* Optimization to avoid recounting tuples */
   if (!(self->flags & CFGMAP_NTUPLES_KNOWN_FLG)) {
      MAP_visitAllEntries(&self->entry_tbl, count_tuples, &self->nTuples);
      self->flags |= CFGMAP_NTUPLES_KNOWN_FLG;
   }

   return self->nTuples;
}

unsigned
CFGMAP_find_tuples(CFGMAP * self, struct CFGMAP_tuple rtn_arr[],
		    const char *symbol)
/************************************************************************
 * Find all tuples with a key matching symbol
 * returns the number of tuples which were populated.
 */
{
   unsigned i, rtn = 0;
   const CFGMAP_ENTRY *e;

   /* See if there is a corresponding entry */
   if (!(e = CFGMAP_find(self, symbol)))
      return 0;

   /* Now, load up the return buffer */
   for (i = 0; i < CFGMAP_ENTRY_numValues(e); ++i) {
      struct CFGMAP_tuple *t = rtn_arr + rtn;
      t->key = e->symbol;
      t->value = CFGMAP_ENTRY_value_sn(e, &t->serial_no, i);
      ++rtn;
   }
   return rtn;
}

/*===========================================================================================*/
/*========================== VALUE ==========================================================*/
/*===========================================================================================*/
static VALUE*
VALUE_constructor(VALUE * self, const char *str, unsigned serial_no)
/************************************************************************
 * Prepare for use.
 */
{
   self->str = strdup(str);
   self->serial_no = serial_no;
   return self;
}

static void*
VALUE_destructor(VALUE * self)
/************************************************************************
 * Free resources.
 */
{
   if (self->str)
      free(self->str);
   return self;
}

/*===========================================================================================*/
/*======================== CFGMAP_ENTRY ===================================================*/
/*===========================================================================================*/
void
CFGMAP_ENTRY_print(const CFGMAP_ENTRY * self, FILE * fh)
/************************************************************************
 * Print information for debugging.
 */
{
   unsigned count, i;

   fprintf(fh, "\"%s\":", self->symbol);

   for (i = 0, count = CFGMAP_ENTRY_numValues(self); i < count; ++i) {
      unsigned sn;
      const char *str = CFGMAP_ENTRY_value_sn(self, &sn, i);
      fprintf(fh, "\n\t%u\t\"%s\"", sn, str);
   }

   fprintf(fh, "\n");
}

const char*
CFGMAP_ENTRY_symbol(const CFGMAP_ENTRY * self)
/******************************************************************
 * Return the number of values stored for a given entry.
 */
{
   return self->symbol;
}

unsigned int
CFGMAP_ENTRY_numValues(const CFGMAP_ENTRY * self)
/******************************************************************
 * Return the number of values stored for a given entry.
 */
{
   return PTRVEC_numItems(&self->value_lst);
}

static VALUE*
CFGMAP_ENTRY_append_value(CFGMAP_ENTRY * self,
					 const char *val_str,
					 unsigned serial_no)
/******************************************************************
 * append a value to this entry
 */
{
   VALUE *v;
   VALUE_create(v, val_str, serial_no);
   assert(v);
   PTRVEC_addTail(&self->value_lst, v);
   return v;
}

const char*
CFGMAP_ENTRY_value(const CFGMAP_ENTRY * self, unsigned ndx)
/******************************************************************
 * Return the string for a the value indicated by ndx.
 */
{
   const VALUE *v = PTRVEC_ndxPtr(&self->value_lst, ndx);
   return v ? v->str : NULL;
}

const char*
CFGMAP_ENTRY_value_sn(const CFGMAP_ENTRY * self, unsigned *sn_buf, unsigned ndx)
/******************************************************************
 * Return the string *and* serial number for a the value indicated by ndx.
 */
{
   const VALUE *v = PTRVEC_ndxPtr(&self->value_lst, ndx);
   /* Put serial number into supplied buffer */
   if (v && sn_buf) {
      *sn_buf = v->serial_no;
   }

   return v ? v->str : NULL;
}

static CFGMAP_ENTRY*
CFGMAP_ENTRY_constructor(CFGMAP_ENTRY * self, const char *symbol, unsigned symLen)
/************************************************************************
 * Prepare a new CFGMAP_ENTRY for use.
 */
{
   if (!self)
      return NULL;
   memset(self, 0, sizeof(*self));

   self->symLen = symLen;
   if (!PTRVEC_constructor(&self->value_lst, 10) ||
       !(self->symbol = strdup(symbol)))
      return NULL;

   return self;
}

static void*
CFGMAP_ENTRY_destructor(CFGMAP_ENTRY * self)
/************************************************************************
 * Free resources associated with a configuration entry.
 */
{
   VALUE *v;

   while ((v = PTRVEC_remHead(&self->value_lst))) {
      VALUE_destroy(v);
   }

   PTRVEC_destructor(&self->value_lst);

   if (self->symbol)
      free(self->symbol);
   return self;
}


static int
str_extract_string(char **rtnVal, const char **pStr)
/*****************************************************************
 * Extract a substring from the string, advance pointer forward.
 */
{
   const char *str = *pStr;
   int rtn = 1, len;

   /* Skip leading whitespace */
//   for (; **pStr && isspace(**pStr); (*pStr)++) ;
   *pStr= skipspace(*(char**)pStr);

   if (**pStr == '"') {		/* Quoted string */

      /* Skip the quote */
      ++(*pStr);

      /* Find how far to matching quote */
      for (len = 0; (*pStr)[len] && (*pStr)[len] != '"'; ++len) ;

      /* Make sure there was a matching quote */
      if ((*pStr)[len] == '\0')
	 goto abort;

      /* Allocate buffer to store string */
      *rtnVal = malloc(len + 1);
      assert(*rtnVal);

      /* Copy string into buffer */
      memcpy(*rtnVal, *pStr, len);

      /* Null terminate buffer */
      (*rtnVal)[len] = '\0';

      /* Advance pointer past second quote */
      *pStr += len + 1;

   } else {			/* Unquoted string */

      /* Find how far to end of string */
      for (len = 0; ((*pStr)[len] && !isspace((*pStr)[len])); ++len) ;

      /* Allocate buffer to store string */
      *rtnVal = malloc(len + 1);
      assert(*rtnVal);

      /* Copy string into buffer */
      memcpy(*rtnVal, *pStr, len);

      /* Null terminate string */
      (*rtnVal)[len] = '\0';

      /* Advance pointer past end of string */
      *pStr += len;

   }

   /* Successful return */
   rtn = 0;

 abort:
   if (rtn)
      eprintf("ERROR: \"%s\" is not a valid string specification.", str);
   return rtn;
}

int
CFGMAP_obtain_prefix(char *rtnBuf, size_t buf_sz, const char *path)
/******************************************************************
 * Given a full path string, place the prefix in rtnBuf.
 */
{
   assert(path);

   int rtn= EOF-1;
   const char *pc,
              *end= strrchr(path, '\\');

   if(!end || path == end) 
      end= strchr(path, '\0');

   if(('\\' != *path) || (path == end) || (path+1 == end)) {
      eprintf("ERROR: cannot obtain prefix from \"%s\"", path);
      goto abort;
   }

   unsigned nChars= 0;
   for(pc= path; nChars < buf_sz && *pc && pc != end; ++pc) {
      rtnBuf[nChars]= *pc;
      ++nChars;
   }

   if(nChars == buf_sz) {
      rtn= EOF;
      goto abort;
   }

   rtnBuf[nChars]= '\0';

   rtn= 0;
abort:
   return rtn;
}

