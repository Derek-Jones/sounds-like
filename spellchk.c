		     /* spellchk.c, 27 Mar 12 */


#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "spellchk.h"

int filter_let_bigram;
float bigram_limit = 1.0;
float any_bigram[MAX_SPELL_CH][MAX_SPELL_CH];
float start_bigram[MAX_SPELL_CH][MAX_SPELL_CH];


int let_seq_ok(char *word_str)
{
const int word_len = strlen(word_str);
int i,
    num_cons,
    num_vowel;

if (!filter_let_bigram)
   return 1;

if (start_bigram[word_str[0]][word_str[1]] < bigram_limit)
   return 0;

for (i=1; i < word_len-1; i++)
   if (any_bigram[word_str[i]][word_str[i+1]] < bigram_limit)
      return 0;

// Must contain at least one vowel and no sequence of five consonants
num_cons=0;
num_vowel=0;
for (i=0; i < word_len; i++)
   if ((word_str[i] == 'a') ||
       (word_str[i] == 'e') ||
       (word_str[i] == 'i') ||
       (word_str[i] == 'o') ||
       (word_str[i] == 'u') ||
       (word_str[i] == 'y'))
      {
      num_cons=0;
      num_vowel++;
      }
   else
      {
      num_cons++;
      if (num_cons == 5)
	 return 0;
      }

//printf("lso: %s\n", word_str);

return (num_vowel > 0);
}


void init_spellchk(void)
{
}

