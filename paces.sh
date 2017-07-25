
#!/bin/bash

declare -A tests=(
	[cilk]=1 [openmp]=1 [single]=1
	[jigstack]=1   #[oneatom]=   [atomstack]=
	[jigstackxd]=1 #[oneatomxd]= [atomstackxd]=
	[swiftt]=1)

. internals.sh

comp		fib
run single	fib -f42
run openmp	fib -f31  -w{}
run cilk	fib -f32  -w{}
run jigstack	fib -f31  -w{}
run jigstackxd	fib -f31  -w{}
run oneatom	fib -f31  -w{}
run oneatomxd	fib -f31  -w{}
run atomstack	fib -f31  -w{}
run atomstackxd	fib -f31  -w{}
run swiftt	fib -f=21

comp		nap
run single	nap -s50000000
run openmp	nap -s50000000 -w{}
run cilk	nap -s70000000 -w{}
run jigstack	nap -s7000000  -w{}
run jigstackxd	nap -s7000000  -w{}
run oneatom	nap -s7000000  -w{}
run oneatomxd	nap -s7000000  -w{}
run atomstack	nap -s7000000  -w{}
run atomstackxd	nap -s7000000  -w{}
run swiftt	nap -s=100000

comp		matrix
run single	matrix -n6000
run openmp	matrix -n6000 -w{}
run cilk	matrix -n6000 -w{}
run jigstack	matrix -n2000 -w{}
run oneatom	matrix -n2500 -w{}
run atomstack	matrix -n2500 -w{}
run swiftt	matrix -n=1000

comp		qsort
run single	qsort -n10000000
run openmp	qsort -n10000000 -w{}
run cilk	qsort -n10000000 -w{}
run jigstack	qsort -n5000000 -w{}
run oneatom	qsort -n5000000 -w{}
run atomstack	qsort -n5000000 -w{}

comp mdlite
run single	mdlite 3 200 100 0.1
run jigstack	mdlite 3 200 100 0.1 {}
run oneatom	mdlite 3 200 100 0.1 {}
run atomstack	mdlite 3 200 100 0.1 {}
