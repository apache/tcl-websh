/*
 * filelock.h --
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
#include "log.h"

#ifndef FILELOCK_H

#ifdef SYSV
#    include <unistd.h>
#endif
#ifdef WIN32
#  include <windows.h>
#endif
#ifdef FREEBSD
#    include <unistd.h>
#    include <sys/lockf.h>
#    include <sys/file.h>
#endif
#ifdef BSDI
#    include <unistd.h>
#    include <sys/types.h>
#    include <fcntl.h>
#endif
/* BSDI need sys/types.h, too, for ftruncate */
#ifdef AIX
#    include <unistd.h>
#    include <sys/lockf.h>
#endif


int truncate_file(ClientData handle);

int lock_file(ClientData handle);
int unlock_file(ClientData handle);

int lock_TclChannel(Tcl_Interp *interp, Tcl_Channel channel);
int unlock_TclChannel(Tcl_Interp *interp, Tcl_Channel channel);

int Web_LockChannel(ClientData clientData, 
		    Tcl_Interp *interp, 
		    int objc, Tcl_Obj *CONST objv[]);

int Web_UnLockChannel(ClientData clientData, 
		    Tcl_Interp *interp, 
		    int objc, Tcl_Obj *CONST objv[]);

int Web_TruncateFile(ClientData clientData, 
		     Tcl_Interp *interp, 
		     int objc, Tcl_Obj *CONST objv[]);

#endif
