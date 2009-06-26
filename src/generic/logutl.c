/*
 * logutl.c --- logging module for websh3 -- helpers
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
#include <time.h>
#ifdef WIN32
#  include <string.h>
#  include <process.h>
#else
#  include <strings.h>
#  include <unistd.h>
#endif
#include <errno.h>
#include "log.h"
#include "webutl.h"
#include <sys/types.h>
#include "macros.h"

static char *severityName[] = { "none",
    "alert",
    "error",
    "warning",
    "info",
    "debug"
};


/* ----------------------------------------------------------------------------
 * webLog -- distribute a log message (C API)
 * 1: something like "websh.info"
 * 2: string to log
 * ------------------------------------------------------------------------- */
int webLog(Tcl_Interp * interp, char *levelStr, char *msg)
{

    LogData *logData = NULL;
    int res = -1;
    Tcl_Obj *fmsg = NULL;

    if ((levelStr == NULL) || (msg == NULL) || (interp == NULL))
	return TCL_ERROR;
    if (Tcl_InterpDeleted(interp))
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * try to get the logData data
     * ----------------------------------------------------------------------- */
    logData = (LogData *) Tcl_GetAssocData(interp, WEB_LOG_ASSOC_DATA, NULL);
    if (logData == NULL) {

	Tcl_SetResult(interp, "cannot access private data.", NULL);
	return TCL_ERROR;
    }

    fmsg = Tcl_NewStringObj(msg, -1);

    Tcl_IncrRefCount(fmsg);
    res = logImpl(interp, logData, levelStr, fmsg);
    Tcl_DecrRefCount(fmsg);

    return res;
}

/* ----------------------------------------------------------------------------
 * logImpl -- implementation of log command
 * 1: Tcl interpreter
 * 2: internal data
 * 3: log level definition, like "test.info-alert"
 * 4: message, including special tags
 * R: TCL_OK if [web::config safelog] is set to 1 actual result otherwise
 * ------------------------------------------------------------------------- */
int logImpl(Tcl_Interp * interp, LogData * logData,
	    char *levelStr, Tcl_Obj * msg)
{

    LogLevel *logLevel = NULL;
    int res;

    if ((logData == NULL) || (levelStr == NULL) || (msg == NULL))
	return TCL_ERROR;

    logLevel = parseLogLevel(interp, levelStr, "user", -1);
    if (logLevel == NULL)
	return TCL_ERROR;

    /* ------------------------------------------------------------------
     * does level pass the filters --> send to dests
     * --------------------------------------------------------------- */
    if (doesPassFilters(logLevel, logData->listOfFilters, logData->filterSize) == TCL_OK)
      res = sendMsgToDestList(interp, logData, logLevel, msg);

    destroyLogLevel(logLevel, NULL);

    /* in case we log safely, always return TCL_OK */
    if (logData->safeLog == 1)
      return TCL_OK;

    return res;
}

/* ----------------------------------------------------------------------------
 * sendMsgToDestList --
 * ------------------------------------------------------------------------- */
int sendMsgToDestList(Tcl_Interp * interp, LogData * logData,
		       LogLevel * logLevel, Tcl_Obj * msg)
{

    LogDest *logDest = NULL;
    Tcl_Obj *fmsg = NULL;
    Tcl_Obj *emsg = NULL;	/* eval'd message */
    Tcl_Obj *subst = NULL;
    int res = 0;
    int err = 0;
    int i = 0;

    LogDest **logDests = logData->listOfDests;

    if ((interp == NULL) || (logDests == NULL) ||
	(logLevel == NULL) || (msg == NULL))
	return TCL_ERROR;

    for (i = 0; i < logData->destSize; i++) {

	logDest = logDests[i];
	if (logDest != NULL) {

	    if ((logDest->plugIn != NULL) &&
		(logDest->plugIn->handler != NULL) &&
		(logDest->plugInData != NULL) &&
		(logDest->filter != NULL) && (logDest->format != NULL)) {

		if (doesPass(logLevel, logDest->filter) == TCL_OK) {

		    /* ------------------------------------------------------------------
		     * do we need to eval the message ?
		     * --------------------------------------------------------------- */
		    if (logData->logSubst) {

			/* message already evaluated ? */
			if (emsg == NULL) {

			    /* no, evaluate now */
			    subst = Tcl_NewStringObj("subst", 5);
			    Tcl_IncrRefCount(subst);

			    if (Tcl_ListObjAppendElement(NULL, subst, msg) !=
				TCL_OK) {

				Tcl_DecrRefCount(subst);
				err++;

			    }
			    else {

				res =
				    Tcl_EvalObjEx(interp, subst,
						  TCL_EVAL_DIRECT);
				Tcl_DecrRefCount(subst);

				if (res != TCL_OK) {

				    err++;

				}
				else {

				    emsg = Tcl_GetObjResult(interp);
				    Tcl_IncrRefCount(emsg);	/* keep */
				    Tcl_ResetResult(interp);
				    fmsg =
					formatMessage(logLevel,
						      logDest->format,
						      logDest->maxCharInMsg,
						      emsg);
				    Tcl_IncrRefCount(fmsg);
				}
			    }
			}
			else {
			    /* already evaluated. format */
			    fmsg = formatMessage(logLevel, logDest->format,
						 logDest->maxCharInMsg, emsg);
			    Tcl_IncrRefCount(fmsg);
			}

		    }
		    else {

			/* ----------------------------------------------------------------
			 * ..no, just format it
			 * ------------------------------------------------------------- */
			fmsg = formatMessage(logLevel, logDest->format,
					     logDest->maxCharInMsg, msg);
			Tcl_IncrRefCount(fmsg);
		    }

		    /* ------------------------------------------------------------------
		     * fallback if subst did not work
		     * --------------------------------------------------------------- */
		    if (fmsg == NULL) {

			fmsg = formatMessage(logLevel, logDest->format,
					     logDest->maxCharInMsg, msg);
			Tcl_IncrRefCount(fmsg);
		    }

		    /* ------------------------------------------------------------------
		     * and send it to the handler
		     * --------------------------------------------------------------- */
		    if (logDest->plugIn->handler(interp, logDest->plugInData,
						 Tcl_GetString(fmsg)) != TCL_OK)
		      err++;

		    Tcl_DecrRefCount(fmsg);
		}
	    }
	}
    }
    if (emsg != NULL) {
	Tcl_DecrRefCount(emsg);	/* need it no longer */
    }
    if (err > 0)
      return TCL_ERROR;
    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * getLogSeverity -- convert name to enum using first char of name for speed
 * ------------------------------------------------------------------------- */
Severity getLogSeverity(char *name)
{

    if (name == NULL)
	return invalid;

    switch (name[0]) {
    case 'n':
	return none;
    case 'a':
	return alert;
    case 'e':
	return error;
    case 'w':
	return warning;
    case 'i':
	return info;
    case 'd':
	return debug;
    }
    return invalid;
}

/* ----------------------------------------------------------------------------
 *
 * ------------------------------------------------------------------------- */
char *getSeverityName(Severity aSeverity)
{

    if (aSeverity == invalid)
	aSeverity = none;
    return (severityName[aSeverity]);
}


/* ----------------------------------------------------------------------------
 * parseLogLevel --
 *   given input like "aFacility.info-alert" create LogLevel struct
 *   from Andrej (logutil.c::parseLogLevel)
 *   definition      -> like "websh.info"
 *   defaultfacility -> like "user" (in case websh was ommitted above)
 *   cnt             -> for naming of filters (-1 if log-level)
 * ------------------------------------------------------------------------- */
LogLevel *parseLogLevel(Tcl_Interp * interp,
			char *definition, char *defaultfacility, int cnt)
{

    LogLevel *result = NULL;
    char *facility = NULL;
    char *levelname = NULL;
    char *dot;
    char *dash;
    char *tolevelname = NULL;
    char *fromlevelname = NULL;
    Severity fromlevel, tolevel;
    int len;

    dot = strchr(definition, '.');
    if (dot) {
	/* we have a dot, copy facility name */
	len = dot - definition;
	facility = (char *) Tcl_Alloc(len + 1);
	strncpy(facility, definition, len);
	facility[len] = '\0';
	levelname = dot + 1;
    }
    else
	levelname = definition;

    /* check if there is a '-' in levelname there */
    dash = strchr(levelname, '-');
    if (dash) {
	/* mark position */
	*dash = 0;
	tolevelname = dash + 1;
	fromlevelname = levelname;
	/* if not zero length */
	fromlevel = (*fromlevelname) ? getLogSeverity(fromlevelname) : alert;
	/* if not zero length */
	tolevel = (*tolevelname) ? getLogSeverity(tolevelname) : debug;
	/* reset marked position */
	*dash = '-';

	if (tolevel < fromlevel) {
	    Severity temp = fromlevel;
	    fromlevel = tolevel;
	    tolevel = temp;
	}
    }
    else {
	fromlevel = getLogSeverity(levelname);
	tolevel = fromlevel;
    }

    if ((fromlevel == invalid) || (tolevel == invalid)) {
	if (facility)
	    Tcl_Free(facility);
	if (interp)
	    Tcl_AppendResult(interp, "wrong log level \"", definition, "\"",
			     NULL);
	return NULL;
    }

    if (!facility)
	facility = allocAndSet(defaultfacility);

    result = createLogLevel();
    result->facility = facility;
    result->minSeverity = fromlevel;
    result->maxSeverity = tolevel;

    if (result) {
	return result;
    }
    else {
	if (facility)
	    Tcl_Free(facility);
	if (interp)
	    Tcl_AppendResult(interp, "cannot allocate memory for filter",
			     NULL);
	return NULL;
    }
    /* return NULL; */
}



/* ----------------------------------------------------------------------------
 * doesPass -- low level comparison between a level and a filter
 *   (form Andrej logutils.c::doesPass)
 * ------------------------------------------------------------------------- */
int doesPass(LogLevel * level, LogLevel * filter)
{

    if ((level == NULL) || (filter == NULL))
	return TCL_ERROR;
    if (!((level->minSeverity > filter->maxSeverity) ||
	  (level->maxSeverity < filter->minSeverity))) {
	/* it qualifies, now check facility string */
	if (Tcl_StringMatch(level->facility, filter->facility) == 1) {
	    return TCL_OK;
	}
    }

    return TCL_ERROR;
}

/* ----------------------------------------------------------------------------
 * doesPassFilters -- search through all filters and see if level passes one
 * ------------------------------------------------------------------------- */
int doesPassFilters(LogLevel * logLevel, LogLevel ** logLevels, int size)
{
    int i;

    if ((logLevel == NULL) || (logLevels == NULL))
	return TCL_ERROR;

    for (i = 0; i < size; i++) {
	if (doesPass(logLevel, logLevels[i]) == TCL_OK)
	    return TCL_OK;
    }

    return TCL_ERROR;
}

/* ----------------------------------------------------------------------------
 * formatMessage --
 *
 * ------------------------------------------------------------------------- */
Tcl_Obj *formatMessage(LogLevel * level, char *fmt,
		       long maxCharInMsg, Tcl_Obj * msg)
{

    time_t t;
    char timeStr[2048];
    char tmpStr[32] = "no pid";
    char *c;
    Tcl_Obj *fmsg = NULL;
    int len = 0;
    char *cmsg = NULL;

    /* --------------------------------------------------------------------------
     * create
     * ----------------------------------------------------------------------- */
    fmsg = Tcl_NewObj();

    /* --------------------------------------------------------------------------
     * time part of log string
     * ----------------------------------------------------------------------- */
    time(&t);
#ifdef WIN32
    {
	struct tm *badts;
	/* fixme: race condition under WIN NT */
	badts = localtime(&t);
	strftime(timeStr, sizeof(timeStr) - 1, fmt, badts);
    }
#else
    {
	struct tm ts;
	localtime_r(&t, &ts);
	strftime(timeStr, sizeof(timeStr) - 1, fmt, &ts);
    }
#endif
    /* --------------------------------------------------------------------------
     * add our special fields
     * ----------------------------------------------------------------------- */
    c = timeStr;
    while (*c) {
	if (*c == '$') {
	    c++;
	    if (*c) {
		switch (*c) {
		case 'm':
		    cmsg = Tcl_GetStringFromObj(msg, &len);
		    if (maxCharInMsg == -1) {
			Tcl_AppendObjToObj(fmsg, msg);
		    }
		    else {
			if (len < maxCharInMsg)
			    Tcl_AppendObjToObj(fmsg, msg);
			else
			    Tcl_AppendToObj(fmsg, cmsg, maxCharInMsg);
		    }
		    break;
		case 'p':
#ifndef WIN32
		    sprintf(tmpStr, "%d", (int) getpid());
#else
		    sprintf(tmpStr, "%d", (int) _getpid());
#endif
		    Tcl_AppendToObj(fmsg, tmpStr, -1);
		    break;
		case 't':
#ifdef TCL_THREADS
		    sprintf(tmpStr, "%d", (int) Tcl_GetCurrentThread());
		    Tcl_AppendToObj(fmsg, tmpStr, -1);
#else
		    /* no thread id */
		    Tcl_AppendToObj(fmsg, "-", -1);
#endif

		    break;
		case 'n':
		    sprintf(tmpStr, "%d", level->minSeverity);
		    Tcl_AppendToObj(fmsg, tmpStr, -1);
		    break;
		case 'l':
		    Tcl_AppendToObj(fmsg, getSeverityName(level->minSeverity),
				    -1);
		    break;
		case 'f':
		    Tcl_AppendToObj(fmsg, level->facility, -1);
		    break;
		case '$':
		    Tcl_AppendToObj(fmsg, "$", 1);
		    break;
		default:
		    Tcl_AppendToObj(fmsg, (c - 1), 2);
		}
	    }
	}
	else {
	    Tcl_AppendToObj(fmsg, c, 1);
	}
	c++;
    }
    return fmsg;
}
