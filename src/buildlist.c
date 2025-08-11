/*
 * Copyright (c) 1983, 1985, 1991 Peter J. Nicklin.
 * Copyright (c) 1991 Version Technology.
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
 * $Header: buildlist.c,v 4.10 91/11/25 19:44:59 nicklin Exp $
 *
 * Author: Peter J. Nicklin
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Mkmf.h"
#include "config.h"
#include "dir.h"
#include "hash.h"
#include "null.h"
#include "path.h"
#include "slist.h"
#include "stringx.h"
#include "suffix.h"
#include "yesno.h"

extern SLIST *HEADLIST;			/* header file name list */
extern SLIST *SRCLIST;			/* source file name list */
extern SLIST *LIBLIST;			/* library pathname list */
extern HASH  *MDEFTABLE;		/* macro definition table */
extern int   MKSYMLINK;			/* make symbolic links to current dir ?*/
extern int   AFLAG;			/* list .source files? */

int libbuftolist(char*, SLIST*, SLIST*);
void uniqsrcfile(int, int, SLBLK**);
int uniqsrclist(void);

/*
 * buftolist() copies the items from a buffer to a singly-linked list.
 * Returns integer YES if successful, otherwise NO.
 */
int buftolist(char *buf, SLIST *list)
// char *buf;			/* item buffer */
// SLIST *list;			/* receiving list */
{
	char token[PATHSIZE];		/* item buffer */

	while ((buf = gettoken(token, buf)) != NULL)
		{
		if (slappend(token, list) == NULL)
			return(NO);
		}
	return(YES);
}



/*
 * buildliblist() reads library pathnames from the LIBLIST macro
 * definition, and adds them to the library pathname list. Libraries
 * may be specified as `-lx'. Returns integer YES if successful,
 * otherwise NO.
 */
int buildliblist(void)
{
	char *lp;			/* library path pointer */
	char lpath[PATHSIZE];		/* library path buffer */
	HASHBLK *htb;			/* hash table block */
	SLIST *libpathlist;		/* library directory search path */

	/* create the library search path list */
	libpathlist = slinit();		

	/* -L library path specification */
	if (htlookup(MLDFLAGS, MDEFTABLE) != NULL)
		{
		lp = htdef(MDEFTABLE);
		while ((lp = getoption(lpath, lp, "-L")) != NULL)
			{
			if (*lpath == '\0')
				{ 
				warns("missing library in %s macro definition",
				       htkey(MDEFTABLE));
				break;
				}
			else	{
				strcat(lpath, "/lib");
				if (slappend(lpath, libpathlist) == NULL)
					return(NO);
				}
			}
		}

	/* LPATH environment variable library path specification */
	if (htlookup(MLPATH, MDEFTABLE) != NULL && *htdef(MDEFTABLE) != '\0')
		{
		lp = htdef(MDEFTABLE);
		while ((lp = getpath(lpath, lp)) != NULL)
			{
			strcat(lpath, "/lib");
			if (slappend(lpath, libpathlist) == NULL)
				return(NO);
			}
		}
	else	{
		if (slappend("/lib/lib", libpathlist) == NULL ||
		    slappend("/usr/lib/lib", libpathlist) == NULL)		
			return(NO);
		}

	/* search for the libraries */
	LIBLIST = NULL;
	if ((htb = htlookup(MLIBLIST, MDEFTABLE)) != NULL)
		{
		LIBLIST = slinit();
		if (libbuftolist(htb->h_def, libpathlist, LIBLIST) == NO)
			return(NO);
		}
	slrm(libpathlist);
	htrm(MLIBLIST, MDEFTABLE);
	return(YES);
}



/*
 * buildsrclist() takes source and header file names from command line
 * macro definitions or the current directory and appends them to source
 * or header file name lists as appropriate. Returns integer YES if
 * successful, otherwise NO.
 */
int buildsrclist(void)
{
	HASHBLK *headhtb;		/* HEADERS macro hash table block */
	HASHBLK *srchtb;		/* SOURCES macro hash table block */
	int needhdr = 1;		/* need header file names */
	int needsrc = 1;		/* need source file names */

	HEADLIST = slinit();
	SRCLIST = slinit();

	/* build lists from command line macro definitions */
	if ((headhtb = htlookup(MHEADERS, MDEFTABLE)) != NULL &&
	     headhtb->h_val == VREADWRITE)
		{
		if (buftolist(headhtb->h_def, HEADLIST) == NO)
			return(NO);
		needhdr = 0;
		}
	if ((srchtb = htlookup(MSOURCES, MDEFTABLE)) != NULL &&
	     srchtb->h_val == VREADWRITE)
		{
		if (buftolist(srchtb->h_def, SRCLIST) == NO)
			return(NO);
		needsrc = 0;
		}
	
	/* read directories to get source and header file names */
	if (needhdr || needsrc)
		{
		if (MKSYMLINK)
			{
			if (mksymlink(needsrc, needhdr) == NO)
				return(NO);
			}
		else	{
			if (mksrclist(needsrc, needhdr) == NO)
				return(NO);
			}
		}

	if (slsort(strcmp, HEADLIST) == NO)
		return(NO);
	if (slsort(strcmp, SRCLIST) == NO)
		return(NO);
	if (SLNUM(SRCLIST) > 0 && uniqsrclist() == NO)
		return(NO);
	return(YES);
}



/*
 * expandlibpath() converts a library file specified by `-lx' into a full
 * pathname. Directories are searched for the library in the form libx.a.
 * An integer YES is returned if the library was found, otherwise NO.
 */
int expandlibpath(char *libtoken, char *libpath, SLIST *libpathlist)
// char *libtoken;			/* library file option */
// char *libpath;			/* library pathname */
// SLIST *libpathlist;		/* library directory search path */
{
	char *lp;			/* library pathname pointer */
	SLBLK *cblk;			/* current list block */

	libtoken += 2;			/* skip -l option */
	for (cblk = libpathlist->head; cblk != NULL; cblk = cblk->next)
		{
		lp = strpcpy(libpath, cblk->key);
		lp = strpcpy(lp, libtoken);
		(void) strpcpy(lp, ".a");
		if (FILEXIST(libpath))
			{
			return(YES);
			}
		}
	return(NO);
}



/*
 * libbuftolist() appends each library pathname specified in libbuf to
 * the liblist library pathname list.
 */
int libbuftolist(char *libmacrobuf, SLIST *libpathlist, SLIST *liblist)
// char *libmacrobuf;		/* LIBS macro definition buffer */
// SLIST *libpathlist;		/* library directory search path */
// SLIST *liblist;			/* library pathname list */
{
	char libpath[PATHSIZE];		/* library pathname */
	char libtoken[PATHSIZE];	/* library file option */

	while ((libmacrobuf = gettoken(libtoken, libmacrobuf)) != NULL)
		{
		if (libtoken[0] == '-' && libtoken[1] == 'l')
			{
			if (expandlibpath(libtoken, libpath, libpathlist) == NO)
				{
				warns("can't find library %s", libtoken);
				}
			else	{
				if (slappend(libpath, liblist) == NULL)
					return(NO);
				}
			}
		else	{
			if (slappend(libtoken, liblist) == NULL)
				return(NO);
			}
		}
	return(YES);
}



/*
 * read_dir() reads filenames from the designated directory and adds them
 * to the source or header file name lists as appropriate. Returns
 * integer YES if successful, otherwise NO.
 */
int read_dir(char *dirname, int (*addfile)(char*, char*, int), int needsrc, int needhdr)
// char *dirname;		/* specified directory name */
// int (*addfile)()		/* function for adding source files */
// int needsrc;			/* need source file names */
// int needhdr;			/* need header file names */
{
	char *suffix;			/* pointer to file name suffix */
	DIR *dirp;			/* directory stream */
	DIRENT *dp;			/* directory entry pointer */
	int sfxtyp;			/* type of suffix */

	if ((dirp = opendir(dirname)) == NULL)
		{
		warns("%s: can't open directory", dirname);
		return(NO);
		}
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		{
		if ((suffix = (char*)strrchr(dp->d_name, '.')) == 0 ||
		    (*dp->d_name == '.' && AFLAG == NO) ||
		    (*dp->d_name == '#'))
			continue;
		suffix++;
		sfxtyp = lookupsfx(suffix);
		if (sfxtyp == SFXSRC)
			{
			if (needsrc)
				{
				if (strncmp(dp->d_name, "s.", 2) == 0)
					continue; /* skip SCCS files */
				if ((*addfile)(dirname, dp->d_name, 's') == NO)
					return(NO);
				}
			}
		else if (sfxtyp == SFXHEAD)
			{
			if (needhdr)
				{
				if (strncmp(dp->d_name, "s.", 2) == 0)
					continue; /* skip SCCS files */
				if ((*addfile)(dirname, dp->d_name, 'h') == NO)
					return(NO);
				}
			}
		}
	closedir(dirp);
	return(YES);
}



/*
 * uniqsrclist() scans the source file list and removes the names of
 * any files that could have been generated by make rules from files
 * in the same list. Returns NO if no source list or out of memory,
 * otherwise YES.
 */
int uniqsrclist(void)
{
	int cbi;		/* current block vector index */
	int fbi;		/* first matching block vector index */
	int lbi;		/* last block vector index */
	uintptr_t length;		/* source file basename length */
	SLBLK **slv;			/* ptr to singly-linked list vector */

	if ((slv = slvect(SRCLIST)) == NULL)
		return(NO);
	lbi = SLNUM(SRCLIST) - 1;
	for (fbi=0, cbi=1; cbi <= lbi; cbi++)
		{
		length = (uintptr_t)strrchr(slv[cbi]->key, '.') - (uintptr_t)slv[cbi]->key + 1;
		if (strncmp(slv[fbi]->key, slv[cbi]->key, length) == 0)
			{
			continue;	/* find end of matching block */
			}
		else if (cbi - fbi > 1)
			{
			uniqsrcfile(fbi, cbi-fbi, slv);
			}
		fbi = cbi;
		}
	if (cbi - fbi > 1)		/* do last matching block */
		{
		uniqsrcfile(fbi, cbi-fbi, slv);
		}
	slvtol(SRCLIST, slv);
	return(YES);
}



/*
 * uniqsrcfile() scans a block of source files and removes the names of
 * any files that could have been generated by make rules from files
 * in the same block. The names of a source file is removed by setting
 * the pointer to the source file block to NULL in the vector. To maintain
 * the transitive property of each rule, the name of the source file is
 * still maintained in the singly-linked list.
 */

void uniqsrcfile(int fbi, int nb, SLBLK **slv)
// int fbi;	/* index to first matching block */
// int nb;				/* number of blocks */
// SLBLK **slv;			/* ptr to singly-linked list vector */
{
	SLBLK *ibp;		/* block pointer (i-loop) */
	SLBLK *jbp;		/* block pointer (j-loop) */
	int i;			/* i-loop index */
	int j;			/* j-loop index */
	char rule[2*SUFFIXSIZE+3];	/* rule buffer */
	int nu;				/* number of unique blocks */
	SLBLK *fbp;			/* pointer to first block */

	nu  = nb;
	fbp = slv[fbi];

	for (i=0, ibp=fbp; i < nb; i++, ibp=ibp->next)
		{
		for (j=0, jbp=fbp; j < nb; j++, jbp=jbp->next)
			{
			if (i != j && slv[fbi+j] != NULL)
				{
				makerule(rule, ibp->key, jbp->key);
				if (lookuprule(rule) == YES)
					{
					slv[fbi+j] = NULL;
					if (--nu < 2) return;
					}
				}
			}
		}
}
