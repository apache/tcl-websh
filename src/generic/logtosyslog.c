/*
 * logtosyslog.c -- plugin for log module of websh3
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
#include <syslog.h>
#include "macros.h"
#include "logtosyslog.h"
#include "webutl.h"		/* args */

/* ----------------------------------------------------------------------------
 * createLogToSyslogData --
 * ------------------------------------------------------------------------- */
LogToSyslogData *createLogToSyslogData()
{

    LogToSyslogData *logToSyslogData;

    logToSyslogData = WebAllocInternalData(LogToSyslogData);
    return logToSyslogData;
}

/* ----------------------------------------------------------------------------
 * destroyLogToSyslogData --
 * ------------------------------------------------------------------------- */
int destroyLogToSyslogData(Tcl_Interp * interp,
			   LogToSyslogData * logToSyslogData)
{

    WebFreeIfNotNull(logToSyslogData);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * constructor
 * ------------------------------------------------------------------------- */
ClientData createLogToSyslog(Tcl_Interp * interp, ClientData clientData,
			     int objc, Tcl_Obj * CONST objv[])
{

    LogToSyslogData *logToSyslogData = NULL;
    int priority = -1;

    /* --------------------------------------------------------------------------
     * syntax is:  [web::logbag add] syslog priority
     *                               0      1
     * ----------------------------------------------------------------------- */
    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, WEB_LOGTOSYSLOG_USAGE);
	return NULL;
    }

    if (strcmp(Tcl_GetString(objv[0]), "syslog") != 0) {
	Tcl_SetResult(interp, WEB_LOGTOSYSLOG_USAGE, NULL);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * try to get priority
     * ----------------------------------------------------------------------- */
    if (Tcl_GetIntFromObj(interp, objv[1], &priority) == TCL_ERROR) {
	Tcl_SetResult(interp, WEB_LOGTOSYSLOG_USAGE, NULL);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * create data
     * ----------------------------------------------------------------------- */
    logToSyslogData = createLogToSyslogData();
    if (logToSyslogData == NULL) {
	Tcl_SetResult(interp, "cannot alloc memory for internal data.", NULL);
	return NULL;
    }
    *logToSyslogData = priority;

    return logToSyslogData;
}


/* ----------------------------------------------------------------------------
 * destructor
 * ------------------------------------------------------------------------- */
int destroyLogToSyslog(Tcl_Interp * interp, ClientData clientData)
{
    return destroyLogToSyslogData(interp, (LogToSyslogData *) clientData);
}

/* ----------------------------------------------------------------------------
 * logToSyslog
 * ------------------------------------------------------------------------- */
int logToSyslog(Tcl_Interp * interp, ClientData clientData, char *msg)
{

    LogToSyslogData *logToSyslogData;

    if ((interp == NULL) || (clientData == NULL) || (msg == NULL))
	return TCL_ERROR;

    logToSyslogData = (LogToSyslogData *) clientData;

    /* ----------------------------------------------------------------------
     * gonna call syslogName msg
     * ------------------------------------------------------------------- */
    syslog((int) (*logToSyslogData), "%s", msg);

    return TCL_OK;
}
