/*
 * mod_websh.h --- header for mod_websh
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

#ifndef MOD_WEBSH_H
#define MOD_WEBSH_H

/* ----------------------------------------------------------------------------
 * httpd includes
 * ------------------------------------------------------------------------- */

#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_main.h"
#include "http_log.h"
#include "util_script.h"

/* define APACHE2 if appropriate */
#ifdef AP_SERVER_BASEREVISION
#define APACHE2
#if AP_SERVER_MINORVERSION_NUMBER < 2
#define CMDFUNC
#else
#define CMDFUNC (cmd_func)
#endif
#else
#define APACHE1
#define CMDFUNC
#endif

#ifndef APACHE2
#include "http_conf_globals.h"
#endif /* APACHE2 */

#define WEB_AP_ASSOC_DATA "web::ap"
#define WEB_INTERP_ASSOC_DATA "web::interp"

typedef struct
{
#ifndef APACHE2
    char *scriptName;
#else				/* APACHE2 */
    const char *scriptName;
#endif				/* APACHE2 */
    Tcl_Interp *mainInterp;
    Tcl_Mutex mainInterpLock;
    Tcl_HashTable *webshPool;
    Tcl_Mutex webshPoolLock;
    server_rec *server;
}
websh_server_conf;

int createApchannel(Tcl_Interp * interp, request_rec * r);
int destroyApchannel(Tcl_Interp * interp);

#endif
