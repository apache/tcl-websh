/*
 * url.c -- url generation
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
#include <time.h>
#include "url.h"
#include "paramlist.h"
#include "crypt.h"
#include "stdlib.h"		/* getenv */
#include "log.h"




#define WEB_URL_GETFROMREQDATA(scheme,key) \
    if( urlData->scheme != NULL ) \
      scheme = urlData->scheme; \
    if( scheme == NULL ) \
      if( urlData->requestData != NULL ) \
        scheme = paramListGetObjectByString(interp,urlData->requestData->request,key);


static TCLCONST char *urlElementOpts[] = {
    "-scheme",
    "-host",
    "-port",
    "-scriptname",
    "-pathinfo",
    "-querystring",
    NULL
};

/* to which flags the names above correspond */
static int urlElementFlags[] = {
    WEB_URL_WITH_SCHEME,
    WEB_URL_WITH_HOST,
    WEB_URL_WITH_PORT,
    WEB_URL_WITH_SCRIPTNAME,
    WEB_URL_WITH_PATHINFO,
    WEB_URL_WITH_QUERYSTRING
};

enum urlElement
{
    SCHEME,
    HOST,
    PORT,
    SCRIPTNAME,
    PATHINFO,
    QUERYSTRING
};


/* ----------------------------------------------------------------------------
 * Init --
 * ------------------------------------------------------------------------- */
int url_Init(Tcl_Interp * interp)
{

    UrlData *urlData;

    /* --------------------------------------------------------------------------
     * interpreter running ?
     * ----------------------------------------------------------------------- */
    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * new data
     * ----------------------------------------------------------------------- */
    urlData = createUrlData();

    /* --------------------------------------------------------------------------
     * register commands
     * ----------------------------------------------------------------------- */
    Tcl_CreateObjCommand(interp, "web::cmdurlcfg",
			 Web_CmdUrlCfg, (ClientData) urlData, NULL);

    Tcl_CreateObjCommand(interp, "web::cmdurl",
			 Web_CmdUrl, (ClientData) urlData, NULL);

    /* --------------------------------------------------------------------------
     * associate data with Interpreter
     * ----------------------------------------------------------------------- */
    Tcl_SetAssocData(interp, WEB_URL_ASSOC_DATA,
		     destroyUrlData, (ClientData) urlData);

    /* --------------------------------------------------------------------------
     * done
     * ----------------------------------------------------------------------- */
    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * create
 * ------------------------------------------------------------------------- */
UrlData *createUrlData()
{

    UrlData *urlData = NULL;

    urlData = WebAllocInternalData(UrlData);

    if (urlData != NULL) {
	urlData->defaultscheme = NULL;
	urlData->scheme = NULL;
	/* we want to read port from request if available */
	/*WebNewStringObjFromStringIncr(urlData->port,WEB_DEFAULT_PORT); */
	urlData->port = NULL;
	urlData->host = NULL;
	urlData->scriptname = NULL;
	urlData->pathinfo = NULL;
	urlData->querystring = NULL;

	urlData->requestData = NULL;

	urlData->urlformat = WEB_URL_URLFORMAT;
    }

    return urlData;
}

/* ----------------------------------------------------------------------------
 * reset
 * ------------------------------------------------------------------------- */
int resetUrlData(Tcl_Interp * interp, UrlData * urlData)
{
    if ((interp == NULL) || (urlData == NULL))
	return TCL_ERROR;

    WebDecrRefCountIfNotNullAndSetNull(urlData->defaultscheme);
    WebDecrRefCountIfNotNullAndSetNull(urlData->scheme);

    WebDecrRefCountIfNotNullAndSetNull(urlData->port);
    /* we want to read port from request if available */
    /*WebNewStringObjFromStringIncr(urlData->port,WEB_DEFAULT_PORT); */

    WebDecrRefCountIfNotNullAndSetNull(urlData->host);

    WebDecrRefCountIfNotNullAndSetNull(urlData->scriptname);

    WebDecrRefCountIfNotNullAndSetNull(urlData->pathinfo);

    WebDecrRefCountIfNotNullAndSetNull(urlData->querystring);

    /* do not touch requestData */

    urlData->urlformat = WEB_URL_URLFORMAT;

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * destroy
 * ------------------------------------------------------------------------- */
void destroyUrlData(ClientData clientData, Tcl_Interp * interp)
{

    UrlData *urlData = NULL;

    if (clientData != NULL) {

	urlData = (UrlData *) clientData;

	WebDecrRefCountIfNotNull(urlData->defaultscheme);
	WebDecrRefCountIfNotNull(urlData->scheme);
	WebDecrRefCountIfNotNull(urlData->port);
	WebDecrRefCountIfNotNull(urlData->host);
	WebDecrRefCountIfNotNull(urlData->scriptname);
	WebDecrRefCountIfNotNull(urlData->pathinfo);

	WebFreeIfNotNull(urlData);

    }
}

/* ----------------------------------------------------------------------------
 * parseUrlFormat -- parse the -with list and turn corresponding bits on,
 *   or 0 in case of error
 * ------------------------------------------------------------------------- */
int parseUrlFormat(Tcl_Interp * interp, Tcl_Obj * list)
{

    int objc = -1;
    Tcl_Obj **objv = NULL;
    int i = -1;
    int res = 0;

    TCLCONST char *accepted[20];
    enum urlElement e;


    /* --------------------------------------------------------------------------
     * minimal
     * ----------------------------------------------------------------------- */
    if (list == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::cmdurl -urlformat", WEBLOG_ERROR,
		"cannot access list", NULL);
	return 0;
    }

    for (e = SCHEME; e <= QUERYSTRING; e++)
	accepted[e] = &(urlElementOpts[e][1]);
    accepted[e] = NULL;

    /* --------------------------------------------------------------------------
     * convert list to array of objs
     * ----------------------------------------------------------------------- */
    if (Tcl_ListObjGetElements(interp, list, &objc, &objv) == TCL_ERROR) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::cmdurl -urlformat", WEBLOG_ERROR,
		"cannot convert \"", Tcl_GetString(list), "\" to list", NULL);
	return 0;
    }

    /* empty list */
    if (objc == 0) {
	Tcl_SetResult(interp, "no url elements specified", TCL_STATIC);
	return 0;
    }

    /* --------------------------------------------------------------------------
     * now see what we got
     * ----------------------------------------------------------------------- */
    for (i = 0; i < objc; i++) {
	int idx = 0;
	if (Tcl_GetIndexFromObj(interp,
				objv[i],
				accepted,
				"url element", 0, &idx) == TCL_ERROR)
	    return 0;
	else {
	    res |= urlElementFlags[idx];
	}
    }
    return res;
}


/* ----------------------------------------------------------------------------
 * mergeLists -- assume key-value paired list; take value from staticP if no
 *   key is found in cmdlineP
 * ------------------------------------------------------------------------- */
Tcl_Obj *mergeLists(Tcl_Interp * interp, Tcl_Obj * cmdlineP,
		    Tcl_Obj * staticP)
{

    int staticPLen = -1;
    int cmdlinePLen = -1;
    int i = -1;
    int j = -1;
    Tcl_Obj *res = NULL;
    Tcl_Obj *key1 = NULL;
    Tcl_Obj *key2 = NULL;
    Tcl_Obj *val = NULL;
    int keyOnCmdLine = TCL_ERROR;

    if ((staticP == NULL) || (cmdlineP == NULL))
	return NULL;

    staticPLen = tclGetListLength(interp, staticP);
    cmdlinePLen = tclGetListLength(interp, cmdlineP);

    if ((staticPLen % 2) != 0) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::cmdurl", WEBLOG_INFO,
		"key-value list \"", Tcl_GetString(staticP),
		"\" must be even-numbered", NULL);
	return NULL;
    }

    if ((cmdlinePLen % 2) != 0) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::cmdurl", WEBLOG_INFO,
		"key-value list \"", Tcl_GetString(cmdlineP),
		"\" must be even-numbered", NULL);
	return NULL;
    }

    res = Tcl_NewObj();

    for (i = 0; i < staticPLen; i += 2) {

	key1 = NULL;
	key2 = NULL;
	val = NULL;

	if (Tcl_ListObjIndex(interp, staticP, i, &key1) == TCL_ERROR) {
	    Tcl_DecrRefCount(res);
	    return NULL;
	}

	keyOnCmdLine = TCL_ERROR;

	for (j = 0; j < cmdlinePLen; j += 2) {

	    if (Tcl_ListObjIndex(interp, cmdlineP, j, &key2) == TCL_ERROR) {
		Tcl_DecrRefCount(res);
		return NULL;
	    }

	    if (strcmp(Tcl_GetString(key1), Tcl_GetString(key2)) == 0) {

		keyOnCmdLine = TCL_OK;
		break;
	    }
	}

	if (keyOnCmdLine == TCL_ERROR) {

	    if (Tcl_ListObjIndex(interp, staticP, i + 1, &val) == TCL_ERROR) {
		Tcl_DecrRefCount(res);
		return NULL;
	    }

	    if (Tcl_ListObjAppendElement(interp, res, key1) == TCL_ERROR) {
		Tcl_DecrRefCount(res);
		return NULL;
	    }

	    if (Tcl_ListObjAppendElement(interp, res, val) == TCL_ERROR) {
		Tcl_DecrRefCount(res);
		return NULL;
	    }
	}
    }

    return res;
}

/* ----------------------------------------------------------------------------
 * createQueryList -- put elements of query_string together to form a list
 * - cmd may be NULL. In this case it is ignored.
 * - plist may be NULL. In this case it is ignored.
 * ------------------------------------------------------------------------- */
Tcl_Obj *createQueryList(Tcl_Interp * interp, Tcl_Obj * cmd, Tcl_Obj * plist,
			 UrlData * urlData, int flag)
{

    Tcl_Obj *qStr = NULL;
    Tcl_Obj *tmp = NULL;
    int errCnt = 0;

    if ((urlData == NULL))
	return NULL;

    errCnt = 0;
    qStr = Tcl_NewObj();

    if (qStr == NULL)
	return NULL;

    if (plist != NULL)
	if (Tcl_ListObjAppendList(interp, qStr, plist) == TCL_ERROR)
	    errCnt++;

    /* ..........................................................................
     * append static params
     * ....................................................................... */
    if (urlData->requestData != NULL) {

	if (urlData->requestData->staticList != NULL) {

	    tmp = paramListAsListObj(urlData->requestData->staticList);

	    /* ----------------------------------------------------------------------
	     * merge
	     * ------------------------------------------------------------------- */
	    if (plist != NULL) {

		Tcl_Obj *tmp2 = NULL;
		tmp2 = mergeLists(interp, plist, tmp);

		if (tmp2 == NULL) {

		    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__,
			    __LINE__, "web::cmdurl", WEBLOG_INFO,
			    "error mergings parameters from cmdline ",
			    "and static parameters. Details: ",
			    Tcl_GetStringResult(interp), NULL);

		    Tcl_DecrRefCount(qStr);
		    return NULL;
		}

		Tcl_DecrRefCount(tmp);
		tmp = tmp2;
	    }
	}
    }

    if (Tcl_ListObjAppendList(interp, qStr, tmp) == TCL_ERROR)
	errCnt++;

    Tcl_DecrRefCount(tmp);

    /* After appending each element in elemListPtr,
     * Tcl_ListObjAppendList increments the element's reference count
     * since listPtr now also refers to it. For the same reason,
     * Tcl_ListObjAppendElement increments objPtr's reference count. If
     * no error occurs, the two procedures return TCL_OK after appending
     * the objects.  */

    /* ..........................................................................
     * append command tag
     * ....................................................................... */
    if (cmd != NULL) {
	if ((flag & WEB_URL_NOCMD) == 0) {
	    if (urlData->requestData != NULL)
		if (urlData->requestData->cmdTag != NULL)
		    if (Tcl_ListObjAppendElement(interp, qStr,
						 urlData->requestData->cmdTag)
			== TCL_ERROR)
			errCnt++;
	    if (errCnt < 1)
		if (Tcl_ListObjAppendElement(interp, qStr, cmd) == TCL_ERROR)
		    errCnt++;
	}
    }

    /* ..........................................................................
     * append time
     * ....................................................................... */
    if ((flag & WEB_URL_NOTIMESTAMP) == 0) {
	if (urlData->requestData != NULL)
	    if (urlData->requestData->timeTag != NULL)
		if (Tcl_ListObjAppendElement(interp, qStr,
					     urlData->requestData->timeTag)
		    == TCL_ERROR)
		    errCnt++;
	if (errCnt < 1)
	    if (Tcl_ListObjAppendElement
		(interp, qStr, Tcl_NewLongObj(time(NULL))) == TCL_ERROR)
		errCnt++;
    }

    /* ..........................................................................
     * finish
     * ....................................................................... */
    if (errCnt > 0) {

	Tcl_DecrRefCount(qStr);
	return NULL;
    }

    return qStr;
}


/* ----------------------------------------------------------------------------
 * Web_CmdUrl -- url generation
 * ------------------------------------------------------------------------- */
int Web_CmdUrl(ClientData clientData,
	       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    static TCLCONST char *params[] = { "-urlformat",
	"-notimestamp", NULL
    };
    enum params
    { URLFORMAT,
	NOTIMESTAMP
    };
    int Nparams[] = { 1, 0 };

    int iCurArg = 0;
    UrlData *urlData = NULL;
    Tcl_Obj *plist = NULL;
    Tcl_Obj *cmd = NULL;
    Tcl_Obj *qStrList = NULL;
    int plistLen = 0;
    int i = 0;
    int flag = 0;
    int bool = 1;
    int urlformat = 0;
    Tcl_Obj *urlFmt = NULL;
    Tcl_Obj *res = NULL;

    /* --------------------------------------------------------------------------
     * internal data ?
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_CmdUrl", TCL_ERROR)
	urlData = (UrlData *) clientData;

    /* make sure we have request data */
    if (requestFillRequestValues(interp, urlData->requestData) == TCL_ERROR)
	return TCL_ERROR;


    /* --------------------------------------------------------------------------
     * first arg is cmd
     * ----------------------------------------------------------------------- */
    iCurArg = argIndexOfFirstArg(objc, objv, params, Nparams);
    if ((objc - iCurArg) < 1) {
	Tcl_WrongNumArgs(interp, 1, objv, "cmdName");
	return TCL_ERROR;
    }
    if (Tcl_GetCharLength(objv[iCurArg]) > 0) {
	cmd = objv[iCurArg];
    }
    iCurArg++;

    /* --------------------------------------------------------------------------
     * any params we don't accept ?
     * ----------------------------------------------------------------------- */
    WebAssertArgs(interp, objc, objv, params, i, -1);

    /* --------------------------------------------------------------------------
     * check for flags
     * ----------------------------------------------------------------------- */
    urlformat = urlData->urlformat;
    if ((urlFmt = argValueOfKey(objc, objv, (char *)params[URLFORMAT])) != NULL) {

	urlformat = parseUrlFormat(interp, urlFmt);

	if (urlformat == 0)
	    return TCL_ERROR;
    }

    if (argIndexOfKey(objc, objv, (char *)params[NOTIMESTAMP]) > 0)
	flag = (flag | WEB_URL_NOTIMESTAMP);

    Tcl_GetBooleanFromObj(interp, urlData->requestData->cmdUrlTimestamp, &bool);
    if (bool == 0)
	flag = (flag | WEB_URL_NOTIMESTAMP);

    /* --------------------------------------------------------------------------
     * do we need to create a querystring ?
     * ----------------------------------------------------------------------- */
    if ((urlformat & WEB_URL_WITH_QUERYSTRING) != 0) {

	if (urlData->querystring != NULL) {
	    /* take the one which was configured in web::cmdurlcfg */
	    qStrList = Tcl_DuplicateObj(urlData->querystring);

	}
	else {
	    /* create a new one */

	    /* ---------------------------------------------------------------------
	     * create query_string
	     * ------------------------------------------------------------------ */
	    switch (objc - iCurArg) {
	    case 0:
		/* ...................................................................
		 * web::cmdurl [options] cmd
		 * ................................................................ */
		qStrList = createQueryList(interp, cmd, NULL, urlData, flag);
		break;
	    case 1:
		/* ...................................................................
		 * web::cmdurl [options] cmd list
		 * ................................................................ */
		if ((plistLen =
		     tclGetListLength(interp, objv[iCurArg])) == -1)
		    return TCL_ERROR;
		if ((plistLen % 2) != 0) {
		    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__,
			    __LINE__, "web::cmdurl", WEBLOG_INFO,
			    "key-value list \"", Tcl_GetString(objv[iCurArg]),
			    "\" must be even-numbered", NULL);
		    return TCL_ERROR;
		}
		qStrList =
		    createQueryList(interp, cmd, objv[iCurArg], urlData,
				    flag);
		break;
	    default:
		/* ................................................................
		 * web::cmdurl [options] "" k1 v1 ... kn vn
		 * ................................................................ */
		if (((objc - iCurArg) % 2) != 0) {
		    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__,
			    __LINE__, "web::cmdurl", WEBLOG_INFO,
			    "key without the matching value (uneven list), starting at \"",
			    Tcl_GetString(objv[iCurArg]), "\"", NULL);
		    return TCL_ERROR;
		}
		plist = Tcl_NewObj();
		if (plist == NULL)
		    return TCL_ERROR;
		for (i = iCurArg; i < objc; i += 2) {
		    if (Tcl_ListObjAppendElement(interp, plist, objv[i]) ==
			TCL_ERROR)
			return TCL_ERROR;
		    if (Tcl_ListObjAppendElement(interp, plist, objv[i + 1])
			== TCL_ERROR)
			return TCL_ERROR;
		}
		qStrList = createQueryList(interp, cmd, plist, urlData, flag);
		Tcl_DecrRefCount(plist);
	    }

	    /* ------------------------------------------------------------------
	     * crypt
	     * ------------------------------------------------------------------ */
	    Tcl_IncrRefCount(qStrList);
	    if (doencrypt(interp, qStrList, 1) != TCL_OK) {

		LOG_MSG(interp, WRITE_LOG, __FILE__, __LINE__,
			"web::cmdurl", WEBLOG_ERROR,
			"error encrypting \"", Tcl_GetString(qStrList), "\"",
			NULL);
		Tcl_DecrRefCount(qStrList);
		return TCL_ERROR;

	    }
	    else {

		Tcl_DecrRefCount(qStrList);
		qStrList = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
		Tcl_ResetResult(interp);
	    }
	}
    }

    /* ==========================================================================
     * url (stuff before query_string)
     * ======================================================================= */
    res = Tcl_NewObj();

    if ((urlformat & WEB_URL_WITH_SCHEME) != 0) {
	if (urlData->defaultscheme != NULL) {
	    Tcl_AppendObjToObj(res, urlData->defaultscheme);
	    Tcl_AppendToObj(res, WEBURL_SCHEME_SEP, -1);
	} else {
	    Tcl_Obj *schemeObj = NULL;
	    char *scheme = NULL;
	    if( urlData->requestData != NULL ) {
		schemeObj = paramListGetObjectByString(interp, urlData->requestData->request, "HTTPS");
		if (schemeObj != NULL)
		    scheme = Tcl_GetString(schemeObj);
	    }
	    if (!strcmp(scheme, "on")) {
		Tcl_AppendObjToObj(res, Tcl_NewStringObj("https", -1));
		Tcl_AppendToObj(res, WEBURL_SCHEME_SEP, -1);
	    } else {
		Tcl_AppendObjToObj(res, Tcl_NewStringObj(WEB_DEFAULT_SCHEME, -1));
		Tcl_AppendToObj(res, WEBURL_SCHEME_SEP, -1);
	    }
	}
    }

    if ((urlformat & WEB_URL_WITH_HOST) != 0) {
	Tcl_Obj *host = NULL;
	/* try to get requested host */
	WEB_URL_GETFROMREQDATA(host, "HTTP_HOST");
	if (host == NULL) {
	    /* fall back use server name */
	    WEB_URL_GETFROMREQDATA(host, "SERVER_NAME");
	}
	if (host != NULL) {
	    Tcl_AppendToObj(res, WEBURL_HOST_SEP, -1);
	    Tcl_AppendObjToObj(res, host);
	}
    }

    if ((urlformat & WEB_URL_WITH_PORT) != 0) {
	Tcl_Obj *port = NULL;
	WEB_URL_GETFROMREQDATA(port, "SERVER_PORT");
	Tcl_AppendToObj(res, WEBURL_PORT_SEP, -1);
	if (port != NULL) {
	    /* found one in the environment */
	    Tcl_AppendObjToObj(res, port);
	}
	else {
	    /* output the default port */
	    Tcl_AppendToObj(res, WEB_DEFAULT_PORT, -1);
	}
    }

    if ((urlformat & WEB_URL_WITH_SCRIPTNAME) != 0) {
	Tcl_Obj *scriptname = NULL;
	WEB_URL_GETFROMREQDATA(scriptname, "SCRIPT_NAME");
	if (scriptname != NULL) {
	    Tcl_AppendObjToObj(res, scriptname);
	}
    }

    if ((urlformat & WEB_URL_WITH_PATHINFO) != 0) {
	Tcl_Obj *pathinfo = NULL;
	WEB_URL_GETFROMREQDATA(pathinfo, "PATH_INFO");
	if (pathinfo != NULL) {
	    Tcl_AppendObjToObj(res, pathinfo);
	}
    }

    if ((urlformat & WEB_URL_WITH_QUERYSTRING) != 0) {
	if (qStrList != NULL) {
	    Tcl_AppendToObj(res, WEBURL_QUERY_STRING_SEP, -1);
	    Tcl_AppendObjToObj(res, qStrList);
	    Tcl_DecrRefCount(qStrList);
	}
    }

    Tcl_SetObjResult(interp, res);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_CmdUrlCfg -- configuration of url generation
 * syntax: web::cmdurlcfg get|set|add|delete tag|param [key] [value] [default]
 * param -- manage static parameters which are appended to
 *          every query_string
 *          set - set entry with name "key" to "value",
 *                overwriting any existing
 *          add - append "value" to entry with name "key" (entry is a list)
 *          del - delete entry with name "key"
 *          get - return list from entry with name "key"
 *                or list of keys, if now key is given, or "default", if
 *                no entry with name "key" exists
 * tag -- manage tags for special entries in query_string. Syntax as above,
 *        except that "add" is mapped to "set", since tags must be
 *        single-valued lists.
 *        For the same reason, "del" is not implemented.
 *        Needed tags are: session-id, command, time
 *        Defaults are:
 *        session-id: id
 *        command:    cmd
 *        time:       t
 * ------------------------------------------------------------------------- */
int Web_CmdUrlCfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{


#define URLCFGRESET (enum urlElement)QUERYSTRING+1
#define URLCFGURLFORMAT (enum urlElement)QUERYSTRING+2
#define URLCFGEND (enum urlElement)QUERYSTRING+3

    UrlData *urlData = NULL;
    /* note: this could be dynamic, but 20 is enough ... */
    char *params[20];
    int i;
    int res;

    for (i = SCHEME; i <= QUERYSTRING; i++)
	params[i] = (char *) urlElementOpts[i];

    params[URLCFGRESET] = "-reset";
    params[URLCFGURLFORMAT] = "-urlformat";
    params[URLCFGEND] = NULL;


    /* --------------------------------------------------------------------------
     *
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_CmdUrlCfg", TCL_ERROR)
	urlData = (UrlData *) clientData;

    /* --------------------------------------------------------------------------
     *
     * ----------------------------------------------------------------------- */

    res = paramGet((ParamList *) urlData->requestData->staticList,
		   interp, objc, objv, 1);

    if (res == TCL_CONTINUE) {
	int opt;
	Tcl_Obj *tmpObj = NULL;

	if (objc <= 1) {
	    Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?args?");
	    return TCL_ERROR;
	}

	if (paramGetIndexFromObj
	    (interp, objv[1], params, "subcommand", 0, &opt) == TCL_ERROR)
	    return TCL_ERROR;

	/* ----------------------------------------------------------------------
	 * it is one of my options
	 * -------------------------------------------------------------------- */

	/* ----------------------------------------------------------------------
	 * web::cmdurlcfg -protocol value
	 * 0              1         2
	 * ------------------------------------------------------------------- */
	if (objc == 3)
	    tmpObj = objv[2];
	else
	    tmpObj = NULL;

	switch ((enum urlElement) opt) {
	case SCHEME:
	    WebAssertObjc(objc > 3, 2, "?value?");
	    if (urlData->defaultscheme != NULL) {
		Tcl_SetObjResult(interp,
				 Tcl_DuplicateObj(urlData->defaultscheme));
		if (tmpObj != NULL) {
		    Tcl_DecrRefCount(urlData->defaultscheme);
		    urlData->defaultscheme = Tcl_DuplicateObj(tmpObj);
		}
		return TCL_OK;
	    } else {
		Tcl_SetObjResult(interp,
				 Tcl_NewStringObj(WEB_DEFAULT_SCHEME, -1));
		if (tmpObj != NULL) {
		    if (!strcmp(Tcl_GetString(tmpObj), "")) {
			urlData->defaultscheme = NULL;
		    } else {
			urlData->defaultscheme = Tcl_DuplicateObj(tmpObj);
		    }
		}
		return TCL_OK;
	    }
	    break;
	case HOST:
	    WebAssertObjc(objc > 3, 2, "?value?");
	    return handleConfig(interp, &urlData->host, tmpObj, 1);
	    break;
	case PORT:
	    WebAssertObjc(objc > 3, 2, "?value?");
	    return handleConfig(interp, &urlData->port, tmpObj, 1);
	    break;
	case SCRIPTNAME:
	    WebAssertObjc(objc > 3, 2, "?value?");
	    return handleConfig(interp, &urlData->scriptname, tmpObj, 1);
	    break;
	case PATHINFO:
	    WebAssertObjc(objc > 3, 2, "?value?");
	    return handleConfig(interp, &urlData->pathinfo, tmpObj, 1);
	    break;
	case QUERYSTRING:
	    WebAssertObjc(objc > 3, 2, "?value?");
	    return handleConfig(interp, &urlData->querystring, tmpObj, 1);
	    break;
	case URLCFGRESET:
	    WebAssertObjc(objc != 2, 2, NULL);
	    return resetUrlData(interp, urlData);
	    break;
	case URLCFGURLFORMAT:{
		int urlformat = 0;
		Tcl_Obj *res = NULL;
		Tcl_Obj *tmpres = NULL;
		enum urlElement i;
		WebAssertObjc(objc > 3, 2, "?value?");
		/* format current */
		res = Tcl_NewObj();
		for (i = SCHEME; i <= QUERYSTRING; i++) {
		    if (((urlData->urlformat) & urlElementFlags[i]) != 0) {
			tmpres =
			    Tcl_NewStringObj(&(urlElementOpts[i][1]), -1);
			Tcl_ListObjAppendElement(interp, res, tmpres);
		    }
		}

		if (tmpObj != NULL) {
		    /* we have to set it as well */
		    urlformat = parseUrlFormat(interp, tmpObj);
		    if (urlformat == 0) {
			/* cleanup */
			Tcl_DecrRefCount(res);
			return TCL_ERROR;
		    }
		    urlData->urlformat = urlformat;
		}
		Tcl_SetObjResult(interp, res);
		return TCL_OK;
		break;
	    }
	default:
	    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		    "web::cmdurl", WEBLOG_INFO, "unknown option", NULL);
	    return TCL_ERROR;
	}
    }
    return res;
}
