/*
 * logtoap.c -- log to apache error log
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

#include "args.h"
#include "logtoap.h"
#include "macros.h"
#include "mod_websh.h"
#include "tcl.h"
#include <stdio.h>
#include <string.h>

/* ----------------------------------------------------------------------------
 * createLogToApData --
 * ------------------------------------------------------------------------- */
LogToApData *createLogToApData()
{

    return NULL;
}

/* ----------------------------------------------------------------------------
 * destroyLogToApData --
 * ------------------------------------------------------------------------- */
int destroyLogToApData(Tcl_Interp * interp, LogToApData * logToApData)
{

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * constructor -- initializes internal data for "logToAp".
 * Called by "web::logbag add channel chName".
 * ------------------------------------------------------------------------- */
ClientData createLogToAp(Tcl_Interp * interp, ClientData clientData,
			 int objc, Tcl_Obj * CONST objv[])
{

    /* --------------------------------------------------------------------------
     * syntax is: apache
     *            0    1             2
     * ----------------------------------------------------------------------- */
    if (objc != 1) {
	Tcl_WrongNumArgs(interp, 0, objv, "");
	return NULL;
    }

    return (ClientData) 1;
}


/* ----------------------------------------------------------------------------
 * destructor
 * ------------------------------------------------------------------------- */
int destroyLogToAp(Tcl_Interp * interp, ClientData clientData)
{
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * logToAp
 * ------------------------------------------------------------------------- */
int logToAp(Tcl_Interp * interp, ClientData clientData, char *msg, ...)
{

    request_rec *r = NULL;

    va_list args;
    va_start(args, msg);
   

    if ((interp == NULL) || (msg == NULL))
	return TCL_ERROR;

    r = (request_rec *) Tcl_GetAssocData(interp, WEB_AP_ASSOC_DATA, NULL);

    if (r != NULL)
	if (r->server != NULL)
#ifndef APACHE2
	    ap_log_printf(r->server, msg, args);
#else /* APACHE2 */
	    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r, msg, args);
#endif /* APACHE2 */

    va_end(args);
   
    return TCL_OK;
}
