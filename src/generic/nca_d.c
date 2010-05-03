/*
 * nca_d.c  -- encryption plug-In for websh
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
#include "nca_d.h"
#include "checksum.h"
#include "webutl.h"


/* ----------------------------------------------------------------------------
 * init
 * ------------------------------------------------------------------------- */
int nca_d_Init(Tcl_Interp * interp)
{

    ClientData ncaD;

    /* --------------------------------------------------------------------------
     * interpreter running ?
     * ----------------------------------------------------------------------- */
    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * init internal data, and register with interp
     * ----------------------------------------------------------------------- */
    ncaD = createNcaD();
    WebAssertData(interp, ncaD, "web::encryptd init", TCL_ERROR);

    /* --------------------------------------------------------------------------
     * register data with interp
     * ----------------------------------------------------------------------- */
    Tcl_SetAssocData(interp, WEB_NCAD_ASSOC_DATA, destroyNcaD, ncaD);

    /* --------------------------------------------------------------------------
     * register commands
     * ----------------------------------------------------------------------- */
    Tcl_CreateObjCommand(interp, "web::encryptd",
			 Web_EncryptD, ncaD, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateObjCommand(interp, "web::decryptd",
			 Web_DecryptD, ncaD, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateObjCommand(interp, "web::cryptdkey",
			 Web_CryptDcfg, ncaD, (Tcl_CmdDeleteProc *) NULL);

    /* --------------------------------------------------------------------------
     * done
     * ----------------------------------------------------------------------- */
    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * createNcaD
 * ------------------------------------------------------------------------- */
ClientData createNcaD()
{

    Tcl_Obj *key = NULL;

    key = Tcl_NewObj();
    Tcl_IncrRefCount(key);

    setKeyNcaD(key, NULL);

    return (ClientData) key;
}


/* ----------------------------------------------------------------------------
 * destroyNcaD
 * ------------------------------------------------------------------------- */
void destroyNcaD(ClientData clientData, Tcl_Interp * interp)
{

    if (clientData != NULL)
	Tcl_DecrRefCount((Tcl_Obj *) clientData);
}

/* ----------------------------------------------------------------------------
 * Web_CryptDcfg
 * ------------------------------------------------------------------------- */
int Web_CryptDcfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "web::encryptd", TCL_ERROR);

    switch (objc) {
    case 1:
	return setKeyNcaD((Tcl_Obj *) clientData, NULL);
    case 2:
	return setKeyNcaD((Tcl_Obj *) clientData, objv[1]);
    default:
	Tcl_WrongNumArgs(interp, 0, objv, "?key?");
	return TCL_ERROR;
    }
}

/* ----------------------------------------------------------------------------
 * Web_EncryptD
 * ------------------------------------------------------------------------- */
int Web_EncryptD(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    Tcl_Obj *out = NULL;
    Tcl_Obj *in = NULL;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "web::encryptd", TCL_ERROR);

    /* --------------------------------------------------------------------------
     * arg check: web::encryptD msg
     *            0             1
     * ----------------------------------------------------------------------- */
    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "msg");
	return TCL_ERROR;
    }

    /* --------------------------------------------------------------------------
     * encrypt
     * ----------------------------------------------------------------------- */

    in = Tcl_DuplicateObj(objv[1]);
    Tcl_IncrRefCount(in);

    /* --------------------------------------------------------------------------
     * empty string
     * ----------------------------------------------------------------------- */
    if (Tcl_GetCharLength(in) < 1) {
	Tcl_ResetResult(interp);
	Tcl_DecrRefCount(in);
	return TCL_OK;
    }

    out = encryptNcaD(interp, clientData, in);

    Tcl_DecrRefCount(in);

    if (out == NULL) {
	return TCL_CONTINUE;
    }

    Tcl_SetObjResult(interp, out);
    Tcl_DecrRefCount(out);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_DecryptD
 * ------------------------------------------------------------------------- */
int Web_DecryptD(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    Tcl_Obj *key = NULL;
    unsigned char *keyBytes = NULL;
    int keyLen = -1;
    char *str = NULL;
    int strLen = -1;
    Tcl_Obj *out = NULL;
    Tcl_Obj *tmp = NULL;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "msg");
	return TCL_ERROR;
    }
    WebAssertData(interp, clientData, "web::decryptd", TCL_ERROR);
    key = (Tcl_Obj *) clientData;
    keyBytes = Tcl_GetByteArrayFromObj(key, &keyLen);

    if (keyLen < 1) {
	LOG_MSG(interp, SET_RESULT,
		__FILE__, __LINE__,
		"web::decryptd", WEBLOG_ERROR, "too short key", NULL);
	return TCL_ERROR;
    }

    /* --------------------------------------------------------------------------
     * check crypt tag
     * ----------------------------------------------------------------------- */
    str = Tcl_GetStringFromObj(objv[1], &strLen);
    if ((strLen >= 2) && (str[0] == 'X') && (str[1] == 'D')) {

	/* XD --> "" */
	if (strLen == 2) {

	    Tcl_SetResult(interp, "", NULL);
	    return TCL_OK;
	}

	/* ------------------------------------------------------------------------
	 * decrypt
	 * --------------------------------------------------------------------- */
	tmp = decryptNcaD(key, objv[1]);

	if (tmp == NULL) {
	    LOG_MSG(interp, SET_RESULT,
		    __FILE__, __LINE__,
		    "web::decryptd", WEBLOG_DEBUG, "internal error", NULL);
	    return TCL_ERROR;
	}

	/* ------------------------------------------------------------------------
	 * veryfy checksum
	 * --------------------------------------------------------------------- */
	out = crcCheck(tmp);	/* rturns NULL in case of error */

	if (out == NULL) {
	  /* just set interp result, but don't actually log: will be logged by
	     caller */
	    LOG_MSG(interp, SET_RESULT,
		    __FILE__, __LINE__,
		    "web::decryptd", WEBLOG_ERROR, "checksum mismatch", NULL);

	    WebDecrRefCountIfNotNull(tmp);

	    return TCL_ERROR;
	}

	Tcl_DecrRefCount(tmp);

	Tcl_SetObjResult(interp, out);

	Tcl_DecrRefCount(out);
	return TCL_OK;

    }

    LOG_MSG(interp, WRITE_LOG,
	    __FILE__, __LINE__,
	    "web::decryptd", WEBLOG_DEBUG, "crypt type not recognized", NULL);

    return TCL_CONTINUE;
}

/* ----------------------------------------------------------------------------
 * encryptNcaD
 * ------------------------------------------------------------------------- */
Tcl_Obj *encryptNcaD(Tcl_Interp * interp, ClientData clientData, Tcl_Obj * in)
{

    Tcl_Obj *out;
    int pack, type, prev = 0, newc, pos = 0;
    Tcl_Obj *key;
    unsigned char *keyBytes;
    int keyLen = -1;
    char outc;
    int i;
    char *str;
    int strLen = -1;

    if ((clientData == NULL) || (in == NULL))
	return NULL;
    key = (Tcl_Obj *) clientData;

    keyBytes = Tcl_GetByteArrayFromObj(key, &keyLen);

    if (keyLen < 1)
	return NULL;

    if (Tcl_GetCharLength(in) < 1) {
      Tcl_Obj *empty = Tcl_NewObj();
      Tcl_IncrRefCount(empty);
      return empty;
    }

    if (crcAdd(in) != TCL_OK)
	return NULL;

    str = Tcl_GetStringFromObj(in, &strLen);

    out = Tcl_NewStringObj("XD", 2);
    Tcl_IncrRefCount(out);

    for (i = 0; i < strLen; i++) {

	pack = crypt_packD((unsigned char) str[i]);

	if (pack > 256) {

	    type = pack / 256;
	    /* rotate pack according to Key3 */
	    newc = (type + 57 + (int) keyBytes[pos++] + prev) % 62;
	    pos %= keyLen;
	    prev = newc;

	    outc = crypt_tocharD(newc);

	    Tcl_AppendToObj(out, &outc, 1);
	    pack = pack - 256 * type;
	}

	/* rotate pack according to keyBytes */
	newc = (pack + (int) keyBytes[pos++] + prev) % 62;
	pos %= keyLen;
	prev = newc;

	outc = crypt_tocharD(newc);

	Tcl_AppendToObj(out, &outc, 1);
    }

    return out;
}

/* ----------------------------------------------------------------------------
 * decryptNcaD
 * ------------------------------------------------------------------------- */
Tcl_Obj *decryptNcaD(Tcl_Obj * key, Tcl_Obj * in)
{

    char *str = NULL;
    int strLen = -1;
    unsigned char *keyBytes = NULL;
    int keyLen = -1;

    int pack = 0, type = 0, prev = 0, newc = 0, pos = 0;
    int i;
    Tcl_Obj *out = NULL;
    char outc;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if ((key == NULL) || (in == NULL))
	return NULL;

    /* --------------------------------------------------------------------------
     * go ahead
     * ----------------------------------------------------------------------- */
    keyBytes = Tcl_GetByteArrayFromObj(key, &keyLen);
    str = Tcl_GetStringFromObj(in, &strLen);

    out = Tcl_NewObj();
    Tcl_IncrRefCount(out);

    for (i = 2; i < strLen; i++) {

	pack = crypt_fromcharD(str[i]);

	/* back rotation according to keyBytes */
	newc = (620 + pack - (int) keyBytes[pos++] - prev) % 62;
	pos %= keyLen;

	prev = pack;

	if (newc > 57) {

	    type = newc - 57;

	    i++;
	    pack = crypt_fromcharD(str[i]);

	    /* back rotation according to keyBytes */
	    newc = (620 + pack - (int) keyBytes[pos++] - prev) % 62;
	    pos %= keyLen;
	    prev = pack;
	}
	else
	    type = 0;

	outc = (char) crypt_unpackD((type * 256) + newc);
	Tcl_AppendToObj(out, &outc, 1);
    }

    return out;
}

/* ----------------------------------------------------------------------------
 * setKeyNcaD
 * ------------------------------------------------------------------------- */
int setKeyNcaD(Tcl_Obj * key, Tcl_Obj * in)
{

    unsigned char dum[50] = {
	    0xbb, 0x65, 0xf6, 0x72, 0x13, 0x3e, 0x54, 0x8d, 0x7a, 0x58,
	0x47, 0xca, 0xae, 0x94, 0x1b, 0x98, 0x4e, 0xdb, 0x02, 0x64,
	0x9f, 0x81, 0x70, 0x3a, 0x43, 0x4c, 0x00, 0xe4, 0x89, 0x3d,
	0x39, 0x43, 0x97, 0xd0, 0x95, 0xc9, 0xac, 0xc5, 0x0b, 0x29,
	0x4f, 0xcc, 0xa9, 0x7b, 0x1f, 0x33, 0xc8, 0x0b, 0x89, 0x30
    };
    unsigned char *inBytes = NULL;
    int inLen = -1;

    if (key == NULL)
	return TCL_ERROR;

    if (in == NULL) {

	/* ------------------------------------------------------------------------
	 * set default key
	 * --------------------------------------------------------------------- */
	Tcl_SetByteArrayObj(key, dum, 50);
	return TCL_OK;
    }

    inBytes = Tcl_GetByteArrayFromObj(in, &inLen);

    Tcl_SetByteArrayObj(key, inBytes, inLen);

    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * crypt_packD --
 * ------------------------------------------------------------------------- */
int crypt_packD(unsigned char in)
{
    int out;
    int type = 0;
    if (in < 48) {
	type = 1;
	out = (int) in;
    }
    else if (in <= 57) {
	out = ((int) in - 48);
    }
    else if (in < 65) {
	type = 1;
	out = ((int) in - 10);
    }
    else if (in <= 86) {
	out = ((int) in - 55);
    }
    else if (in <= 90) {
	type = 4;
	out = ((int) in - 33);
    }
    else if (in < 97) {
	type = 4;
	out = ((int) in - 60);
    }
    else if (in <= 122) {
	out = ((int) in - 65);
    }
    else if (in <= 173) {
	type = 2;
	out = ((int) in - 123);
    }
    else if (in <= 224) {
	type = 3;
	out = ((int) in - 167);
    }
    else {
	type = 4;
	out = ((int) in - 225);
    }
    out += 256 * type;
    return out;
}

/* ----------------------------------------------------------------------------
 * crypt_packD --
 * ------------------------------------------------------------------------- */
unsigned char crypt_unpackD(int in)
{
    int out, type;
    type = in / 256;
    in = in - 256 * type;
    switch (type) {
    case 0:
	if (in < 10)
	    out = (in + 48);
	else if (in <= 31)
	    out = (in + 55);
	else
	    out = (in + 65);
	break;
    case 1:
	if (in < 48)
	    out = in;
	else
	    out = (in + 10);
	break;
    case 2:
	out = (in + 123);
	break;
    case 3:
	out = (in + 167);
	break;
    default:
	if (in < 31)
	    out = (in + 225);
	else if (in < 37)
	    out = (in + 60);
	else
	    out = (in + 33);
	break;
    }
    return (unsigned char) out;
}

/* ----------------------------------------------------------------------------
 * crypt_packD --
 * ------------------------------------------------------------------------- */
char crypt_tocharD(int in)
{
    char out;
    if (in < 10)
	out = (char) (in + 48);
    else if (in < 36)
	out = (char) (in + 55);
    else
	out = (char) (in + 61);
    return out;
}

/* ----------------------------------------------------------------------------
 * crypt_packD --
 * ------------------------------------------------------------------------- */
int crypt_fromcharD(char in)
{
    int out;
    if (in < 58)
	out = (int) in - 48;
    else if (in < 91)
	out = (int) in - 55;
    else
	out = (int) in - 61;
    return out;
}
