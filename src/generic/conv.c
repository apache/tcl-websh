/* ----------------------------------------------------------------------------
 * conv.c --- conversion (htmlify, uricode)
 * nca-073-9
 * 
 * Copyright (c) 1996-2000 by Netcetera AG.
 * Copyright (c) 2001 by Apache Software Foundation.
 * All rights reserved.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id$
 * ------------------------------------------------------------------------- */

#include <tcl.h>
#include <stdio.h>
#include "conv.h"
#include "log.h"

/* ----------------------------------------------------------------------------
 * init -- entry point for module webenc of websh3
 * ------------------------------------------------------------------------- */
int conv_Init(Tcl_Interp *interp) {

  ConvData *convData = NULL;

  /* --------------------------------------------------------------------------
   * interpreter running ?
   * ----------------------------------------------------------------------- */
  if (interp == NULL)  return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * get memory for data structure
   * ----------------------------------------------------------------------- */
  convData = createConvData();

  /* --------------------------------------------------------------------------
   * init config-data
   * ----------------------------------------------------------------------- */
  Tcl_CreateObjCommand(interp, "web::htmlify",
		       Web_Htmlify, 
		       (ClientData)convData,
		       NULL);

  Tcl_CreateObjCommand(interp, "web::dehtmlify",
		       Web_DeHtmlify,
		       (ClientData) convData,
		       NULL);

  /* for test purposes only */
  Tcl_CreateObjCommand(interp, "web::html::removecomments",
		       Web_Html_RemoveComments,
		       (ClientData) NULL,
		       NULL);

  Tcl_CreateObjCommand(interp, "web::uriencode",
		       Web_UriEncode, 
		       (ClientData) NULL, 
		       NULL);

  Tcl_CreateObjCommand(interp, "web::uridecode",
		       Web_UriDecode, 
		       (ClientData) NULL, 
		       NULL);

  /* --------------------------------------------------------------------------
   * tie interpreter and clientdata together
   * ----------------------------------------------------------------------- */
  Tcl_SetAssocData(interp,WEB_CONV_ASSOC_DATA,
		   (Tcl_InterpDeleteProc *)destroyConvData,
		   (ClientData) convData);

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_Htmlify -- convert string from ISO-8859-1 to HTML
 * ------------------------------------------------------------------------- */
int Web_Htmlify(ClientData clientData, 
		Tcl_Interp *interp, 
		int objc, Tcl_Obj *CONST objv[]) {

  ConvData   *convData = NULL;
  Tcl_Obj    *res = NULL;
  int        useNumeric = TCL_ERROR;
  int        iCurArg = 0;
  static char *params[] = {"-numeric",NULL};
  enum params {NUMERIC};

  
  /* --------------------------------------------------------------------------
   * sanitiy
   * ----------------------------------------------------------------------- */
  WebAssertData(interp,clientData,"web::hmtlify",TCL_ERROR)
  convData = (ConvData *)clientData;

  /* --------------------------------------------------------------------------
   * args
   * ----------------------------------------------------------------------- */
  if( objc < 2 ) {
    LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	    "web::htmlify",WEBLOG_INFO,
	    "usage: ?-numeric? string",
	    NULL);
    return TCL_ERROR;
  } else if ( objc == 2 ) {
    /* ------------------------------------------------------------------------
     * assume that there are no options
     * --------------------------------------------------------------------- */
    iCurArg = 1;

  } else {

    iCurArg = argIndexOfFirstOpt(objc,objv);
    if( iCurArg == -1 ) {
      iCurArg = argIndexOfFirstArg(objc,objv,params,NULL);
      if( iCurArg != (objc - 1) ) {
	LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
		"web::htmlify",WEBLOG_INFO,
		"usage: ?-numeric? string",
		NULL);
	return TCL_ERROR;
      }
    } else {

      if( strcmp(Tcl_GetString(objv[iCurArg]),params[NUMERIC]) == 0 ) {

	int numArgs = objc - iCurArg;
	/* --------------------------------------------------------------------
	 * NUMERIC
	 * ----------------------------------------------------------------- */
	switch( numArgs ) {
	case 2:
	  iCurArg++;
	  useNumeric = TCL_OK;
	  break;
	default:
	  LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
		  "web::htmlify",WEBLOG_INFO,
		  "usage: ?-numeric? string",
		  NULL);
	  return TCL_ERROR;
	}
      }
    }
  }

  res = webHtmlify(convData,objv[iCurArg],useNumeric);

  if( res == NULL ) {
    LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	    __FILE__,__LINE__,
	    "web::htmlify",WEBLOG_ERROR,
	    "error converting \"",Tcl_GetString(objv[iCurArg]),"\"",NULL);
    return TCL_ERROR;
  }
    
  Tcl_SetObjResult(interp,res);
  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_DeHtmlify --
 *   convert string from HTML to ISO-8859-1
 * ------------------------------------------------------------------------- */
int Web_DeHtmlify (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int objc, Tcl_Obj *CONST objv[]) {

  ConvData *convData = NULL;
  Tcl_Obj  *res1 = NULL;
  Tcl_Obj  *res2 = NULL;

  /* --------------------------------------------------------------------------
   * check for private data
   * ----------------------------------------------------------------------- */
  WebAssertData(interp,clientData,"Web_DeHtmlify",TCL_ERROR)
  convData = (ConvData *)clientData;

  /* --------------------------------------------------------------------------
   * arg check
   * ----------------------------------------------------------------------- */
  WebAssertObjc(objc != 2, 1, "string");

  /* --------------------------------------------------------------------------
   * handle HTML
   * ----------------------------------------------------------------------- */
  res1 = Tcl_NewObj();
  webDeHtmlify(convData, objv[1], res1);
  Tcl_SetObjResult(interp,res1);
  return TCL_OK;
}





/* ----------------------------------------------------------------------------
 * Web_Html_RemoveComments --
 *   remove HTML comments from input
 *   this command is exported for testing purposes only.
 * ------------------------------------------------------------------------- */
int Web_Html_RemoveComments(ClientData clientData, 
			    Tcl_Interp *interp, 
			    int objc, Tcl_Obj *CONST objv[]) {

  Tcl_Obj  *res = NULL;

  /* --------------------------------------------------------------------------
   * arg check
   * ----------------------------------------------------------------------- */
  WebAssertObjc(objc != 2, 1, "string");

  res = Tcl_NewObj();
  removeHtmlComments(interp,objv[1],res);

  /* ------------------------------------------------------------------------
   * done
   * --------------------------------------------------------------------- */
  Tcl_SetObjResult(interp,res);

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * create private data -- see below
 * ------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------
 * destroy private data
 * ------------------------------------------------------------------------- */
void destroyConvData(ClientData clientData, Tcl_Interp *interp) {

  ConvData          *convData = NULL;
  HashTableIterator iterator;
  int               i = 0;

  if( clientData != NULL ) {

    convData = (ConvData *)clientData;

    /* hashutl */
    assignIteratorToHashTable(convData->etu,&iterator);
    while( nextFromHashIterator(&iterator) != TCL_ERROR )
      Tcl_DecrRefCount((Tcl_Obj *)valueOfCurrent(&iterator));

    HashUtlDelFree(convData->etu);

    /* ute */
    for( i = 0; i < WEBENC_LATIN_TABLE_LENGTH; i++)
      if( convData->ute[i] != NULL )
        Tcl_DecrRefCount(convData->ute[i]);

    WebFreeIfNotNull(convData);
  }
}

/* ----------------------------------------------------------------------------
 * convDataAddValue
 * ------------------------------------------------------------------------- */
int convDataAddValue(ConvData *convData, char *entity, int num) {

  if( (convData == NULL) || (entity == NULL) ) return TCL_ERROR;

  /* html-entity -> iso lookup (just add once) */
  if( appendToHashTable(convData->etu,entity,
  	(ClientData)Tcl_NewIntObj(num)) == TCL_ERROR )
    return TCL_ERROR;

  /* iso -> html-entity lookup (last wins) */
  WebDecrRefCountIfNotNull(convData->ute[num]);
  convData->ute[num] = Tcl_NewStringObj(entity,-1);

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * create private data
 * ------------------------------------------------------------------------- */
ConvData *createConvData() {

  int      i = 0;
  ConvData *convData = NULL;

  /* --------------------------------------------------------------------------
   * get mem
   * ----------------------------------------------------------------------- */
  convData = WebAllocInternalData(ConvData);
  if( convData == NULL ) return NULL;

  /* --------------------------------------------------------------------------
   * setup
   * ----------------------------------------------------------------------- */
  for( i = 0; i < WEBENC_LATIN_TABLE_LENGTH; i++) {
    convData->need[i] = TCL_OK;
    convData->ute[i] = NULL;
  }
  
  HashUtlAllocInit(convData->etu,TCL_STRING_KEYS);
  IfNullLogRetNull(NULL,convData->etu,
		   "createConvData - cannot create hashtable");

  /* --------------------------------------------------------------------------
   * set "need"
   * ----------------------------------------------------------------------- */
  for (i = 0; i < 160; i++)
    convData->need[i] = TCL_ERROR;

  /* &quot; */
  convData->need[34] = TCL_OK;
  /* &amp;*/
  convData->need[38] = TCL_OK;
  /* &lt; */
  convData->need[60] = TCL_OK;
  /* &gt; */
  convData->need[62] = TCL_OK;


  /* ------------------------------------------------------------------------
   * set etu and ute
   * --------------------------------------------------------------------- */
  convDataAddValue(convData,"quot", 34);
  convDataAddValue(convData,"amp", 38);
  convDataAddValue(convData,"lt", 60);
  convDataAddValue(convData,"gt", 62);
  /* nbsp is the exception:
   * nbsp --> 32, but
   * 32 --> " " and 160 --> nbsp
   */
  convDataAddValue(convData,"nbsp",32); /* not 160 */
  /* convData->unicodeToEntity[160] = allocAndSet("nbsp"); */
  convData->ute[160] = Tcl_NewStringObj("nbsp",-1);
  convDataAddValue(convData,"iexcl",161);
  convDataAddValue(convData,"cent",162);
  convDataAddValue(convData,"pound",163);
  convDataAddValue(convData,"curren",164);
  convDataAddValue(convData,"yen",165);
  convDataAddValue(convData,"brvbar",166);
  convDataAddValue(convData,"sect",167);
  convDataAddValue(convData,"uml",168);
  convDataAddValue(convData,"copy",169);
  convDataAddValue(convData,"ordf",170);
  convDataAddValue(convData,"laquo",171);
  convDataAddValue(convData,"not",172);
  convDataAddValue(convData,"shy",173);
  convDataAddValue(convData,"reg",174);
  /* Note: macr is the official tag for "¯" 
   *       hibar maps to the same character for convenience
   * Do hibar first. In the UTE table, it will be overwritten
   * in the second call, and the official "macr" will be used.
   */
  convDataAddValue(convData,"hibar",175);
  convDataAddValue(convData,"macr",175);
  convDataAddValue(convData,"deg",176);
  convDataAddValue(convData,"plusmn",177);
  convDataAddValue(convData,"sup2",178);
  convDataAddValue(convData,"sup3",179);
  convDataAddValue(convData,"acute",180);
  convDataAddValue(convData,"micro",181);
  convDataAddValue(convData,"para",182);
  convDataAddValue(convData,"middot",183);
  convDataAddValue(convData,"cedil",184);
  convDataAddValue(convData,"sup1",185);
  convDataAddValue(convData,"ordm",186);
  convDataAddValue(convData,"raquo",187);
  convDataAddValue(convData,"frac14",188);
  convDataAddValue(convData,"frac12",189);
  convDataAddValue(convData,"frac34",190);
  convDataAddValue(convData,"iquest",191);
  convDataAddValue(convData,"Agrave",192);
  convDataAddValue(convData,"Aacute",193);
  convDataAddValue(convData,"Acirc",194);
  convDataAddValue(convData,"Atilde",195);
  convDataAddValue(convData,"Auml",196);
  convDataAddValue(convData,"Aring",197);
  convDataAddValue(convData,"AElig",198);
  convDataAddValue(convData,"Ccedil",199);
  convDataAddValue(convData,"Egrave",200);
  convDataAddValue(convData,"Eacute",201);
  convDataAddValue(convData,"Ecirc",202);
  convDataAddValue(convData,"Euml",203);
  convDataAddValue(convData,"Igrave",204);
  convDataAddValue(convData,"Iacute",205);
  convDataAddValue(convData,"Icirc",206);
  convDataAddValue(convData,"Iuml",207);
  convDataAddValue(convData,"ETH",208);
  convDataAddValue(convData,"Ntilde",209);
  convDataAddValue(convData,"Ograve",210);
  convDataAddValue(convData,"Oacute",211);
  convDataAddValue(convData,"Ocirc",212);
  convDataAddValue(convData,"Otilde",213);
  convDataAddValue(convData,"Ouml",214);
  convDataAddValue(convData,"times",215);
  convDataAddValue(convData,"Oslash",216);
  convDataAddValue(convData,"Ugrave",217);
  convDataAddValue(convData,"Uacute",218);
  convDataAddValue(convData,"Ucirc",219);
  convDataAddValue(convData,"Uuml",220);
  convDataAddValue(convData,"Yacute",221);
  convDataAddValue(convData,"THORN",222);
  convDataAddValue(convData,"szlig",223);
  convDataAddValue(convData,"agrave",224);
  convDataAddValue(convData,"aacute",225);
  convDataAddValue(convData,"acirc",226);
  convDataAddValue(convData,"atilde",227);
  convDataAddValue(convData,"auml",228);
  convDataAddValue(convData,"aring",229);
  convDataAddValue(convData,"aelig",230);
  convDataAddValue(convData,"ccedil",231);
  convDataAddValue(convData,"egrave",232);
  convDataAddValue(convData,"eacute",233);
  convDataAddValue(convData,"ecirc",234);
  convDataAddValue(convData,"euml",235);
  convDataAddValue(convData,"igrave",236);
  convDataAddValue(convData,"iacute",237);
  convDataAddValue(convData,"icirc",238);
  convDataAddValue(convData,"iuml",239);
  convDataAddValue(convData,"eth",240);
  convDataAddValue(convData,"ntilde",241);
  convDataAddValue(convData,"ograve",242);
  convDataAddValue(convData,"oacute",243);
  convDataAddValue(convData,"ocirc",244);
  convDataAddValue(convData,"otilde",245);
  convDataAddValue(convData,"ouml",246);
  convDataAddValue(convData,"divide",247);
  convDataAddValue(convData,"oslash",248);
  convDataAddValue(convData,"ugrave",249);
  convDataAddValue(convData,"uacute",250);
  convDataAddValue(convData,"ucirc",251);
  convDataAddValue(convData,"uuml",252);
  convDataAddValue(convData,"yacute",253);
  convDataAddValue(convData,"thorn",254);
  convDataAddValue(convData,"yuml",255);
  return convData;
}
