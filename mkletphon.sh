#
# mkletphon.sh, 21 Mar 12

awk -f rule2reg.awk < eng-phon.txt
#awk -f mkpvoc.awk < phonvoc.hyman
awk -f mkpvoc.awk < phonvoc.brinton
awk -f mkspell.awk < letbigram.txt
awk -f mkaccent.awk < accent.txt

