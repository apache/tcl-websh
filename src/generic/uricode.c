/*
 * uricode.c --- uri-en/decoding
 *
 * (c) 1996-20009 netcetera AG (simon.hefti@netcetera.ch)
 *
 * @(#) $Id$
 *
 */

#include <tcl.h>
#include <limits.h>
#include "web.h"
#include "stdlib.h"		/* strtol() */
#include "conv.h"

/* ----------------------------------------------------------------------------
 * Web_UriEncode -- convert string to standard URI format for querystring
 * Use Web_UriDecode to revert to plain text.
 * ------------------------------------------------------------------------- */
int Web_UriEncode(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    Tcl_Obj *tclo = NULL;

    /* ------------------------------------------------------------------------
     * arg check
     * --------------------------------------------------------------------- */
    WebAssertObjc(objc != 2, 1, "string");

    Tcl_IncrRefCount(objv[1]);
    tclo = uriEncode(objv[1]);
    Tcl_DecrRefCount(objv[1]);

    /* ------------------------------------------------------------------------
     * done
     * --------------------------------------------------------------------- */
    if (tclo != NULL) {
	Tcl_SetObjResult(interp, tclo);
	return TCL_OK;
    }

    Tcl_SetResult(interp, "web::uriencode failed.", NULL);
    return TCL_ERROR;
}

/* ----------------------------------------------------------------------------
 * Web_UriDecode -- from URI-compliant querystring back to plain
 * ------------------------------------------------------------------------- */
int Web_UriDecode(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    Tcl_Obj *res = NULL;

    /* ------------------------------------------------------------------------
     * arg check
     * --------------------------------------------------------------------- */
    WebAssertObjc(objc != 2, 1, "string");

    /* ------------------------------------------------------------------------
     *
     * --------------------------------------------------------------------- */
    res = uriDecode(objv[1]);

    if (res != NULL) {
	Tcl_SetObjResult(interp, res);
	return TCL_OK;
    }

    Tcl_SetResult(interp, "web::uridecode failed.", NULL);
    return TCL_ERROR;
}


/* ----------------------------------------------------------------------------
 * uriEncode -- encode input using %.. syntax.
 * ------------------------------------------------------------------------- */
Tcl_Obj *uriEncode(Tcl_Obj * inString)
{

    long length = 0;
    long i = 0;
    char str[32];		/* for hex representation of i. 64 bit -> string
				   with 16 chars. 32 should be long enough, then. */
    Tcl_Obj *tclo = NULL;
    char *utfs = NULL;
    Tcl_UniChar unic = 0;
    unsigned char *bytes = NULL;
    int bytesLen = -1;

    IfNullLogRetNull(NULL, inString, "uriEncode: got NULL as input.");

    tclo = Tcl_NewStringObj("", 0);

    bytes = Tcl_GetByteArrayFromObj(inString, &bytesLen);

    for (i = 0; i < bytesLen; i++) {

	/* note: switch is a factor 2 faster than if .. else if */
	switch (bytes[i]) {
	case 0:
	    break;
	case ' ':
	    Tcl_AppendToObj(tclo, "+", 1);
	    break;
	case '-':
	    Tcl_AppendToObj(tclo, "-", 1);
	    break;
	case '_':
	    Tcl_AppendToObj(tclo, "_", 1);
	    break;
	default:
	    if (bytes[i] < '0' || (bytes[i] > '9' && bytes[i] < 'A')
		|| (bytes[i] > 'Z' && bytes[i] < 'a') || bytes[i] > 'z') {
		if (bytes[i] < 16)
		    Tcl_AppendToObj(tclo, "%0", 2);
		else
		    Tcl_AppendToObj(tclo, "%", 1);
		sprintf(str, "%x", bytes[i]);
		Tcl_AppendToObj(tclo, str, -1);
	    }
	    else {
		unic = bytes[i];
		Tcl_AppendUnicodeToObj(tclo, &unic, 1);
	    }
	    break;
	}
    }

    return tclo;
}

/* ----------------------------------------------------------------------------
 * uriDecode -- decode input from %.. syntax.
 * ------------------------------------------------------------------------- */
Tcl_Obj *uriDecode(Tcl_Obj * in)
{

    int length;
    Tcl_Obj *res = NULL;
    char *utf = NULL;
    Tcl_UniChar unic;
    char buf[3];

    IfNullLogRetNull(NULL, in, "uriDecode: got NULL as input.");

    utf = Tcl_GetStringFromObj(in, &length);

    res = Tcl_NewObj();

    while (utf[0] != 0) {

	switch (utf[0]) {
	case '+':
	    Tcl_AppendToObj(res, " ", 1);
	    break;
	case '%':
	    utf = Tcl_UtfNext(utf);


	    if (utf[0] & 0x80) {
		/* case: %[7bit] */
		buf[0] = utf[0];

		utf = Tcl_UtfNext(utf);
		if (utf[0] & 0x80) {
		    /* case: %[7bit][7bit] */
		    buf[1] = utf[0];
		    buf[2] = 0;

		    unic = (Tcl_UniChar) strtol(buf, (char **) NULL, 16);
		    Tcl_AppendUnicodeToObj(res, &unic, 1);
		}
		else {

		    Tcl_AppendToObj(res, "%", 1);
		    Tcl_AppendToObj(res, buf, 1);

		    if (utf[0] == 0) {
			/* case: %[7bit][end] */
			return res;
		    }
		    else {

			/* case: %[7bit][8bit] */
			Tcl_UtfToUniChar(utf, &unic);
			Tcl_AppendUnicodeToObj(res, &unic, 1);
		    }
		}
	    }
	    else {

		/* case: %[8bit] ? */

		Tcl_AppendToObj(res, "%", 1);

		if (utf[0] == 0) {
		    /* case: %[end] */
		    return res;
		}
		else {
		    /* case: %[8bit] */
		    Tcl_UtfToUniChar(utf, &unic);
		    Tcl_AppendUnicodeToObj(res, &unic, 1);
		}
	    }
	    break;
	default:

	    Tcl_UtfToUniChar(utf, &unic);
	    Tcl_AppendUnicodeToObj(res, &unic, 1);
/*Tcl_AppendToObj(res,utf,1); *//* just the first char */
	    break;
	}
	utf = Tcl_UtfNext(utf);
    }
    return res;
}
