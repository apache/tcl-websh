/*
 * webl.h --- header for websh3
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

#ifndef WEB_H
#define WEB_H

#include "conv.h"
#include "log.h"
#include "webout.h"
#ifndef WIN32
#  include "messages.h"
#endif
#include "crypt.h"
#include "webutl.h"
#include "webutlcmd.h"

#include "url.h"
#include "request.h"
#include "cfg.h"
#include "filecounter.h"
#include "modwebsh.h"

int __declspec(dllexport) Websh_Init(Tcl_Interp * interp);
int __declspec(dllexport) ModWebsh_Init(Tcl_Interp * interp);

int Script_Init(Tcl_Interp * interp);

int State_Init(Tcl_Interp * interp);

#endif
