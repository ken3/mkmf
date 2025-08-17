#ifndef _MKMF_H_INCLUDED
#define _MKMF_H_INCLUDED

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
 * $Header: Mkmf.h,v 4.7 93/05/25 22:16:40 nicklin Exp $
 *
 * Mkmf definitions
 *
 * Author: Peter J. Nicklin
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlist.h"
#include "target.h"

/*
 * Buffer sizes
 */
#define ANSWERBUFSIZE		  256	/* interactive answer buffer size */
#define INCLUDETABLESIZE	 1021	/* include file table size */
#define SOURCETABLESIZE		 1021	/* source file table size */
#define MACRODEFSIZE		16384	/* macro definition body size */
#define MACRONAMSIZE		   64	/* macro definition name size */
#define MDEFTABLESIZE		  127	/* macro definition table size */
#define RULETABSIZE	    	  256	/* rule table size */
#define SFXTABSIZE		  256	/* suffix table size */
#define SUFFIXSIZE		   16	/* suffix size */

/*
 * Predefined macro names
 */
#define MCXXFLAGS	"CXXFLAGS"	    /* C++ flags macro for HP 'make'     */
#define MCPXFLAGS	"C++FLAGS"      /* C++ flags macro for SYSVR4 'make' */
#define MCCFLAGS	"CCFLAGS"       /* C++ flags macro for SunOS 'make'  */
#define MCFLAGS		"CFLAGS"
#define MCOMPILESYSTYPE	"COMPILESYSTYPE"
#define MDESTDIR	"DEST"
#define MEXTERNALS	"EXTHDRS"
#define MFFLAGS		"FFLAGS"
#define MHEADERS	"HDRS"
#define MLDFLAGS	"LDFLAGS"
#define MLIBLIST	"LIBS"
#define MLIBRARY	"LIBRARY"
#define MLPATH		"LPATH"
#define MMAKEFILE	"MAKEFILE"
#define MOBJECTS	"OBJS"
#define MPFLAGS		"PFLAGS"
#define MPROGRAM	"PROGRAM"
#define MSOURCES	"SRCS"
#define MSUFFIX		"SUFFIX"
#define MSYSHDRS	"SYSHDRS"
#define MVPATH		"VPATH"

/*
 * Predefined $(macro) instances
 */
#define DCXXFLAGS	"$(CXXFLAGS)"	/* C++ flags macro for HP 'make'     */
#define DCPXFLAGS	"$(C++FLAGS)"   /* C++ flags macro for SYSVR4 'make' */
#define DCCFLAGS	"$(CCFLAGS)"    /* C++ flags macro for SunOS 'make'  */
#define DCFLAGS		"$(CFLAGS)"
#define DDESTDIR	"$(DEST)"
#define DEXTERNALS	"$(EXTHDRS)"
#define DFFLAGS		"$(FFLAGS)"
#define DHEADERS	"$(HDRS)"
#define DLDFLAGS	"$(LDFLAGS)"
#define DLIBLIST	"$(LIBS)"
#define DLIBRARY	"$(LIBRARY)"
#define DLPATH		"$(LPATH)"
#define DMAKEFILE	"$(MAKEFILE)"
#define DOBJECTS	"$(OBJS)"
#define DPFLAGS		"$(PFLAGS)"
#define DPROGRAM	"$(PROGRAM)"
#define DSOURCE		"$(SRCS)"
#define DSUFFIX		"$(SUFFIX)"
#define DSYSHDRS	"$(SYSHDRS)"
#define DVPATH		"$(VPATH)"

/*
 * Predefined ${macro} instances
 */
#define dCXXFLAGS	"${CXXFLAGS}"	/* C++ flags macro for HP 'make'     */
#define dCPXFLAGS	"${C++FLAGS}"   /* C++ flags macro for SYSVR4 'make' */
#define dCCFLAGS	"${CCFLAGS}"    /* C++ flags macro for SunOS 'make'  */
#define dCFLAGS		"${CFLAGS}"
#define dDESTDIR	"${DEST}"
#define dEXTERNALS	"${EXTHDRS}"
#define dFFLAGS		"${FFLAGS}"
#define dHEADERS	"${HDRS}"
#define dLDFLAGS	"${LDFLAGS}"
#define dLIBLIST	"${LIBS}"
#define dLIBRARY	"${LIBRARY}"
#define dLPATH		"${LPATH}"
#define dMAKEFILE	"${MAKEFILE}"
#define dOBJECTS	"${OBJS}"
#define dPFLAGS		"${PFLAGS}"
#define dPROGRAM	"${PROGRAM}"
#define dSOURCE		"${SRCS}"
#define dSUFFIX		"${SUFFIX}"
#define dSYSHDRS	"${SYSHDRS}"
#define dVPATH		"${VPATH}"

/*
 * Predefined macro values
 */
#define VERROR		       -1
#define VUNKNOWN		0
#define VREADONLY		1
#define VREADWRITE		2
#define VDYNAMIC		3
#define VDESTDIR		4
#define VPROGRAM		5
#define VLIBRARY		6

/*
 * Predefined template suffixes
 */
#define SPROGRAM		".p"
#define SLIBRARY		".l"
#define SLIBRARY2		".L"

/*
 * Include statement styles
 */
#define INCLUDE_NONE		0	/* no include file */
#define INCLUDE_C		1	/* #include "file" */
#define INCLUDE_CXX		2	/* #include "file" for C++ */
#define INCLUDE_FORTRAN		3	/* include "file" or #include "file" */
#define INCLUDE_PASCAL		4	/* #include "file" */

/*
 * Marker to indicate start of included file dependencies
 */
#define DEPENDMARK		"###"

/*
 * Mkmf directories
 */
#ifndef INSTALLDIR
#define INSTALLDIR	"/usr"
#endif
#define MKMFLIB		"lib/mkmf"

/*
 * Functions defined in badopt.c
 */
void  badopt(char, char);

/*
 * Functions defined in buildlist.c
 */
int   buftolist(char*, SLIST*);
int   buildliblist(void);
int   buildsrclist(void);
int   uniqsrclist(void);
int   expandlibpath(char*, char*, SLIST*);
int   libbuftolist(char*, SLIST*, SLIST*);
int   read_dir(char*, int (*)(char*, char*, int), int, int);

/*
 * Functions defined in depend.c
 */
void     addincdir(void);
int      findinclude(char*, char*, char*, int);
void     getI(char*, char*, SLIST*);
int      getinclude(char*, char*, int, FILE*);
INCBLK*  inclink(HASHBLK*);
HASHBLK* instalinclude(char*, char*, int);
HASHBLK* lookupinclude(char*, int);
DLIST*   mkdepend(void);
void     notfound(char*, int, char *);
INCBLK*  readC(char*, int, char *, int);
INCBLK*  readF(char*, int, char *, int);
INCBLK*  readP(char*, int, char *, int);
int      initsysinclude(void);

/*
 * Functions defined in editmf.c
 */
void  editmf(char*, char*);

/*
 * Functions defined in getcwp.c
 */
char* getcwp(void);

/*
 * Functions defined in getproject.c
 */
void  getproject(void);

/*
 * Functions defined in iolin.c
 */
char* getlin(FILE*);
void  purgcontinue(FILE*);
void  putlin(FILE*);

/*
 * Functions defined in macro.c
 */
char* findmacro(char*, char*);
char* getmacro(char*, FILE*);
void  putmacro(char*, FILE*);
void  putobjmacro(FILE*);
void  putslmacro(SLIST*, FILE*);
int   storemacro(char*);
int   storenvmacro(void);
int   storedynmacro(void);

/*
 * Functions defined in misc.c
 */
void  answer(char*, int);
int   fastcopy(char*, FILE*);
int   findmf(char*, char*, TARGET*);
void  findmftemplate(char*, TARGET*);
char* gettoken(char*, char*);
char* getoption(char*, char*, char*);
char* getpath(char*, char*);
int   libobj(char*);
char* mktname(char*, char*);
void  nocore(void);
int   putobj(char*, FILE*);
int   readmf(char*, TARGET*);

/*
 * Functions defined in mksrclist.c
 */
int   mksrclist(int, int);
int   addsrclist(char*, char*, int);

/*
 * Functions defined in mksymlink.c
 */
int   mksymlink(int, int);

/*
 * Functions defined in mustfopen.c
 */
FILE* mustfopen(char*, char*);

/*
 * Functions defined in optpath.c
 */
char* optpath(char*);

/*
 * Functions defined in pathcat.c
 */
char* pathcat(char*, char*, char*);

/*
 * Functions defined in pathhead.c
 */
char* pathhead(char*);

/*
 * Functions defined in pperror.c
 */
void  pperror(char*);

/*
 * Functions defined in slsort.c
 */
int   slsort(int (*)(const char*, const char*), SLIST*);

/*
 * Functions defined in strpcpy.c
 */
char* strpcpy(char*, char*);

/*
 * Functions defined in strsav.c
 */
char* strsav(char*);

/*
 * Functions defined in usage.c
 */
void  usage(char*);

/*
 * Functions defined in warn.c
 */
void  warn(char*);
void  warns(char*, char*);
void  warn2(char*, char*, char*);

#endif /* _MKMF_H_INCLUDED */

