                        /* lplmapping.c, 11 Apr 15 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pcre.h>

#include "spellchk.h"
#include "lplmapping.h"

#define DEBUG_L2P 0
#define DEBUG_P2L 0


int trace_letchk_fail,
    print_rule_match,
    print_phon_seq,
    do_word_to_phon,
    do_phon_to_word;

pcre *let_re,
     *phon_re;
int num_left_context;
pcre *(left_context_pat[MAX_LEFT_CONTEXT_PAT]);

short l_match_pos[MAX_PHON_SEQ_LEN],
      p_match_pos[MAX_PHON_SEQ_LEN];
int l_word_len,
    num_rules;
int p2l_callout_num;
char l_rev_word[MAX_WORD_LEN];
char gen_word[MAX_WORD_LEN];
char rule_phon_list[MAX_MAPPING_RULES][MAX_LET_PHON_LEN];

int num_phon_left_pat;
int phon_let_left_ind[MAX_MAPPING_RULES];
pcre *(phon_left_pat[MAX_MAPPING_RULES]);
char phon_rule_let_seq[MAX_MAPPING_RULES][MAX_LET_SEQ_LEN];
pcre * (phon_right_pat[MAX_MAPPING_RULES]);

// Phoneme symbol/single character mapping variable
struct phon_hash_rec phon_hash_tab[MAX_DIFF_PHON];
int num_diff_phon = 0;

char generated_words[MAX_GEN_WORDS][MAX_WORD_LEN];
int num_generated_words;


int left_matches(pcre_callout_block *match_info)
{
/*
 * Match the left context of a pattern.
 * Works by matching left to right on the reversed character sequence.
 */
int rc = pcre_exec(
           left_context_pat[match_info->callout_number],
           NULL,           // we didn't study the pattern
           l_rev_word+
	    (l_word_len-match_info->start_match),       // subject string
           match_info->start_match+1,          // length of the subject string
           0,              // start at offset 0 in the subject
           0,              // default options
           NULL,           // vector of integers for substring information
           0);             // number of elements (NOT size in bytes)

if (rc < -1)
   printf("Error: something unexpected happened in lm.pcre_exec: %d\n", rc);

#if DEBUG_L2P
printf("%d %s %d %s %d\n", match_info->callout_number,
				l_rev_word,
				match_info->start_match,
				l_rev_word+
				   (l_word_len-match_info->start_match),
				rc);
#endif

return rc < 0; // phon_call needs to return 0 for the match to continue
}


int let2phon_callout(pcre_callout_block *match_info)
{
int i;

#if 0
PRINT_FIELD(subject_length);
PRINT_FIELD(start_match);
PRINT_FIELD(current_position);
PRINT_FIELD(capture_top);
PRINT_FIELD(capture_last);
PRINT_FIELD(pattern_position);
PRINT_FIELD(next_item_length);
/*
for (i=0; i < match_info->capture_top; i++)
   printf(": %d", match_info->offset_vector[i]);
printf("\n");
*/
#endif

/*
 * A pcre ?C callout has been encountered during pattern match.
 * It could be information about a left context that needs matching
 * or the rule number we have just matched.
 */
if (match_info->callout_number <= num_left_context)
   return left_matches(match_info);

if (match_info->callout_number >= CALLOUT_START+CALLOUT_NUM_BASE)
   l_match_pos[match_info->start_match]=9*CALLOUT_NUM_BASE+
				match_info->callout_number;
else
   {
   for (i=match_info->start_match+1; i < MAX_MATCH_POINTS; i++)
      l_match_pos[i]=0;
   if (l_match_pos[match_info->start_match] > 10*CALLOUT_NUM_BASE)
      l_match_pos[match_info->start_match]=CALLOUT_NUM_BASE*
	(l_match_pos[match_info->start_match]-10*CALLOUT_NUM_BASE-CALLOUT_START)+
			 match_info->callout_number-CALLOUT_START;
   else
      l_match_pos[match_info->start_match]=match_info->callout_number-
							CALLOUT_START;
   }

#if DEBUG_L2P
printf("(%d:%d) ", match_info->callout_number-CALLOUT_START,
			 match_info->start_match);
printf("%d", l_match_pos[match_info->start_match]);
#endif

return 0;
}


int match_gen_word(int rule_num)
{
char p_rev_word[MAX_WORD_LEN];
int i,
    rc,
    gen_word_len;

gen_word_len=strlen(gen_word);

for (i=0; i < gen_word_len; i++)
   p_rev_word[gen_word_len-1-i]=gen_word[i];
p_rev_word[gen_word_len]='\0';

#if DEBUG_P2L
printf("rw: %s %d", p_rev_word, rule_num);
#endif

rc = pcre_exec(
	   phon_left_pat[phon_let_left_ind[rule_num]], // result of pcre_compile()
           NULL,           // we didn't study the pattern
           p_rev_word,     // subject string
           gen_word_len,   // length of the subject string
           0,              // start at offset 0 in the subject
           0,              // default options
           NULL,     // vector of integers for substring information
           0);       // number of elements (NOT size in bytes)

#if DEBUG_P2L
printf(" rc %d\n", rc);
#endif

if (rc < -1)
   printf("Error: something unexpected happened in gw.pcre_exec: %d\n", rc);

return rc >= 0;
}


void save_gen_word(char *gen_word)
{
int i;

for (i=0; i < num_generated_words; i++)
   if (strcmp(generated_words[i], gen_word) == 0)
      return;
if (num_generated_words < MAX_GEN_WORDS)
   {
   strcpy(generated_words[num_generated_words], gen_word);
   num_generated_words++;
   }
}


void check_gen_words(phon_seq_t phon_seq)
{
// Addtional checks on the generated words

int i;
phon_seq_t gen_word_phon;

for (i=0; i < num_generated_words; i++)
   {
// Does the letter sequence following characteristics of English?
   if (let_seq_ok(generated_words[i]))
      {
      gen_word_phon=let2phon(generated_words[i], FALSE);
      if (strcmp(gen_word_phon.phon_list, phon_seq.phon_list) != 0)
	 generated_words[i][0]='\0';
      }
   else
      {
      if (trace_letchk_fail)
	 printf("letchk fail: %s\n", generated_words[i]);
      generated_words[i][0]='\0';
      }
   }
for (i=0; i < num_generated_words; i++)
   {
   if (generated_words[i][0] != '\0')
      printf("%s\n", generated_words[i]);
   }
}


int phon2let_callout(pcre_callout_block *match_info)
{
int i;

#if 0
PRINT_FIELD(subject_length);
PRINT_FIELD(start_match);
PRINT_FIELD(current_position);
PRINT_FIELD(capture_top);
PRINT_FIELD(capture_last);
PRINT_FIELD(pattern_position);
PRINT_FIELD(next_item_length);
/*
for (i=0; i < match_info->capture_top; i++)
   printf(": %d", match_info->offset_vector[i]);
printf("\n");
*/
#endif

/*
 * A pcre ?C callout has been encountered during pattern match.
 * It could be information about a left context that needs matching
 * or the rule number we have just matched.
 */

if (match_info->callout_number == 255) // Unconditionally fail
   {
   save_gen_word(gen_word);
   return 1;
   }
if (match_info->callout_number >= CALLOUT_NUM_BASE)
   {
/*
 * Adding in another CALLOUT_NUM_BASE lets the next call
 * distinguish between a 'stale' value and a partially
 * created value that it needs to complete.
 */
   p2l_callout_num=9*CALLOUT_NUM_BASE+match_info->callout_number;
   return 0;
   }
else
   {
   if (p2l_callout_num > 10*CALLOUT_NUM_BASE)
      p2l_callout_num=CALLOUT_NUM_BASE*
				(p2l_callout_num-10*CALLOUT_NUM_BASE)+
			 match_info->callout_number;
   else
      p2l_callout_num=match_info->callout_number;
   if (match_gen_word(p2l_callout_num))
      {
      for (i=match_info->start_match+1; i < MAX_MATCH_POINTS; i++)
         p_match_pos[i]=0;
      if (match_info->start_match == 0)
	 gen_word[0]='\0';
      else
	 {
	 if (p_match_pos[match_info->start_match] == 0)
	    p_match_pos[match_info->start_match]=strlen(gen_word);
	 else
            gen_word[p_match_pos[match_info->start_match]]='\0';
	 }
      strcat(gen_word, phon_rule_let_seq[p2l_callout_num]);
      }
   else
      return 1;
   }

#if DEBUG_P2L
printf("[%d:%d:%d] ", match_info->callout_number,
			 match_info->start_match,
			p_match_pos[match_info->start_match]);
#endif

return 0;
}


char phon_to_mapch(char *p_str)
{
int i;

for (i=0; i < num_diff_phon; i++)
   if (strcmp(phon_hash_tab[i].phon_str, p_str) == 0)
      return phon_hash_tab[i].mapch;

printf("Unknown phoneme: %s\n", p_str);

return '\0';
}


char *mapch_to_phon(char ch)
{
int i;

for (i=0; i < num_diff_phon; i++)
   if (phon_hash_tab[i].mapch == ch)
      return phon_hash_tab[i].phon_str;

printf(">> Internal error (mapch phon): %c\n", ch);

return "";
}


void print_phoneme_seq(phon_seq_t ph_seq)
{
printf("%s\n", ph_seq.phon_list);
}


phon_seq_t phon_let2str(phon_seq_t phon_seq)
{
phon_seq_t phon_vseq;
char *p_str = phon_seq.phon_list;

phon_vseq.phon_list[0]='\0';

while (*p_str != '\0')
   {
   strcat(phon_vseq.phon_list, mapch_to_phon(*p_str));
   strcat(phon_vseq.phon_list, " ");
   p_str++;
   }

return phon_vseq;
}


phon_seq_t let2phon(char *word_str, BOOL do_print)
{
int rc;
int i;
int substr_vec[MAX_SUBSTRING_ELEM];
phon_seq_t phon_seq;

#if DEBUG_L2P
printf(">> %s\n", word_str);
#endif

phon_seq.phon_list[0]='\0';
l_word_len=strlen(word_str);
if (l_word_len == 0)
   return phon_seq;

l_match_pos[0]=0;
pcre_callout=let2phon_callout;

for (i=0; i < l_word_len; i++)
   l_rev_word[l_word_len-1-i]=word_str[i];
l_rev_word[l_word_len]='\0';
for (i=0; i < MAX_SUBSTRING_ELEM; i++)
   l_match_pos[i]=0;

rc = pcre_exec(
           let_re,         // result of pcre_compile()
           NULL,           // we didn't study the pattern
           word_str,       // subject string
           l_word_len,       // length of the subject string
           0,              // start at offset 0 in the subject
           0,              // default options
           substr_vec,     // vector of integers for substring information
           MAX_SUBSTRING_ELEM);  // number of elements (NOT size in bytes)

if (rc < -1)
   printf("Error: something unexpected happened in lp.pcre_exec: %d\n", rc);

for (i=0; i < MAX_SUBSTRING_ELEM; i++)
   {
   if (l_match_pos[i] != 0)
      {
      strcat(phon_seq.phon_list, rule_phon_list[l_match_pos[i]]);
      if (do_print && print_rule_match)
         printf("%3d %s\n", l_match_pos[i],
			mapch_to_phon(*rule_phon_list[l_match_pos[i]]));
      }
   }

if (do_print && print_phon_seq)
   {
   phon_seq_t dummy = phon_let2str(phon_seq); // hostage to l-value rules
   printf("%s\n", dummy.phon_list);
   }

return phon_seq;
}


let_seq_t phon2let(phon_seq_t phon_seq)
{
int i,
    rc,
    phon_len;
int substr_vec[MAX_SUBSTRING_ELEM];
let_seq_t let_seq;

num_generated_words=0;
let_seq.let_list[0]='\0';
phon_len=strlen(phon_seq.phon_list);
if (phon_len == 0)
   return let_seq;

for (i=0; i < MAX_SUBSTRING_ELEM; i++)
   p_match_pos[i]=0;

gen_word[0]='\0';

pcre_callout=phon2let_callout;

#if DEBUG_P2L
printf(">> %s | ", phon_seq.phon_list);
print_phoneme_seq(phon_let2str(phon_seq));
#endif

rc = pcre_exec(
           phon_re,        // result of pcre_compile()
           NULL,           // we didn't study the pattern
           phon_seq.phon_list,       // subject string
           phon_len,       // length of the subject string
           0,              // start at offset 0 in the subject
           0,              // default options
           substr_vec,     // vector of integers for substring information
           MAX_SUBSTRING_ELEM);  // number of elements (NOT size in bytes)

if (rc < -1)
   printf("Error: something unexpected happened in pcre_exec: %d\n", rc);

#if DEBUG_P2L
{
printf("rc %d %s\n", rc, gen_word);
}
#endif

check_gen_words(phon_seq);

//printf("<< %s\n", let_seq.let_list);

return let_seq;
}


pcre *compile_pat(char *new_pat, int options)
{
const char *err_str;
int err_offset;
pcre *new_re=pcre_compile(new_pat,
			PCRE_ANCHORED | options, &err_str, &err_offset, NULL);

if (new_re == NULL)
   {
   printf("Pattern compiler failed: %s (%d)\n", err_str, err_offset);
   exit(1);
   }

return new_re;
}

