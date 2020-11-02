#!/bin/sh

CHDMAN="/home/nathanh/github/mame/chdman"

if [ ! -d "$1" ] ; then
	cat << __EOS__
usage: gditest.sh <title>

where <title> is both the directory name and the filenames within the directory

the directory must contain both Redump .cue/.bin and Redump .gdi files

    Crazy Taxi (USA)/Crazy Taxi (USA).cue
    Crazy Taxi (USA)/Crazy Taxi (USA).gdi
    Crazy Taxi (USA)/Crazy Taxi (USA) (Track 1).bin
    Crazy Taxi (USA)/Crazy Taxi (USA) (Track 2).bin
    Crazy Taxi (USA)/Crazy Taxi (USA) (Track 3).bin

test script will create a CHD from both formats and compare the CHD checksums

if same the .cue conversion is *NO WORSE* than the existing chdman .gdi support

oh yeah, no error checking in this script, use at your own risk
__EOS__
	exit 0
fi

cd "$1"

$CHDMAN createcd -i "$1".gdi -o "${1}-gdi.chd"
$CHDMAN createcd -i "$1".cue -o "${1}-cue.chd"

GDISUM=$(md5sum "${1}-gdi.chd" | awk '{print $1}')
CUESUM=$(md5sum "${1}-cue.chd" | awk '{print $1}')

printf "%20s   %s\n" $GDISUM "${1}-gdi.chd"
printf "%20s   %s\n" $CUESUM "${1}-cue.chd"

if [ "$GDISUM" = "$CUESUM" ] ; then
	echo "PASS"
else
	echo "FAIL"
fi
