/*
 * logtochannel.c -- plugin for log module of websh3 to handle Tcl channels
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
#include "logtochannel.h"
#include "webutl.h"		/* args */

/* ----------------------------------------------------------------------------
 * createLogToChannelData --
 * ------------------------------------------------------------------------- */
LogToChannelData *createLogToChannelData()
{

    LogToChannelData *logToChannelData = NULL;

    logToChannelData = WebAllocInternalData(LogToChannelData);
    logToChannelData->channel = NULL;
    logToChannelData->channelName = NULL;
    logToChannelData->mode = 0;
    logToChannelData->isBuffered = TCL_OK;

    return logToChannelData;
}

/* ----------------------------------------------------------------------------
 * destroyLogToChannelData --
 * ------------------------------------------------------------------------- */
int destroyLogToChannelData(Tcl_Interp * interp,
			    LogToChannelData * logToChannelData)
{

    if ((interp == NULL) || (logToChannelData == NULL))
	return TCL_ERROR;

    if (logToChannelData->channel != NULL)
	Tcl_Flush(logToChannelData->channel);

    WebFreeIfNotNull(logToChannelData->channelName);
    WebFreeIfNotNull(logToChannelData);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * constructor -- initializes internal data for "logToChannel".
 * Called by "web::logbag add channel chName".
 * ------------------------------------------------------------------------- */
ClientData createLogToChannel(Tcl_Interp * interp, ClientData clientData,
			      int objc, Tcl_Obj * CONST objv[])
{

    LogToChannelData *logToChannelData = NULL;
    int iCurArg;
    char *channelName = NULL;
    Tcl_Channel channel;
    int mode;

    /* --------------------------------------------------------------------------
     * syntax is: file [-unbuffered] channelName
     *            0    1             2
     * ----------------------------------------------------------------------- */
    if ((objc < 2) || (objc > 4)) {
	Tcl_WrongNumArgs(interp, 1, objv, WEB_LOGTOCHANNEL_USAGE);
	return NULL;
    }

    if (strcmp(Tcl_GetString(objv[0]), "channel") != 0) {
	Tcl_SetResult(interp, WEB_LOGTOCHANNEL_USAGE, NULL);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * do we have the channelName ?
     * ----------------------------------------------------------------------- */
    iCurArg = argIndexOfFirstArg(objc, objv, NULL, NULL);
    if (iCurArg >= objc) {
	Tcl_SetResult(interp, WEB_LOGTOCHANNEL_USAGE, NULL);
	return NULL;
    }
    channelName = Tcl_GetString(objv[iCurArg]);

    /* --------------------------------------------------------------------------
     * get channel
     * ----------------------------------------------------------------------- */
    channel = Tcl_GetChannel(interp, channelName, &mode);
    if (channel == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::logdest", WEBLOG_ERROR,
		"cannot get channel \"", channelName, "\"", NULL);
	return NULL;
    }

    if (!(mode & TCL_WRITABLE)) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::logdest", WEBLOG_ERROR,
		"channel \"", channelName, "\" not open for writing", NULL);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * everything ok so far. Get memory
     * ----------------------------------------------------------------------- */
    logToChannelData = createLogToChannelData();
    if (logToChannelData == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::logdest", WEBLOG_ERROR,
		"cannot get memory for internal data", NULL);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * and set values
     * ----------------------------------------------------------------------- */
    logToChannelData->channel = channel;
    logToChannelData->channelName = allocAndSet(channelName);
    logToChannelData->isBuffered = mode;
    if (argKeyExists(objc, objv, "-unbuffered") == TCL_OK)
	logToChannelData->isBuffered = TCL_ERROR;
    else
	logToChannelData->isBuffered = TCL_OK;

    return (ClientData) logToChannelData;
}

/* ----------------------------------------------------------------------------
 * destructor
 * ------------------------------------------------------------------------- */
int destroyLogToChannel(Tcl_Interp * interp, ClientData clientData)
{
    return destroyLogToChannelData(interp, (LogToChannelData *) clientData);
}

/* ----------------------------------------------------------------------------
 * logToChannel
 * ------------------------------------------------------------------------- */
int logToChannel(Tcl_Interp * interp, ClientData clientData, char *msg)
{

    LogToChannelData *logToChannelData;
    int res;

    if ((interp == NULL) || (clientData == NULL) || (msg == NULL))
	return TCL_ERROR;

    logToChannelData = (LogToChannelData *) clientData;

    /* FIXME: should we do this on a channel ? */
    /* Tcl_Seek(logToChannelData->channel, 0, SEEK_END); */

    res = Tcl_WriteChars(logToChannelData->channel, msg, -1);
    if (res < 0)
	return TCL_ERROR;
    if (logToChannelData->isBuffered == TCL_ERROR)
	Tcl_Flush(logToChannelData->channel);

    return TCL_OK;
}
