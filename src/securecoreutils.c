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

///////////////
//           //
//  Headers  //
//           //
///////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Headers
#endif

#include "securecoreutils.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "widget-cat.h"
#include "widget-pathcheck.h"
#include "widget-rm.h"
#include "widget-tail.h"


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Prototypes
#endif

int main(int argc, char * argv[]);
const char * scu_basename(const char * path);
const scu_widget * scu_widget_lookup(const char * wname, int exact);


/////////////////
//             //
//  Variables  //
//             //
/////////////////
#ifdef __TALLYMARK_PMARK
#pragma mark - Variables
#endif

const scu_widget scu_widget_map[] =
{
   {
      "cat",                                          // widget name
      "Writes contents of file to standard out.",     // widget description
      "seccat",                                       // widget alias
      scu_widget_cat,                                 // widget function
   },
   {
      "pathcheck",                                    // widget name
      "Validates path using internal checks.",        // widget description
      "secpath",                                      // widget alias
      scu_widget_pathcheck,                           // widget function
   },
   {
      "rm",                                           // widget name
      "Removes a file.",                              // widget description
      "secrm",                                        // widget alias
      scu_widget_rm,                                  // widget function
   },
   {
      "tail",                                         // widget name
      "Writes contents of file to standard out.",     // widget description
      "sectail",                                      // widget alias
      scu_widget_tail,                                // widget function
   },
   { NULL, NULL, NULL, NULL }
};


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Functions
#endif

int main(int argc, char * argv[])
{
   int            c;
   int            opt_index;
   scu_config     cnf;

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

   memset(&cnf, 0, sizeof(scu_config));

   // skip argument processing if called via alias
   if ((cnf.widget = scu_widget_lookup(scu_basename(argv[0]), 1)) != NULL)
   {
      cnf.argc = argc;
      cnf.argv = argv;
      return(cnf.widget->func(&cnf));
   };

   while((c = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:	/* no more arguments */
         case 0:	/* long options toggles */
         break;

         case 'h':
         scu_usage();
         return(0);

         case 's':
         cnf.quiet = 1;
         if ((cnf.verbose))
         {
            fprintf(stderr, "%s: incompatible options\n", PROGRAM_NAME);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         break;

         case 'V':
         scu_version();
         return(0);

         case 'v':
         cnf.verbose++;
         if ((cnf.quiet))
         {
            fprintf(stderr, "%s: incompatible options\n", PROGRAM_NAME);
            fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
            return(1);
         };
         break;

         case '?':
         fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
         return(1);

         default:
         fprintf(stderr, "%s: unrecognized option `--%c'\n", PROGRAM_NAME, c);
         fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
         return(1);
      };
   };

   if ((argc - optind) < 1)
   {
      fprintf(stderr, "%s: missing required argument\n", PROGRAM_NAME);
      fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
      return(1);
   };
   cnf.argc = (argc - optind);
   cnf.argv = &argv[optind];
   optind   = 1;

   if (!(cnf.widget = scu_widget_lookup(cnf.argv[0], 0)))
   {
      fprintf(stderr, "%s: unknown or ambiguous widget -- \"%s\"\n", PROGRAM_NAME, cnf.argv[0]);
      fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
      return(1);
   };

   return(cnf.widget->func(&cnf));
}


const char * scu_basename(const char * path)
{
   const char * ptr;
   assert(path != NULL);
   if ((ptr = rindex(path, '/')))
      return(&ptr[1]);
   return(path);
}


/// checks paths
int scu_pathcheck(const char * path)
{
   int            c;
   size_t         s;
   size_t         p;
   char         * str;
   char         * ptr;
   struct stat    sb;

   assert(path != NULL);

   s = strlen(path);

   // verify file is anchored
   if (path[0] != '/')
      return(SCU_EANCHOR);

   // verify path does not end in slash
   if (path[s-1] == '/')
      return(SCU_EFILE);

   // verify there are no adjacent
   for(p = 1; p < s; p++)
   {
      if ( ((path[p-1] == '.')||(path[p-1] == '/')) &&
           ((path[p+0] == '.')||(path[p+0] == '/')) )
      return(SCU_EPATH);
   };

   // verify file is not a symbolic link
   if ((c = lstat(path, &sb)) == -1)
      return(SCU_ERRNO);
   if ((sb.st_mode & S_IFMT) != S_IFREG)
      return(SCU_EFILE);

   // check path for symlinks
   if ((str = strdup(path)) == NULL)
      return(SCU_ERRNO);
   while((ptr = rindex(str, '/')) != NULL)
   {
      ptr[0] = '\0';
      if (str == ptr)
         continue;
      if ((c = lstat(str, &sb)) == -1)
      {
         free(str);
         return(SCU_ERRNO);
      };
      if ((sb.st_mode & S_IFMT) == S_IFLNK)
      {
         free(str);
         return(SCU_EFILE);
      };
   };

   free(str);

   return(0);
}


void scu_usage(void)
{
   int  i;
   char pair[30];

   printf("Usage: %s [OPTIONS] widget [WIDGETOPTIONS]\n", PROGRAM_NAME);
   printf("       widget [OPTIONS]\n");
   printf("\n");

   scu_usage_options();
   printf("\n");

   printf("WIDGETS:\n");
   for (i = 0; scu_widget_map[i].name; i++)
   {
      if (scu_widget_map[i].func != NULL)
      {
         if (scu_widget_map[i].alias != NULL)
            snprintf(pair, 29, "%s, %s", scu_widget_map[i].name, scu_widget_map[i].alias);
         else
            snprintf(pair, 29, "%s", scu_widget_map[i].name);
         printf("   %-24s %s\n", pair, scu_widget_map[i].desc);
      };
   };
   printf("\n");

   scu_usage_restrictions();
   printf("\n");

   return;
}


void scu_usage_options(void)
{
   printf("OPTIONS:\n");
   printf("  -h, --help                print this help and exit\n");
   printf("  -q, --quiet, --silent     do not print messages\n");
   printf("  -V, --version             print version number and exit\n");
   printf("  -v, --verbose             print verbose messages\n");
   return;
}


void scu_usage_restrictions(void)
{
   printf("RESTRICTIONS:\n");
   printf("   File path must begin with a '/' character.\n");
   printf("   File path may not end with a '/' character.\n");
   printf("   File path may not be a symlink.\n");
   printf("   File path may not contain a symlink as part of the path.\n");
   printf("   File path may not contain \"..\", \"./\", \"/.\", or \"//\".\n");
   return;
}


void scu_version(void)
{
   printf(( "%s (%s) %s\n"
            "%s\n"),
            PROGRAM_NAME, PACKAGE_NAME, PACKAGE_VERSION,
            PACKAGE_COPYRIGHT
   );
   return;
}


const scu_widget * scu_widget_lookup(const char * wname, int exact)
{
   int                i;
   const scu_widget * widget;

   assert(wname != NULL);

   widget = NULL;
   for(i = 0; scu_widget_map[i].name != NULL; i++)
   {
      if (scu_widget_map[i].func == NULL)
         continue;

      if (strcasecmp(wname, scu_widget_map[i].name) != 0)
      {
         if (scu_widget_map[i].alias == NULL)
            continue;
         else if (strcasecmp(wname, scu_widget_map[i].alias) != 0)
            continue;
      };

      if (strcasecmp(scu_widget_map[i].name, wname) == 0)
         return(&scu_widget_map[i]);
      if (scu_widget_map[i].alias != NULL)
         if (strcasecmp(scu_widget_map[i].alias, wname) == 0)
            return(&scu_widget_map[i]);

      if (widget != NULL)
         return(NULL);

      widget = &scu_widget_map[i];
   };

   if (exact == 1)
      return(NULL);

   return(widget);
}


const char * scu_strerror(int err)
{
   if (err < 0)
      return(strerror(err));
   switch(err)
   {
      case 0:
      return("success");

      case SCU_ERRNO:
      return(strerror(errno));

      case SCU_EPATH:
      return("path contains illegal pattern");

      case SCU_EFILE:
      return("not a regular file");

      case SCU_EANCHOR:
      return("not an absolute path");

      default:
      break;
   };
   return("unknown error");
}

int scu_is_ascii_buffer(const char * buff, ssize_t len)
{
   ssize_t pos;
   assert(buff != NULL);

   for(pos = 0; pos < len; pos++)
      if ((buff[pos] < 32) || (buff[pos] > 126))
         return(0);

   return(1);
}


/* end of source */
