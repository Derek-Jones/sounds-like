                       /* wordmap.c, 27 Mar 12 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pcre.h>

#include "lplmapping.h"
#include "spellchk.h"
#include "accent.h"
#include "sndlike.h"
#include "lplinit.h"


phon_seq_t mapped_phon_seq;
char word_str[MAX_WORD_LEN];


void phon_to_word(void)
{
int i,
    num_phon = 0;
size_t word_len;

fgets(word_str, sizeof(word_str), stdin);

word_len = strlen(word_str);

// Convert null terminate each phoname and convert to upper case
for (i=0; i < word_len; i++)
   if (isspace(word_str[i]))
      word_str[i]='\0';
   else
      word_str[i]=toupper(word_str[i]);

// Map phoneme to single character internal representation
i=0;
while (i < word_len)
   {
   if (word_str[i] != '\0')
      {
      mapped_phon_seq.phon_list[num_phon]=phon_to_mapch(word_str+i);
      num_phon++;
      while ((i < word_len) && (word_str[i] != '\0'))
	 i++;
      }
   i++;
   }
mapped_phon_seq.phon_list[num_phon]='\0';

gen_sounds_like(mapped_phon_seq);
do_sound_same=TRUE;
phon2let(mapped_phon_seq);
}


void word_to_phon(void)
{
print_phon_seq=FALSE;

mapped_phon_seq=let2phon(word_str, TRUE);
printf("%s\n", mapped_phon_seq);
}


void sounds_like(void)
{
mapped_phon_seq=let2phon(word_str, TRUE);
gen_sounds_like(mapped_phon_seq);
phon2let(mapped_phon_seq);
}


void same_sound(void)
{
mapped_phon_seq=let2phon(word_str, TRUE);
phon2let(mapped_phon_seq);
}


void rythm_sound(void)
{
mapped_phon_seq=let2phon(word_str, TRUE);
gen_rhyming(mapped_phon_seq);
}

void language_accent(char *lang_accent)
{
mapped_phon_seq=let2phon(word_str, TRUE);
gen_accent(mapped_phon_seq, lang_accent);
}


int main(int argc, char *argv[])
{
do_sound_like=TRUE;
do_sound_same=FALSE;
do_word_to_phon=FALSE;
do_phon_to_word=FALSE;
filter_let_bigram=TRUE;

init(argc, argv);

if (do_phon_to_word)
   phon_to_word();
else while (!feof(stdin))
   {
   scanf("%s\n", word_str);
   if (do_word_to_phon)
      word_to_phon();
   else if (do_sound_same)
      same_sound();
   else if (strcmp(accent_language, NO_LANGUAGE) != 0)
      language_accent(accent_language);
   else if (rhym_phon_fixed != 0)
      rythm_sound();
   else if (do_sound_like)
      sounds_like();
   }

return 0;
}

