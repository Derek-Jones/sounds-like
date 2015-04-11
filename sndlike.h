			/* sndlike.h, 17 Mar 12 */

#define MAX_DISTINCT_FEATURES 25

struct feature_rec {
	char f_str[10];
	int  f_val;
       };

struct p_distinct_rec {
	char p_str[10];
	int  features;
       };

extern int do_sound_like;
extern int do_sound_same;
extern int rhym_phon_fixed;
extern int num_dis_feat;
extern int num_df_phon;

extern struct feature_rec feature_info[MAX_DISTINCT_FEATURES];
extern struct p_distinct_rec phon_df_info[MAX_DIFF_PHON];

extern void gen_sounds_like(phon_seq_t orig_word_phon);
extern void gen_rhyming(phon_seq_t orig_word_phon);
extern void init_sounds_like(void);

