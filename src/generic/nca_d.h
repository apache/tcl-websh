/*
 * nca_d.h --
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

#include "tcl.h"

#ifndef NCAD_H
#define NCAD_H

#define WEB_NCAD_ASSOC_DATA "web::ncad"

int nca_d_Init(Tcl_Interp * interp);


ClientData createNcaD();
void destroyNcaD(ClientData clientData, Tcl_Interp * interp);
Tcl_Obj *encryptNcaD(Tcl_Interp * interp, ClientData clientData,
		     Tcl_Obj * in);
Tcl_Obj *decryptNcaD(Tcl_Obj * key, Tcl_Obj * in);
int setKeyNcaD(Tcl_Obj * key, Tcl_Obj * in);

int Web_EncryptD(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_DecryptD(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_CryptDcfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int crypt_fromcharD(char in);
char crypt_tocharD(int in);
unsigned char crypt_unpackD(int in);
int crypt_packD(unsigned char in);

#endif
