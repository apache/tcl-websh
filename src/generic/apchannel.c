/*
 * apchannel.c - simple channel implementation for mod_websh for apache
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
#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#else
#include <errno.h>
#endif
#include "tcl.h"

#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_main.h"
#include "http_log.h"
#include "util_script.h"
#include "mod_websh.h"
#ifndef APACHE2
#include "http_conf_globals.h"
#endif /* APACHE2 */

#include "request.h"

/* ----------------------------------------------------------------------------
 * close apache channel
 * ------------------------------------------------------------------------- */
int apchannelCloseProc(ClientData clientData, Tcl_Interp * interp)
{

    /* nothing to do */
    return 0;
}

/* ----------------------------------------------------------------------------
 * input from apache channel
 * ------------------------------------------------------------------------- */
int apchannelInputProc(ClientData clientData,
		       char *buf, int bufSize, int *errorCodePtr)
{

    int res = -1;
    request_rec *r = NULL;


    if ((clientData == NULL) || (buf == NULL))
	return res;
    r = (request_rec *) clientData;

    if (bufSize > 0)
	res = ap_get_client_block(r, buf, bufSize);

    if (res <= 0)
	return -1;

    return res;
}

/* ----------------------------------------------------------------------------
 * output to apache channel
 * ------------------------------------------------------------------------- */
int apchannelOutputProc(ClientData clientData,
			TCLCONST char *buf, int toWrite, int *errorCodePtr)
{

    int res = -1;

    if ((clientData == NULL) || (buf == NULL))
	return res;


    if (toWrite > 0) {
	res = ap_rwrite((void *) buf, toWrite, (request_rec *) clientData);
    }
    if (res < 0)
	return -1;
    return res;
}

/* ----------------------------------------------------------------------------
 * watch apache channel
 * ------------------------------------------------------------------------- */
void apchannelWatchProc(ClientData clientData, int mask)
{
}

/* ----------------------------------------------------------------------------
 * getHandle for apache channel
 * ------------------------------------------------------------------------- */
int apchannelGetHandleProc(ClientData clientData, int direction,
			   ClientData * handlePtr)
{

    /* "not implemented" */
    return EINVAL;
}

/* ----------------------------------------------------------------------------
 * apChannelType
 * ------------------------------------------------------------------------- */
static Tcl_ChannelType apChannelType = {
    "file",			/* Type name. */
    NULL,			/* Set blocking/nonblocking mode. */
    apchannelCloseProc,		/* Close proc. */
    apchannelInputProc,		/* Input proc. */
    apchannelOutputProc,	/* Output proc. */
    NULL,			/* Seek proc. */
    NULL,			/* Set option proc. */
    NULL,			/* Get option proc. */
    apchannelWatchProc,		/* Initialize notifier. */
    apchannelGetHandleProc,	/* Get OS handles out of channel. */
};

/* ----------------------------------------------------------------------------
 * createApchannel
 * ------------------------------------------------------------------------- */
int createApchannel(Tcl_Interp * interp, request_rec * r)
{

    Tcl_Channel channel = NULL;
    int flag = 0;

    if ((interp == NULL) || (r == NULL))
	return TCL_ERROR;

    flag = TCL_WRITABLE;
    if (ap_should_client_block(r)) {
	flag = TCL_WRITABLE | TCL_READABLE;
    }

    channel = Tcl_CreateChannel(&apChannelType, APCHANNEL,
				(ClientData) r, flag);

    if (channel == NULL)
	return TCL_ERROR;

    Tcl_RegisterChannel(interp, channel);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * destroyApchannel
 * ------------------------------------------------------------------------- */
int destroyApchannel(Tcl_Interp * interp)
{

    Tcl_Channel channel = NULL;
    int mode = 0;

    if (interp == NULL)
	return TCL_ERROR;

    channel = Tcl_GetChannel(interp, APCHANNEL, &mode);

    mode = 0;
    if (channel == NULL) {
	mode++;
    }
    else {

	if (Tcl_UnregisterChannel(interp, channel) != TCL_OK)
	    mode++;
    }

    if (mode)
	return TCL_ERROR;
    else
	return TCL_OK;

}
