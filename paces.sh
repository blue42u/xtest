#!/bin/bash

. internals.sh

# Compile it all
comp fib
comp nap
comp matrix

# Run it all
run single   fib -f42
run openmp   fib -f31  -w{}
run jigstack fib -f31  -w{}
run oneatom  fib -f31  -w{}
run swiftt   fib -f=21 -w={}

run single   nap -s50000000
run openmp   nap -s50000000 -w{}
run jigstack nap -s7000000  -w{}
run oneatom  nap -s7000000  -w{}
run swiftt   nap -s=100000 -w={}
