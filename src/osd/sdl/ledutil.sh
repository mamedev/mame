#!/bin/sh
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert, R. Belmont
# ============================================================
# 
#   ledutil.sh - Example script for output notifiers
# 
#   This is a very basic implementation which
#
#   a) Sets kbd leds if led0, led1, led2 is received
#   b) Beeps if led0 is set to on (state=1)
#   c) Writes a message when pause is received
# 
#   use "sh ledutil.sh -h" to get more information
#
# ============================================================

SDLMAME_OUTPUT=/tmp/sdlmame_out

verbose=0
autoclose=0
myname=`basename $0`
paused=0

while [ $# -gt 0 ]; do
	case $1 in
		-v)
		  	verbose=1
			;;
		-a)
			autoclose=1
			;;
		-h)
			echo "Usage: $myname [-a] [-v]"
			echo ""
			echo "  -a   Automatically close when sdlmame ends game" 
			echo "  -v   LOG all messages received"
			echo "  -h   Get help"
			echo ""
			exit
			;; 
		*)
			echo "$myname: invalid option $1"
			echo "Try \`$myname -h' for more information."
			exit
			;;
	esac
	shift
done

if [ ! -e ${SDLMAME_OUTPUT} ]; then
  mkfifo ${SDLMAME_OUTPUT}
fi

while true; do
	cat  ${SDLMAME_OUTPUT} | while read class pidnum what state; do
		[ $verbose = 1 ] && echo LOG: $class $pidnum $what $state
		if [ "$class" = "MAME" ]; then
			case "$what" in
				START)
					echo Process $pidnum starting game $state
					paused=0
					;;
				STOP)
					echo Process $pidnum stopping game $state
					;;
			esac
		fi
		if [ "$class" = "OUT" ]; then
			case "$what" in
				led0)
					[ "$state" = 1 ] && beep
					[ "$state" = 1 ] && xset led 1
					[ "$state" = 0 ] && xset -led 1
					;;
				led1)
					[ "$state" = 1 ] && xset led 2
					[ "$state" = 0 ] && xset -led 2
					;;
				led2)
					[ "$state" = 1 ] && xset led 3
					[ "$state" = 0 ] && xset -led 3
					;;
				pause)
					paused=$state
					echo Pause $paused!
					;;
			esac
		fi	
	done
	[ $autoclose = 1 ] && break;
done

rm -f ${SDLMAME_OUTPUT}

