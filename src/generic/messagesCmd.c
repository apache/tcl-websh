/*
 * messagesCmd.c --- init for "messages on streams" module of websh3
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
#include "messages.h"

/* ----------------------------------------------------------------------------
 * Init --
 * ------------------------------------------------------------------------- */
int messages_Init(Tcl_Interp *interp) {

  /* --------------------------------------------------------------------------
   * interpreter running ?
   * ----------------------------------------------------------------------- */
  if (interp == NULL) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * register commands
   * ----------------------------------------------------------------------- */
  Tcl_CreateObjCommand(interp, "web::send",
		       Web_Send,
  		       (ClientData)NULL,
  		       (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "web::recv",
		       Web_Recv,
  		       (ClientData)NULL,
  		       (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "web::msgflag",
		       Web_MsgFlag,
  		       (ClientData)NULL,
  		       (Tcl_CmdDeleteProc *) NULL);

  /* --------------------------------------------------------------------------
   * done
   * ----------------------------------------------------------------------- */
  return TCL_OK;
}
