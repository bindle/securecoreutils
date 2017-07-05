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
#include "widget-tail.h"

///////////////
//           //
//  Headers  //
//           //
///////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Headers
#endif

#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


///////////////////
//               //
//  Definitions  //
//               //
///////////////////

#define SCU_TAIL_OBOL      0x01
#define SCU_TAIL_OBYTES    0x02
#define SCU_TAIL_OFOLLOW   0x04


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////

int scu_widget_tail_bytes(scu_config * cnf, int fd, struct stat * sb,
   size_t opts, off_t optnum);
int scu_widget_tail_follow(scu_config * cnf, int fd);
int scu_widget_tail_lines(scu_config * cnf, int fd, struct stat * sb,
   size_t opts, off_t optnum);


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Functions
#endif

int scu_widget_tail(scu_config * cnf)
{
   int            c;
   int            rc;
   int            opt_index;
   int            fd;
   off_t          optnum;
   char         * endptr;
   size_t         opts;
   struct stat    sb;

   // getopt options
   static char   short_opt[] = "+c:fhn:qVv";
   static struct option long_opt[] =
   {
      {"bytes",            no_argument,       NULL, 'c' },
      {"follow",           no_argument,       NULL, 'f' },
      {"help",             no_argument,       NULL, 'h' },
      {"lines",            no_argument,       NULL, 'n' },
      {"quiet",            no_argument,       NULL, 'q' },
      {"silent",           no_argument,       NULL, 'q' },
      {"version",          no_argument,       NULL, 'V' },
      {"verbose",          no_argument,       NULL, 'v' },
      { NULL, 0, NULL, 0 }
   };

   assert(cnf != NULL);

   opts     = 0;
   optnum   = 10;

   while((c = getopt_long(cnf->argc, cnf->argv, short_opt, long_opt, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:	/* no more arguments */
         case 0:	/* long options toggles */
         break;


         case 'h':
         scu_widget_tail_usage();
         return(0);


         case 'c':
         opts |= SCU_TAIL_OBYTES;
         switch(optarg[0])
         {
            case '-':
            optnum = strtoimax(&optarg[1], &endptr, 10);
            break;

            case '+':
            opts |= SCU_TAIL_OBOL;
            optnum = strtoimax(&optarg[1], &endptr, 10);
            break;

            default:
            optnum = strtoimax(optarg, &endptr, 10);
            break;
         };
         if ((endptr == optarg) || (endptr[0] != '\0'))
         {
            fprintf(stderr, "%s: %s: invalid value for `-c' -- %s\n", PROGRAM_NAME, cnf->widget->name, optarg);
            return(1);
         };
         break;


         case 'f':
         opts |= SCU_TAIL_OFOLLOW;
         break;


         case 'n':
         opts = (~SCU_TAIL_OBYTES) & opts;
         switch(optarg[0])
         {
            case '-':
            optnum = strtoimax(&optarg[1], &endptr, 10);
            break;

            case '+':
            opts |= SCU_TAIL_OBOL;
            optnum = strtoimax(&optarg[1], &endptr, 10);
            break;

            default:
            optnum = strtoimax(optarg, &endptr, 10);
            break;
         };
         if ((endptr == optarg) || (endptr[0] != '\0'))
         {
            fprintf(stderr, "%s: %s: invalid value for `-n' -- %s\n", PROGRAM_NAME, cnf->widget->name, optarg);
            return(1);
         };
         break;


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

   if ((rc = scu_pathcheck(cnf->argv[optind])) != 0)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, scu_strerror(rc));
      return(1);
   };

   if ((fd = open(cnf->argv[optind], O_RDONLY)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      return(1);
   };

   if ((rc = fstat(fd, &sb)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      close(fd);
      return(1);
   };

   if ((opts & SCU_TAIL_OBYTES) != 0)
   {
      if ((rc = scu_widget_tail_bytes(cnf, fd, &sb, opts, optnum)) == -1)
      {
         close(fd);
         return(1);
      };
   };

   if ((opts & SCU_TAIL_OBYTES) == 0)
   {
      if ((rc = scu_widget_tail_lines(cnf, fd, &sb, opts, optnum)) == -1)
      {
         close(fd);
         return(1);
      };
   };

   if ((opts & SCU_TAIL_OFOLLOW) != 0)
   {
      if (!(scu_widget_tail_follow(cnf, fd)))
      {
         close(fd);
         return(1);
      };
   };

   close(fd);

   return(0);
}


int scu_widget_tail_bytes(scu_config * cnf, int fd, struct stat * sb,
   size_t opts, off_t optnum)
{
   off_t    rc;
   off_t    off;
   char     buff[SCU_BUFF_MAX];
   ssize_t  len;

   if ((opts & SCU_TAIL_OBOL) == 0)
      off = (sb->st_size < optnum) ? 0 : sb->st_size - optnum;
   else
      off = (sb->st_size < optnum) ? sb->st_size : optnum;

   if ((rc = lseek(fd, off, SEEK_SET)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      return(-1);
   };

   len = 1;
   while ((len = read(fd, buff, sizeof(buff))) > 0)
   {
      if ((len = write(STDOUT_FILENO, buff, len)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         return(-1);
      };
   };

   return(0);
}


int scu_widget_tail_follow(scu_config * cnf, int fd)
{
   char              buff[SCU_BUFF_MAX];
   ssize_t           len;
   struct timespec   ts;

   ts.tv_sec  = 0;
   ts.tv_nsec = 10000000;

   while (1)
   {
      switch (len = read(fd, buff, sizeof(buff)))
      {
         case -1:
         fprintf(stderr, "%s: %s: read: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         return(-1);

         case 0:
         nanosleep(&ts, NULL);
         break;

         default:
         if ((len = write(STDOUT_FILENO, buff, len)) == -1)
         {
            fprintf(stderr, "%s: %s: write: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
            return(-1);
         };
         break;
      };
   };

   return(0);
}


int scu_widget_tail_lines(scu_config * cnf, int fd, struct stat * sb,
   size_t opts, off_t optnum)
{
   char     buff[SCU_BUFF_MAX];
   off_t    rc;
   ssize_t  bufflen;
   ssize_t  len;
   off_t    linecount;
   off_t    seek;
   ssize_t  pos;
   off_t    found;

   // start line count from beginning of file
   if ((opts & SCU_TAIL_OBOL) != 0)
   {
      len         = 1;
      linecount   = 1;
      pos         = 0;

      while ((linecount < optnum) && (len > 0))
      {
         if ((len = read(fd, buff, sizeof(buff))) == -1)
         {
            fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
            return(-1);
         };

         for (pos = 0; ((pos < len) && (linecount < optnum)); pos++)
         {
            if (buff[pos] == '\n')
               linecount++;
            if (linecount >= optnum)
            {
               if ((len = write(STDIN_FILENO, &buff[pos+1], (len-pos-1))) == -1)
               {
                  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
                  return(-1);
               };
            };
         };
      };
   };

   // start line count from end of file
   if ((opts & SCU_TAIL_OBOL) == 0)
   {
      linecount = 0;
      seek      = sb->st_size;
      found     = 0;

      while ((linecount <= optnum) && (seek > 0))
      {
         if (seek > (off_t)sizeof(buff))
         {
            bufflen  = sizeof(buff);
            seek    -= bufflen;
         } else {
            bufflen  = seek;
            seek     = 0;
         };

         if ((rc = lseek(fd, seek, SEEK_SET)) == -1)
         {
            fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
            return(-1);
         };

         if ((len = read(fd, buff, bufflen)) == -1)
         {
            fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
            return(-1);
         };

         for (pos = len; ((pos > 0) && (linecount <= optnum)); pos--)
         {
            if (buff[pos-1] == '\n')
               linecount++;
            if (linecount > optnum)
               found = seek + pos;
         };
      };

      if ((rc = lseek(fd, found, SEEK_SET)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         return(-1);
      };
   };

   len = 1;
   while ((len = read(fd, buff, sizeof(buff))) > 0)
   {
      if ((len = write(STDOUT_FILENO, buff, len)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         return(-1);
      };
   };

   return(0);
}


void scu_widget_tail_usage(void)
{
   printf("Usage: %s tail [OPTIONS] file\n", PROGRAM_NAME);
   printf("       tail [OPTIONS] file\n");
   printf("       sectail [OPTIONS] file\n");
   printf("\n");
   scu_usage_options();
   printf("  -c, --bytes=X             output the last X bytes\n");
   printf("  -f, --follow              output appended data as the file grows\n");
   printf("  -n, --lines=X             output the last X lines\n");
   printf("\n");
   scu_usage_restrictions();
   printf("\n");
   return;
}


/* end of source */
