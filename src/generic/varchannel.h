/*
 * varchannel.h --- varchannel data type
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

#ifndef VARCHANNEL_H
#define VARCHANNEL_H

typedef struct VarChannel
{
    Tcl_Obj *varName;
    Tcl_Interp *interp;
    int readCursor;
}
VarChannel;

VarChannel *createVarChannel();
int destroyVarChannel(ClientData clientData, Tcl_Interp * interp);

Tcl_Channel Web_GetChannelOrVarChannel(Tcl_Interp * interp,
				       char *name, int *mode);
Tcl_Channel Web_GetVarChannel(Tcl_Interp * interp, char *name, int *mode);

int Web_UnregisterVarChannel(Tcl_Interp * interp, char *name,
			     Tcl_Channel channel);

#endif
