/*
 * Copyright (c) 1983, 1985, 1991, 1993 Peter J. Nicklin.
 * Copyright (c) 1991, 1993 Version Technology.
 * All Rights Reserved.
 *
 * $License: VT.1.1 $
 * Redistribution and use in source and binary forms,  with or without
 * modification,  are permitted provided that the following conditions
 * are met:  (1) Redistributions of source code must retain the  above
 * copyright  notice,  this  list  of  conditions  and  the  following
 * disclaimer.  (2) Redistributions in binary form must reproduce  the
 * above  copyright notice,  this list of conditions and the following
 * disclaimer in the  documentation  and/or other  materials  provided
 * with  the  distribution.  (3) All advertising materials  mentioning
 * features or  use  of  this  software  must  display  the  following
 * acknowledgement:  ``This  product  includes  software  developed by
 * Version Technology.''  Neither the name of Version  Technology  nor
 * the  name  of  Peter J. Nicklin  may  be used to endorse or promote
 * products derived from this software without specific prior  written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY VERSION TECHNOLOGY ``AS IS''  AND  ANY
 * EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO, THE
 * IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL  VERSION  TECHNOLOGY  BE
 * LIABLE  FOR ANY DIRECT,  INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR  CONSEQUENTIAL DAMAGES   (INCLUDING,   BUT   NOT   LIMITED   TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR  TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING  IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE,  EVEN  IF  ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Report problems and direct questions to nicklin@netcom.com
 *
 * $Header: editmf.c,v 4.6 93/05/25 21:49:09 nicklin Exp $
 *
 * Author: Peter J. Nicklin
 */
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include "Mkmf.h"
#include "config.h"
#include "dlist.h"
#include "hash.h"
#include "macro.h"
#include "null.h"
#include "slist.h"
#include "yesno.h"

extern char IOBUF[];		/* I/O buffer line */
extern int DEPEND;		/* dependency analysis? */
extern SLIST *EXTLIST;		/* external header file name list */
extern SLIST *HEADLIST;		/* header file name list */
extern SLIST *LIBLIST;		/* library pathname list */
extern SLIST *SRCLIST;		/* source file name list */
extern SLIST *SYSLIST;		/* system header file name list */
extern HASH *MDEFTABLE;		/* macro definition table */

static char Mftemp[] = "mkmfXXXXXX";		/* temporary makefile */

/*
 * cleanup() removes the temporary makefile and dependency file, and
 * calls exit(1).
 */
void
cleanup(int num)
// int num;			/* received signal number */
{
	unlink(Mftemp);
	_exit(1);
}

/* change signal handler */
static void set_signal_handlers(void (*handler)(int))
// void (*handler)(int);			/* signal handler */
{
	signal(SIGINT,  handler);
	signal(SIGHUP,  handler);
	signal(SIGQUIT, handler);
}

/* macro entry table */
typedef struct {
	const char *name;
	SLIST **list;
	int require_depend;
	void (*special_handler)(FILE *ofp);
} macro_entry_t;

/* Find macro entry by name in macro table */
static macro_entry_t* find_macro_entry(const char *name, macro_entry_t *table)
// const char *name;			/* macro name to find */
// macro_entry_t *table;		/* macro entry table */
{
	for (macro_entry_t *entry = table; entry->name != NULL; entry++) {
		if (EQUAL(name, entry->name)) return entry;
	}
	return NULL;
}

static void handle_objmacro(FILE *ofp) {
	putobjmacro(ofp);
}

/*
 * editmf() replaces macro definitions within a makefile.
 */
void
editmf(char *mfname, char *mfpath)
// char *mfname;			/* makefile name */
// char *mfpath;			/* makefile template pathname */
{
	char mnam[MACRONAMSIZE];	/* macro name buffer */
	DLIST *dlp;			/* dependency list */
	FILE *ifp;			/* input stream */
	FILE *ofp;			/* output stream */
	HASHBLK *htb;			/* hash table block */
	macro_entry_t *entry;
	macro_entry_t macro_table[] = {
		{ MHEADERS,   &HEADLIST,   0, NULL },
		{ MOBJECTS,   NULL,        0, handle_objmacro },
		{ MSOURCES,   &SRCLIST,    0, NULL },
		{ MSYSHDRS,   &SYSLIST,    0, NULL },
		{ MEXTERNALS, &EXTLIST,    1, NULL },
		{ MLIBLIST,   &LIBLIST,    0, NULL },
		{ NULL,       NULL,        0, NULL }
	};

	set_signal_handlers(cleanup);

	mkstemp(Mftemp);
	ofp = mustfopen(Mftemp, "w");
	ifp = mustfopen(mfpath, "r");
	if (DEPEND) dlp = mkdepend();

	while (getlin(ifp) != NULL) {
		if (DEPEND && EQUAL(IOBUF, DEPENDMARK)) break;
		if (findmacro(mnam, IOBUF) != NULL) {
			entry = find_macro_entry(mnam, macro_table);
			if (entry) {
				if (entry->require_depend && !DEPEND) {
					putlin(ofp);
				} else if (entry->special_handler) {
					entry->special_handler(ofp);
				} else {
					putslmacro(*(entry->list), ofp);
				}
				purgcontinue(ifp);
			} else {
				htb = htlookup(mnam, MDEFTABLE);
				if (htb && htb->h_val == VREADWRITE) {
					putmacro(htb->h_def, ofp);
					purgcontinue(ifp);
				} else {
					putlin(ofp);
				}
			}
		} else {
			putlin(ofp);
		}
	}

	if (DEPEND) dlprint(dlp, ofp);
	fclose(ifp);
	fclose(ofp);

	RENAME(Mftemp, mfname);
	set_signal_handlers(SIG_IGN);

	return;
}
