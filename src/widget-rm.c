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
#include "widget-rm.h"

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
#include <unistd.h>


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Prototypes
#endif

void scu_widget_rm_usage(scu_config * cnf);


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Functions
#endif

int scu_widget_rm(scu_config * cnf)
{
   int            force;
   int            prompt;
   int            c;
   int            opt_index;
   int            rc;
   int            readonly;
   ssize_t        len;
   char           anwser[16];
   uid_t          uid;
   struct stat    sb;

   // getopt options
   static char   short_opt[] = "+fhiqVv";
   static struct option long_opt[] =
   {
      {"help",             no_argument,       NULL, 'h' },
      {"quiet",            no_argument,       NULL, 'q' },
      {"silent",           no_argument,       NULL, 'q' },
      {"version",          no_argument,       NULL, 'V' },
      {"verbose",          no_argument,       NULL, 'v' },
      {"force",            no_argument,       NULL, 'f' },
      { NULL, 0, NULL, 0 }
   };

   assert(cnf != NULL);
   cnf->short_opt = short_opt;

   force    = 0;
   prompt   = 0;
   readonly = 0;
   uid      = getuid();

   while((c = getopt_long(cnf->argc, cnf->argv, short_opt, long_opt, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:	/* no more arguments */
         case 0:	/* long options toggles */
         break;

         case 'f':
         force  = 1;
         prompt = 0;
         break;

         case 'h':
         scu_widget_rm_usage(cnf);
         return(0);

         case 'i':
         prompt = 1;
         force  = 0;
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

   // checks file for restriction validations
   if ((rc = stat(cnf->argv[optind], &sb)) == -1)
   {
      if (errno == ENOENT)
         return(0);
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
      return(1);
   };
   if ((rc = scu_pathcheck(cnf->argv[optind], 0)) != 0)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, scu_strerror(rc));
      return(1);
   };
   if (!(S_ISREG(sb.st_mode)))
   {
      fprintf(stderr, "%s: %s: not a regular file\n", PROGRAM_NAME, cnf->widget->name);
      return(1);
   };

   // determine if file is read-only
   if ( (!(sb.st_mode & S_IWUSR)) && (uid != 0) )
      readonly = 1;
   if ( (sb.st_uid != uid) && (uid != 0) )
      readonly = 1;

   // prompt for confirmation
   if (force != 1)
   {
      if (readonly == 1)
         printf("%s: %s: remove read-only file %s? ", PROGRAM_NAME, cnf->widget->name, cnf->argv[optind]);
      else if (prompt == 1)
         printf("%s: %s: remove file %s? ", PROGRAM_NAME, cnf->widget->name, cnf->argv[optind]);
      if ( (prompt == 1) || (readonly == 1) )
      {
         fflush(stdout);
         if ((len = read(STDIN_FILENO, anwser, sizeof(anwser))) == -1)
         {
            fprintf(stderr, "%s: %s:%s\n", PROGRAM_NAME, cnf->widget->name, strerror(errno));
            return(1);
         };
         if ((anwser[0] != 'y') && (anwser[0] != 'Y'))
            return(0);
      };
   };

   if ((cnf->verbose))
      printf("removing %s\n", cnf->argv[optind]);

   if ((rc = unlink(cnf->argv[optind])) == -1)
   {
      fprintf(stderr, "%s: %s: %s: %s\n", PROGRAM_NAME, cnf->widget->name,
              cnf->argv[optind], strerror(errno));
      return(1);
   };

   return(0);
}


void scu_widget_rm_usage(scu_config * cnf)
{
   scu_usage_summary(cnf, " [OPTIONS] file");
   printf("\n");
   scu_usage_options(cnf);
   printf("  -i                        prompt for confirmation\n");
   printf("  -f, --force               ignore nonexistent files\n");
   printf("\n");
   scu_usage_restrictions();
   printf("\n");
   return;
}


/* end of source */
