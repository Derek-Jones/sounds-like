#
# mkpvoc.awk, 14 Mar 12
#
# Map phoneme distintive features to numeric value

BEGIN {
	num_features=0
	num_phonemes=0
	max_voc_val=0
	voc2hex[" "]=0
	}

$0 == "" {
	next
	}

$1 == "#" {  # skip comments
	next
	}

function map_voc(voc_str)
{
if (voc2hex[voc_str] == "")
   {
   num_features++
   if (max_voc_val == 0)
      max_voc_val=1
   else
      max_voc_val=max_voc_val*2
   voc2hex[voc_str]=max_voc_val
   }
return voc2hex[voc_str]
}


	{
	num_phonemes++
	num_field=split($0, voc, " ")
	phon_str=voc[1]
	phon_val=0
	for (i=2; i <= num_field; i++)
	   phon_val=phon_val+map_voc(voc[i])
	phon_str_val[phon_str]=phon_val	
	next
	}

END {
	print num_phonemes
	for (p in phon_str_val)
	   printf("%s  0x%x\n", p, phon_str_val[p])
	print num_features
	for (m in voc2hex)
	   if (m != " ")
	      printf("%s  0x%x\n", m, voc2hex[m])
	}
