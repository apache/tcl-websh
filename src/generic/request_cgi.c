/*
 * request_cgi.c -- get request data from env
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


#include <stdio.h>

#include <tcl.h>
#include "hashutl.h"
#include "webutl.h"
#include "request.h"

/* ----------------------------------------------------------------------------
 * web::request -channel: where input for request obj comes from
 * ------------------------------------------------------------------------- */
Tcl_Obj *requestGetDefaultChannelName() {
   return Tcl_NewStringObj("stdin",5);
}


int requestFillRequestValues(Tcl_Interp *interp, RequestData *requestData) {
  if (requestData->requestIsInitialized)
    return TCL_OK;
  requestData->requestIsInitialized = 1;
  return Tcl_Eval(interp, "web::cgi::copyenv");
}