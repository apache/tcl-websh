/*
 * varchannel.c - simple channel implementation to write to var
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
#ifdef WIN32
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include <tcl.h>
#include "varchannel.h"
#include "webout.h"		/* belongs to output module of websh */
#include "webutl.h"		/* Web_IntIncrObj */

/* ----------------------------------------------------------------------------
 * close var channel
 * ------------------------------------------------------------------------- */
int varchannelCloseProc(ClientData clientData, Tcl_Interp * interp)
{

/*   printf("DBG varchannelCloseProc\n"); fflush(stdout); */
    destroyVarChannel(clientData, interp);

    /* nothign to do - var is managed by interp */
    return 0;
}

/* ----------------------------------------------------------------------------
 * input from var channel
 * ------------------------------------------------------------------------- */
int varchannelInputProc(ClientData clientData,
			char *buf, int bufSize, int *errorCodePtr)
{

    char *str = NULL;
    int strLen = 0;
    int remains = 0;
    VarChannel *varChannel = NULL;
    int isNew = 0;
    Tcl_Obj *var = NULL;

    if ((clientData == NULL) || (buf == NULL))
	return -1;

    varChannel = (VarChannel *) clientData;
    if (varChannel->varName == NULL)
	return -1;

    /* --------------------------------------------------------------------------
     * get var
     * ----------------------------------------------------------------------- */
    var = Web_GetOrCreateGlobalVar(varChannel->interp,
				   varChannel->varName, &isNew);
    if (var == NULL)
	return -1;
    if (isNew)
	varChannel->readCursor = 0;

    /* str = Tcl_GetStringFromObj(var,&strLen); */
    str = (char *) Tcl_GetByteArrayFromObj(var, &strLen);

    if (varChannel->readCursor < strLen) {

	remains = strLen - varChannel->readCursor;

	if (bufSize < remains) {
	    strncpy(buf, &(str[varChannel->readCursor]), bufSize);
	    varChannel->readCursor += bufSize;
	    return bufSize;
	}
	else {
	    strncpy(buf, &(str[varChannel->readCursor]), remains);
	    varChannel->readCursor += remains;
	    return remains;
	}
    }

    return 0;
}

/* ----------------------------------------------------------------------------
 * output to var channel
 * ------------------------------------------------------------------------- */
int varchannelOutputProc(ClientData clientData,
			 char *buf, int toWrite, int *errorCodePtr)
{

    VarChannel *varChannel = NULL;
    int res = -1;
    int destLen = 0;
    char *dest = NULL;
    int bytesConv = 0;
    Tcl_Obj *tmp = NULL;
    int isNew = 0;
    Tcl_Obj *var = NULL;

    /* sanity */
    if ((clientData == NULL) || (buf == NULL))
	return res;

    /* get interna */
    varChannel = (VarChannel *) clientData;
    if (varChannel->varName == NULL)
	return -1;

    /* --------------------------------------------------------------------------
     * get var
     * ----------------------------------------------------------------------- */
    var = Web_GetOrCreateGlobalVar(varChannel->interp,
				   varChannel->varName, &isNew);
    if (var == NULL)
	return -1;
    if (isNew)
	varChannel->readCursor = 0;

    /* the conversion
     *
     * what we get from Tcl is the string in the encoding of this channel.
     * Since we are writing to a variable, we need to back-translate
     * the string to the encding currently in use by Tcl.
     */

    /* convert to UTF */
    destLen = (toWrite + 1) * TCL_UTF_MAX + 1;
    dest = Tcl_Alloc(destLen);
    if (dest == NULL)
	return -1;

    res = Tcl_ExternalToUtf(NULL, NULL, buf, toWrite, 0, NULL, dest, destLen,
			    NULL, &bytesConv, NULL);

    if (res != TCL_OK) {

	Tcl_Free(dest);
	return -1;
    }

    /* now put UTF into Tcl_object */

    tmp = Tcl_NewStringObj(dest, bytesConv);

    if (tmp == NULL) {

	Tcl_Free(dest);
	return -1;
    }

    /* and append it to the global var */
    var = Tcl_ObjSetVar2(varChannel->interp,
			 varChannel->varName,
			 NULL,
			 tmp,
			 TCL_GLOBAL_ONLY | TCL_APPEND_VALUE |
			 TCL_LEAVE_ERR_MSG);

    /* cleanup */

    Tcl_Free(dest);
    Tcl_DecrRefCount(tmp);

    if (var == NULL)
	return -1;

    return toWrite;
}

/* ----------------------------------------------------------------------------
 * watch var channel
 * ------------------------------------------------------------------------- */
void varchannelWatchProc(ClientData clientData, int mask)
{
}

/* ----------------------------------------------------------------------------
 * getHandle for var channel
 * ------------------------------------------------------------------------- */
int varchannelGetHandleProc(ClientData clientData, int direction,
			    ClientData * handlePtr)
{

    /* "not implemented" */
    return EINVAL;
}

/* ----------------------------------------------------------------------------
 * varchannelType
 * ------------------------------------------------------------------------- */
static Tcl_ChannelType varchannelType = {
    "varchannel",		/* Type name. */
    NULL,			/* Set blocking/nonblocking mode. */
    varchannelCloseProc,	/* Close proc. */
    varchannelInputProc,	/* Input proc. */
    varchannelOutputProc,	/* Output proc. */
    NULL,			/* Seek proc. */
    NULL,			/* Set option proc. */
    NULL,			/* Get option proc. */
    varchannelWatchProc,	/* Initialize notifier. */
    varchannelGetHandleProc,	/* Get OS handles out of channel. */
};

/* ----------------------------------------------------------------------------
 * createVarChannel
 * ------------------------------------------------------------------------- */
VarChannel *createVarChannel()
{

    VarChannel *varChannel = NULL;

    varChannel = WebAllocInternalData(VarChannel);

    if (varChannel != NULL) {

	varChannel->varName = NULL;
	varChannel->interp = NULL;
	varChannel->readCursor = 0;
    }

    return varChannel;
}

/* ----------------------------------------------------------------------------
 * destroyVarChannel
 * ------------------------------------------------------------------------- */
int destroyVarChannel(ClientData clientData, Tcl_Interp * interp)
{

    VarChannel *varChannel = NULL;

    /* printf("DBG - destroyVarChannel\n"); fflush(stdout); */

    if (clientData != NULL) {

	varChannel = (VarChannel *) clientData;
	WebDecrRefCountIfNotNull(varChannel->varName);

	Tcl_Free((char *) varChannel);
    }

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_GetChannelOrVarChannel - Tcl_GetChannel alike
 * ------------------------------------------------------------------------- */
Tcl_Channel Web_GetChannelOrVarChannel(Tcl_Interp * interp,
				       char *name, int *mode)
{

    Tcl_Channel channel = NULL;

    if ((interp == NULL) || (name == NULL) || (mode == NULL))
	return NULL;

    if (name[0] == '#') {

	/* get varchannel */

	channel = Web_GetVarChannel(interp, name, mode);

    }
    else {

	/* get normal channel */

	channel = Tcl_GetChannel(interp, name, mode);
    }

    return channel;
}

/* ----------------------------------------------------------------------------
 * Web_GetVarChannel - Tcl_GetChannel alike
 * ------------------------------------------------------------------------- */
Tcl_Channel Web_GetVarChannel(Tcl_Interp * interp, char *name, int *mode)
{

    Tcl_Obj *varNameObj = NULL;
    Tcl_Obj *var = NULL;
    VarChannel *varChannel = NULL;
    Tcl_Channel channel = NULL;
    int isNew = 0;

    /* --------------------------------------------------------------------------
     * expect two characters at least
     * ----------------------------------------------------------------------- */
    if (strlen(name) < 2)
	return NULL;

    /* --------------------------------------------------------------------------
     * does channel already exist - good
     * ----------------------------------------------------------------------- */
    channel = Tcl_GetChannel(interp, name, mode);
    if (channel != NULL)
	return channel;

    /* --------------------------------------------------------------------------
     * unset Tcl_Result (we are not intetested, thank you)
     * ----------------------------------------------------------------------- */
    Tcl_ResetResult(interp);

    /* --------------------------------------------------------------------------
     * create VarChannel
     * ----------------------------------------------------------------------- */
    varChannel = createVarChannel();
    if (varChannel == NULL)
	return NULL;

    /* --------------------------------------------------------------------------
     * create name
     * ----------------------------------------------------------------------- */
    varNameObj = Tcl_NewStringObj(&name[1], -1);

    varChannel->varName = varNameObj;
    varChannel->interp = interp;

    /* --------------------------------------------------------------------------
     * create name
     * ----------------------------------------------------------------------- */
    var = Web_GetOrCreateGlobalVar(varChannel->interp,
				   varChannel->varName, &isNew);

    /* --------------------------------------------------------------------------
     * create channel
     * ----------------------------------------------------------------------- */
    *mode = TCL_READABLE | TCL_WRITABLE;
    channel = Tcl_CreateChannel(&varchannelType, name,
				(ClientData) varChannel, *mode);

/*   printf("DBG Web_GetVarChannel - gonna register '%s'\n",name); fflush(stdout); */
    Tcl_RegisterChannel(interp, channel);

    /* Tcl_DecrRefCount(varNameObj); */

    return channel;
}

/* ----------------------------------------------------------------------------
 * Web_UnregisterVarChannel - Tcl_UnregisterChannel
 * ------------------------------------------------------------------------- */
int Web_UnregisterVarChannel(Tcl_Interp * interp, char *name,
			     Tcl_Channel channel)
{

    if ((interp == NULL) || (name == NULL))
	return TCL_ERROR;

    /* if name does not start with # we don't care */
    if (name[0] != '#')
	return TCL_OK;

    if (channel == NULL) {

	int mode = 0;
	channel = Tcl_GetChannel(interp, name, &mode);

	if (channel == NULL)
	    return TCL_ERROR;
    }

    return Tcl_UnregisterChannel(interp, channel);
}
