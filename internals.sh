#!/bin/bash

set -e

# Config parameters, for where to find stuff
XTASK_DIR="${XTASK_DIR:-${HOME}/xtask}"
SWIFT_DIR="${SWIFT_DIR:-${HOME}/swift-t-install}"

# Test params
MAXW=`lscpu | grep '^CPU(s)' | awk '{print $2}'`
RUNS=1
declare -A tests=([jigstack]=1 [openmp]=1 [single]=1 [swiftt]=1 [oneatom]=1 \
	[atomstack]=1 [cilk]=1)
while getopts r:w:x:Xo:Sc:C: o; do
	case "${o}" in
	w) MAXW=${OPTARG} ;;
	r) RUNS=${OPTARG} ;;
	x) unset tests["$OPTARG"] ;;
	o) tests["$OPTARG"]=1 ;;
	X) unset tests[*]; declare -A tests=( ) ;;

	c) CFLAGS="${CFLAGS} -${OPTARG}" ;;
	C) CC="${OPTARG}" ;;
	s) STCFLAGS="${STCFLAGS} -${OPTARG}" ;;
	S) DISABLE_STC=1 ;;
	esac
done
echo "Using runtimes: ${!tests[*]}"

# Setup the environ
export TURBINE_USER_LIB=$SWIFT_DIR/turbine/lib
export LD_LIBRARY_PATH=$SWIFT_DIR/lb/lib:$SWIFT_DIR/c-utils/lib:$HOME

# Function to compile the different versions of a test
if ! [[ -d bin ]]; then mkdir bin; fi
echo "Compiling C with \${CC} ${CFLAGS:=-O3}"
if [[ -z $DISABLE_STC ]]
then echo "Compiling Swift with \${STC} ${STCFLAGS:=-O3}"
fi

# The timer wrapper
clang $CFLAGS -std=gnu99 time.c -o bin/time
function comp() {
	# This isn't actually an imp, it counts tasks. Its... slow.
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.counter -pthread -L$XTASK_DIR -lxtask-counter -lm

	# The different XTask implementations
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.jigstack -pthread -L$XTASK_DIR -lxtask-jigstack -lm
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.oneatom -pthread -L$XTASK_DIR -lxtask-oneatom -lm
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.atomstack -pthread -L$XTASK_DIR -lxtask-atomstack -lm

	# The different XTask implementations, but using XData
	if [[ -a tests/$1.xd.c ]]; then
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.xd.c \
		-o bin/$1.jigstackxd -pthread -L$XTASK_DIR -lxtask-jigstack -lm
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.xd.c \
		-o bin/$1.oneatomxd -pthread -L$XTASK_DIR -lxtask-oneatom -lm
	clang $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.xd.c \
		-o bin/$1.atomstackxd -pthread -L$XTASK_DIR -lxtask-atomstack -lm
	fi

	# OpenMP, baseline parallel competitor
	clang $CFLAGS -std=gnu99 -DUSE_openmp tests/$1.c -o bin/$1.openmp \
		-fopenmp -lm

	# Cilk, like OpenMP but Intel-grown.
	gcc $CFLAGS -std=gnu99 -DUSE_cilk tests/$1.c -o bin/$1.cilk -fcilkplus -lm

	# Single-threaded, currently best-case.
	clang $CFLAGS -std=gnu99 -DUSE_single -o bin/$1.single tests/$1.c -lm

	# Swift/T. The tortise.
	if [[ -z "$DISABLE_STC" ]]
	then $SWIFT_DIR/stc/bin/stc $STCFLAGS tests/$1.swift bin/$1.tic \
		|| echo STC failed, ignoring...
	fi
}

# Function to run a test
if ! [[ -d out ]]; then mkdir out; fi
function run() {
	C="$2"
	T="$1"
	shift 2
	if [[ ${tests["$T"]} ]]; then
		if [[ ${#T} -le 5 ]]; then AL="\\t\\t"; else AL="\\t"; fi
		echo -en "[$T]${AL}Counting for test $C..."
		args=( "${@/\{\}/1}" )
		TCNT=`bin/"$C".counter "${args[@]/=/}" |& grep '^TASKCOUNT' \
			| awk '{print $2}'`
		echo " $TCNT tasks"

		CMD=bin/"$C.$T"
		MINW=1
		if [[ $T = swiftt ]]; then
			CMD="$SWIFT_DIR/turbine/bin/turbine -n {} bin/$C.tic"
			MINW=2
		fi

		echo -en "[$T]${AL}     Running test $C..."
		ERR=
		for n in `seq $MINW $MAXW`; do
			for r in `seq 1 $RUNS`; do
				echo "${CMD/\{\}/$n} ${@/\{\}/$n}" >&2
				echo -n "$n "
				bin/time ${CMD/\{\}/$n} "${@/\{\}/$n}" \
					|| echo ERR
				echo >&2
			done
		done 2> out/tmp.log | lua data.lua "$TCNT" > out/"$C-$T".dat | :
		X=${PIPESTATUS[1]}
		if [[ $X -ne 0 ]]; then
			echo "Error while running test (code $X):"
			cat out/tmp.log
			rm out/tmp.log
			exit 1
		else rm out/tmp.log; fi
	fi
}
