/*
 * logtosyslog.h -- plugin for log module of websh3
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
#ifndef WEB_LOGTOSYSLOG_H
#define WEB_LOGTOSYSLOG_H

/* --------------------------------------------------------------------------
 * Commands
 * ------------------------------------------------------------------------*/ 

/* ----------------------------------------------------------------------------
 * SubCommands
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Switches (like "string -binary")
 * ------------------------------------------------------------------------- */
#define WEB_LOGTOSYSLOG_SWITCH_UNBUFFERED "-unbuffered"

/* ----------------------------------------------------------------------------
 * Parameters (like "web::cmdurl -cmd aCommand", where there is an argument
 * ------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------
 * Registered Data
 * ------------------------------------------------------------------------*/ 

/* --------------------------------------------------------------------------
 * messages
 * ------------------------------------------------------------------------*/ 
#define WEB_LOGTOSYSLOG_USAGE "priority"


/* ----------------------------------------------------------------------------
 * plugin logger: tosyslog
 * ------------------------------------------------------------------------- */
typedef int LogToSyslogData;
LogToSyslogData *createLogToSyslogData();
int             destroyLogTosySlogData(Tcl_Interp *interp,
				       LogToSyslogData *logToSyslogData);

ClientData createLogToSyslog(Tcl_Interp *interp, ClientData clientData,
			     int objc, Tcl_Obj *CONST objv[]);
int destroyLogToSyslog(Tcl_Interp *interp, ClientData clientData);
int logToSyslog(Tcl_Interp *interp,ClientData clientData, char *msg);

#endif
