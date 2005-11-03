/* ----------------------------------------------------------------------------
 * mod_websh.c -- handler for websh applications for Apache-1.3
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
 * ------------------------------------------------------------------------- */

/* ====================================================================
 * Copyright (c) 1995-1999 The Apache Group.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on software written by Netcetera AG, Zurich, Switzerland.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */

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

/* ----------------------------------------------------------------------------
 * tcl/websh includes
 * ------------------------------------------------------------------------- */
#include "tcl.h"		/* tcl headers */
#include "web.h"		/* websh headers */
#include "mod_websh.h"		/* apchannel stuff */
#include "interpool.h"
#include "logtoap.h"

#ifndef APACHE2
#include "http_conf_globals.h"
#endif /* APACHE2 */

#define WEBSHHANDLER "websh"

#ifndef APACHE2
module MODULE_VAR_EXPORT websh_module;
#define APPOOL pool
#else /* APACHE2 */
module AP_MODULE_DECLARE_DATA websh_module;
#define APPOOL apr_pool_t
#endif /* APACHE2 */

/* ============================================================================
 * httpd config and log handling
 * ========================================================================= */

/* Configuration stuff */

#ifndef APACHE2
static void cleanup_websh_pool(void *conf)
{
    /* cleanup the pool when server is restarted (-HUP) */
    destroyPool((websh_server_conf *) conf);
}
#else /* APACHE2 */
static apr_status_t cleanup_websh_pool(void *conf)
{
    /* cleanup the pool when server is restarted (-HUP) */
    destroyPool((websh_server_conf *) conf);
    return APR_SUCCESS;
}
#endif /* APACHE2 */

#ifndef APACHE2
static void exit_websh_pool(server_rec * s, APPOOL * p)
{
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(s->module_config,
						   &websh_module);
    /* cleanup the pool when server is restarted (-HUP) */
    destroyPool(conf);
}
#else
static apr_status_t exit_websh_pool(void *data)
{
    server_rec *s = data;
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(s->module_config,
						   &websh_module);
    /* cleanup the pool when server is restarted (-HUP) */
    destroyPool(conf);
    return APR_SUCCESS;
}
#endif

static void *create_websh_config(APPOOL * p, server_rec * s)
{

    websh_server_conf *c =
#ifndef APACHE2
	(websh_server_conf *) ap_palloc(p, sizeof(websh_server_conf));
#else				/* APACHE2 */
	(websh_server_conf *) apr_palloc(p, sizeof(websh_server_conf));
#endif /* APACHE2 */
    c->scriptName = NULL;
    c->mainInterp = NULL;
    c->mainInterpLock = NULL;
    c->webshPool = NULL;
    c->webshPoolLock = NULL;
    c->server = s;

    /* make sure we call cleanup the our websh pool when this memory is freed */
#ifndef APACHE2
    ap_register_cleanup(p, (void *) c, cleanup_websh_pool, ap_null_cleanup);
#else /* APACHE2 */
    apr_pool_cleanup_register(p, (void *) c, cleanup_websh_pool,
			      apr_pool_cleanup_null);
#endif /* APACHE2 */

    return c;
}

static void *merge_websh_config(APPOOL * p, void *basev, void *overridesv)
{
    /* fixme-later: is this correct? (reset the locks) */

    /* When we have seperate interpreters for seperate virtual hosts,
     * and things of that nature, then we can worry about this -
     * davidw. */

/*     ((websh_server_conf *) overridesv)->mainInterpLock = NULL;
       ((websh_server_conf *) overridesv)->webshPoolLock = NULL;  */
    return basev;
}

static const char *set_webshscript(cmd_parms * cmd, void *dummy, char *arg)
{
    server_rec *s = cmd->server;
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(s->module_config,
						   &websh_module);
    conf->scriptName = ap_server_root_relative(cmd->pool, arg);

    return NULL;
}

#ifdef APACHE2
static void websh_init_child(apr_pool_t * p, server_rec * s)
{
    /* here we create our main Interp and Pool */
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(s->module_config,
						   &websh_module);
    if (!initPool(conf)) {
	ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
		     "Could not init interp pool\n");
	return;
    }
    apr_pool_cleanup_register(p, s, exit_websh_pool, exit_websh_pool);
}

#else /* APACHE2 */
static void websh_init_child(server_rec *s, pool *p)
{
    /* here we create our main Interp and Pool */
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(s->module_config,
						   &websh_module);
    if (!initPool(conf)) {
	ap_log_error(APLOG_MARK, APLOG_ERR, s,
		     "Could not init interp pool\n");
	return;
    }
}
#endif

static const command_rec websh_cmds[] = {
    {"WebshConfig", set_webshscript, NULL, RSRC_CONF, TAKE1,
     "the name of the main websh configuration file"},
    {NULL}
};

/* ----------------------------------------------------------------------------
 * run_websh_script
 * ------------------------------------------------------------------------- */
static int run_websh_script(request_rec * r)
{

    WebInterp *webInterp = NULL;
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(r->server->module_config,
						   &websh_module);

    /* checkme: check type of timeout in MP case */
    /* ap_soft_timeout("!!! timeout for run_websh_script expired", r); */

#ifndef APACHE2

    /* ap_log_printf(r->server,"mtime of %s: %ld\n",r->filename,r->finfo.st_mtime); */
    webInterp = poolGetWebInterp(conf, r->filename, r->finfo.st_mtime, r);
    if (webInterp == NULL || webInterp->interp == NULL) {
	ap_log_printf(r->server, "mod_websh - no interp !\n");
	return 0;
    }

#else /* APACHE2 */

    /* ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, r, "mtime of %s: %ld\n",r->filename,r->finfo.mtime); */
    webInterp = poolGetWebInterp(conf, r->filename, (long) r->finfo.mtime, r);
    /* ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, r, "got pool %p", webInterp); */
    if (webInterp == NULL || webInterp->interp == NULL) {
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
		      "mod_websh - no interp !\n");
	return 0;
    }

#endif /* APACHE2 */

    if (Tcl_InterpDeleted(webInterp->interp)) {
#ifndef APACHE2
	ap_log_printf(r->server,
		      "mod_websh - hey, somebody is deleting the interp !\n");
#else /* APACHE2 */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
		      "mod_websh - hey, somebody is deleting the interp !\n");
#endif /* APACHE2 */
	return 0;
    }

    Tcl_SetAssocData(webInterp->interp, WEB_AP_ASSOC_DATA, NULL,
		     (ClientData) r);

    Tcl_SetAssocData(webInterp->interp, WEB_INTERP_ASSOC_DATA, NULL,
		     (ClientData) webInterp);

    if (createApchannel(webInterp->interp, r) != TCL_OK) {
#ifndef APACHE2
	ap_log_printf(r->server, "mod_websh - cannot create apchannel\n");
#else /* APACHE2 */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
		      "mod_websh - cannot create apchannel\n");
#endif /* APACHE2 */
	return 0;
    }

    if (Tcl_Eval(webInterp->interp, "web::ap::perReqInit") != TCL_OK) {
#ifndef APACHE2
	ap_log_printf(r->server,
		      "mod_websh - cannot init per-request Websh code\n");
#else /* APACHE2 */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
		      "mod_websh - cannot init per-request Websh code\n");
#endif /* APACHE2 */
	return 0;
    }

    if (webInterp->code != NULL) {
	int res = 0;

	Tcl_IncrRefCount(webInterp->code);
	res = Tcl_EvalObjEx(webInterp->interp, webInterp->code, 0);
	Tcl_DecrRefCount(webInterp->code);

	if (res != TCL_OK) {

	    char *errorInfo = NULL;
	    errorInfo =
		(char *) Tcl_GetVar(webInterp->interp, "errorInfo", TCL_GLOBAL_ONLY);
	    logToAp(webInterp->interp, NULL, errorInfo);
	}

	Tcl_ResetResult(webInterp->interp);
    }

    if (Tcl_Eval(webInterp->interp, "web::ap::perReqCleanup") != TCL_OK) {
#ifndef APACHE2
	ap_log_printf(r->server, "mod_websh - error while cleaning-up\n");
#else /* APACHE2 */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
		      "mod_websh - error while cleaning-up\n");
#endif /* APACHE2 */
	return 0;
    }

    if (destroyApchannel(webInterp->interp) != TCL_OK) {
#ifndef APACHE2
	ap_log_printf(r->server, "mod_websh - error closing ap-channel\n");
#else /* APACHE2 */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
		      "mod_websh - error closing ap-channel\n");
#endif /* APACHE2 */
	return 0;
    }

    Tcl_DeleteAssocData(webInterp->interp, WEB_AP_ASSOC_DATA);
    Tcl_DeleteAssocData(webInterp->interp, WEB_INTERP_ASSOC_DATA);

    poolReleaseWebInterp(webInterp);

    /* ap_kill_timeout(r); */

    return 1;
}

/* ----------------------------------------------------------------------------
 * websh_handler
 * ------------------------------------------------------------------------- */

static int websh_handler(request_rec * r)
{

    int res;
    void *sconf = r->server->module_config;
    websh_server_conf *conf =
	(websh_server_conf *) ap_get_module_config(sconf, &websh_module);

#ifdef APACHE2
    if (strcmp(r->handler, WEBSHHANDLER))
	return DECLINED;
#endif /* APACHE2 */

    /* We don't check to see if the file exists, because it might be
     * mapped with web::interpmap. */

    if ((res = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)))
	return res;

    /* SERVER_SIGNATURE, REMOTE_PORT, .... */
    ap_add_common_vars(r);

    /* GATEWAY_INTERFACE, SERVER_PROTOCOL, ... */
    ap_add_cgi_vars(r);

#ifdef CHARSET_EBCDIC
    ap_bsetflag(r->connection->client, B_EBCDIC2ASCII, 1);
#endif /*CHARSET_EBCDIC */

    /* ---------------------------------------------------------------------
     * ready to rumble
     * --------------------------------------------------------------------- */
    if (!run_websh_script(r)) {
#ifndef APACHE2
	ap_log_rerror(APLOG_MARK, APLOG_ERR, r,
#else /* APACHE2 */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, r,
#endif /* APACHE2 */
		      "couldn't run websh script: %s",
		      r->filename);
	return HTTP_INTERNAL_SERVER_ERROR;
    }

    return OK;			/* NOT r->status, even if it has changed. */
}

#ifndef APACHE2

static const handler_rec websh_handlers[] = {
    {WEBSHHANDLER, websh_handler},
    {NULL}
};

module MODULE_VAR_EXPORT websh_module = {
    STANDARD_MODULE_STUFF,
    NULL,			/* initializer */
    NULL,			/* dir config creater */
    NULL,			/* dir merger --- default is to override */
    create_websh_config,	/* server config */
    merge_websh_config,		/* merge server config */
    websh_cmds,			/* command table */
    websh_handlers,		/* handlers */
    NULL,			/* filename translation */
    NULL,			/* check_user_id */
    NULL,			/* check auth */
    NULL,			/* check access */
    NULL,			/* type_checker */
    NULL,			/* fixups */
    NULL,			/* logger */
    NULL,			/* header parser */
    websh_init_child,		/* child_init */
    exit_websh_pool,		/* child_exit */
    NULL			/* post read-request */
};

#else /* APACHE2 */

static void register_websh_hooks(apr_pool_t *p) {
    ap_hook_handler(websh_handler, NULL, NULL, APR_HOOK_MIDDLE);

    ap_hook_child_init(websh_init_child, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA websh_module = {
    STANDARD20_MODULE_STUFF,
    NULL,			/* dir config creater */
    NULL,			/* dir merger --- default is to override */
    create_websh_config,	/* server config */
    merge_websh_config,		/* merge server config */
    websh_cmds,			/* command table */
    register_websh_hooks	/* register hooks */
};

#endif /* APACHE2 */
