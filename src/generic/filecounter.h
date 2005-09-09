/*
 * filecounter.h --- file-based unique ID generator
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

#include "args.h"
#include "filelock.h"
#include "log.h"
#include "request.h"
#include "tcl.h"

#ifndef FILECOUNTER_H
#define FILECOUNTER_H

#define WEB_FILECOUNTER_MAXVAL 2147483647
#define WEB_FILECOUNTER_MINVAL 0
#define WEB_FILECOUNTER_INCR 1
#define WEB_FILECOUNTER_SEED 0
#define WEB_FILECOUNTER_WRAP 0


/* --------------------------------------------------------------------------
 * SeqNoGenerator
 * --------------------------------------------------------------------------*/
typedef struct SeqNoGenerator
{
    char *fileName;
    char *handleName;
    int seed;
    int minValue;
    int maxValue;
    int incrValue;
    int currValue;
    int mask;
    int doWrap;
    int hasCurrent;
}
SeqNoGenerator;

SeqNoGenerator *createSeqNoGenerator(RequestData * requestData,
				     Tcl_Obj * hn, Tcl_Obj * fn,
				     Tcl_Obj * seed, Tcl_Obj * min,
				     Tcl_Obj * max, Tcl_Obj * incr,
				     Tcl_Obj * mask, Tcl_Obj * wrap);

int deleteSeqNoGenerator(SeqNoGenerator * seqnogen);
int destroySeqNoGenerator(ClientData clientData, Tcl_Interp * interp);
int nextSeqNo(Tcl_Interp * interp, SeqNoGenerator * seqnogen, int *seqno);

/* --------------------------------------------------------------------------
 * The function to register with TCL
 * ------------------------------------------------------------------------*/
int filecounter(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Filecounter(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * objv[]);

int filecounter_Init(Tcl_Interp * interp);

#endif
