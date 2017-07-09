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
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>
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
#include "widget-rmdir.h"
#include "widget-tail.h"
#include "widget-zcat.h"


///////////////////
//               //
//  Definitions  //
//               //
///////////////////

#undef _PREFIX
#define _PREFIX SCU_PREFIX


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
int scu_widget_syzdek(scu_config * cnf);


/////////////////
//             //
//  Variables  //
//             //
/////////////////
#ifdef __SECURECOREUTILS_PMARK
#pragma mark - Variables
#endif

const scu_widget scu_widget_map[] =
{
   {
      "cat",                                          // widget name
      "Writes contents of file to standard out.",     // widget description
      (const char * const[]) { _PREFIX"cat", NULL },  // widget alias
      scu_widget_cat,                                 // widget function
   },
   {
      "pathcheck",                                    // widget name
      "Validates path using internal checks.",        // widget description
      (const char * const[]) { _PREFIX"path", NULL }, // widget alias
      scu_widget_pathcheck,                           // widget function
   },
   {
      "rm",                                           // widget name
      "Removes a file.",                              // widget description
      (const char * const[]) { _PREFIX"rm", NULL },   // widget alias
      scu_widget_rm,                                  // widget function
   },
   {
      "rmdir",                                        // widget name
      "Removes a directory.",                         // widget description
      (const char * const[]) { _PREFIX"rmdir", NULL },// widget alias
      scu_widget_rmdir,                               // widget function
   },
#ifdef SCU_EASTER_EGGS
   {
      "syzdek",                                       // widget name
      NULL,                                           // widget description
      (const char * const[]) { "david", NULL },       // widget alias
      scu_widget_syzdek,                              // widget function
   },
#endif
   {
      "tail",                                         // widget name
      "Writes contents of file to standard out.",     // widget description
      (const char * const[]) { _PREFIX"tail", NULL }, // widget alias
      scu_widget_tail,                                // widget function
   },
   {
      "zcat",                                         // widget name
      "Uncompresses file and write to standard out.", // widget description
      (const char * const[]) { _PREFIX"zcat",
#ifdef USE_BZIP2
         "bzcat", _PREFIX"bzcat",
#endif
#ifdef USE_ZLIB
         "gzcat", _PREFIX"gzcat",
#endif
#ifdef USE_LZMA
         "xzcat", _PREFIX"xzcat",
#endif
         NULL },                                      // widget alias
      scu_widget_zcat,                                // widget function
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

   cnf.prog_name = PROGRAM_NAME;

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
         scu_usage(&cnf);
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
int scu_pathcheck(const char * path, int isdir)
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
      return(SCU_EPATH);

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
   if (isdir == 0)
      if ((sb.st_mode & S_IFMT) != S_IFREG)
         return(SCU_EFILE);
   if (isdir != 0)
      if ((sb.st_mode & S_IFMT) != S_IFDIR)
         return(SCU_EDIR);

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


void scu_usage(scu_config * cnf)
{
   int  x;
   int  y;

   printf("Usage: %s [OPTIONS] widget [WIDGETOPTIONS]\n", PROGRAM_NAME);
   printf("       widget [OPTIONS]\n");
   printf("\n");

   scu_usage_options(cnf);
   printf("\n");

   printf("WIDGETS:\n");
   for (x = 0; scu_widget_map[x].name; x++)
   {
      if (scu_widget_map[x].func == NULL)
         continue;
      if (scu_widget_map[x].desc == NULL)
         continue;
      printf("   %-24s %s\n", scu_widget_map[x].name, scu_widget_map[x].desc);
   };
   printf("\n");

   printf("WIDGET ALIASES:\n");
   for (x = 0; scu_widget_map[x].name; x++)
   {
      if (scu_widget_map[x].func == NULL)
         continue;
      if (scu_widget_map[x].alias == NULL)
         continue;
      if (scu_widget_map[x].desc == NULL)
         continue;
      printf("   %-24s %s\n", scu_widget_map[x].name, scu_widget_map[x].alias[0]);
      for (y = 1; scu_widget_map[x].alias[y] != NULL; y++)
         printf("      %-21s %s\n", " ", scu_widget_map[x].alias[y]);
   };
   printf("\n");

   scu_usage_restrictions();
   printf("\n");

   return;
}


void scu_usage_options(scu_config * cnf)
{
   int i;

   cnf->short_opt = (cnf->short_opt == NULL) ? "hqVv" : cnf->short_opt;
   printf("OPTIONS:\n");

   for (i = 0; ((cnf->short_opt[i])); i++)
   {
      switch (cnf->short_opt[i])
      {
         case 'h':
         printf("  -h, --help                print this help and exit\n");
         break;

         case 'q':
         printf("  -q, --quiet, --silent     do not print messages\n");
         break;

         case 'V':
         printf("  -V, --version             print version number and exit\n");
         break;

         case 'v':
         printf("  -v, --verbose             print verbose messages\n");
         break;

         default:
         break;
      };
   };

   return;
}


void scu_usage_restrictions(void)
{
   printf("RESTRICTIONS:\n");
   printf("   File path must be a regular file or directory.\n");
   printf("   File path must begin with a '/' character.\n");
   printf("   File path may not end with a '/' character.\n");
   printf("   File path may not contain a symlink as part of the path.\n");
   printf("   File path may not contain \"..\", \"./\", \"/.\", or \"//\".\n");
   return;
}


void scu_usage_summary(scu_config * cnf, const char * suffix)
{
   int x;

   suffix = (suffix == NULL) ? "" : suffix;

   printf("Usage: %s %s%s\n", cnf->prog_name, cnf->widget->name, suffix);
   if (cnf->widget->alias)
      for(x = 0; (cnf->widget->alias[x] != NULL); x++)
         printf("       %s %s%s\n", cnf->prog_name, cnf->widget->alias[x], suffix);

   printf("       %s%s\n", cnf->widget->name, suffix);

   if (cnf->widget->alias)
      for(x = 0; (cnf->widget->alias[x] != NULL); x++)
         printf("       %s%s\n", cnf->widget->alias[x], suffix);

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
   int                x;
   int                y;
   int                z;
   int                prefix_uniq;
   int                prefix_common;
   const scu_widget * matched_widget;
   const scu_widget * widget;

   assert(wname != NULL);

   matched_widget = NULL;
   prefix_uniq    = -1;
   prefix_common  = -1;

   if (!(strcasecmp(PROGRAM_NAME, wname)))
      return(NULL);

   for(x = 0; scu_widget_map[x].name != NULL; x++)
   {
      widget = &scu_widget_map[x];

      // skip place holders
      if (widget->func == NULL)
         continue;

      // compares widget name
      if (strcmp(widget->name, wname) == 0)
         return(widget);
      for(z = 0; ( (widget->name[z] != '\0') &&
                   (wname[z] != '\0') &&
                   (widget->name[z] == wname[z]) ); z++)
      {
         if (z > prefix_uniq)
         {
            matched_widget = widget;
            prefix_uniq    = z;
         }
         else if (z == prefix_uniq)
         {
            prefix_common = z;
         };
      };

      // compares widget aliases
      if (!(widget->alias))
         continue;
      for(y = 0; widget->alias[y]; y++)
      {
         if (strcmp(widget->alias[y], wname) == 0)
            return(widget);
         for(z = 0; ( (widget->alias[y][z] != '\0') &&
                      (wname[z] != '\0') &&
                      (widget->alias[y][z] == wname[z]) ); z++)
         {
            if (z > prefix_uniq)
            {
               matched_widget = widget;
               prefix_uniq    = z;
            }
            else if (z == prefix_uniq)
            {
               prefix_common = z;
            };
         };
      };
   };

   if (prefix_common >= prefix_uniq)
      return(NULL);

   return(matched_widget);
}


#ifdef SCU_EASTER_EGGS
int scu_widget_syzdek(scu_config * cnf)
{
   unsigned  z;
   double    x;
   double    w;
   double    h;
#ifdef TIOCGSIZE
   struct ttysize b;
#else
   struct winsize b;
#endif
   const char * package = "Secure Core Utilities";
   const char * author  = "by David M. Syzdek";
   const char * msg     = PACKAGE_NAME " v" PACKAGE_VERSION " by David M. Syzdek";

   assert(cnf != NULL);

   x = 0.0;

   while(1)
   {
#ifdef TIOCGSIZE
      w = ioctl(0,TIOCGSIZE,&b) ? 80 : b.ts_cols;
      h = b.ts_lines;
#else
      w = ioctl(0,TIOCGWINSZ,&b) ? 80 : b.ws_col;
      h = b.ws_row;
#endif
      for (z = 0; z < strlen(msg)+1; z++)
      {
         printf("\033[%i;%iH\033[1;%im%c\033[0m",
            //(int)((sin(((x-z)*(360.0/h))/57.3)*(h/2.0))+((h/2.0)+1.0)),
            (int)(( sin(((x-z)*(360.0/h))/80) * (h/2.0)) + ((h/2.0)+1.0) ),
            (int)(( sin(((x-z)*(360.0/w))/80) * (w/2.0)) + ((w/2.0)+1.0) ),
            30+(z%8),
            msg[z]);
      };
      printf("\033[%i;%iH%s", (int)h-1, (int)w - (int)strlen(package), package);
      printf("\033[%i;%iH%s", (int)h, (int)w - (int)strlen(author), author);
      fflush(stdout);
      usleep(100000);
      printf("\033[2J\033[512;512H");
      x += 1;
   };

   return(0);
}
#endif


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
