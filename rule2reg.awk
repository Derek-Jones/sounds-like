#
# rule2reg.awk, 13 Mar 12

BEGIN {

# Rules from Elovitz paper.
# $ - one or more vowels
# ! - two or more vowels
# + - one front vowel
# : - zero or more consonants
# ^ - one consonant
# ; - one or more consonants
# . - one voiced consonant
# @ - one special consonant
# & - one sibilant
# % - one suffix

	vowel="(a|e|i|o|u|y)"
	front_vowel="(e|i|y)"
	consonant="(b|c|d|f|g|h|j|k|l|m|n|p|q|r|s|t|v|w|x|z)"
	voiced_consonant="(b|d|v|g|j|l|m|n|r|w|z)"
	special_consonant="(t|s|r|d|l|z|n|j|th|ch|sh)"
	sibilant="(s|c|g|z|x|j|ch|sh)"
	suffix="(er|e|es|ed|ing|ely)"
	right_sym["$"]=vowel "+"
	right_sym["!"]=vowel "{2,}"
	right_sym["+"]=front_vowel
	right_sym[":"]=consonant "*"
	right_sym["^"]=consonant
	right_sym[";"]=consonant "+"
	right_sym["."]=voiced_consonant
	right_sym["@"]=special_consonant
	right_sym["&"]=sibilant
	right_sym["%"]=suffix
	right_sym[" "]="$"

# At Least One Char, aloc, is needed to match this pattern
	aloc["$"]=1
	aloc["!"]=1
	aloc["+"]=1
	aloc[":"]=0
	aloc["^"]=1
	aloc[";"]=1
	aloc["."]=1
	aloc["@"]=1
	aloc["&"]=1
	aloc["%"]=1
	aloc[" "]=0

	NOT_FIXED="#"
	fixed_1st["$"]=vowel
	fixed_1st["!"]=vowel "{2,}"
	fixed_1st["+"]=front_vowel
	fixed_1st[":"]=""
	fixed_1st["^"]=consonant
	fixed_1st[";"]=consonant
	fixed_1st["."]=voiced_consonant
	fixed_1st["@"]=NOT_FIXED
	fixed_1st["&"]=NOT_FIXED
	fixed_1st["%"]=NOT_FIXED
	fixed_1st[" "]="^"

	fixed_any["$"]=NOT_FIXED
	fixed_any["!"]=NOT_FIXED
	fixed_any["+"]=front_vowel
	fixed_any[":"]=NOT_FIXED
	fixed_any["^"]=consonant
	fixed_any[";"]=NOT_FIXED
	fixed_any["."]=voiced_consonant
	fixed_any["@"]=NOT_FIXED
	fixed_any["&"]=NOT_FIXED
	fixed_any["%"]=NOT_FIXED
	fixed_any[" "]="}"

	alpha_str="abcdefghijklmnopqrstuvwxyz"
	for (i=1; i <= length(alpha_str); i++)
	   {
	   ch=substr(alpha_str, i, 1)
	   right_sym[ch]=ch
	   aloc[ch]=1
	   fixed_1st[ch]=ch
	   fixed_any[ch]=ch
	   }

	one_phon="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
	map_phon[999]=0
	num_phon_sym=0
	num_callout_pat=0
	left_let_callout_num=0
	callout_num[" "]=0
	callout_pat[0]=" "
	num_phon_callout_pat=0
	phon_callout_num[" "]=0
	phon_callout_pat[0]=" "
	seen_phon_pat[" "]=0

	LET_RULE_OFFSET=50
	PHON_RULE_OFFSET=0
	RULE_NUM_BASE=50
	rule_num=0
# Set when invoking awk
#	RULE_FILE="rule.lst"
#	RULE_FILE=""
	if (RULE_FILE != "")
	   print "" > RULE_FILE
	printf("(")
	}


function map_right_syms(sym_str)
{
cur_mapping=""
cur_aloc=0
for (i=1; i <= length(sym_str); i++)
   {
   cur_sym=substr(sym_str, i, 1)
   if (right_sym[cur_sym] == "")
      print "Unknown symbol in rule, " rule_num ": " sym_str
   cur_aloc+=aloc[cur_sym]
   cur_mapping=cur_mapping right_sym[cur_sym]
   }

#print sym_str " _ " cur_mapping
return cur_mapping
}


function map_fixed_left_syms(sym_str)
{
# Build two regexes from the same set of left symbols
# One that has a fixed number of character backtrack
# and the other the variable number of character backtrack.
#
cur_mapping=""
left_context_fixed=1
if (length(sym_str) >= 1)
   {
   cur_sym=substr(sym_str, 1, 1)
   if (fixed_1st[cur_sym] == "")
      print "Unknown symbol in rule: " sym_str
   if (fixed_1st[cur_sym] == NOT_FIXED)
      left_context_fixed=0
   else
      cur_mapping=fixed_1st[cur_sym]
   }
for (i=2; i <= length(sym_str); i++)
   {
   cur_sym=substr(sym_str, i, 1)
   if (fixed_any[cur_sym] == "")
      print "Unknown symbol in rule: " sym_str
   if (fixed_any[cur_sym] == NOT_FIXED)
      left_context_fixed=0
   else
      cur_mapping=cur_mapping fixed_any[cur_sym]
   }

if (left_context_fixed == 1)
   {
   left_let_callout_num=0
   return cur_mapping
   }

if (callout_num[sym_str] != 0)
   {
   left_let_callout_num=callout_num[sym_str]
   return ""
   }

cur_callout_pat=""
for (i=length(sym_str); i > 0; i--)
   {
   cur_sym=substr(sym_str, i, 1)
   if (cur_sym == " ")
      cur_callout_pat=cur_callout_pat "$"
   else
      cur_callout_pat=cur_callout_pat right_sym[cur_sym]
   }
num_callout_pat++
callout_num[sym_str]=num_callout_pat
callout_pat[num_callout_pat]=cur_callout_pat
left_let_callout_num=num_callout_pat

return ""
}


function callout_str(r_num, c_offset)
{
# pcre only supports callout numbers up to 255
# so we use a base 50 numbering scheme coupled with
# upto two callout invocations.
if (r_num < RULE_NUM_BASE)
   return "(?C" c_offset+r_num ")"
else
   return "(?C" c_offset+RULE_NUM_BASE+int(r_num/RULE_NUM_BASE) ")(?C" \
					c_offset+r_num%RULE_NUM_BASE ")"
}


function map_all_left_syms(sym_str)
{
# Build a regex for all of the left context.
# Used by phoneme -> letter mapping
if (phon_callout_num[sym_str] != 0)
   {
   return phon_callout_num[sym_str]
   }

cur_mapping=""
for (i=length(sym_str); i > 0; i--)
   {
   cur_sym=substr(sym_str, i, 1)
   cur_mapping=cur_mapping right_sym[cur_sym]
   }

num_phon_callout_pat++
phon_callout_num[sym_str]=num_phon_callout_pat
phon_callout_pat[num_phon_callout_pat]=cur_mapping
return num_phon_callout_pat
}


function map_phon_seq(phon_str)
{
# Map phoneme symbols, which might contain more than one
# character, to a single non-regex character.
n=split(phon_str, phon_list, " ")
cur_phon_list=""
for (i=1; i <= n; i++)
   {
   if (map_phon[phon_list[i]] == "")
      {
      num_phon_sym++
      map_phon[phon_list[i]]=substr(one_phon, num_phon_sym, 1)
      }
   cur_phon_list=cur_phon_list map_phon[phon_list[i]]
   }
return cur_phon_list 
}

$0 == "" {
	next
	}

$1 == "#" {  # skip comments
	next
	}

	{
	rule_num++
	if (RULE_FILE != "")
	   printf("%3d: %s\n", rule_num, $0) >> RULE_FILE
# Lines have the format:  left[charseq]right=/phonemes/
# left/right are optional.
	n=split($0, left_right, "=")
	if (n!= 2)
	   {
	   print "Missing = in rule, " rule_num ": " $0
	   next
	   }
	letter_str=left_right[1]
	phon_str=left_right[2]

# Build regex for letter to phoneme mapping
# Process left context
	n=split(letter_str, left_context, "[")
	if (n!= 2)
	   {
	   print "Missing [ in rule:" $0
	   next
	   }
	left_pat=map_fixed_left_syms(left_context[1])
	rule_left_pat[rule_num]=map_all_left_syms(left_context[1])
	if (left_pat != "")
	   cur_pat="(?<=" left_pat ")"  # a backwards assertion
	else
	   cur_pat=""
	cur_pat=cur_pat "\\K" # needed so start_match has desired value
# Everthing between [ and ]
	n=split(left_context[2], right_context, "]")
	if (n!= 2)
	   {
	   print "Missing ] in rule:" $0
	   next
	   }
	rule_letter_seq[rule_num]=right_context[1]
	cur_pat=cur_pat "(" rule_letter_seq[rule_num]
	cur_matching_letters=left_context[1] cur_pat

# Process right context
	right_pat=map_right_syms(right_context[2])
	rule_right_pat[rule_num]=right_pat
	if (right_pat != "")
	   cur_pat=cur_pat "(?=" right_pat ")" # a forwards assertion
# Put callouts last so everything has to match before we call them
	if (left_let_callout_num != 0)
	   cur_pat=cur_pat "(?C" left_let_callout_num ")"
	rule_num_callout_str=callout_str(rule_num, LET_RULE_OFFSET)
	cur_pat=cur_pat rule_num_callout_str ")"
	if (rule_num > 1)
	   printf("|")
	if ((left_pat != "") || (right_pat != ""))
	   printf("(%s)", cur_pat)
	else
	   printf("%s", cur_pat)
# Save phoneme sequence
	n=split(phon_str, phon_seq, "/")
	if (n != 3)
	   print "Missing delimiters in phoneme sequence: " $0
	phoneme_list[rule_num]=map_phon_seq(phon_seq[2])

# Build regex for phoneme to letter mapping
	rule_num_callout_str=callout_str(rule_num, PHON_RULE_OFFSET)
	if (phoneme_list[rule_num] == "")
	   cur_phon_pat="999" # can never match
	else
	   cur_phon_pat=phoneme_list[rule_num]
# We can deduce some phoneme pattern right context from the
# letter sequence right context (the following is a small subset)
	if (cur_aloc > 0)
	   {
	   cur_phon_pat=cur_phon_pat "(?=.+)"
	   cur_matching_letters=cur_matching_letters "(?=.+)"
	   }
	if (right_pat == "$")
	   {
	   cur_phon_pat=cur_phon_pat "$"
	   cur_matching_letters=cur_matching_letters "$"
	   }
# Some letter patterns that map to the same phoneme sequence are
# only distinguished by right context.
# No point in generating more than one of these for phoneme -> letter
# conversion.
	cur_matching_letters=cur_matching_letters cur_phon_pat
	if (seen_phon_pat[cur_matching_letters] == 1)
	   next
	seen_phon_pat[cur_matching_letters]=1
	cur_phon_pat=cur_phon_pat rule_num_callout_str
	if (rule_num > 1)
	   phon_pat=phon_pat "|\\K" cur_phon_pat
	else
	   phon_pat="\\K" cur_phon_pat
	next
	}

END {
	print ")+"
	print "(" phon_pat ")+(?C255)"
	print num_callout_pat
	for (i=1; i <= num_callout_pat; i++)
	   print callout_pat[i]

	print num_phon_sym
	for (p in map_phon)
	   if (p != 999)
	      print p " " map_phon[p]
	print rule_num
	for (i=1; i <= rule_num; i++)
	   print phoneme_list[i]
	print num_phon_callout_pat
	for (i=1; i <= num_phon_callout_pat; i++)
	   print phon_callout_pat[i]
	for (i=1; i <= rule_num; i++)
	   print rule_left_pat[i]
	for (i=1; i <= rule_num; i++)
	   print rule_letter_seq[i]
	for (i=1; i <= rule_num; i++)
	   print rule_right_pat[i]
	if (RULE_FILE != "")
	   close(RULE_FILE)
	}

