#!/bin/bash

set -e

# Config parameters, for where to find stuff
XTASK_DIR="${XTASK_DIR:-${HOME}/xtask}"
SWIFT_DIR="${SWIFT_DIR:-${HOME}/swift-t-install}"

# Test params
MAXW=
FIB_POWER=30
while getopts f:w: o; do
	case "${o}" in
	f)
		FIB_POWER=${OPTARG}
		;;
	w)
		MAXW=${OPTARG}
		;;
	esac
done

# Setup the environ
export TURBINE_USER_LIB=$SWIFT_DIR/turbine/lib
export LD_LIBRARY_PATH=$SWIFT_DIR/lb/lib:$SWIFT_DIR/c-utils/lib:$HOME
export MAXW

# Compile the XTask tests
gcc -O2 -o fib -I"$XTASK_DIR" tests/fib.c -L"$XTASK_DIR" -lxtask -pthread

# Run the tests
mkdir out
echo "XTask on Fib($FIB_POWER):"
./run.sh ./fib -w{} -f${FIB_POWER} | tee out/fib-xtask.dat
echo "Swift on Fib($FIB_POWER):"
./run.sh $SWIFT_DIR/turbine/bin/turbine -n {} tests/fib.tic -arg=${FIB_POWER} \
	| tee out/fib-swift.dat

# Cleanup
rm fib
