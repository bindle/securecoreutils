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
#include "widget-zcat.h"

///////////////
//           //
//  Headers  //
//           //
///////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Headers
#endif

#include <assert.h>
#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_BZIP2
#include <bzlib.h>
#endif

#ifdef USE_XZ
#include <lzma.h>
#endif


/////////////////
//             //
//  Variables  //
//             //
/////////////////

const uint8_t scm_magic_bz2[3]   = { 0x42, 0x5a, 0x68 };                   // tar.bz2
const uint8_t scm_magic_gz[2]    = { 0x1f, 0x8b };                         // tar.gz
const uint8_t scm_magic_lz4[4]   = { 0x04, 0x22, 0x4d, 0x18 };             // tar.lz4
const uint8_t scm_magic_z_lzw[2] = { 0x1f, 0x9d };                         // tar.Z
const uint8_t scm_magic_z_lzh[2] = { 0x1f, 0xa0 };                         // tar.Z
const uint8_t scm_magic_xz[6]    = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00 }; // tar.xz


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#ifdef __TALLYMARK_PMARK
#pragma mark -
#endif

#ifdef USE_XZ
int scu_widget_zcat_xz_perror(scu_config * cnf, lzma_ret ret);
#endif


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Functions
#endif

int scu_widget_zcat(scu_config * cnf)
{
   int            fd;
   int            c;
   int            opt_index;
   int            rc;
   int            type;
   uint8_t        magic[16];
   ssize_t        len;

   // getopt options
   static char   short_opt[] = "+hqVv";
   static struct option long_opt[] =
   {
      {"help",             no_argument,       NULL, 'h' },
      {"quiet",            no_argument,       NULL, 'q' },
      {"silent",           no_argument,       NULL, 'q' },
      {"version",          no_argument,       NULL, 'V' },
      {"verbose",          no_argument,       NULL, 'v' },
      { NULL, 0, NULL, 0 }
   };

   assert(cnf != NULL);

   type = 0;
   bzero(magic, sizeof(magic));

   while((c = getopt_long(cnf->argc, cnf->argv, short_opt, long_opt, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:	/* no more arguments */
         case 0:	/* long options toggles */
         break;

         case 'h':
         scu_widget_zcat_usage(cnf);
         return(0);

         case 's':
         cnf->quiet = 1;
         if ((cnf->verbose))
         {
            fprintf(stderr, "%s: %s: incompatible options\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         break;

         case 'V':
         printf("%s widget\n", cnf->widget->name);
         scu_version();
         return(0);

         case 'v':
         cnf->verbose++;
         if ((cnf->quiet))
         {
            fprintf(stderr, "%s: %s: incompatible options\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s %s --help' for more information.\n", PROGRAM_NAME, cnf->widget->name);
            return(1);
         };
         break;

         case '?':
         fprintf(stderr, "Try `%s %s --help' for more information.\n", PROGRAM_NAME, cnf->widget->name);
         return(1);

         default:
         fprintf(stderr, "%s: %s: unrecognized option `--%c'\n", PROGRAM_NAME, cnf->widget->name, c);
         fprintf(stderr, "Try `%s %s --help' for more information.\n", PROGRAM_NAME, cnf->widget->name);
         return(1);
      };
   };

   if ((cnf->argc - optind) < 1)
   {
      fprintf(stderr, "%s: %s: missing required argument\n", PROGRAM_NAME, cnf->widget->name);
      fprintf(stderr, "Try `%s %s --help' for more information.\n", PROGRAM_NAME, cnf->widget->name);
      return(1);
   };
   if ((cnf->argc - optind) > 1)
   {
      fprintf(stderr, "%s: unrecognized argument `-- %s'\n", PROGRAM_NAME, cnf->argv[optind+1]);
      fprintf(stderr, "Try `%s %s --help' for more information.\n", PROGRAM_NAME, cnf->widget->name);
      return(1);
   };

   // checks file for restriction validations
   if ((rc = scu_pathcheck(cnf->argv[optind], 0)) != 0)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, scu_strerror(rc));
      return(1);
   };

   // read magic number
   if ((fd = open(cnf->argv[optind], O_RDONLY)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      return(1);
   };
   if ((len = read(fd, magic, sizeof(magic))) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      close(fd);
      return(1);
   };
   close(fd);


   // processes gzip file
#ifdef USE_ZLIB
   if (!( memcmp(magic, scm_magic_gz, sizeof(scm_magic_gz)) ))
      return(scu_widget_zcat_gz(cnf));
#endif

#ifdef USE_BZIP2
   if (!( memcmp(magic, scm_magic_bz2, sizeof(scm_magic_bz2)) ))
      return(scu_widget_zcat_bz2(cnf));
#endif

#ifdef USE_XZ
   if (!( memcmp(magic, scm_magic_xz, sizeof(scm_magic_xz)) ))
      return(scu_widget_zcat_xz(cnf));
#endif

#ifdef USE_Z
   if (!( memcmp(magic, scm_magic_z_lzw, sizeof(scm_magic_z_lzw)) ))
      return(scu_widget_zcat_prog(cnf, "/usr/bin/uncompress -c"));
   if (!( memcmp(magic, scm_magic_z_lzh, sizeof(scm_magic_z_lzh)) ))
      return(scu_widget_zcat_prog(cnf, "/usr/bin/uncompress -c"));
#endif

   fprintf(stderr, "%s: %s: unable to determine compression algorithms\n", PROGRAM_NAME, cnf->widget->name);
   return(1);
}


#ifdef USE_BZIP2
int scu_widget_zcat_bz2(scu_config * cnf)
{
   int         bzerror;
   char        buff[16384];
   ssize_t     len;
   FILE      * fs;
   BZFILE    * bz2;

   if ((fs = fopen(cnf->argv[optind], "r")) == NULL)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      return(1);
   };

   bz2 = BZ2_bzReadOpen(&bzerror, fs, 0, 1, NULL, 0);
   if (bzerror != BZ_OK)
   {
      fprintf(stderr, "%s: %s: error with decompression\n", PROGRAM_NAME, cnf->widget->name);
      BZ2_bzReadClose(&bzerror, bz2);
      fclose(fs);
      return(1);
   };

   while ((len = BZ2_bzRead(&bzerror, bz2, buff, sizeof(buff))) > 0)
   {
      if ((len = write(STDOUT_FILENO, buff, len)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         BZ2_bzReadClose(&bzerror, bz2);
         fclose(fs);
         return(1);
      };
   };

   BZ2_bzReadClose(&bzerror, bz2);
   fclose(fs);

   return(0);
}
#endif


#ifdef USE_ZLIB
int scu_widget_zcat_gz(scu_config * cnf)
{
   gzFile   gz;
   char     buff[4096];
   int      gzlen;
   ssize_t  len;

   errno = 0;

   if ((gz = gzopen(cnf->argv[optind], "rb")) == NULL)
   {
      if (errno == 0)
      {
         fprintf(stderr, "%s: %s: insufficient memory\n", PROGRAM_NAME, cnf->widget->name);
         return(1);
      };
   };

   while((gzlen = gzread(gz, buff, sizeof(buff))) > 0)
   {
      if ((len = write(STDOUT_FILENO, buff, gzlen)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         gzclose(gz);
         return(1);
      };
   };
   if (gzlen == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, gzerror(gz, NULL));
      gzclose(gz);
      return(1);
   };

   gzclose(gz);

   return(0);
}
#endif


int scu_widget_zcat_prog(scu_config * cnf, const char * prog)
{
   char buff[2048];

   bzero(buff, sizeof(buff));
   snprintf(buff, sizeof(buff), "%s %s", prog, cnf->argv[optind]);

   return(system(buff));
}


void scu_widget_zcat_usage(scu_config * cnf)
{
   scu_usage_summary(cnf, " [OPTIONS] file");
   printf("\n");
   scu_usage_options();
   printf("\n");
   scu_usage_restrictions();
   printf("\n");
   return;
}


#ifdef USE_XZ
int scu_widget_zcat_xz(scu_config * cnf)
{
   int            fd;
   lzma_ret       ret;
   lzma_stream    strm;
   lzma_action    action;
   uint8_t        in[16384];
   uint8_t        out[16384];
   size_t         len;

   bzero(&strm, sizeof(strm));
   action         = LZMA_RUN;
   strm.next_in   = in;
   strm.next_out  = out;
   strm.avail_in  = 0;
   strm.avail_out = sizeof(out);

   if ((fd = open(cnf->argv[optind], O_RDONLY)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", cnf->prog_name, cnf->widget->name, strerror(errno));
      return(1);
   };

   if ((ret = lzma_stream_decoder(&strm, UINT64_MAX, LZMA_CONCATENATED)) != LZMA_OK)
   {
      scu_widget_zcat_xz_perror(cnf, ret);
      close(fd);
      return(1);
   };


   while (action == LZMA_RUN)
   {
      // read from compressed file
      strm.next_in = in;
      if (strm.avail_in != sizeof(in))
      {
         if ((len = read(fd, &in[strm.avail_in], (sizeof(in) - strm.avail_in))) == -1)
         {
            fprintf(stderr, "%s: %s: %s\n", cnf->prog_name, cnf->widget->name, strerror(errno));
            close(fd);
            return(1);
         };
         if (len == 0)
            action = LZMA_FINISH;
         else
            strm.avail_in += len;
      };

      // uncompress file
      if ((ret = lzma_code(&strm, action)) != LZMA_OK)
      {
         if (ret != LZMA_STREAM_END)
         {
            scu_widget_zcat_xz_perror(cnf, ret);
            close(fd);
            return(1);
         };
      };

      // print uncompressed data
      if (strm.avail_out == sizeof(out))
         continue;
      if ((len = write(STDOUT_FILENO, out, (sizeof(out) - strm.avail_out))) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", cnf->prog_name, cnf->widget->name, strerror(errno));
         close(fd);
         return(1);
      };
      strm.avail_out += len;
   };

   close(fd);

   // finish uncompressing data
   while ( (ret != LZMA_OK) && (ret != LZMA_STREAM_END))
   {
      // uncompress remaining data in buffer
      if ((ret = lzma_code(&strm, action)) != LZMA_OK)
      {
         if (ret != LZMA_STREAM_END)
         {
            scu_widget_zcat_xz_perror(cnf, ret);
            return(1);
         };
      };

      // print uncompressed data
      if (strm.avail_out == sizeof(out))
         continue;
      if ((len = write(STDOUT_FILENO, out, (sizeof(out) - strm.avail_out))) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", cnf->prog_name, cnf->widget->name, strerror(errno));
         close(fd);
         return(1);
      };
      strm.avail_out += len;
   };

   return(0);
}


int scu_widget_zcat_xz_perror(scu_config * cnf, lzma_ret ret)
{
   switch(ret)
   {
      case LZMA_BUF_ERROR:
      fprintf(stderr, "%s: %s: truncated or corrupt file\n", cnf->prog_name, cnf->widget->name);
      return(1);

      case LZMA_DATA_ERROR:
      fprintf(stderr, "%s: %s: compressed file is corrupt\n", cnf->prog_name, cnf->widget->name);
      return(1);

      case LZMA_FORMAT_ERROR:
      fprintf(stderr, "%s: %s: invalid .xz format\n", cnf->prog_name, cnf->widget->name);
      return(1);

      case LZMA_MEM_ERROR:
      fprintf(stderr, "%s: %s: out of virtual memory\n", cnf->prog_name, cnf->widget->name);
      return(1);

      case LZMA_OPTIONS_ERROR:
      fprintf(stderr, "%s: %s: unsupported LZMA options\n", cnf->prog_name, cnf->widget->name);
      return(1);

      case LZMA_STREAM_END:
      fprintf(stderr, "%s: %s: end of LZMA stream\n", cnf->prog_name, cnf->widget->name);
      return(0);

      default:
      fprintf(stderr, "%s: %s: unknown LZMA error\n", cnf->prog_name, cnf->widget->name);
      break;

   };
   return(1);
}
#endif

/* end of source */
