/* 
 * htmlify.c --- convert ISO-8859-1 to HTML and back
 * nca-073-9
 * 
 * Copyright (c) 1996-2000 by Netcetera AG.
 * All rights reserved.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id$
 *
 */

#include <tcl.h>
#include <stdio.h>
#include <string.h>
#include "webutl.h"
#include "conv.h"
#include "log.h"

/* ----------------------------------------------------------------------------
 * htmlifyAppendNum
 * ------------------------------------------------------------------------- */
void htmlifyAppendNum(Tcl_Obj *tclo, int num) {

  Tcl_Obj *intObj = NULL;

  if( tclo == NULL ) return;

  intObj = Tcl_NewIntObj(num);

  if( intObj != NULL ) {
    Tcl_AppendToObj(tclo,"&#",2);
    Tcl_AppendObjToObj(tclo,intObj);
    Tcl_DecrRefCount(intObj);
    Tcl_AppendToObj(tclo,";",1);
  }
}


/* ----------------------------------------------------------------------------
 * webHtmlify -- convert string from ISO-8859-1 to HTML
 * ------------------------------------------------------------------------- */
Tcl_Obj *webHtmlify(ConvData *convData, Tcl_Obj *in, int useNumeric) {

  int          iPos = 0;
  int          len = 0;
  Tcl_UniChar  unic = 0;
  Tcl_Obj      *entity = NULL;
  Tcl_Obj      *res = NULL;

  if( (convData == NULL) || (in == NULL) ) return NULL;

  res = Tcl_NewObj();

  /* ------------------------------------------------------------------------
   * loop over input string
   * --------------------------------------------------------------------- */
  len = Tcl_GetCharLength(in);

  for( iPos = 0; iPos < len; iPos++ ) {

    unic = Tcl_GetUniChar(in, iPos);

    if( unic == 0 ) break;

    if(unic > WEBENC_LATIN_TABLE_LENGTH ) continue;

    /* --------------------------------------------------------------------
     * translation needed ?
     * ----------------------------------------------------------------- */
    if( convData->need[unic] == TCL_OK ) {

      /* yes */

      if( useNumeric == TCL_OK) {

	/* numeric ? */
	htmlifyAppendNum(res,unic);
      } else {

	/* no, entity */

	entity = convData->ute[unic];

	if( entity == NULL ) {
	  htmlifyAppendNum(res,unic);
	} else {
	  
	  Tcl_AppendToObj(res,"&",1);
	  Tcl_AppendObjToObj(res,entity);
	  Tcl_AppendToObj(res,";",1);
	}
      }
    } else {

      /* no, no translation needed */
      Tcl_AppendUnicodeToObj(res,&unic,1);
    }
  }

  return res;
}

/* ----------------------------------------------------------------------------
 * Macros for webDeHtmlify
 * 
 * Note: All macros end with 'pos' on the last position that belonged
 *       to the tag or entity
 * ------------------------------------------------------------------------- */

/* <!> */
/* <-- */
#define HANDLE_TAG(unic,length,out,pos,err) { \
  int open = 1;  /* number of open '<' */ \
  int begin = pos; \
  int isCmt = 0; \
  if( length >= 4 ) \
    if( (unic[pos+1] == '!') && (unic[pos+2] == '-') && (unic[pos+3] == '-') ) isCmt = 1; \
  pos++; \
  while (pos < length ) { \
    if (unic[pos] == '>') { \
      if( isCmt ) { \
	if( (unic[pos-1] == '-') && (unic[pos-2] == '-') ) { \
          open--; \
          isCmt = 0; \
	  pos++; \
	  break; \
	} else { \
	  /* unfinished comment. continue */ \
	} \
      } else { \
	/* end-tag */ \
        open--; \
	pos++; \
	break; \
      } \
    } \
    pos++; \
  } \
  pos--; /* to be on last char of tag */ \
  if (isCmt || open) { \
    /* unfinished comment. append */ \
    Tcl_AppendUnicodeToObj(out,&(unic[begin]),pos - begin + 1); \
  } \
}

#define HANDLE_ENTITY(convData, unic, length, out, pos, err) { \
  int begin = pos; \
  int end = ++pos; \
  int first = end; \
  int nobreak = 1; \
   \
  if (first >= length) {  \
    /* an ampersand at the very last position, just write it */ \
    Tcl_AppendUnicodeToObj(out,&(unic[begin]),1); \
  } else { \
    /* search for end of entity */ \
\
	     while (nobreak) { \
	       switch (unic[end]) { \
	       case ';': \
		 pos = end; \
		 nobreak = 0; \
		 break; \
	       case ' ': \
                 pos = end - 1; \
		 nobreak = 0; \
		 break; \
	       case '<': \
                 pos = end - 1; \
		 nobreak = 0; \
		 break; \
	       default : \
		 if (end >= length) { \
                   pos = length - 1; \
				       /*end++; */ \
		   nobreak = 0; \
		   break; \
		 } \
		 end++; \
                 break; \
	       } \
	     } \
     \
    if (unic[first] == '#') { \
      /* a number */ \
      HANDLE_UNICODE_ENTITY(unic, length, out, begin, first, end, err); \
    } else { \
      HANDLE_KEY_ENTITY(convData, unic, length, out, begin, first, end, err); \
    } \
  } \
} 

#define HANDLE_UNICODE_ENTITY(unic, length, out, begin, first, end, err) { \
  int tInt = 0; \
  Tcl_UniChar tmp = 0; \
  Tcl_Obj* entity; \
  first ++; \
   \
  entity = Tcl_NewUnicodeObj(&(unic[first]),end-first); \
  if( Tcl_GetIntFromObj(NULL,entity,&tInt) == TCL_ERROR ) { \
    /* no valid number, we write the string instead */ \
    Tcl_AppendUnicodeToObj(out,&(unic[begin]),end-begin); \
    err++; \
  } else { \
    /* check if within range of Tcl_UniChar */ \
    if (tInt > 32768 - 1) { \
      /* no, we write the string instead */ \
      Tcl_AppendUnicodeToObj(out,&(unic[begin]),end-begin); \
      if (end < length && unic[end] == ';') /* don't forget this one! */ \
	Tcl_AppendUnicodeToObj(out,&(unic[end]),1); \
      err++; \
    } else { \
      tmp = (Tcl_UniChar)tInt; \
      Tcl_AppendUnicodeToObj(out,&tmp,1); \
    } \
  } \
}

#define HANDLE_KEY_ENTITY(convData, unic, length, out, begin, first, end, err) { \
  /* use lookup table */ \
  Tcl_Obj* iObj = NULL; \
  Tcl_Obj* entity = Tcl_NewUnicodeObj(&(unic[first]),end-first); \
  iObj   = (Tcl_Obj *)getFromHashTable(convData->etu, \
				      Tcl_GetString(entity)); \
  Tcl_DecrRefCount(entity); \
   \
  if( iObj != NULL ) { \
    /* got it in table */ \
    int tInt = 0; \
    if( Tcl_GetIntFromObj(NULL,iObj,&tInt) != TCL_ERROR ) { \
      Tcl_UniChar tmp = (Tcl_UniChar) tInt; \
      Tcl_AppendUnicodeToObj(out,&tmp,1); \
      /* don't kill iObj, it's owned by the hashtable */ \
    } else { \
      /* we do not have invalid values in the hashtable !*/ \
    } \
  } else { \
    /* not in table, we write the string instead */ \
    Tcl_AppendUnicodeToObj(out,&(unic[begin]),end-begin); \
    if (end < length && unic[end] == ';') /* don't forget this one! */ \
      Tcl_AppendUnicodeToObj(out,&(unic[end]),1); \
    err++; \
  }	 \
}

/* ----------------------------------------------------------------------------
 *  webDeHtmlify -- de-htmlifies input string 'in' and writes to 'out'
 * ------------------------------------------------------------------------- */
int webDeHtmlify(ConvData *convData, Tcl_Obj *in, Tcl_Obj* out) {

  int length = 0; /* length of input */
  int pos = 0;    /* actual position in string */
  Tcl_UniChar* unic = NULL;
  int plainfirst = 0;
  int plainend = 0;
  int dump = 0;
  int err = 0;    /* temporary use, may be removed */

  Tcl_Obj* err_buffer = NULL;

  if (in == NULL || out == NULL) {
    return TCL_ERROR;
  }

  unic   = Tcl_GetUnicode(in);
  length = Tcl_GetCharLength(in);

  if( length == 1 ) {
    if( (unic[0] == '>') || (unic[pos] == '>') ) {
      /* nada */
      return TCL_OK;
    } else {
      Tcl_AppendUnicodeToObj(out,&unic[0],1);
      return TCL_OK;
    }
  }

  while (pos < length) {

    plainend = pos;
    if (unic[pos] == '<') {

      /* dump */
      Tcl_AppendUnicodeToObj(out,&unic[plainfirst],plainend - plainfirst);

      /* --------------------------------------------------------------------- 
       * we're in a tag, thus we skip everything 
       * --------------------------------------------------------------------*/
      HANDLE_TAG(unic, length, out, pos, err);
      plainfirst = pos + 1;

    } else if (unic[pos] == '>') {

      /* dump */
      Tcl_AppendUnicodeToObj(out,&unic[plainfirst],plainend - plainfirst);
      /* syntax error, too many closing '>' */
      Tcl_AppendUnicodeToObj(out,&(unic[pos]),1);
      /*Tcl_SetStringObj(out,"Error: web::dehtmlify, unbalanced '>'",-1);*/
      plainfirst = pos + 1;

    } else if (unic[pos] == '&') {

      /* dump */
      Tcl_AppendUnicodeToObj(out,&unic[plainfirst],plainend - plainfirst);
      /*
       * it's an entity
       */
      HANDLE_ENTITY(convData, unic, length, out, pos, err);
      plainfirst = pos + 1;
    }

    /* ------------------------------------------------------------------------
     * search on
     * ----------------------------------------------------------------------*/
    pos++;
  }
  /* final dump */
  if (plainend >= plainfirst && plainend > 0) {
    Tcl_AppendUnicodeToObj(out,&unic[plainfirst],plainend - plainfirst + 1);
  }

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * findCmtClose 
 * ------------------------------------------------------------------------- */
char *findHtmlCmtClose(char *utf) {

  char *cmtclose = NULL;
  char *next1 = NULL;
  char *next2 = NULL;

  if( utf == NULL ) return NULL;

  while( (cmtclose = Tcl_UtfFindFirst(utf,'-')) != NULL) {

    next1 = NULL;
    next2 = NULL;

    next1 = Tcl_UtfNext(cmtclose);
    if( next1 != NULL ) next2 = Tcl_UtfNext(next1);

    if( (next1[0] == '-') && (next2[0] == '>' ) ) {
      return next2;
    }
    utf = Tcl_UtfNext(cmtclose);
  }
  return NULL;
}

/* ----------------------------------------------------------------------------
 * removeHtmlComments --
 *   Scans inString for HTML comments. Upon completion, inString will
 *   contain the input minus all HTML comments.
 * ------------------------------------------------------------------------- */
int removeHtmlComments(Tcl_Interp *interp, Tcl_Obj *in, Tcl_Obj *res) {

  int  len = 0;
  char *utf = NULL;
  char *cmtopen = NULL;
  char *cmtclose = NULL;
  char *next1 = NULL;
  char *next2 = NULL;
  char *next3 = NULL;

  if( (in == NULL) || (res == NULL) ) return TCL_ERROR;

  utf = Tcl_GetStringFromObj(in,&len);

  if( len == 0 ) return TCL_OK;

  /* --------------------------------------------------------------------------
   * fast forward to first "<"
   * ----------------------------------------------------------------------- */
  while( (cmtopen = Tcl_UtfFindFirst(utf,'<')) != NULL) {

    next1 = NULL;
    next2 = NULL;
    next3 = NULL;

    next1 = Tcl_UtfNext(cmtopen);
    if( next1 != NULL ) next2 = Tcl_UtfNext(next1);
    if( next2 != NULL ) next3 = Tcl_UtfNext(next2);

    if( next1[0] == '!' ) {
      /* ----------------------------------------------------------------------
       * starts like a comment.
       * ------------------------------------------------------------------- */
      if( (next2[0] == '-') &&  (next3[0] == '-' ) ) {
	Tcl_AppendToObj(res,utf,cmtopen-utf);
	cmtclose = findHtmlCmtClose(Tcl_UtfNext(next3));
	if( cmtclose == NULL ) {
	  Tcl_AppendToObj(res,cmtopen,-1);
	  LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,"removeHtmlComments",WEBLOG_INFO,
		  "end of string encountered while searching for comment-end",
		  NULL);
	  return TCL_OK;
	} else {
	  utf = Tcl_UtfNext(cmtclose);
	}
      } else {

	if( next2[0] == '>') {
	  Tcl_AppendToObj(res,utf,cmtopen-utf);
	  utf = next3;
	} else {
	  
	  Tcl_AppendToObj(res,utf,cmtopen-utf+1);
	  utf = next1;
	}
      }
    } else {
      /* ----------------------------------------------------------------------
       * not a comment. proceed.
       * ------------------------------------------------------------------- */
      Tcl_AppendToObj(res,utf,cmtopen-utf+1);
      utf = next1;
    }
  }
  /* ------------------------------------------------------------------------
   * append rest
   * --------------------------------------------------------------------- */
  if( utf != NULL ) Tcl_AppendToObj(res,utf,-1);

  return TCL_OK;
}

