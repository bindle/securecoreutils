/*
 *  Secure Core Utilities
 *  Copyright (C) 2015 Bindle Binaries <syzdek@bindlebinaries.com>.
 *
 *  @BINDLE_BINARIES_BSD_LICENSE_START@
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Bindle Binaries nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BINDLE BINARIES BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *  @BINDLE_BINARIES_BSD_LICENSE_END@
 */
/**
 *  @file securecoreutils.c
 *  Secure Core Utils widget wrapper
 */
#ifndef __SRC_SECURECOREUTILS_H
#define __SRC_SECURECOREUTILS_H 1


///////////////
//           //
//  Headers  //
//           //
///////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Headers
#endif

#ifdef HAVE_CONFIG_H
#   include "config.h"
#else
#   include "git-package-version.h"
#endif

#include <getopt.h>
#include <unistd.h>
#include <limits.h>


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Definitions
#endif

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "securecoreutils"
#endif
#ifndef PACKAGE_BUGREPORT
#define PACKAGE_BUGREPORT "syzdek@bindlebinaries.com"
#endif
#ifndef PACKAGE_COPYRIGHT
#define PACKAGE_COPYRIGHT ""
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME ":-|"
#endif
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION ""
#endif


#ifdef LINE_MAX
#   define SCU_LINE_MAX LINE_MAX
#elif defined _POSIX2_LINE_MAX
#   define SCU_LINE_MAX _POSIX2_LINE_MAX
#else
#   define SCU_LINE_MAX 2048
#endif
#define SCU_BUFF_MAX (SCU_LINE_MAX*20)


#define SCU_ESUCCESS 0
#define SCU_ERRNO    1
#define SCU_EPATH    2
#define SCU_EFILE    3
#define SCU_EANCHOR  4
#define SCU_EDIR     5


//////////////////
//              //
//  Data Types  //
//              //
//////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Data Types
#endif

typedef struct scu_config     scu_config;
typedef struct scu_widget     scu_widget;

struct scu_config
{
   int                  quiet;
   int                  verbose;
   int                  opt_index;
   int                  argc;
   char               * prog_name;
   char              ** argv;
   const scu_widget   * widget;
};


struct scu_widget
{
   const char        * name;
   const char        * desc;
   const char * const * alias;
   int  (*func)(scu_config * cnf);
};


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark -
#endif

/// checks paths
int scu_pathcheck(const char * path, int isdir);

/// Displays secure core utils wrapper usage
void scu_usage(void);
void scu_usage_options(void);
void scu_usage_restrictions(void);
void scu_usage_summary(scu_config * cnf, const char * suffix);


const char * scu_strerror(int err);


/// Displays secure core utils version
void scu_version(void);

int scu_is_ascii_buffer(const char * buff, ssize_t len);


#endif /* end of header */
