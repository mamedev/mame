<?xml version="1.0" encoding="UTF-8"?>
<!--
Prints a tree of runnable machines, excluding non-runnable devices.

Clones are indented below parents.  May be slow for full builds.

$ ./build/linux_clang/bin/x64/Debug/test64d -listxml | xsltproc scripts/xslt/list-runnable-tree.xslt -
bluehawk         Blue Hawk
    bluehawkn        Blue Hawk (NTC)
flytiger         Flying Tiger (set 1)
    flytigera        Flying Tiger (set 2)
gulfstrm         Gulf Storm (set 1)
    gulfstrma        Gulf Storm (set 2)
    gulfstrmb        Gulf Storm (set 3)
    gulfstrmk        Gulf Storm (Korea)
    gulfstrmm        Gulf Storm (Media Shoji)
gundl94          Gun Dealer '94
    primella         Primella
intlc44          INTELLEC 4/MOD 4
intlc440         INTELLEC 4/MOD 40
lastday          The Last Day (set 1)
    ddaydoo          Chulgyeok D-Day (Korea)
    lastdaya         The Last Day (set 2)
pollux           Pollux (set 1)
    polluxa          Pollux (set 2)
    polluxa2         Pollux (set 3)
    polluxn          Pollux (Japan, NTC license, distributed by Atlus)
popbingo         Pop Bingo
rshark           R-Shark
sadari           Sadari
superx           Super-X (NTC)
    superxm          Super-X (Mitchell)
thehand          The Hand
    gotya            Got-Ya (12/24/1981)
whousetc         Test Console Serial #5
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="text" omit-xml-declaration="yes" indent="no" />
	<xsl:template match="/">
		<xsl:for-each select="mame/machine[(@runnable!='no') and not(@cloneof)]">
			<xsl:variable name="parent" select="@name" />
			<xsl:value-of select="concat(substring(concat(@name, '               '), 1, 16), ' ' , description, '&#xA;')" />
			<xsl:for-each select="../machine[(@runnable!='no') and (@cloneof=$parent)]">
				<xsl:value-of select="concat('    ', substring(concat(@name, '               '), 1, 16), ' ' , description, '&#xA;')" />
			</xsl:for-each>
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>
