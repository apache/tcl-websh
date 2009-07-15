/*
 * web.c --- init for websh3
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
#include "web.h"
#include "nca_d.h"
#include <stdio.h>
#include "messages.h"

int modwebsh_createcmd(Tcl_Interp * interp);

/* ----------------------------------------------------------------------------
 * init --
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) Websh_Init(Tcl_Interp * interp)
{

    UrlData *urlData;
    RequestData *requestData;
    LogData *logData;

    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * stubs
     * ----------------------------------------------------------------------- */
    Tcl_InitStubs(interp, "8.2", 0);

    /* --------------------------------------------------------------------------
     * the encoding module (htmlify,uricode)
     * ----------------------------------------------------------------------- */
    if (conv_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * output handler
     * ----------------------------------------------------------------------- */
    if (webout_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * messages on streams
     * ----------------------------------------------------------------------- */
    if (messages_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * cryptography
     * ----------------------------------------------------------------------- */
    if (nca_d_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    if (crypt_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * url generation
     * ----------------------------------------------------------------------- */
    if (url_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * request data management
     * ----------------------------------------------------------------------- */
    if (request_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * logging (needs to be after request_Init, because it needs requestData)
     * ----------------------------------------------------------------------- */
    if (log_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * filecounter (needs to be after request_Init, because it needs requestData)
     * ----------------------------------------------------------------------- */
    if (filecounter_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * interlink some data
     * ----------------------------------------------------------------------- */
    requestData =
	(RequestData *) Tcl_GetAssocData(interp, WEB_REQ_ASSOC_DATA, NULL);
    urlData = (UrlData *) Tcl_GetAssocData(interp, WEB_URL_ASSOC_DATA, NULL);
    logData =  (LogData *) Tcl_GetAssocData(interp, WEB_LOG_ASSOC_DATA, NULL);

    urlData->requestData = requestData;
    logData->requestData = requestData;

    /* --------------------------------------------------------------------------
     * utilities
     * ----------------------------------------------------------------------- */
    if (webutlcmd_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * config
     * ----------------------------------------------------------------------- */
    if (cfg_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * tcl-code
     * ----------------------------------------------------------------------- */
    if (Script_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * mod_websh look-alike
     * ----------------------------------------------------------------------- */
    if (modwebsh_createcmd(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* ------------------------------------------------------------------------
     * we provide the websh package
     * --------------------------------------------------------------------- */

    return Tcl_PkgProvide(interp, WEBSH, VERSION);

}


/* -------------------------------------------------------------------------
 * ModWebsh_Init --
 * Init log Plugin and stubs for mod_websh main interpreter 
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) ModWebsh_Init(Tcl_Interp * interp)
{

    if (interp == NULL)
      return TCL_ERROR;

    /* ---------------------------------------------------------------------
     * stubs
     * --------------------------------------------------------------------- */
    Tcl_InitStubs(interp, "8.2", 0);

    /* ---------------------------------------------------------------------
     * register Log Module in here
     * --------------------------------------------------------------------- */
    if (log_Init(interp) == TCL_ERROR) {
      return TCL_ERROR;
    }

    /* ---------------------------------------------------------------------
     * init callbacks
     * --------------------------------------------------------------------- */
    if (modwebsh_createcmd(interp) == TCL_ERROR)
      return TCL_ERROR;

    return TCL_OK;
}
