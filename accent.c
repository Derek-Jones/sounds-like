			/* accent.c, 21 Mar 12 */


#include <string.h>
#include <stdio.h>

#include <pcre.h>

#include "lplmapping.h"
#include "accent.h"


int num_accent_lang;
char *accent_language = NO_LANGUAGE;

struct lang_accent_rec lang_accent_info[MAX_ACCENT_LANG];


static struct accent_rec *cur_accent_map;


char map_phon_accent(char phon)
{
int i;

for (i=0; i < MAX_ACCENT_MAPPING; i++)
   if (cur_accent_map[i].mapped_src == phon)
      return cur_accent_map[i].mapped_dest;

return phon;
}


void gen_accent(phon_seq_t orig_word_phon, char *language)
{
int word_phon_len = strlen(orig_word_phon.phon_list),
    i;
phon_seq_t new_phon_seq;

cur_accent_map=NULL;

for (i=0; i < num_accent_lang; i++)
   if (strcmp(language, lang_accent_info[i].language) == 0)
      {
      cur_accent_map=lang_accent_info[i].acc;
      break;
      }

if (cur_accent_map == NULL)
   {
   printf("Unknown accent language: %s\n", language);
   return;
   }

new_phon_seq=orig_word_phon;

for (i=0; i < word_phon_len; i++)
   {
   new_phon_seq.phon_list[i]=map_phon_accent(orig_word_phon.phon_list[i]);
   }
phon2let(new_phon_seq);
// print list
}


void init_accent(void)
{
int i, j;

// Map language accent phonemes to single characters
for (i=0; i < num_accent_lang; i++)
   if (strlen(lang_accent_info[i].language) != 0)
      {
      for (j=0; j < lang_accent_info[i].num_mapping; j++)
	 {
	 lang_accent_info[i].acc[j].mapped_src=
			phon_to_mapch(lang_accent_info[i].acc[j].src);
	 lang_accent_info[i].acc[j].mapped_dest=
			phon_to_mapch(lang_accent_info[i].acc[j].dest);
	 }
      }
}

