/*
 * messages.h --- messages on streams module of websh3
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

#include "tcl.h"

#ifndef MESSAGES_H
#define MESSAGES_H

#include <sys/types.h>
#include <stdio.h>
#include <tcl.h>

#define WMSG_MAGIC        0xa5a53333
#define WMSG_VERSION      0x1
#define WMSG_TIMEOUT 100	/* seconds */

#define WMSG_SLAVELET_BASE 0x2000

#define WMSG_FLAG_MULT 0x00010000

/* ----------------------------------------------------------------------------
 * list of filters
 * ------------------------------------------------------------------------- */
typedef struct MsgHeader
{
    u_long magic;		/* synchronization token */
    u_long version;		/* protocol version */
    u_long command;		/* type of message and flags (16bit cmd, 16bit flags ) */
    u_long size;		/* size of appended data (size*sizeof(int)) */
}
MsgHeader;

int send_msg(Tcl_Channel f, int command, int flags, int size, void *data);

  /* if data is NULL receive_msg allocates memory (using malloc, use
     free() to free) */
  /* ownership is always caller */
int receive_msg(Tcl_Channel f, int *command, int *flags, int *size,
		void **data);

int Web_Send(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);
int Web_Recv(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);
int Web_MsgFlag(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int messages_Init(Tcl_Interp * interp);
int parseFlags(Tcl_Interp * interp, char *flaglist, int *flags);

#endif
