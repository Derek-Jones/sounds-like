                        /* lplmapping.h, 11 Apr 15 */

#define MAX_SUBSTRING_ELEM  30
#define MAX_MATCH_POINTS    30
#define MAX_PAT_LEN 1024*20
#define MAX_LEFT_CONTEXT_PAT 50
#define MAX_WORD_LEN 80
#define MAX_MAPPING_RULES 1000
#define MAX_LET_PHON_LEN  15
#define MAX_PHON_LEN 10
#define MAX_DIFF_PHON 62  // A-Za-z0-9
#define MAX_LET_SEQ_LEN  10
#define MAX_PHON_SEQ_LEN MAX_WORD_LEN
#define CALLOUT_START 50
#define CALLOUT_NUM_BASE 50
#define MAX_GEN_WORDS 200

#define PRINT_FIELD(f) printf("%s: %d\n", #f, match_info->f)

#define TRUE 1
#define FALSE 0
typedef unsigned char BOOL;

typedef char phon_t;
typedef char phon_str_t[MAX_PHON_LEN];  // string representation of a phoneme
typedef char phon_seq_str[MAX_PHON_SEQ_LEN];// sequence of phonemes
// C does not allow arrays to be returned from functions.
// So we have to embed the array in a struct.
typedef struct {
		phon_seq_str phon_list;
	       } phon_seq_t;
typedef struct {
		char let_list[MAX_WORD_LEN];
	       } let_seq_t;

extern int trace_letchk_fail,
           print_rule_match,
           print_phon_seq,
	   do_word_to_phon,
	   do_phon_to_word;
extern int num_rules;
// The compiled form of the L->P and P->L rules
extern pcre *let_re,
	    *phon_re;
// Number of rule left contexts and their compiled patterns
extern int num_left_context;
extern pcre *(left_context_pat[MAX_LEFT_CONTEXT_PAT]);

// Phoneme sequence corresponding to each rule
extern char rule_phon_list[MAX_MAPPING_RULES][MAX_LET_PHON_LEN];

// Number of rule left contexts used by P->L rules
extern int num_phon_left_pat;
extern int phon_let_left_ind[MAX_MAPPING_RULES];
extern pcre *(phon_left_pat[MAX_MAPPING_RULES]);
// Leter sequence consumed when a rule matches
extern char phon_rule_let_seq[MAX_MAPPING_RULES][MAX_LET_SEQ_LEN];
// Compiled form of right context used in P->L
extern pcre * (phon_right_pat[MAX_MAPPING_RULES]);

// Phoneme symbol/single character mapping variable
struct phon_hash_rec {
        phon_str_t phon_str;
	char mapch;
       };
extern struct phon_hash_rec phon_hash_tab[MAX_DIFF_PHON];
extern int num_diff_phon;

extern char generated_words[MAX_GEN_WORDS][MAX_WORD_LEN];
extern int num_generated_words;

extern char phon_to_mapch(char *p_str);
extern pcre *compile_pat(char *new_pat, int options);
extern phon_seq_t let2phon(char *word_str, BOOL do_print);
extern let_seq_t phon2let(phon_seq_t phon_seq);

