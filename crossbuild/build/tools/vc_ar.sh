#!/bin/sh

cmdline=
nosp=1
#set -evx
for j in $*
do
	i=`echo $j | sed -e 's_/_\\\_g' `
	case "$i" in
	-cr)
				true;									;;
	*)
		if [ $nosp = 0 ]
		then
			cmdline="$cmdline $i" 
		else
			cmdline="${cmdline} /out:$i" 
		fi
		nosp=0
		;;
	esac
done
echo lib /lib /nologo $cmdline  >> t.cmd


