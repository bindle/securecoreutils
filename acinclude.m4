#
#   Secure Core Utilities
#   Copyright (C) 2015 Bindle Binaries <syzdek@bindlebinaries.com>.
#
#   @BINDLE_BINARIES_BSD_LICENSE_START@
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are
#   met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of Bindle Binaries nor the
#        names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BINDLE BINARIES BE LIABLE FOR
#   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#   SUCH DAMAGE.
#
#   @BINDLE_BINARIES_BSD_LICENSE_END@
#
#   acinclude.m4 - custom m4 macros used by configure.ac
#

# AC_SCU_WIDGET_TAIL
# ______________________________________________________________________________
AC_DEFUN([AC_SCU_WIDGET_TAIL],[dnl

   withval=""
   AC_ARG_WITH(
      tail-timeout,
      [AS_HELP_STRING([--with-tail-timeout=seconds], [terminate tail after seconds [0]])],
      [ WTAIL_TIMEOUT=$withval ],
      [ WTAIL_TIMEOUT=$withval ]
   )

   if test "x${WTAIL_TIMEOUT}" == "xyes";then
      WTAIL_TIMEOUT=3600
   elif test "x${WTAIL_TIMEOUT}" == "xno";then
      WTAIL_TIMEOUT=0
   elif test "x${WTAIL_TIMEOUT}" == "x";then
      WTAIL_TIMEOUT=0
   else
      case $WTAIL_TIMEOUT in
      ''|*[!0-9]*)
      AC_MSG_ERROR([tail timeout must be a numeric value.])
      ;;

      *)
      ;;
      esac
   fi
   SCU_TAIL_TIMEOUT=${WTAIL_TIMEOUT}

   AC_DEFINE_UNQUOTED(SCU_TAIL_TIMEOUT, ${SCU_TAIL_TIMEOUT}, [Timeout value for tail widget])

])dnl


# AC_SCU_WIDGET_ZCAT
# ______________________________________________________________________________
AC_DEFUN([AC_SCU_WIDGET_ZCAT],[dnl

   enableval=""
   AC_ARG_ENABLE(
      bzip2,
      [AS_HELP_STRING([--disable-bzip2], [disable bzip2 support [auto]])],
      [ EBZIP2=$enableval ],
      [ EBZIP2=$enableval ]
   )
   enableval=""
   AC_ARG_ENABLE(
      lzma,
      [AS_HELP_STRING([--disable-lzma], [disable lzma support [auto]])],
      [ ELZMA=$enableval ],
      [ ELZMA=$enableval ]
   )
   enableval=""
   AC_ARG_ENABLE(
      zlib,
      [AS_HELP_STRING([--disable-zlib], [disable zlib support [auto]])],
      [ EZLIB=$enableval ],
      [ EZLIB=$enableval ]
   )
   withval=""
   AC_ARG_WITH(
      tail-timeout,
      [AS_HELP_STRING([--with-tail-timeout=seconds], [terminate tail after seconds [0]])],
      [ WTAIL_TIMEOUT=$withval ],
      [ WTAIL_TIMEOUT=$withval ]
   )

   # check zlib
   USE_ZLIB=no;
   if test "x${EZLIB}" != "xno";then
      USE_ZLIB=yes;
      AC_CHECK_HEADERS([zlib.h],      [], [USE_ZLIB=no])
      AC_SEARCH_LIBS([gzclose],  [z], [], [USE_ZLIB=no])
      AC_SEARCH_LIBS([gzerror],  [z], [], [USE_ZLIB=no])
      AC_SEARCH_LIBS([gzopen],   [z], [], [USE_ZLIB=no])
      AC_SEARCH_LIBS([gzread],   [z], [], [USE_ZLIB=no])
      if test "x${USE_ZLIB}" = "xyes";then
         AC_DEFINE_UNQUOTED(USE_ZLIB, 1, [Use zlib])
      elif test "x${EZLIB}" == "xyes";then
         AC_MSG_ERROR([unable to locate zlib library and headers])
      fi
   fi

   # check libbz2
   USE_BZIP2=no
   if test "x${EBZIP2}" != "xno";then
      USE_BZIP2=yes;
      AC_CHECK_HEADERS([bzlib.h],                   [], [USE_BZIP2=no])
      AC_SEARCH_LIBS([BZ2_bzRead],           [bz2], [], [USE_BZIP2=no])
      AC_SEARCH_LIBS([BZ2_bzReadClose],      [bz2], [], [USE_BZIP2=no])
      AC_SEARCH_LIBS([BZ2_bzReadGetUnused],  [bz2], [], [USE_BZIP2=no])
      AC_SEARCH_LIBS([BZ2_bzReadOpen],       [bz2], [], [USE_BZIP2=no])
      if test "x${USE_BZIP2}" = "xyes";then
         AC_DEFINE_UNQUOTED(USE_BZIP2, 1, [Use bzip2])
      elif test "x${EBZIP2}" == "xyes";then
         AC_MSG_ERROR([unable to locate bzip2 library and headers])
      fi
   fi

   # check liblzma
   USE_LZMA=no;
   if test "x${ELZMA}" != "xno";then
      USE_LZMA=yes;
      AC_CHECK_HEADERS([lzma.h],                     [], [USE_LZMA=no])
      AC_SEARCH_LIBS([lzma_stream_decoder],  [lzma], [], [USE_LZMA=no])
      AC_SEARCH_LIBS([lzma_code],            [lzma], [], [USE_LZMA=no])
      if test "x${USE_LZMA}" = "xyes";then
         AC_DEFINE_UNQUOTED(USE_LZMA, 1, [Use liblzma])
      elif test "x${ELZMA}" == "xyes";then
         AC_MSG_ERROR([unable to locate LZMA library and headers])
      fi
   fi

])dnl


# end of m4 file
