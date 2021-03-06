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
#include "widget-pathcheck.h"

///////////////
//           //
//  Headers  //
//           //
///////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Headers
#endif

#include <assert.h>
#include <stdio.h>


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Prototypes
#endif

void scu_widget_pathcheck_usage(scu_config * cnf);


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Functions
#endif

int scu_widget_pathcheck(scu_config * cnf)
{
   int            c;
   int            opt_index;
   int            rc;
   int            opts;

   // getopt options
   static char   short_opt[] = "+dihqVv";
   static struct option long_opt[] =
   {
      {"help",             no_argument,       NULL, 'h' },
      {"ignore",           no_argument,       NULL, 'i' },
      {"quiet",            no_argument,       NULL, 'q' },
      {"silent",           no_argument,       NULL, 'q' },
      {"version",          no_argument,       NULL, 'V' },
      {"verbose",          no_argument,       NULL, 'v' },
      {"directory",        no_argument,       NULL, 'd' },
      { NULL, 0, NULL, 0 }
   };

   assert(cnf != NULL);
   cnf->short_opt = short_opt;

   opts = 0;

   while((c = getopt_long(cnf->argc, cnf->argv, short_opt, long_opt, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:	/* no more arguments */
         case 0:	/* long options toggles */
         break;

         case 'd':
         opts |= SCU_ODIR;
         break;

         case 'i':
         opts |= SCU_ONOTEXISTS;
         break;

         case 'h':
         scu_widget_pathcheck_usage(cnf);
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

   if ((rc = scu_pathcheck(cnf->argv[optind], opts)) != 0)
   {
      if (!(cnf->quiet))
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, cnf->widget->name, scu_strerror(rc));
      return(1);
   };

   return(0);
}


void scu_widget_pathcheck_usage(scu_config * cnf)
{
   scu_usage_summary(cnf, " [OPTIONS] file");
   printf("\n");
   scu_usage_options(cnf);
   printf("  -d, --directory           check path for directory instead of file\n");
   printf("  -i, --ignore              ignore non-existent files and directories\n");
   printf("\n");
   scu_usage_restrictions();
   printf("\n");
   return;
}

/* end of source */
