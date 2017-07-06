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

# AC_OPENVPN_LDAPCMD_COMPONENTS
# ______________________________________________________________________________
AC_DEFUN([AC_SCU_ZCAT_DEPS],[dnl

   # check zlib
   USE_ZLIB=yes;
   AC_CHECK_HEADERS([zlib.h],      [], [USE_ZLIB=no])
   AC_SEARCH_LIBS([gzclose],  [z], [], [USE_ZLIB=no])
   AC_SEARCH_LIBS([gzerror],  [z], [], [USE_ZLIB=no])
   AC_SEARCH_LIBS([gzopen],   [z], [], [USE_ZLIB=no])
   AC_SEARCH_LIBS([gzread],   [z], [], [USE_ZLIB=no])
   if test "x${USE_ZLIB}" = "xyes";then
      AC_DEFINE_UNQUOTED(USE_ZLIB, 1, [Use zlib])
   fi

   # check libbz2
   USE_BZIP2=yes;
   AC_CHECK_HEADERS([bzlib.h],                   [], [USE_BZIP2=no])
   AC_SEARCH_LIBS([BZ2_bzRead],           [bz2], [], [USE_BZIP2=no])
   AC_SEARCH_LIBS([BZ2_bzReadClose],      [bz2], [], [USE_BZIP2=no])
   AC_SEARCH_LIBS([BZ2_bzReadGetUnused],  [bz2], [], [USE_BZIP2=no])
   AC_SEARCH_LIBS([BZ2_bzReadOpen],       [bz2], [], [USE_BZIP2=no])
   if test "x${USE_BZIP2}" = "xyes";then
      AC_DEFINE_UNQUOTED(USE_BZIP2, 1, [Use bzip2])
   fi

   # check liblzma
   USE_XZ=yes;
   AC_CHECK_HEADERS([lzma.h],                     [], [USE_XZ=no])
   AC_SEARCH_LIBS([lzma_stream_decoder],  [lzma], [], [USE_XZ=no])
   AC_SEARCH_LIBS([lzma_code],            [lzma], [], [USE_XZ=no])
   if test "x${USE_XZ}" = "xyes";then
      AC_DEFINE_UNQUOTED(USE_XZ, 1, [Use liblzma])
   fi

   AC_CHECK_PROG([USE_Z], [uncompress], [yes], [no])
   if test "x${USE_Z}" = "xyes";then
      AC_DEFINE_UNQUOTED(USE_Z, 1, [Use uncompress])
   fi

])dnl


# end of m4 file
