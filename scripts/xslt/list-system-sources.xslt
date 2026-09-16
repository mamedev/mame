<?xml version="1.0" encoding="UTF-8"?>
<!--
Prints unique source files for system drivers.

$ ./mamenld -listxml | xsltproc scripts/xslt/list-system-sources.xslt -
sega/segag80r.cpp
capcom/1942.cpp
midw8080/mw8080bw.cpp
sega/segas16b.cpp
sega/segas16a.cpp
atari/atarittl.cpp
cinemat/cinemat.cpp
exidy/exidyttl.cpp
misc/electra.cpp
irem/m62.cpp
misc/pse.cpp
ramtek/ramtek.cpp
misc/fungames.cpp
meadows/meadows.cpp
sega/vicdual.cpp
atari/pong.cpp
exidy/carpolo.cpp
univers/cheekyms.cpp
misc/cocoloco.cpp
sega/zaxxon.cpp
misc/crazybal.cpp
misc/chicago.cpp
sega/segag80v.cpp
exidy/starfire.cpp
taito/taitottl.cpp
atari/flyball.cpp
sega/segattl.cpp
misc/bailey.cpp
vtech/gamemachine.cpp
skeleton/hazeltin.cpp
sega/segas16b_isgsm.cpp
jpm/jpmsru.cpp
zaccaria/zaccaria.cpp
nintendo/mario.cpp
sega/monacogp.cpp
skeleton/palestra.cpp
nintendo/popeye.cpp
alliedl/aleisttl.cpp
misc/usbilliards.cpp
taito/sspeedr.cpp
ramtek/starcrus.cpp
skeleton/testpat.cpp
misc/a1supply.cpp
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="text" omit-xml-declaration="yes" indent="no" />

	<xsl:key name="sourcekey" match="machine" use="@sourcefile" />

	<xsl:template match="/">
		<xsl:for-each select="*/machine[(@runnable = 'yes') and (generate-id() = generate-id(key('sourcekey', @sourcefile)))]">
			<xsl:value-of select="concat(@sourcefile, '&#xA;')" />
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>
