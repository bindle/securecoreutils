/*
 *  Secure Core Utilities
 *  Copyright (C) 2015, 2017 David M. Syzdek <david@syzdek.net>.
 *
 *  @SYZDEK_BSD_LICENSE_START@
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
 *     * Neither the name of David M. Syzdek nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVID M SYZDEK BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *  @SYZDEK_BSD_LICENSE_END@
 */
#include "widget-touch.h"

///////////////
//           //
//  Headers  //
//           //
///////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Headers
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Prototypes
#endif

void scu_widget_touch_usage(scu_config * cnf);
time_t scu_widget_touch_strtime(char *restrict s, time_t t);


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Functions
#endif

int scu_widget_touch(scu_config * cnf)
{
   int            c;
   int            opt_index;
   int            rc;
   int            fd;
   int            opts;
   int            oflags;
   int            sflags;
   time_t         t;
   char         * reffile;
   struct stat    sb;
   struct stat    refsb;
   struct timeval curtime[2];
   struct timeval settime[2];

   // getopt options
   static char   short_opt[] = "+cfhqr:t:Vv";
   static struct option long_opt[] =
   {
      {"no-create",        no_argument,       NULL, 'c' },
      {"help",             no_argument,       NULL, 'h' },
      {"quiet",            no_argument,       NULL, 'q' },
      {"reference",        no_argument,       NULL, 'r' },
      {"silent",           no_argument,       NULL, 'q' },
      {"version",          no_argument,       NULL, 'V' },
      {"verbose",          no_argument,       NULL, 'v' },
      { NULL, 0, NULL, 0 }
   };

   assert(cnf != NULL);
   cnf->short_opt = short_opt;

   opts                = -1;
   oflags              = O_CREAT;
   sflags              = SCU_ONOTEXISTS;
   reffile             = NULL;
   t                   = time(NULL);
   curtime[0].tv_sec   = curtime[1].tv_sec  = t;
   curtime[0].tv_usec  = curtime[1].tv_usec = 0;
   settime[0].tv_sec   = settime[1].tv_sec  = t;
   settime[0].tv_usec  = settime[1].tv_usec = 0;

   while((c = getopt_long(cnf->argc, cnf->argv, short_opt, long_opt, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:	/* no more arguments */
         case 0:	/* long options toggles */
         break;

         case 'a':
         if (opts != 2)
         {
            fprintf(stderr, "%s: %s: incompatible options\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         opts = 0;
         break;

         case 'c':
         oflags ^= O_CREAT;
         break;

         case 'f':
         break;

         case 'h':
         scu_widget_touch_usage(cnf);
         return(0);

         case 'm':
         if (opts != 2)
         {
            fprintf(stderr, "%s: %s: incompatible options\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         opts = 1;
         break;

         case 'r':
         if (settime[0].tv_sec != t)
         {
            fprintf(stderr, "%s: %s: incompatible options\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         reffile = optarg;
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

         case 't':
         if ((reffile))
         {
            fprintf(stderr, "%s: %s: incompatible options\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         if ((settime[1].tv_sec = scu_widget_touch_strtime(optarg, t)) == 0)
         {
            fprintf(stderr, "%s: %s: invalid time specification\n", PROGRAM_NAME, cnf->widget->name);
            fprintf(stderr, "Try `%s %s --help' for more information.\n", PROGRAM_NAME, cnf->widget->name);
            return(1);
         };
         settime[0].tv_sec = settime[1].tv_sec;
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
   if ((rc = scu_pathcheck(cnf->argv[optind], sflags)) != 0)
   {
      if ((rc = scu_pathcheck(cnf->argv[optind], sflags | SCU_ODIR)) != 0)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, scu_strerror(rc));
         return(1);
      };
   };


   // obtains current atime/time or creates file if needed
   if ((rc = stat(cnf->argv[optind], &sb)) != -1)
   {
      curtime[0].tv_sec  = sb.st_atime;
      curtime[1].tv_sec  = sb.st_mtime;
   }
   else if ((oflags & O_CREAT) == O_CREAT)
   {
      if ((fd = open(cnf->argv[optind], oflags)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         return(1);
      };
      close(fd);
   }
   else
      return(0);
   

   // obtains time stamp from reference file
   if (reffile != NULL)
   {
      if ((rc = scu_pathcheck(reffile, 0)) != 0)
      {
         fprintf(stderr, "%s: %s: reference file: %s\n", PROGRAM_NAME, cnf->widget->name, scu_strerror(rc));
         return(1);
      };
      if ((rc = stat(reffile, &refsb)) == -1)
      {
         fprintf(stderr, "%s: %s: reference file: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
         return(1);
      };
      settime[0].tv_sec  = refsb.st_atime;
      settime[1].tv_sec  = refsb.st_mtime;
   };


   // determines whether to update atime, mtime, or both
   if ( (opts == 0) || (opts == -1) )
   {
      curtime[0].tv_sec = settime[0].tv_sec;
   };
   if ( (opts == 1) || (opts == -1) )
   {
      curtime[1].tv_sec = settime[1].tv_sec;
   };


   // updates time stamps
   if ((rc = utimes(cnf->argv[optind], curtime)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      return(1);
   };

   return(0);
}


time_t scu_widget_touch_strtime(char *restrict s, time_t t)
{

   size_t         len;
   char         * ptr;
   struct tm      tm_time;

   ptr = NULL;
   len = strlen(s);

   localtime_r(&t, &tm_time);

   // CCYYMMDDhhmm.SS
   if (len == 15)
   {
      if ((ptr = strptime(s, "%Y%m%d%H%M.%S\0", &tm_time)) != NULL)
         t = mktime(&tm_time);
   };

   // YYMMDDhhmm.SS
   if (len == 13)
   {
      localtime_r(&t, &tm_time);
      if ((ptr = strptime(s, "%y%m%d%H%M.%S\0", &tm_time)) != NULL)
         t = mktime(&tm_time);
   };

   // MMDDhhmm.SS
   if (len == 11)
   {
      localtime_r(&t, &tm_time);
      if ((ptr = strptime(s, "%m%d%H%M.%S\0", &tm_time)) != NULL)
         t = mktime(&tm_time);
   };

   // CCYYMMDDhhmm
   if (len == 12)
   {
      localtime_r(&t, &tm_time);
      if ((ptr = strptime(s, "%Y%m%d%H%M\0", &tm_time)) != NULL)
         t = mktime(&tm_time);
   };

   // YYMMDDhhmm
   if (len == 10)
   {
      localtime_r(&t, &tm_time);
      if ((ptr = strptime(s, "%y%m%d%H%M\0", &tm_time)) != NULL)
         t = mktime(&tm_time);
   };

   // MMDDhhmm
   if (len == 8)
   {
      localtime_r(&t, &tm_time);
      if ((ptr = strptime(s, "%m%d%H%M\0", &tm_time)) != NULL)
         t = mktime(&tm_time);
   };

   // return invalid if string contains more than a date
   if ( (ptr == NULL) || (ptr[0] != '\0') )
      return(0);

   return(t);
}


void scu_widget_touch_usage(scu_config * cnf)
{
   scu_usage_summary(cnf, " [OPTIONS] file");
   printf("\n");
   scu_usage_options(cnf);
   printf("  -a                        change only access time\n");
   printf("  -c, --no-create           do not create files\n");
   printf("  -f                        ignored (for compatibility with GNU coreutils)\n");
   printf("  -m                        change only modification time\n");
   printf("  -r, --reference=FILE      use file's timestamp instead of current time\n");
   printf("  -t STAMP                  use time STAMP with format of [[CC]YY]MMDDhhmm[.ss]\n");
   printf("\n");
   scu_usage_restrictions();
   printf("\n");
   return;
}


/* end of source */
