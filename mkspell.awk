#
# mkspell.awk, 15 Mar 12

BEGIN {
	}

$0 == "" {
	next
	}

$1 == "#" {  # skip comments
	next
	}

	{
	n=split($0, freq, ",")
	if (n != 27)
	   print "Only seen " n " values in:" $0
	if (freq[2] == "a")
	   next
	for (i=2; i <=27; i++)
	   printf("%4.2f ", freq[i])
	printf("\n")
	next
	}
