#!/bin/sh

cmdline=
nosp=0
#set -evx
for j in $*
do
	i=`echo $j | sed -e 's_/_\\\_g' `
	case "$i" in
	-l*)		cmdline="$cmdline ${i#-l}.lib"; 		;;
	-o*)		cmdline="$cmdline /out:${i#-o}";		;;
	-Wl,-Map,*)	cmdline="$cmdline /map:${i#-Wl,-Map,}";	;;
	-Wl,--allow-multiple-definition)
				cmdline="$cmdline /force:multiple";		;;
	-Wl,--warn-common)	
				true;									;;
	-mno-cygwin)
				true;									;;
	-s)			true;									;;
	-WO)		true;									;;
	-W*)		true;									;;
	-mconsole)	cmdline="$cmdline /subsystem:console";	;;
	-mwindows)	cmdline="$cmdline /subsystem:windows";	;;
	-shared)	cmdline="$cmdline /dll";				;;
	*)
		if [ $nosp = 0 ]
		then
			cmdline="$cmdline $i" 
		else
			cmdline="${cmdline}$i" 
		fi
		nosp=0
		;;
	esac
done
echo link /nologo debug $cmdline  >> t.cmd

