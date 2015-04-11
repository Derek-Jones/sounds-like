#
# mkaccent.awk, 16 Mar 12
#
# Language accent phoname mappings

BEGIN {
	num_mapping=0
	}

$0 == "" {
	next
	}

$1 == "#" {  # skip comments
	next
	}

NF == 1 {
	language=$0
	next
	}

	{
	if (NF != 2)
	   print "Expected two fields, saw: " $0
	num_mapping++
	mapfrom[num_mapping]=language " " $1 " " $2
	next
	}

END {
	print num_mapping
	for (i=1; i <= num_mapping; i++)
	   printf("%s\n", mapfrom[i])
	}
