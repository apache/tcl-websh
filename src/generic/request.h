/*
 * request.h --- websh3 request data processing
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

/* --- doc/plain --------------------------------------------------------------

typical way of communication:

browser:        sends http://server/script
webserver:      starts up script
web::dispatch   parses querystring and stores data in requestData
                in this case, the querystring is empty
web::cmdurl     driven by arguments and configuration, generates URL
web::eval       sends HTML containing links back to browser
browser:        sends http://server/script?querystring
webserver:      starts up script
web::dispatch   parses querystring and stores data in requestData
...

an URL has the following items:

http://www3.netcetera.ch/bin/hefti/test.ws2/some/test/path?HalloDuDa
|--||-||---------------||-----------------||-------------|^|-------|
|   |  SERVER_NAME      |SCRIPT_NAME       PATH_INFO      |QUERY_STRING
|   | PROTOCOLSEPARATOR                                   |QUERYSTRINGSEPARATOR
| PROTOCOL

 * ------------------------------------------------------------------------- */

#include "tcl.h"
#include "webutl.h"
#include <stdio.h>		/* tempnam */
#include <stdlib.h>		/* free: char* from tempnam() */

#ifdef AIX
#  include <strings.h>
#  include <string.h>
#else
#  include <string.h>
#endif

#ifdef WIN32
#  define STRCASECMP _stricmp
#else
#  define STRCASECMP strcasecmp
#endif

#ifndef REQUEST_H
#define REQUEST_H

/* how many bytes to count per new line */
#ifdef WIN32
#  define WEB_REQ_NLBYTES 2
#else
#  define WEB_REQ_NLBYTES 1
#endif

#define WEB_REQ_ASSOC_DATA "web::requestData"
#define WEB_REQ_DEFAULT "default"

#define APCHANNEL  "apache"
#define CGICHANNEL "stdout"

/* ----------------------------------------------------------------------------
 * RequestData
 * ------------------------------------------------------------------------- */
typedef struct RequestData
{
    Tcl_Obj *cmdTag;		/* key to be used for param in URL (web::cmdurl) */
    /* default: cmd */
    Tcl_Obj *timeTag;		/* key to be used for param in URL (web::cmdurl) */
    /* default: t */
    /* ............ */
    Tcl_HashTable *request;	/* everything from the request obj */
    /* e.g. server_port, server_name and so on */
    /* ............ */
    Tcl_Obj *upLoadFileSize;	/* maximum number of bytes for file upload */
    /* ............ */
    Tcl_HashTable *paramList;	/* after parsing of querystring */
    Tcl_HashTable *formVarList;	/* after parsing of post content */
    Tcl_HashTable *cmdList;	/* registered by web::command */
    Tcl_HashTable *staticList;	/* static params (mod by -track and cmdurlcfg) */
    /* ............ */
    Tcl_HashTable *tmpFnList;	/* list of temporary file names given out */
/* ............ *//* for dispatch: */
    int requestIsInitialized;
    ClientData handleToSpecificReqData;

}
RequestData;


RequestData *createRequestData(Tcl_Interp * interp);
int resetRequestData(Tcl_Interp * interp, RequestData * requestData);
void destroyRequestData(ClientData clientData, Tcl_Interp * interp);

int removeTempFiles(Tcl_Interp * interp, RequestData * requestData);


int request_Init(Tcl_Interp * interp);

int Web_Request(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);


int Web_Param(ClientData clientData,
	      Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_FormVar(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_ParseQueryString(ClientData clientData,
			 Tcl_Interp * interp,
			 int objc, Tcl_Obj * CONST objv[]);
int parseQueryString(RequestData * requestData, Tcl_Interp * interp,
		     Tcl_Obj * query_string);
int Web_GetQueryStringFromUrl(ClientData clientData,
			      Tcl_Interp * interp,
			      int objc, Tcl_Obj * CONST objv[]);

int Web_ParseFormData(ClientData clientData,
		      Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);
int parseMultipartFormData(RequestData * requestData, Tcl_Interp * interp,
			   char *channelName, char *content_type);
int parseUrlEncodedFormData(RequestData * requestData, Tcl_Interp * interp,
			    char *channelName, Tcl_Obj * len);
char *mimeGetParamFromContDisp(const char *contentDisp, const char *name);

Tcl_Obj *tempFileName(Tcl_Interp * interp, RequestData * requestData,
		      Tcl_Obj * path, Tcl_Obj * prefix);
int Web_TempFile(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Command(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);


int Web_GetCommand(ClientData clientData,
		   Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Dispatch(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

/* ----------------------------------------------------------------------------
 * mime header for Content Disp
 * ------------------------------------------------------------------------- */
typedef struct MimeContDispData
{
    char *type;
    char *name;
    char *fileName;
    char *content;
}
MimeContDispData;

MimeContDispData *newMimeContDispData();
void destroyMimeContDispData(MimeContDispData * data);

MimeContDispData *mimeGetContDispFromHeader(Tcl_Interp * interp,
					    const char *header);
int mimeSplitMultipart(Tcl_Interp * interp, Tcl_Channel channel,
		       const char *boundary, RequestData * requestData);
int mimeSplitIsBoundary(Tcl_Obj * cur, Tcl_Obj * prev,
			const char *boundary, int *isLast);
void mimeReadHeader(Tcl_Channel channel, Tcl_Obj * hdr);
void mimeReadBody(Tcl_Channel channel, Tcl_Obj * bdy, const char *boundary,
		  int *isLast);
long readAndDumpBody(Tcl_Interp * interp, Tcl_Channel in,
		     const char *boundary, int *isLast,
		     Tcl_Obj * tmpFileName, long upLoadFileSize,
		     long *bytesSkipped);
char *mimeGetParameterFromContDisp(const char *contentDisp, const char *name);

/* in CGI case: implemented in request_cgi.c 
 * in httpd case: implemented in mod_websh.c */
Tcl_Obj *requestGetDefaultChannelName();

int requestFillRequestValues(Tcl_Interp * interp, RequestData * requestData);

#endif
