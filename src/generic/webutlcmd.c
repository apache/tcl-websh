/*
 * webutlcmd.c --- webshell utility commands
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
#include "webutl.h"
#include "webutlcmd.h"
#include "filelock.h"

/* --------------------------------------------------------------------
 * Init --
 * -------------------------------------------------------------------- */
int webutlcmd_Init(Tcl_Interp * interp)
{


    Tcl_CreateObjCommand(interp, "web::lockfile",
			 Web_LockChannel,
			 (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateObjCommand(interp, "web::unlockfile",
			 Web_UnLockChannel,
			 (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateObjCommand(interp, "web::truncatefile",
			 Web_TruncateFile,
			 (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    return TCL_OK;
}
