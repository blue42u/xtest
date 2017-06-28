#!/bin/bash

set -e

MAXW=$(lscpu | grep '^CPU(s)' | awk '{print $2}')
MAXW=$(($MAXW + $MAXW / 2))

for n in $(seq 1 $MAXW); do
	i_args=( "$@" )
	o_args=( )
	for arg in "${i_args[@]}"; do
		arg="${arg/\{\}/$n}"
		o_args+=( "${arg}" )
	done

	echo
	echo Cores $n
	time -p "${o_args[@]}" &> /dev/null || echo Failed
done |& lua data.lua