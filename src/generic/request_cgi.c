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
#include "modwebsh_cgi.h"

/* ----------------------------------------------------------------------------
 * web::request -channel: where input for request obj comes from
 * ------------------------------------------------------------------------- */
Tcl_Obj *requestGetDefaultChannelName(Tcl_Interp * interp)
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->requestGetDefaultChannelName(interp);

    return Tcl_NewStringObj("stdin", 5);
}

/* default output channel */

char *requestGetDefaultOutChannelName(Tcl_Interp * interp)
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->requestGetDefaultOutChannelName(interp);

    return CGICHANNEL;
}


int requestFillRequestValues(Tcl_Interp * interp, RequestData * requestData)
{
    if (requestData->requestIsInitialized)
	return TCL_OK;
    requestData->requestIsInitialized = 1;

    {
      ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
      if (apFuncs != NULL)
	return apFuncs->requestFillRequestValues(interp, requestData);
    }

    return Tcl_Eval(interp, "web::cgi::copyenv");
}
