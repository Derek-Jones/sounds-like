# 21 Mar 12

# Force make on windows to use a useful shell
SHELL=sh

# Some directories
BIN_DIR=bin

CC=gcc
#CC=/usr1/expsrc/xgcc-4.0.2/objdir/gcc/xgcc -B/usr1/expsrc/xgcc-4.0.2/objdir/gcc/ -fbounds-checking -DVALGRIND

CFLAGS=-I/usr/local/include -Wall -DFULL_POSIX -g
LFLAGS=
LIBS=-L/usr/local/lib -lpcre

E_OBJ = lplmapping.o \
	lplinit.o \
	spellchk.o \
	accent.o \
	sndlike.o \
	wordmap.o

all: $(E_OBJ) letphon.config
	$(CC) $(LFLAGS) $(LIBS) -o slike $(E_OBJ)

letphon.config: eng-phon.txt phonvoc.brinton letbigram.txt accent.txt
	mkletphon.sh > letphon.config

clean:
	rm *.o

# Use make options=-MM to get gcc to generate a list of dependencies
.c.o:
	$(CC) -c $(CFLAGS) $(options) $<

lplmapping.o: lplmapping.c spellchk.h lplmapping.h
lplinit.o: lplinit.c lplmapping.h spellchk.h accent.h sndlike.h lplinit.h
spellchk.o: spellchk.c spellchk.h
accent.o: accent.c lplmapping.h accent.h
sndlike.o: sndlike.c lplmapping.h sndlike.h
wordmap.o: wordmap.c lplmapping.h spellchk.h accent.h sndlike.h lplinit.h
