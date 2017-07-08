#!/bin/bash

. internals.sh

# Compile it all
comp fib
comp nap
comp matrix

# Run it all
run single	fib -q -f42
run openmp	fib -q -f31  -w{}
run jigstack	fib -q -f31  -w{}
run oneatom	fib -q -f31  -w{}
run swiftt	fib -q -f=21

run single	nap -s50000000
run openmp	nap -s50000000 -w{}
run jigstack	nap -s7000000  -w{}
run oneatom	nap -s7000000  -w{}
run swiftt	nap -s=100000

run single	matrix -n6000
run openmp	matrix -n6000 -w{}
run jigstack	matrix -n2000 -w{}
run oneatom	matrix -n2500 -w{}
run swiftt	matrix -n=1000
