/*
 * webout.h --- header file for webout, the output handler of websh3
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

#ifndef WEBOUT_H
#define WEBOUT_H

#include "tcl.h"
#include "macros.h"

#define WEB_OUT_ASSOC_DATA "web::weboutData"

/* Tags to be used with webout_eval_tag */
#define START_TAG "<?"
#define END_TAG "?>"

#define HTTP_RESPONSE "HTTP/1.0 200 OK"
#define HEADER "Content-Type","text/html", "Generator", WEBSH " " VERSION

/* ----------------------------------------------------------------------------
 * typedefs
 * ------------------------------------------------------------------------- */

struct ResponseObj;

typedef int (ResponseHeaderHandler) (Tcl_Interp * interp,
				     struct ResponseObj * responseObj,
				     Tcl_Obj * out);

typedef struct ResponseObj
{
    int sendHeader;
    ResponseHeaderHandler *headerHandler;
    long bytesSent;
    Tcl_HashTable *headers;
    Tcl_Obj *name;
    Tcl_Obj *httpresponse;
}
ResponseObj;

ResponseObj *createResponseObj(Tcl_Interp * interp, char *name,
			       ResponseHeaderHandler * headerHandler);
int destroyResponseObj(ClientData clientData, Tcl_Interp * interp);

typedef enum PutxMarkup
{
    brace = 0, tag = 1
}
PutxMarkup;

typedef struct OutData
{
    Tcl_HashTable *responseObjHash;
    ResponseObj *defaultResponseObj;
    /* private members */
    PutxMarkup putxMarkup;	/* markuptype for putx: brace ({}) |tag (<%%>) */
}
OutData;



/* ----------------------------------------------------------------------------
 * prototypes
 * ------------------------------------------------------------------------- */

OutData *createOutData(Tcl_Interp * interp);
int resetOutData(Tcl_Interp * interp, OutData * outData);
int destroyOutData(ClientData clientData, Tcl_Interp * interp);
int createResponseObjHash(OutData * outData);
int destroyResponsObjHash(OutData * outData, Tcl_Interp * interp);

int webout_Init(Tcl_Interp * interp);
Tcl_Channel getChannel(Tcl_Interp * interp, ResponseObj * responseObj);
ResponseObj *getResponseObj(Tcl_Interp * interp, OutData * outData,
			    char *name);
int putsCmdImpl(Tcl_Interp * interp, ResponseObj * responseObj,
		Tcl_Obj * str);
int webout_eval_brace(Tcl_Interp * interp, ResponseObj * responseObj,
		      Tcl_Obj * in);
int webout_eval_tag(Tcl_Interp * interp, ResponseObj * responseObj,
		    Tcl_Obj * in, const char *strstart, const char *strend);

/* need this for all "real channel based" (vs apache) output */
int objectHeaderHandler(Tcl_Interp * interp, ResponseObj * responseObj,
			Tcl_Obj * out);

/* cgi/apache dependent prototypes */
ResponseObj *createDefaultResponseObj(Tcl_Interp * interp);

int isDefaultResponseObj(char *name);



/* ----------------------------------------------------------------------------
 * The websh3-Commands
 * ------------------------------------------------------------------------- */
int Web_Puts(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Response(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Eval(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

#endif
