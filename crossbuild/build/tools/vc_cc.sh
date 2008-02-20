#!/bin/sh

cmdline=
nosp=0
#set -evx
for j in $*
do
	i=`echo $j | sed -e 's_/_\\\_g' `
	case "$i" in
	-D*)		cmdline="$cmdline /D${i#-D}"; 			;;
	-U*)		cmdline="$cmdline /U${i#-U}";			;;
	-I*)		cmdline="$cmdline /I${i#-I}";			;;
	-o*)		cmdline="$cmdline /Fo${i#-o}";			nosp=1	;;
	-include*)	cmdline="$cmdline /FI${i#-include}";	nosp=1	;;
	-c)			cmdline="$cmdline /c${i#-c}";			;;
	#-E*)		cmdline="$cmdline /E${i#-E}";			;;
	-S*)		cmdline="$cmdline /Fa${i#-S}";			;;
	-O0)		cmdline="$cmdline /Od";					;;
	-O1)		cmdline="$cmdline /O2";					;;
	-O2)		cmdline="$cmdline /O2";					;;
	-O3)		cmdline="$cmdline /O2";					;;
	-Os)		cmdline="$cmdline /Od";					;;
	-g)			cmdline="$cmdline /Zi";					;;
	-fno-strict-aliasing)	
				true;									;;
	-fno-omit-frame-pointer)
				true;									;;
	-Werror)	cmdline="$cmdline /WX";					;;
	-Wall)		cmdline="$cmdline /W0";					;;
	-W*)			true;								;;
	-mwindows*)		true;								;;
	-std=gnu89*)	true;								;;
	-pipe*)			true;								;;
	-march=*)		true;								;;
	-DINLINE=static)		true;								;;
	__inline__)		true;								;;
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
#echo cl /nologo $cmdline >> t.cmd
echo $cmdline > t.opt
wine cmd /c build/tools/vc_env.cmd cl /nologo @t.opt


