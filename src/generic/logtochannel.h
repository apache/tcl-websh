/*
 * logtochannel.h -- 
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

#ifndef WEB_LOGTOCHANNEL_H
#define WEB_LOGTOCHANNEL_H

#define WEB_LOGTOCHANNEL_USAGE "?-unbuffered? channelName"

/* ----------------------------------------------------------------------------
 * plugin logger: toChannel
 * ------------------------------------------------------------------------- */
typedef struct LogToChannelData
{
    char *channelName;
    char isBuffered;
}
LogToChannelData;
LogToChannelData *createLogToChannelData();
int destroyLogToChannelData(Tcl_Interp * interp,
			    LogToChannelData * logToChannelData);
ClientData createLogToChannel(Tcl_Interp * interp, ClientData clientData,
			      int objc, Tcl_Obj * CONST objv[]);
int destroyLogToChannel(Tcl_Interp * interp, ClientData clientData);
int logToChannel(Tcl_Interp * interp, ClientData clientData, char *msg);

#endif
