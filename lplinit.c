                        /* lplinit.c, 11 Apr 15 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pcre.h>

#include "lplmapping.h"
#include "spellchk.h"
#include "accent.h"
#include "sndlike.h"
#include "lplinit.h"



#define TRACE_GETPATH   0	/* very useful at start of a port! */

#if defined(FULL_POSIX) || defined(SUN_UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat ISexec;
#define ISEXECUTABLE(file)      (stat(file,&ISexec),                    \
                                        (S_ISREG(ISexec.st_mode)) &&    \
                                        (access(file, X_OK) == 0))
#endif

#ifndef MAX_PATHNAME_LEN
#define MAX_PATHNAME_LEN 256      /* Maximum length of a path */
#endif
#ifdef NAME_MAX /* chars in the base filename */
   #define MAX_BASE_FILE_LEN (NAME_MAX)
#elif defined _POSIX_NAME_MAX
   #define MAX_BASE_FILE_LEN (_POSIX_NAME_MAX)
#else
   #define MAX_BASE_FILE_LEN 64
#endif

#define MAX_FILENAME_LEN (MAX_PATHNAME_LEN+MAX_BASE_FILE_LEN)

#ifndef PATHVAR
#define PATHVAR         "PATH"
#endif

#define GETVAR(var)      getenv(var)


#define DSEP_LEN 1
#define directory_separator "/"
#define path_separator      ":"  /* string to separate several paths */

#define DEFAULT_LETPHON_INFO "letphon.config"

char *lp_info_file;
static char accent_language_buff[MAX_WORD_LEN];


char minus_plus[2] = {'-', '+'};


/*	Given a relative path, search the PATH environment
 *      variable to find the absolute path
 *	NOTE: if the host sets argv[0] to the absolute path, then either:
 *		that host is MS-DOS and we check relpath first
 *		or that host is NOT unix so you probably need
 *              to modify this routine anyway!
 */
char	*getpath(char * relpath)
{
char	*pathlist;
char	*copy_of_paths;
char	*fullpath;
size_t	curpath = 0,
	nextpath;
FILE	*fp;
unsigned long	copy_path_len;

#if	TRACE_GETPATH
printf("getpath(%s)\n", relpath);
#endif

#ifdef  MS_DOS
/* MS-DOS searches current directory first */
if ((fp = fopen(relpath, "r")) != NULL)
   {
   fclose(fp);
   if (ISEXECUTABLE(relpath))
      {
#if     TRACE_GETPATH
printf("%s...exists...isexec\n", relpath);
#endif
      return relpath;
      }
   }
#endif

/* If the path which is passed contains a directory separator
 * character then the path name is either absolute, or
 * relative to the current directory
 */
if (strstr(relpath, directory_separator) != NULL)
   {
   if ((fp = fopen(relpath, "r")) != NULL)
      {
      fclose(fp);
      if (ISEXECUTABLE(relpath))
	 {
#if     TRACE_GETPATH
   printf("%s...exists...isexec\n", relpath);
#endif
         return relpath;
         }
      }
   return (char *)NULL;		/* ?? Can this be executed ?? */
   }

pathlist = GETVAR(PATHVAR);
if (pathlist == NULL)
   pathlist = "";		/* NOTEXEC under unix - cannot unset PATH */

#if	TRACE_GETPATH
printf("pathlist=\"%s\"\n", pathlist);
#endif

fullpath = (char *)malloc(MAX_FILENAME_LEN+1);

/* we can't modify the environment string! */
copy_path_len = strlen(pathlist) + strlen(path_separator) + 1;
copy_of_paths = (char*)malloc(copy_path_len);

strcpy(copy_of_paths, pathlist);
strcat(copy_of_paths, path_separator);

while (copy_of_paths[curpath]) /* while more paths to search... */
   {
   for (nextpath = curpath; copy_of_paths[nextpath]; ++nextpath)
     /* look for next path separator... */
      if (strncmp(&copy_of_paths[nextpath], path_separator,
                                              strlen(path_separator)) == 0)
         {
         copy_of_paths[nextpath] = '\0';
         /* having chopped string at that point, break out */
         break;
         }
   /* construct full pathname to test */
   strcpy(fullpath, &copy_of_paths[curpath]);
   strcat(fullpath, directory_separator);
   strcat(fullpath, relpath);
#if	TRACE_GETPATH
printf("trying %s...", fullpath);
#endif
   /* attempt to open file: */
   if ((fp = fopen(fullpath, "r")) != NULL)
      {
      /* success: close the file and check it is executable: */
      fclose(fp);
#if	TRACE_GETPATH
printf("exists...");
#endif
      if (ISEXECUTABLE(fullpath))
         {
         /* success: return the path found */
#if	TRACE_GETPATH
printf("exec\n");
#endif
         return fullpath;
         }
      }
#if	TRACE_GETPATH
printf("\n");
#endif
   /* keep searching paths... */
   curpath = nextpath + strlen(path_separator);
   }
#if     TRACE_GETPATH
printf("can't find %s\n", relpath);
#endif
/* no path matches: return NULL to indicate failure */
return (char *)NULL;
}


static char * get_exe_path(char * argv_0)
/*
 * Find the path that the program was run from
 */
{
int path_len;
char * full_exe_path;
#if defined(MS_DOS) || defined(CYGWIN)
char *exe_name;
#endif

if (argv_0 == (char *)NULL)
   return NULL;            /* NOT Really EXEC */

full_exe_path = getpath(argv_0);
 
#if defined(MS_DOS) || defined(CYGWIN)
if (full_exe_path == NULL)
   {
   exe_name=malloc(strlen(argv_0)+1+4);
   strcpy(exe_name, argv_0);
#if defined(MS_DOS)
   strcat(exe_name, ".EXE");
#else
   strcat(exe_name, ".exe");
#endif
   full_exe_path=getpath(exe_name);
   }
#endif

if (full_exe_path == NULL)
   return NULL;

path_len=strlen(full_exe_path);
while (path_len > 0)
   {
   if (strncmp(full_exe_path+path_len-DSEP_LEN,
                                     directory_separator, DSEP_LEN) == 0)
      {
      full_exe_path[path_len]='\0';
      break;
      }
   else
      path_len--;
   }

// The POSIX standard requires that an array of appropriate size
// be passed.  It is making use of a common extension to pass NULL
// and expect the system to allocate the storage.
if (path_len == 0)
   full_exe_path=getcwd(NULL, 0);

return full_exe_path;
}


void get_default_letphon(char *prog_path)
{
char * exe_path = get_exe_path(prog_path);
char * lp_path;
size_t exe_path_len;

if (exe_path != NULL)
   {
// Some environments do not append a directory separator at the
// end of the path.  So create space for a separator and add one
// if necessary.
   exe_path_len=strlen(exe_path);
   lp_path=malloc(exe_path_len+DSEP_LEN+1);
   strcpy(lp_path, exe_path);
   if (strncmp(exe_path+exe_path_len-DSEP_LEN,
                                     directory_separator, DSEP_LEN) != 0)
      strcat(lp_path, directory_separator);
   lp_info_file=malloc(strlen(lp_path)+1+strlen(DEFAULT_LETPHON_INFO));
   strcpy(lp_info_file, lp_path);
   strcat(lp_info_file, DEFAULT_LETPHON_INFO);
   }
}


double get_arg_double(char *arg_str, double max_arg_val)
{
double arg_val;
char *end_ptr;

errno=0;
arg_val = strtod(arg_str, &end_ptr);
if ((errno != 0) || ((arg_val == 0) && (end_ptr == arg_str)))
   {
   printf("Argument %s cannot be converted to a numeric value\n",
							arg_str);
   arg_val=max_arg_val;
   }

if (arg_val > (unsigned long)max_arg_val)
   {
   printf("Argument value (%f) exceeds maximum allowed (%f)\n",
							arg_val, max_arg_val);
   arg_val=max_arg_val;
   }

return arg_val;
}


long get_arg_val(char *arg_str, int max_arg_val)
{
unsigned long arg_val;
char *end_ptr;

errno=0;
arg_val = strtoul(arg_str, &end_ptr, 0);
if ((errno != 0) || ((arg_val == 0) && (end_ptr == arg_str)))
   {
   printf("Argument %s cannot be converted to an integer value\n",
							arg_str);
   arg_val=max_arg_val;
   }

if (arg_val > (unsigned long)max_arg_val)
   {
   printf("Argument value (%lu) exceeds maximum allowed (%d)\n",
							arg_val, max_arg_val);
   arg_val=max_arg_val;
   }

return arg_val;
}


BOOL arg_is_plus(char *arg_str)
{
return arg_str[strlen(arg_str)-1] != '-';
}


char *get_arg_str(char *arg_str)
{
char *new_str = (char *)malloc(strlen(arg_str)+1);

strcpy(new_str, arg_str);

return new_str;
}


void trace_options(int argc, char *argv[])
{
int cur_arg;

for (cur_arg=1; cur_arg < argc; cur_arg++)
   {
   if (strncmp(argv[cur_arg], "-trace", 4) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      if (strncmp(argv[cur_arg], "letchk", 4) == 0)
	 trace_letchk_fail=arg_is_plus(argv[cur_arg]);
      else
	 printf("Unknown option %s\n", argv[cur_arg]);
      }
   }
}


void list_options(int argc, char *argv[])
{
int cur_arg;

for (cur_arg=1; cur_arg < argc; cur_arg++)
   {
   if ((strcmp(argv[cur_arg], "-?") == 0) ||
       (strcmp(argv[cur_arg], "-help") == 0) ||
       (strcmp(argv[cur_arg], "-v") == 0) ||
       (strcmp(argv[cur_arg], "-V") == 0))
      {
      printf("slike version 0.1.3\n");
      printf("Options, first four characters significant (default values in parenthesis):\n");
      printf("    -accent %s\n",
					accent_language);
      printf("    -config %s\n",
					lp_info_file);
      printf("    -filter letter%c\n",
					minus_plus[filter_let_bigram]);
      printf("    -generate phoneme%c\n",
					minus_plus[do_word_to_phon]);
      printf("    -help print version and options\n");
      printf("    -phoneme%c\n",
					minus_plus[do_phon_to_word]);
      printf("    -print phonemes%c | rules matched%c\n",
					minus_plus[print_phon_seq],
					minus_plus[print_rule_match]);
      printf("    -rhyme numb(%1d)\n",
					rhym_phon_fixed);
      printf("    -sound like%c | same%c\n",
					minus_plus[do_sound_like],
					minus_plus[do_sound_same]);
      printf("    -spell bigram_dist(%3.1f)\n",
					bigram_limit);
      printf("    -trace letchk_fail%c\n",
					minus_plus[trace_letchk_fail]);
      printf("    -v, -V print version and options\n");
      exit(0);
      }
   }
}


void handle_options(int argc, char *argv[])
{
int cur_arg;

trace_options(argc, argv);

for (cur_arg=1; cur_arg < argc; cur_arg++)
   {
   if ((strcmp(argv[cur_arg], "?") == 0) ||
       (strcmp(argv[cur_arg], "-help") == 0) ||
       (strcmp(argv[cur_arg], "-v") == 0) ||
       (strcmp(argv[cur_arg], "-V") == 0))
      {
      // handled elsewhere
      }
   if (strncmp(argv[cur_arg], "-accent", 3) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      strcpy(accent_language_buff, argv[cur_arg]);
      accent_language=accent_language_buff;
      }
   else if (strncmp(argv[cur_arg], "-config", 3) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      lp_info_file=get_arg_str(argv[cur_arg]);
      }
   else if (strncmp(argv[cur_arg], "-filter", 3) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      if (strncmp(argv[cur_arg], "letter", 3) == 0)
	 filter_let_bigram=arg_is_plus(argv[cur_arg]);
      }
   else if (strncmp(argv[cur_arg], "-generate", 3) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      if (strncmp(argv[cur_arg], "phoneme", 3) == 0)
	 do_word_to_phon=arg_is_plus(argv[cur_arg]);
      }
   else if (strncmp(argv[cur_arg], "-phoneme", 3) == 0)
      {
      do_phon_to_word=arg_is_plus(argv[cur_arg]);
      }
   else if (strncmp(argv[cur_arg], "-print", 3) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      if (strncmp(argv[cur_arg], "phonemes", 3) == 0)
	 print_phon_seq=arg_is_plus(argv[cur_arg]);
      else if (strncmp(argv[cur_arg], "rules", 3) == 0)
	 print_rule_match=arg_is_plus(argv[cur_arg]);
      }
   else if (strncmp(argv[cur_arg], "-rhym", 4) == 0)
      {
      cur_arg++;
      if (cur_arg < argc)
         rhym_phon_fixed=get_arg_val(argv[cur_arg], 20);
      }
   else if (strncmp(argv[cur_arg], "-spell", 4) == 0)
      {
      cur_arg++;
      if (cur_arg < argc)
         bigram_limit=get_arg_double(argv[cur_arg], 100.0);
      }
   else if (strncmp(argv[cur_arg], "-sound", 3) == 0)
      {
      cur_arg++;
      if (cur_arg == argc)
	 break;
      if (strncmp(argv[cur_arg], "like", 3) == 0)
	 do_sound_like=arg_is_plus(argv[cur_arg]);
      else if (strncmp(argv[cur_arg], "same", 3) == 0)
	 {
	 do_sound_same=arg_is_plus(argv[cur_arg]);
	 do_sound_like=!do_sound_same;
	 }
      }
   else if (strncmp(argv[cur_arg], "-trace", 4) == 0) // Handled in earlier loop
      {
      cur_arg++;
      }
   else
      {
      printf("Unknown option: %s\n", argv[cur_arg]);
      cur_arg++;
      }
   } 
list_options(argc, argv);
}


void get_line(FILE *in_file, char *in_str, int in_str_len)
{
char *ch_read=fgets(in_str, in_str_len, in_file);

if (ch_read != NULL)
   in_str[strlen(ch_read)-1]='\0'; // Remove \n from end
}


void get_freq(float bigram_freq[CHAR_MAX], char *pat_str)
{
// Read off the 26 bigram frequencies
int i;
float f_val;

for (i=0; i < 26; i++)
   {
   sscanf(pat_str, "%f", &f_val);
   bigram_freq['a'+i]=f_val;
   while ((*pat_str != ' ') && (*pat_str != '\0'))
      pat_str++;
   pat_str++;
   }
}



void get_accent_info(char *pat_str)
{
// language src-phoneme dest-phoneme
int i;
char lang_name[MAX_PAT_LEN],
     src_phon[MAX_PAT_LEN],
     dest_phon[MAX_PAT_LEN];

sscanf(pat_str, "%s %s %s", lang_name, src_phon, dest_phon);
for (i=0; i < MAX_ACCENT_LANG; i++)
   if (strcmp(lang_accent_info[i].language, lang_name) == 0)
      {
      strcpy(lang_accent_info[i].acc[lang_accent_info[i].num_mapping].src,
							src_phon);
      strcpy(lang_accent_info[i].acc[lang_accent_info[i].num_mapping].dest,
							dest_phon);
      lang_accent_info[i].num_mapping++;
      return;
      }

if (num_accent_lang == MAX_ACCENT_LANG)
   {
   printf("Maximum number (%d) of accent languages exceeded: %s\n",
						MAX_ACCENT_LANG, pat_str);
   return;
   }

num_accent_lang++;

for (i=0; i < MAX_ACCENT_LANG; i++)
   if (strlen(lang_accent_info[i].language) == 0)
      {
      strcpy(lang_accent_info[i].language, lang_name);
      strcpy(lang_accent_info[i].acc[0].src, src_phon);
      strcpy(lang_accent_info[i].acc[0].dest, dest_phon);
      lang_accent_info[i].num_mapping=1;
      return;
      }
}


void init_phon_gen(char *filename)
{
FILE *phon_file =fopen(filename, "r");
char pat_str[MAX_PAT_LEN];
int i,
    num_accent_lines;

get_line(phon_file, pat_str, MAX_PAT_LEN);
// printf("pat: %s\n", pat_str);

let_re=compile_pat(pat_str, 0);

get_line(phon_file, pat_str, MAX_PAT_LEN);
// printf("pat: %s\n", pat_str);
//phon_re=compile_pat(pat_str, PCRE_UNGREEDY);
phon_re=compile_pat(pat_str, 0);


get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_left_context);
//printf("num left context %d\n", num_left_context);

if (num_left_context > MAX_LEFT_CONTEXT_PAT)
   {
   printf("Maximum number of left contexts exceeded: %d\n", num_left_context);
   exit(1);
   }

// Genereated rules are 1 based (not zero)
for (i=1; i <= num_left_context; i++)
   {
   if (feof(phon_file))
      {
      printf("Insufficient left context patterns: %d\n", num_left_context);
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
//printf("lc: %d %s\n", i, pat_str);
   left_context_pat[i]=compile_pat(pat_str, 0);
   }

// Number of different individual phonemes
get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_diff_phon);

for (i=0; i < num_diff_phon; i++)
   {
   if (feof(phon_file))
      {
      printf("Unexpected end-of-file in phoneme mapping list\n");
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   sscanf(pat_str, "%s %c", &phon_hash_tab[i].phon_str,
					&phon_hash_tab[i].mapch);
   }

get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_rules);
//printf("num rules %d\n", num_rules);

if (num_rules > MAX_MAPPING_RULES)
   {
   printf("Maximum number of letter/phoneme mapping exceeded: %d\n", num_rules);
   exit(1);
   }

// Genereated rules are 1 based (not zero)
for (i=1; i <= num_rules; i++)
   get_line(phon_file, rule_phon_list[i], MAX_LET_PHON_LEN);

// Number of different left contexts
get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_phon_left_pat);
//printf("num phon_left_pat %d\n", num_phon_left_pat);

// Left context of L->P.  Genereated rules are 1 based (not zero)
for (i=1; i <= num_phon_left_pat; i++)
   {
   if (feof(phon_file))
      {
      printf("Insufficient left context patterns: %d\n", num_phon_left_pat);
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   phon_left_pat[i]=compile_pat(pat_str, 0);
//printf("%d %s %d %p\n", i, pat_str, strlen(pat_str), phon_left_pat[i]);
   }

// Index into phon_let_pat.  Genereated rules are 1 based (not zero)
for (i=1; i <= num_rules; i++)
   {
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   sscanf(pat_str, "%d", &phon_let_left_ind[i]);
   }
// Matching letter sequence for rule.  Genereated rules are 1 based (not zero)
for (i=1; i <= num_rules; i++)
   get_line(phon_file, phon_rule_let_seq[i], MAX_LET_PHON_LEN);

// Genereated rules are 1 based (not zero)
for (i=1; i <= num_rules; i++)
   {
   if (feof(phon_file))
      {
      printf("Insufficient right context patterns: %d\n", num_rules);
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
//printf("lc: %d %s\n", i, pat_str);
   phon_right_pat[i]=compile_pat(pat_str, 0);
   }

// Number of different phones in following distinct feature list
get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_df_phon);
if (num_df_phon != num_diff_phon)
   printf("Unique phoneme count disagreement %d:%d\n",
				num_df_phon, num_diff_phon);
for (i=0; i < num_df_phon; i++)
   {
   if (feof(phon_file))
      {
      printf("Unexpected end-of-file in phoneme mapping list\n");
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   sscanf(pat_str, "%s %x", &phon_df_info[i].p_str,
					&phon_df_info[i].features);
   }

// Number of distinct feature and their bit pattern
get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_dis_feat);
for (i=0; i < num_dis_feat; i++)
   {
   if (feof(phon_file))
      {
      printf("Unexpected end-of-file in phoneme mapping list\n");
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   sscanf(pat_str, "%s %x", &feature_info[i].f_str,
					&feature_info[i].f_val);
   }

for (i=0; i < 26; i++)
   {
   if (feof(phon_file))
      {
      printf("Unexpected end-of-file in phoneme mapping list\n");
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   get_freq(any_bigram['a'+i], pat_str);
   }

for (i=0; i < 26; i++)
   {
   if (feof(phon_file))
      {
      printf("Unexpected end-of-file in phoneme mapping list\n");
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   get_freq(start_bigram['a'+i], pat_str);
   }

// Number of language accent mappings
get_line(phon_file, pat_str, MAX_PAT_LEN);
sscanf(pat_str, "%d", &num_accent_lines);
for (i=0; i < num_accent_lines; i++)
   {
   if (feof(phon_file))
      {
      printf("Unexpected end-of-file in phoneme mapping list\n");
      exit(1);
      }
   get_line(phon_file, pat_str, MAX_PAT_LEN);
   get_accent_info(pat_str);
   }

fclose(phon_file);
}


void init(int argc, char *argv[])
{
get_default_letphon(argv[0]);
handle_options(argc, argv);
init_phon_gen(lp_info_file);
init_sounds_like();
init_accent();
}

