			/* accent.h, 16 Mar 12 */


#define MAX_ACCENT_MAPPING 20
#define MAX_ACCENT_LANG 10
#define NO_LANGUAGE "none"


struct accent_rec {
	char mapped_src,
	     mapped_dest;
	phon_str_t src,
		   dest;
       };
struct lang_accent_rec {
	char language[MAX_WORD_LEN];
	int num_mapping;
	struct accent_rec acc[MAX_ACCENT_MAPPING];
	};

extern int num_accent_lang;
extern char *accent_language;

extern struct lang_accent_rec lang_accent_info[MAX_ACCENT_LANG];


extern void gen_accent(phon_seq_t orig_word_phon, char *language);
extern void init_accent(void);

