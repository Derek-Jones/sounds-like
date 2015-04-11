Read words from standard input and output one or more words that sound like each
input word.

Unless otherwise stated English is assumed for people and words.

Details in the following blog post:
http://shape-of-code.coding-guidelines.com/2012/03/16/generating-sounds-like-and-accented-words/

Building

Type make to build

A recent version of the PERL regular expression library, pcre, is required.


Running

The file: letphon.info needs to exist in the same directory as the
slike executable.  It contains data read on program startup.


Options

-accent language

Generate words that sound as if they are spoken by a native speaker
of 'language' who had a strong accent.  At the moment german is the
only supported language.  Default: none

-generate phoneme+/-

Generate phoneme sequence rather than sounding-alike words


-help

Print out a list of options and their default value.


-print phonemes+/-

Print the phoneme sequence generated from the input word.


-filter letter

Control whether letter combination checking is used to filter
generated letter sequences.  Default -filter letter+


-phoneme

Generate words that whose phoneme sequence matches that as input.
The input phoname sequence is a list of whitespace separated
phonemes (represented using arpabet).  The existing rules do
not guarantee that a latter sequence exists for all phoneme sequences.

The letter bigram filter may prevent any letter sequences being
returned.  See -filer option.

Default phon-


-print rules

Print the letter/phoneme rule number that matched and generated
a given phoneme.


-rhyme number

Generate words that rhyme with the input word.  Works the same
as -sound same+ except that the last 'number' of phonemes are
never modified.  Default -rhyme 0


-sound like+/-

Generate words that sound like the input word.  Default: -sound like+


-sound same+/-

Generate words that sound the same as the input word.  Default: -sound same-


-spell dist

dist is a floating-point value used as the limit to filter out
letter sequences containing bigrams whose frequency is less than dist.
Increase this value to decrease the output generated for long input words.
Anything more than 20.0 will result in almost all possibilities being rejected.
Default: 1.0


-trace letchk+/-

Switch on the trracing of letter sequences that failed a 'spell check'
Default: letchk-


Examples

echo "glass" | slike

echo "derek" | slike -sound same+

echo "we love lucy" | slike -sound same+

echo "woven" | slike -accent german

echo "derek" | slike -spell 10.0



Bugs and suggestions

Send bug reports and suggestions for improvements to:

slike@knosof.co.uk

