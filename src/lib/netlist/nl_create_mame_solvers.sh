#!/bin/sh

GENERATED=src/lib/netlist/generated/static_solvers.cpp
FILES=`find src/mame -name "nl_*.cpp" | grep -v pongdoubles`

OUTDIR=/tmp/static_syms

if [ _$OS = "_Windows_NT" ]; then
	NLTOOL=./nltool.exe
else
	NLTOOL=./nltool
fi

rm -rf ${OUTDIR}
mkdir ${OUTDIR}

#--dir src/lib/netlist/generated/static --static-include

if ${NLTOOL} --cmd static --output=${GENERATED}.tmp --include=src/mame/shared ${FILES} ; then
	mv -f ${GENERATED}.tmp ${GENERATED}
	echo Created ${GENERATED} file
else
	echo Failed to create ${GENERATED}
fi
