/*
 * request_cgi.c -- get request data from env
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


#include <stdio.h>

#include <tcl.h>
#include "hashutl.h"
#include "webutl.h"
#include "request.h"

/* ----------------------------------------------------------------------------
 * web::request -channel: where input for request obj comes from
 * ------------------------------------------------------------------------- */
Tcl_Obj *requestGetDefaultChannelName()
{
    return Tcl_NewStringObj("stdin", 5);
}

/* default output channel */

char *requestGetDefaultOutChannelName()
{
    return CGICHANNEL;
}


int requestFillRequestValues(Tcl_Interp * interp, RequestData * requestData)
{
    if (requestData->requestIsInitialized)
	return TCL_OK;
    requestData->requestIsInitialized = 1;
    return Tcl_Eval(interp, "web::cgi::copyenv");
}


int requestScriptName(Tcl_Interp *interp, char **filename) {
  /* fixme: we should return script filen name from the CGI env 
   * what's the status of such a char pointer (who allocates)??
   * Maybe it would be easier to return a Tcl_Obj */
  *filename = "";
  return TCL_OK;
}
