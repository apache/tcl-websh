/*
 * logtocmd.c -- plugin for log module of websh3
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
#include <stdio.h>
#include <string.h>
#include "macros.h"
#include "logtocmd.h"
#include "webutl.h"		/* args */

/* ----------------------------------------------------------------------------
 * destroyLogToCmdData --
 * ------------------------------------------------------------------------- */
int destroyLogToCmdData(Tcl_Interp * interp, LogToCmdData * logToCmdData)
{

    WebFreeIfNotNull(logToCmdData);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * constructor
 * ------------------------------------------------------------------------- */
ClientData createLogToCmd(Tcl_Interp * interp, ClientData clientData,
			  int objc, Tcl_Obj * CONST objv[])
{

    LogToCmdData *logToCmdData = NULL;

    /* --------------------------------------------------------------------------
     * syntax is:  [web::logbag add] command cmdName
     *                               0       1
     * ----------------------------------------------------------------------- */
    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, WEB_LOGTOCMD_USAGE);
	return NULL;
    }

    if (strcmp(Tcl_GetString(objv[0]), "command") != 0) {
	Tcl_SetResult(interp, WEB_LOGTOCMD_USAGE, NULL);
	return NULL;
    }

    logToCmdData = (LogToCmdData *) allocAndSet(Tcl_GetString(objv[1]));

    return (ClientData) logToCmdData;
}


/* ----------------------------------------------------------------------------
 * destructor
 * ------------------------------------------------------------------------- */
int destroyLogToCmd(Tcl_Interp * interp, ClientData clientData)
{
    return destroyLogToCmdData(interp, (LogToCmdData *) clientData);
}

/* ----------------------------------------------------------------------------
 * logToCmd
 * ------------------------------------------------------------------------- */
int logToCmd(Tcl_Interp * interp, ClientData clientData, char *msg)
{

    LogToCmdData *logToCmdData = NULL;
    int res = TCL_ERROR;
    Tcl_DString ds;

    if ((interp == NULL) || (clientData == NULL) || (msg == NULL))
	return TCL_ERROR;

    logToCmdData = (LogToCmdData *) clientData;

    /* ----------------------------------------------------------------------
     * gonna call cmdName msg
     * ------------------------------------------------------------------- */

    /* fixme: ouch! create a proper list to eval here!! */
    Tcl_DStringInit(&ds);
    Tcl_DStringAppend(&ds, (char *) logToCmdData, -1);
    Tcl_DStringAppend(&ds, " {", -1);
    Tcl_DStringAppend(&ds, msg, -1);
    Tcl_DStringAppend(&ds, "}", -1);

    res = Tcl_Eval(interp, Tcl_DStringValue(&ds));

    Tcl_DStringFree(&ds);

    return res;
}
