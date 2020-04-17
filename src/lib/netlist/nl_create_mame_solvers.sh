
OUTDIR=/tmp/static_sysms
GENERATED=src/lib/netlist/generated/static_solvers.cpp

if [ _$OS = "_Windows_NT" ]; then
	NLTOOL=./nltool.exe
	CXX=g++.exe
else
	NLTOOL=./nltool
	CXX=g++
fi

rm -rf $OUTDIR
mkdir $OUTDIR

for i in src/mame/machine/nl_*.cpp src/mame/audio/nl_*.cpp; do
	nn=`basename $i .cpp | sed -e "s/nl_//g"`
	cn=`grep "^//NL_CONTAINS" $i | cut -f2-99 "-d "`
	if [ _"$cn" != _"" ]; then
		nn=$cn
	fi
	for j in $nn; do
		echo $i : $j
		if [ $j != "pongdoubles" ]; then
			$NLTOOL  -c static -f $i -n $j -t 6  --dir $OUTDIR 
		fi
	done
done

# $CXX -shared -x c++ -fPIC -O3 -march=native -mtune=native -ffast-math $OUTDIR/*.c -o nlboost.so

echo '#include "plib/pdynlib.h"' > $GENERATED
cat $OUTDIR/*.c | sed -e 's/extern "C"/static/' >> $GENERATED
echo 'plib::dynlib_static_sym nl_static_syms[] = {'  >> $GENERATED
for i in $OUTDIR/*.c; do
	n=`basename $i .c`
	echo '{ "'${n}'", reinterpret_cast<void *>(&'${n}')},' >> $GENERATED
done
echo '{"", nullptr}'  >> $GENERATED
echo '};'  >> $GENERATED
