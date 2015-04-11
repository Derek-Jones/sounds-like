			/* sndlike.c, 21 Mar 12 */

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <pcre.h>

#include "lplmapping.h"
#include "sndlike.h"


int do_sound_like;
int do_sound_same;
int min_diff_dist = 1;  // Minimum distance between phonemes for sounds-like
int rhym_phon_fixed = 0;

int num_dis_feat;
int num_df_phon;

struct feature_rec feature_info[MAX_DISTINCT_FEATURES];
struct p_distinct_rec phon_df_info[MAX_DIFF_PHON];

int feature_map[CHAR_MAX];



int phon_dist(char phon1, char phon2)
{
// Return number of features that are different between phonemes
int i,
    dist = 0,
    weight = 1, // for the time being
    diff_feat = feature_map[phon1] ^ feature_map[phon2];

for (i=0; i < num_dis_feat; i++)
   dist+=((diff_feat & feature_info[i].f_val) != 0)*weight;

// printf("%c %c %d 0x%x\n", phon1, phon2, dist, diff_feat);

return dist;
}


void get_close_phon(char base_phon, char *phon_str)
{
// Return a string a phonemes that are within min_diff_dist of base_phon
int i;
char *close_str = phon_str;

*phon_str='\0';

for (i=0; i < num_diff_phon; i++)
   if ((base_phon != phon_hash_tab[i].mapch) &&
       (phon_dist(base_phon, phon_hash_tab[i].mapch) <= min_diff_dist))
      {
      *phon_str=phon_hash_tab[i].mapch;
      phon_str++;
      *phon_str='\0';
      }
//printf("orig: %c close:%s\n", base_phon, close_str);
}


void gen_sounds_like(phon_seq_t orig_word_phon)
{
int word_phon_len = strlen(orig_word_phon.phon_list),
    i, j;
phon_seq_t new_phon_seq;
char close_phon_str[MAX_DIFF_PHON+1];

// Iterate over each phoneme, changing one of them at a time
for (i=0; i < word_phon_len; i++)
   {
   get_close_phon(orig_word_phon.phon_list[i], close_phon_str);
// Iterate over all close phonemes
   for (j=0; j < strlen(close_phon_str); j++)
      {
      new_phon_seq=orig_word_phon;
      new_phon_seq.phon_list[i]=close_phon_str[j];
      phon2let(new_phon_seq);
// print list
      }
   }
}


void gen_rhyming(phon_seq_t orig_word_phon)
{
// Generate rhyming letter sequences.
// Differs from gen_sounds_like in that phonemes at the
// end of the sequence are never changed.
int word_phon_len = strlen(orig_word_phon.phon_list),
    i, j,
    max_mod_phon;
phon_seq_t new_phon_seq;
char close_phon_str[MAX_DIFF_PHON+1];

if (rhym_phon_fixed < word_phon_len)
   max_mod_phon=word_phon_len-rhym_phon_fixed;
else
   max_mod_phon=word_phon_len-1;

// Iterate over each phoneme, changing one of them at a time
for (i=0; i < max_mod_phon; i++)
   {
   get_close_phon(orig_word_phon.phon_list[i], close_phon_str);
// Iterate over all 'close' phonemes
   for (j=0; j < strlen(close_phon_str); j++)
      {
      new_phon_seq=orig_word_phon;
      new_phon_seq.phon_list[i]=close_phon_str[j];
      phon2let(new_phon_seq);
// print list
      }
   }
}


void init_sounds_like(void)
{
int i, j;

// Create feature_map so it indexes from a mapped phoneme character
// to that phonemes distinctive feature value set.
for (i=0; i < num_df_phon; i++)
   {
   char *df_str=phon_df_info[i].p_str;
   for (j=0; j < num_diff_phon; j++)
      if (strcmp(phon_hash_tab[j].phon_str, df_str) == 0)
	 {
	 feature_map[phon_hash_tab[j].mapch]=phon_df_info[i].features;
	 break;
	 }
   }
}

