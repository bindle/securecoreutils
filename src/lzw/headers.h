/*
 * Written by Mike Frysinger <vapier@gmail.com>
 * Released into the public domain
 */

#ifndef _LIBLZW_HEADERS_H
#define _LIBLZW_HEADERS_H

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

# include <stdint.h>
# include <inttypes.h>
# include <sys/types.h>
# include <sys/stat.h>
#if defined(HAVE_CTYPE_H)
# include <ctype.h>
#endif
# include <errno.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <string.h>
#if defined(HAVE_FEATURES_H)
# include <features.h>
#endif
# include <stdarg.h>
# include <assert.h>

/* i'll see you in hell windows */
#if !defined(O_BINARY)
# define O_BINARY 0x00
#endif

#endif /* _LIBLZW_HEADERS_H */
