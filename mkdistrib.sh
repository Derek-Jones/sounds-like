#
# mkdistrib.sh 27 Mar 12

SLIKE_DIR="slike.$1"

mkdir "$SLIKE_DIR"

cp -a *.c *.h accent.txt eng-phon.txt flashmemory letbigram.txt letphon.* Changelog Makefile *.awk mkdistrib.sh mkletphon.sh phonvoc.* README TODO slike "$SLIKE_DIR"

tar zcf "$SLIKE_DIR".tgz "$SLIKE_DIR"
