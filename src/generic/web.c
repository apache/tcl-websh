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

/* ----------------------------------------------------------------------------
 * init --
 * ------------------------------------------------------------------------- */
int Websh_Init(Tcl_Interp * interp)
{

    UrlData *urlData;
    RequestData *requestData;

    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * stubs
     * ----------------------------------------------------------------------- */
    Tcl_InitStubs(interp, "8.2", 0);

    /* --------------------------------------------------------------------------
     * the logging module
     * ----------------------------------------------------------------------- */
    if (log_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

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
     * filecounter
     * ----------------------------------------------------------------------- */
    if (filecounter_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * messages on streams
     * ----------------------------------------------------------------------- */
#ifndef WIN32
    if (messages_Init(interp) == TCL_ERROR)
	return TCL_ERROR;
#endif

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
     * interlink the two
     * ----------------------------------------------------------------------- */
    requestData =
	(RequestData *) Tcl_GetAssocData(interp, WEB_REQ_ASSOC_DATA, NULL);
    urlData = (UrlData *) Tcl_GetAssocData(interp, WEB_URL_ASSOC_DATA, NULL);

/*   requestData->urlData = urlData; */
    urlData->requestData = requestData;

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

    /* fixme: dynamic version for [package provide]*/
    return Tcl_PkgProvide(interp, WEBSH, "3.5.0");

}
