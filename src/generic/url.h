/*
 * url.h --- websh3 URL generation (command web::cmdurl)
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

#include "tcl.h"
#include "webutl.h"

#ifndef URL_H
#define URL_H

#include "request.h"


/* --------------------------------------------------------------------------
 * Registered Data
 * ------------------------------------------------------------------------*/ 
#define WEB_URL_ASSOC_DATA "web::urlData"

/* --------------------------------------------------------------------------
 * defaults
 * ------------------------------------------------------------------------*/ 

#define WEB_DEFAULT_SCHEME "http"
#define WEB_DEFAULT_PORT   "80"

/* --------------------------------------------------------------------------
 * Internas
 * ------------------------------------------------------------------------*/ 

#define WEBURL_SCHEME_SEP ":"
#define WEBURL_HOST_SEP "//"
#define WEBURL_PORT_SEP ":"
#define WEBURL_QUERY_STRING_SEP "?"

#define WEB_URL_NOCMD       1
#define WEB_URL_NOTIMESTAMP 2

#define WEB_URL_WITH_SCHEME       1
#define WEB_URL_WITH_HOST         2
#define WEB_URL_WITH_PORT         4
#define WEB_URL_WITH_SCRIPTNAME   8
#define WEB_URL_WITH_PATHINFO    16
#define WEB_URL_WITH_QUERYSTRING 32

#define WEB_URL_URLFORMAT (8 + 16 + 32)

/* -------------------------------------------------------------------
 * UrlData
 * ------------------------------------------------------------------- */
typedef struct UrlData {
  Tcl_Obj  *scheme;    /* e.g http */
  Tcl_Obj  *port; /* e.g. 8080 */
  Tcl_Obj  *host; /* e.g. www.netcetera.ch */
  Tcl_Obj  *scriptname; /* e.g. /bin/tutorial.ws3 */
  Tcl_Obj  *pathinfo;   /* e.g. /some/path */
  Tcl_Obj  *querystring;   /* e.g. foo=bar */
  /* ............ */
  RequestData *requestData; /* link to request module for host etc */
  /* ............ */
  int      urlformat;
} UrlData;

UrlData *createUrlData(void);
int     resetUrlData(Tcl_Interp *interp, UrlData *urlData);
void    destroyUrlData(ClientData clientData, Tcl_Interp *interp);

int url_Init(Tcl_Interp *interp);
int Web_CmdUrlCfg(ClientData clientData, 
		  Tcl_Interp *interp, 
		  int objc, Tcl_Obj *CONST objv[]);
int Web_CmdUrl(ClientData clientData, 
	       Tcl_Interp *interp, 
	       int objc, Tcl_Obj *CONST objv[]);
Tcl_Obj *createQueryList(Tcl_Interp *interpm, Tcl_Obj *cmd, Tcl_Obj *plist, 
                         UrlData *urlData, int flag);

#endif
