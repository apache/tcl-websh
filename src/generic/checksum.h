/*
 * checksum.h -- utils to add Cyclic-Redundancy-Checksums to Tcl_Obj
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

#ifndef CHECKSUM_H
#define CHECKSUM_H

#define WEB_LOW_BYTE(x) ((unsigned char)((x) & 0xFF))
#define WEB_HIG_BYTE(x) ((unsigned char)((x) >> 8))

unsigned short crcCalc(Tcl_Obj * in);
Tcl_Obj *crcAsciify(unsigned short crc);
unsigned short crcDeAsciify(Tcl_Obj * in);
Tcl_Obj *crcCheck(Tcl_Obj * in);
int crcAdd(Tcl_Obj * in);


#endif
