/*
 * conv.h --- header file for conv, the encoding module of websh3
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
 * 
 */

#include <tcl.h>
#include "webutl.h"
#include "hashutl.h"

#ifndef CONV_H
#define CONV_H
/* --------------------------------------------------------------------------
 * Commands
 * ------------------------------------------------------------------------*/ 

/* ----------------------------------------------------------------------------
 * SubCommands 
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Switches
 * Params
 * ------------------------------------------------------------------------- */
#define WEB_CONV_HTMLIFY_SWITCH_NUMERIC "-numeric"

/* --------------------------------------------------------------------------
 * Registered Data (SetAssocData) 
 * ------------------------------------------------------------------------*/ 
#define WEB_CONV_ASSOC_DATA "web::conv"

/* --------------------------------------------------------------------------
 * Internas
 * ------------------------------------------------------------------------*/ 


#define WEBENC_LATIN_TABLE_LENGTH 256

/* ----------------------------------------------------------------------------
 * internal data
 * ------------------------------------------------------------------------- */
typedef struct ConvData {
  int      need[WEBENC_LATIN_TABLE_LENGTH];
  Tcl_Obj  *ute[WEBENC_LATIN_TABLE_LENGTH]; /* ">" --> "gt" */
  Tcl_HashTable *etu; /* "gt" --> ">" */
} ConvData;
ConvData *createConvData();
void     destroyConvData(ClientData clientData, Tcl_Interp *interp);

/* ----------------------------------------------------------------------------
 * Tcl interface
 * ------------------------------------------------------------------------- */
int        conv_Init(Tcl_Interp *interp);

int Web_Htmlify(ClientData clientData, 
		Tcl_Interp *interp, 
		int objc, Tcl_Obj *CONST objv[]);

int Web_UriEncode(ClientData clientData, 
		  Tcl_Interp *interp, 
		  int objc, Tcl_Obj *CONST objv[]);
int Web_UriDecode(ClientData clientData, 
		  Tcl_Interp *interp, 
		  int objc, Tcl_Obj *CONST objv[]);

int Web_DeHtmlify(ClientData clientData, 
		  Tcl_Interp *interp, 
		  int objc, Tcl_Obj *CONST objv[]);

void htmlifyAppendNum(Tcl_Obj *tclo, int num);

Tcl_Obj *webHtmlify(ConvData *convData, Tcl_Obj *in, int useNumeric);
int webDeHtmlify(ConvData *convData, Tcl_Obj *in, Tcl_Obj* out);

Tcl_Obj *uriEncode(Tcl_Obj *inString);
Tcl_Obj *uriDecode(Tcl_Obj *inString);


/* ----------------------------------------------------------------------------
 * prototypes
 * ------------------------------------------------------------------------- */
int removeHtmlComments(Tcl_Interp *interp, Tcl_Obj *in, Tcl_Obj *res);
int removeShortHtmlComments(Tcl_Obj *in, Tcl_Obj *res);
int removeHtmlTags(Tcl_Obj *in, Tcl_Obj *res);
int convertHtmlEntities(ConvData *convData, Tcl_Obj *in, Tcl_Obj *res);
Tcl_UniChar getNumericEntity(Tcl_UniChar **str, int len);
int Web_Html_RemoveComments(ClientData clientData, 
			    Tcl_Interp *interp, 
			    int objc, Tcl_Obj *CONST objv[]);

#endif
