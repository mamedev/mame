<?xml version="1.0" encoding="UTF-8"?>
<!--
Prints a table of runnable machines, excluding non-runnable devices.

Useful for finding out exactly what systems are available in a custom
subtarget build using a small selection of driver sources.

$ ./build/linux_clang/bin/x64/Debug/test64d -listxml | xsltproc scripts/xslt/list-runnable.xslt -
bluehawk         Blue Hawk
bluehawkn        Blue Hawk (NTC)
ddaydoo          Chulgyeok D-Day (Korea)
flytiger         Flying Tiger (set 1)
flytigera        Flying Tiger (set 2)
gotya            Got-Ya (12/24/1981)
gulfstrm         Gulf Storm (set 1)
gulfstrma        Gulf Storm (set 2)
gulfstrmb        Gulf Storm (set 3)
gulfstrmk        Gulf Storm (Korea)
gulfstrmm        Gulf Storm (Media Shoji)
gundl94          Gun Dealer '94
intlc44          INTELLEC 4/MOD 4
intlc440         INTELLEC 4/MOD 40
lastday          The Last Day (set 1)
lastdaya         The Last Day (set 2)
pollux           Pollux (set 1)
polluxa          Pollux (set 2)
polluxa2         Pollux (set 3)
polluxn          Pollux (Japan, NTC license, distributed by Atlus)
popbingo         Pop Bingo
primella         Primella
rshark           R-Shark
sadari           Sadari
superx           Super-X (NTC)
superxm          Super-X (Mitchell)
thehand          The Hand
whousetc         Test Console Serial #5
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="text" omit-xml-declaration="yes" indent="no" />
	<xsl:template match="/">
		<xsl:for-each select="mame/machine[@runnable!='no']">
			<xsl:value-of select="concat(substring(concat(@name, '               '), 1, 16), ' ' , description, '&#xA;')" />
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>
