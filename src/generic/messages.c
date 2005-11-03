/*
 * messages.c -- the "messages on streams" module for websh3
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#  include <unistd.h>
#  include <signal.h>
#  include <sys/types.h>
#  include <sys/uio.h>
#  include <netinet/in.h>
#  include <sys/time.h>
#endif
#include <errno.h>

#include "messages.h"
#include "log.h"
#include "macros.h"

/* ----------------------------------------------------------------------------
 * Web_Send -- 
 * ------------------------------------------------------------------------- */
int Web_Send(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int flags = 0;
    int mode = 0;
    int cmdcode = 0;
    int len = 0;
    Tcl_Channel tc;
    char *tmpStr = NULL;

    /* ------------------------------------------------------------------------
     * arg check
     * --------------------------------------------------------------------- */
    WebAssertObjc((objc < 4)
		  || (objc > 5), 1, "channel cmdnr string ??#?flags?");

    /* ------------------------------------------------------------------------
     * flags there ?
     * --------------------------------------------------------------------- */
    flags = 0;

    if (objc == 5) {

	tmpStr = Tcl_GetString(objv[4]);


	if (tmpStr[0] == '#') {
	    if (Tcl_GetInt(interp, &tmpStr[1], &flags) == TCL_ERROR)
		return TCL_ERROR;
	}
	else {
	    if (parseFlags(interp, tmpStr, &flags) == TCL_ERROR)
		return TCL_ERROR;
	}
    }

    /* ------------------------------------------------------------------------
     * see if we can get this channel
     * --------------------------------------------------------------------- */
    tc = Tcl_GetChannel(interp, Tcl_GetString(objv[1]), &mode);
    if (tc == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__, "web::send", WEBLOG_ERROR,
		"unknown channel \"", Tcl_GetString(objv[1]), "\"", NULL);
	return TCL_ERROR;
    }
    if ((mode & TCL_WRITABLE) == 0) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::send", WEBLOG_ERROR,
		"channel \"", Tcl_GetString(objv[1]),
		"\" not open for writing", NULL);
	return TCL_ERROR;
    }

    /* ------------------------------------------------------------------------
     * get commandcode
     * --------------------------------------------------------------------- */
    if (Tcl_GetIntFromObj(interp, objv[2], &cmdcode) == TCL_ERROR)
	return TCL_ERROR;

    tmpStr = Tcl_GetStringFromObj(objv[3],&len);

    if ( send_msg(tc, cmdcode, flags, len, (void *)tmpStr) == -1 ) {

	Tcl_PosixError(interp);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_Recv -- 
 * ------------------------------------------------------------------------- */
int Web_Recv(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int mode = 0;
    char *data = NULL;
    TCLCONST char *res = NULL;
    int cmdcode = 0;
    int flags = 0;
    int size = 0;
    Tcl_Channel tc;
    Tcl_Obj *to = NULL;

    /* ------------------------------------------------------------------------
     * arg check
     * --------------------------------------------------------------------- */
    WebAssertObjc(objc != 5, 1, "channel cmdvarname resvarname flagsvarname");

    /* ------------------------------------------------------------------------
     * see if we can get this channel
     * --------------------------------------------------------------------- */
    tc = Tcl_GetChannel(interp, Tcl_GetString(objv[1]), &mode);
    if (tc == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__, "web::recv", WEBLOG_ERROR,
		"unknown channel \"", Tcl_GetString(objv[1]), "\"", NULL);
	return TCL_ERROR;
    }
    if ((mode & TCL_READABLE) == 0) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::recv", WEBLOG_ERROR,
		"channel \"", Tcl_GetString(objv[1]),
		"\" not open for reading", NULL);
	return TCL_ERROR;
    }

    if (receive_msg(tc, &cmdcode, &flags, &size, (void **) &data) == -1) {
	if (data)
	    Tcl_Free((char *) data);
	Tcl_PosixError(interp);
	return TCL_ERROR;
    }

    res = Tcl_SetVar(interp, Tcl_GetString(objv[3]), data, TCL_LEAVE_ERR_MSG);
    Tcl_Free(data);
    if (res == NULL)
	return TCL_ERROR;

    to = Tcl_NewIntObj(cmdcode);
    if (Tcl_ObjSetVar2(interp, objv[2], NULL, to, TCL_LEAVE_ERR_MSG) == NULL) {
	Tcl_DecrRefCount(to);
	return TCL_ERROR;
    }

    to = Tcl_NewIntObj(flags);
    if (Tcl_ObjSetVar2(interp, objv[4], NULL, to, TCL_LEAVE_ERR_MSG) == NULL) {
	Tcl_DecrRefCount(to);
	return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewIntObj(size));
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_MsgFlag -- 
 * ------------------------------------------------------------------------- */
int Web_MsgFlag(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int flags;
    int test;
    char buf[30];

    /* ------------------------------------------------------------------------
     * arg check
     * --------------------------------------------------------------------- */
    WebAssertObjc((objc < 1) || (objc > 3), 1, "?flags? ?testflags?");

    if (objc == 1) {
	Tcl_SetResult(interp, "multiple", NULL);
	return TCL_OK;
    }

    if (objc == 2) {

	if (parseFlags(interp, Tcl_GetString(objv[1]), &flags) == TCL_ERROR)
	    return TCL_ERROR;
	sprintf(buf, "%d", flags);
	Tcl_SetResult(interp, buf, TCL_VOLATILE);
	return TCL_OK;
    }

    if (Tcl_GetIntFromObj(interp, objv[1], &flags) == TCL_ERROR)
	return TCL_ERROR;

    if (parseFlags(interp, Tcl_GetString(objv[2]), &test) == TCL_ERROR)
	return TCL_ERROR;

    if ((test & flags) == test && test != 0)
	Tcl_SetResult(interp, "1", NULL);
    else
	Tcl_SetResult(interp, "0", NULL);
    return TCL_OK;

}

/* ----------------------------------------------------------------------------
 * parseFlags (from websh2)
 * ------------------------------------------------------------------------- */
int parseFlags(Tcl_Interp * interp, char *flaglist, int *flags)
{

    TCLCONST char **argv;
    int argc;
    int count;

    *flags = 0;

    Tcl_SplitList(NULL, flaglist, &argc, &argv);

    for (count = 0; count < argc; count++) {
	if (argv[count][0] == 'm')
	    *flags |= WMSG_FLAG_MULT;
	else {
	    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		    "web::recv", WEBLOG_ERROR,
		    "unknown flag \"", argv[count], "\"", NULL);
	    Tcl_Free((char *) argv);
	    return TCL_ERROR;
	}
    }
    Tcl_Free((char *) argv);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * send_msg
 * ------------------------------------------------------------------------- */
int send_msg(Tcl_Channel f, int command, int flags, int size, void *data)
{

    /*  void *origsig; */
    MsgHeader mh;
    int ret;

    /* remember SIGPIPE handler and ignore from now on: */
    /* origsig=signal(SIGPIPE,SIG_IGN); */

    /* fill in message header */
    mh.magic = htonl(WMSG_MAGIC);
    mh.version = htonl(WMSG_VERSION);
    mh.command = htonl((u_long) ((command & 0xffff) | (flags & 0xffff0000)));
    mh.size = htonl((u_long) size);

    /* send header */
    ret = Tcl_Write(f, (char *) &mh, sizeof(MsgHeader));
    if (ret == -1) {
	/*signal(SIGPIPE,origsig); */
#ifdef MSGDEBUG
	printf("Error writing to socket\n");
#endif
	return (-1);
    }
    if (ret != sizeof(MsgHeader)) {
	/*signal(SIGPIPE,origsig); */
#ifdef MSGDEBUG
	printf("Could only write %d instead of %d bytes\n", ret,
	       sizeof(MsgHeader));
#endif
	errno = EIO;
	return (-1);
    }

    /* send data */
    ret = 0;
    if (size != 0) {
	ret = Tcl_Write(f, (char *) data, size * sizeof(char));
	if (ret == -1) {
	    /* signal(SIGPIPE,origsig); */
#ifdef MSGDEBUG
	    printf("Error writing to socket\n");
#endif
	    return (-1);
	}
	if (ret != (int) (size * sizeof(char))) {
	    /*signal(SIGPIPE,origsig); */
#ifdef MSGDEBUG
	    printf("Could only write %d instead of %d bytes\n", ret,
		   sizeof(MsgHeader));
#endif
	    errno = EIO;
	    return (-1);
	}
    }
/*signal(SIGPIPE,origsig); */
    if (!(flags & WMSG_FLAG_MULT)) {
	Tcl_Flush(f);
    }
    return 0;
}

/* ----------------------------------------------------------------------------
 * receive_msg
 * ------------------------------------------------------------------------- */
int receive_msg(Tcl_Channel f, int *command, int *flags, int *size,
		void **data)
{

    /* fd_set rfs;
       struct timeval timeout; */
    int ret, maxsize;
    u_long magic;
    u_long version;
    MsgHeader mh;

    /* timeout.tv_usec=0;
       timeout.tv_sec=WMSG_TIMEOUT; */
    magic = 0;
    if (*data == NULL) {
	maxsize = 0;
    }
    else {
	maxsize = *size;
    }
    /* read from socket until timeout or magic header word appears */
    while (magic != WMSG_MAGIC) {
	ret = Tcl_Read(f, (char *) &magic, sizeof(u_long));
	magic = ntohl(magic);
	if (ret == -1) {
#ifdef MSGDEBUG
	    printf("Error reading socket\n");
#endif
	    return (-1);	/* some error */
	}
	if (ret != sizeof(u_long)) {
#ifdef MSGDEBUG
	    printf("incomplete read or client disconnected\n");
#endif
	    errno = EIO;
	    return (-1);	/* incomplete read */
	}
    }
    /* get rest of message header */
    ret =
	Tcl_Read(f, (char *) &mh.version, sizeof(MsgHeader) - sizeof(u_long));
    if (ret == -1) {
#ifdef MSGDEBUG
	printf("Error reading socket\n");
#endif
	return (-1);		/* some error */
    }
    if (ret != sizeof(MsgHeader) - sizeof(u_long)) {
#ifdef MSGDEBUG
	printf("Incomplete header read: %d but expected %d\n", ret,
	       sizeof(MsgHeader) - sizeof(u_long));
#endif
	errno = EIO;
	return (-1);		/* incomplete read */
    }

    version = (int) ntohl(mh.version);
    if (version > WMSG_VERSION) {
#ifdef MSGDEBUG
	printf("Got unknown version %d\n", version);
#endif

#ifdef EPROTONOSUPPORT
        errno = EPROTONOSUPPORT;
#else
        errno = EIO;
#endif
	return (-1);		/* unknown version */
    }

    *command = (int) ntohl(mh.command);
    *flags = (*command & 0xffff0000);
    *command &= 0xffff;
    *size = (int) ntohl(mh.size);	/* +1 = zero terminate block */

    if (*data == NULL) {	/*alloc a buffer */
	*data = Tcl_Alloc(*size + 1);
	if (*data == NULL) {
	    errno = ENOMEM;
	    return (-1);	/* no more memory */
	}
	maxsize = *size + 1;
    }

    if ((*size + 1) > maxsize) {	/* problem: incoming msg larger than buffer */
#ifdef MSGDEBUG
	printf("Message larger than receive buffer, reallocating memory\n");
#endif
	*data = Tcl_Realloc(*data, *size + 1);
	if (*data == NULL) {
	    errno = ENOMEM;
	    return (-1);	/* no more memory */
	}
    }
    if ((*size) != 0) {
	ret = Tcl_Read(f, (char *) *data, (*size));
	if (ret == -1) {
#ifdef MSGDEBUG
	    printf("Error reading data section of message\n");
#endif
	    return (-1);
	}
	if (ret != *size) {
#ifdef MSGDEBUG
	    printf("Incomplete data read: expected %d got %d (%d)\n", *size, ret, Tcl_Eof(f));
#endif
	    errno = EIO;
	    return (-1);	/* incomplete read */
	}
    }
    /* zero terminate */
    ((char *) *data)[*size] = '\0';
    return (0);
}
