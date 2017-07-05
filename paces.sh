#!/bin/bash

set -e

# Config parameters, for where to find stuff
XTASK_DIR="${XTASK_DIR:-${HOME}/xtask}"
SWIFT_DIR="${SWIFT_DIR:-${HOME}/swift-t-install}"

# Test params
MAXW=
FIB_POWER=30
RUNS=
NAP_POWER=7000000
DISABLE_SWIFT=
while getopts xr:w:f:n: o; do
	case "${o}" in
	x)
		DISABLE_SWIFT=yes
		;;
	f)
		FIB_POWER=${OPTARG}
		;;
	n)
		NAP_POWER=${OPTARG}
		;;
	w)
		MAXW=${OPTARG}
		;;
	r)
		RUNS=${OPTARG}
		;;
	esac
done

# Setup the environ
export TURBINE_USER_LIB=$SWIFT_DIR/turbine/lib
export LD_LIBRARY_PATH=$SWIFT_DIR/lb/lib:$SWIFT_DIR/c-utils/lib:$HOME
export MAXW
export RUNS

# Compile the XTask tests
if ! [[ -d bin ]]; then mkdir bin; fi
echo "Compiling C with ${CC:=gcc} ${CFLAGS:=-O2}"
echo "and Swift with .../stc ${STCFLAGS}"
function comp() {
	$CC $CFLAGS -std=gnu99 -DUSE_openmp -I$XTASK_DIR tests/$1.c \
		-o bin/$1.openmp -fopenmp
	$CC $CFLAGS -std=gnu99 -DUSE_xtask -I$XTASK_DIR tests/$1.c \
		-o bin/$1.jigstack -pthread -L$XTASK_DIR -lxtask-jigstack
#	$CC $CFLAGS -std=gnu99 -DUSE=xtask -I$XTASK_DIR tests/$1.c \
#		-o bin/$1.basicstack -pthread -L$XTASK_DIR -lxtask-basicstack
	$CC $CFLAGS -std=gnu99 -DUSE_single -o bin/$1.single tests/$1.c
	if [[ -z "${DISABLE_SWIFT}" ]]; then
	$SWIFT_DIR/stc/bin/stc $STCFLAGS tests/$1.swift bin/$1.tic
	fi
}
comp fib
comp nap
#comp matrix

# Run the tests
if ! [[ -d out ]]; then mkdir out; fi

function run() {
	X="$1"
	shift 1
	MS=8
	for T in single openmp jigstack; do
		echo -n "Running $X($@) using $T"
		printf '.%.0s' `seq 1 $(($MS-${#T}+1))`
		./run.sh bin/$X.$T "$@" | tee out/$X-$T.dat | while read l
		do echo -n .; done
		echo
	done
	if [[ -z "${DISABLE_SWIFT}" ]]; then
		echo -n "Running $X($@) using Swift"
		printf '.%.0s' `seq 1 $(($MS-5+2))`
		ARGS=""
		for a in "$@"; do
			if [[ -z "${a:2}" ]]
			then ARGS="$ARGS -${a:1:1}"
			else ARGS="$ARGS -${a:1:1}=${a:2}"
			fi
		done
		./run.sh $SWIFT_DIR/turbine/bin/turbine -n {} bin/$X.tic $ARGS \
			| tee out/$X-swift.dat | while read l; do echo -n .; done
		echo
	fi
}

run fib -f$FIB_POWER -w{}
run nap -s$NAP_POWER -w{}
