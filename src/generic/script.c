/*
 * script.c --- the commands of websh3 that are implemented as Tcl scripts
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
#include "script.h"

/* ----------------------------------------------------------------------------
 * init --
 * ------------------------------------------------------------------------- */
int Script_Init(Tcl_Interp * interp)
{

    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * eval code
     * ----------------------------------------------------------------------- */
    return Tcl_Eval(interp, script_h);
}
