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
while getopts r:w:f:n: o; do
	case "${o}" in
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
function comp() {
	gcc -O2 -std=gnu99 -o $1 -I"$XTASK_DIR" tests/$1.c \
		-L"$XTASK_DIR" -lxtask -pthread
}
comp fib
comp fib.single
comp nap
comp nap.single

# Run the tests
if ! [[ -d out ]]; then mkdir out; fi

echo "Single on Fib($FIB_POWER):"
./run.sh ./fib.single ${FIB_POWER} | tee out/fib-single.dat
echo "XTask on Fib($FIB_POWER):"
./run.sh ./fib -w{} -f${FIB_POWER} | tee out/fib-xtask.dat
echo "Swift on Fib($FIB_POWER):"
./run.sh $SWIFT_DIR/turbine/bin/turbine -n {} tests/fib.tic -arg=${FIB_POWER} \
	| tee out/fib-swift.dat

echo "Single on Nap($NAP_POWER):"
./run.sh ./nap.single -s${NAP_POWER} | tee out/nap-single.dat
echo "XTask on Nap($NAP_POWER):"
./run.sh ./nap -w{} -s${NAP_POWER} | tee out/nap-xtask.dat
echo "Swift on Nap($NAP_POWER):"
./run.sh $SWIFT_DIR/turbine/bin/turbine -n {} tests/nap.tic -tasks=${NAP_POWER} \
	| tee out/nap-swift.dat


# Cleanup
rm fib fib.single nap nap.single
