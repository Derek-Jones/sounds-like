			/* spellchk.h, 27 Mar 12 */


#define MAX_SPELL_CH ('z'+1)

extern int filter_let_bigram;
extern float bigram_limit;

extern float any_bigram[MAX_SPELL_CH][MAX_SPELL_CH];
extern float start_bigram[MAX_SPELL_CH][MAX_SPELL_CH];

extern int let_seq_ok(char *word_str);

