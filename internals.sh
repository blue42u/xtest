#!/bin/bash

set -e

# Config parameters, for where to find stuff
XTASK_DIR="${XTASK_DIR:-${HOME}/xtask}"
SWIFT_DIR="${SWIFT_DIR:-${HOME}/swift-t-install}"

# Test params
MAXW=`lscpu | grep '^CPU(s)' | awk '{print $2}'`
RUNS=1
declare -A tests=([jigstack]=1 [openmp]=1 [single]=1 [swiftt]=1 [oneatom]=1)
while getopts r:w:x:Xo: o; do
	case "${o}" in
	w) MAXW=${OPTARG} ;;
	r) RUNS=${OPTARG} ;;
	x) unset tests["$OPTARG"] ;;
	o) tests["$OPTARG"]=1 ;;
	X) unset tests[*]; declare -A tests=( ) ;;
	esac
done
echo "Using runtimes: ${!tests[*]}"

# Setup the environ
export TURBINE_USER_LIB=$SWIFT_DIR/turbine/lib
export LD_LIBRARY_PATH=$SWIFT_DIR/lb/lib:$SWIFT_DIR/c-utils/lib:$HOME

# Function to compile the different versions of a test
if ! [[ -d bin ]]; then mkdir bin; fi
echo "Compiling C with ${CC:=clang} ${CFLAGS:=-O3}"
echo "Compiling Swift with .../stc ${STCFLAGS:=-O3}"
function comp() {
	# This isn't actually an imp, it counts tasks. Its... slow.
	$CC $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.counter -pthread -L$XTASK_DIR -lxtask-counter

	# The different XTask implementations
	$CC $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.jigstack -pthread -L$XTASK_DIR -lxtask-jigstack
	$CC $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.oneatom -pthread -L$XTASK_DIR -lxtask-oneatom

	# OpenMP, baseline parallel competitor
	$CC $CFLAGS -std=gnu99 -DUSE_openmp -I$XTASK_DIR tests/$1.c \
		-o bin/$1.openmp -fopenmp

	# Single-threaded, currently best-case.
	$CC $CFLAGS -std=gnu99 -DUSE_single -o bin/$1.single tests/$1.c

	# Swift/T. The tortise.
	$SWIFT_DIR/stc/bin/stc $STCFLAGS tests/$1.swift bin/$1.tic \
		|| echo STC failed, ignoring...
}

# Function to run a test
if ! [[ -d out ]]; then mkdir out; fi
function run() {
	C="$2"
	T="$1"
	shift 2
	if [[ ${tests["$T"]} ]]; then
		echo -en "[$T]\tCounting for test $C... "
		args=( "${@/\{\}/1}" )
		TCNT=`bin/"$C".counter "${args[@]/=/}" |& grep '^TASKCOUNT' \
			| awk '{print $2}'`
		echo "$TCNT tasks"

		CMD=bin/"$C.$T"
		if [[ $T = swiftt ]]
		then CMD="$SWIFT_DIR/turbine/bin/turbine -n {} bin/$C.tic"
		fi

		echo -en "[$T]\t     Running test $C... "
		for n in `seq 1 $MAXW`; do
			for r in `seq 1 $RUNS`; do
				echo
				echo Cores $n
				time -p ${CMD/\{\}/$n} "${@/\{\}/$n}" \
					&> /dev/null || echo Failed
			done
		done |& lua data.lua "$TCNT" > out/"$C-$T".dat
		echo
	fi
}
