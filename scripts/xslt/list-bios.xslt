<?xml version="1.0" encoding="UTF-8"?>
<!--
Prints BIOS options for machines and devices that have them.

$ ./mame64d -listxml osborne1 saturn | xsltproc scripts/xslt/list-bios.xslt -
osborne1         Osborne-1
    vera             BIOS version A
    ver12            BIOS version 1.2
    ver121           BIOS version 1.2.1
    ver13            BIOS version 1.3
    ver14            BIOS version 1.4
    ver143           BIOS version 1.43
   *ver144           BIOS version 1.44
saturn           Saturn (USA)
    101a             Overseas v1.01a (941115)
    100a             Overseas v1.00a (941115)
satcdb           Saturn CDB (CD Block)
   *cdb106           Saturn CD Block 1.06
    cdb105           Saturn CD Block 1.05
    ygr022           Saturn CD Block (YGR022 315-5962)
ie15_terminal    IE15 Terminal
   *5chip            5-chip firmware (newer)
    6chip            6-chip firmware (older)
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="text" omit-xml-declaration="yes" indent="no" />
	<xsl:template match="/">
		<xsl:for-each select="mame/machine[biosset]">
			<xsl:value-of select="concat(substring(concat(@name, '               '), 1, 16), ' ' , description, '&#xA;')" />
				<xsl:for-each select="biosset">
					<xsl:choose>
						<xsl:when test="@default='yes'">
							<xsl:value-of select="'   *'" />
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of select="'    '" />
						</xsl:otherwise>
					</xsl:choose>
					<xsl:value-of select="concat(substring(concat(@name, '               '), 1, 16), ' ' , @description, '&#xA;')" />
				</xsl:for-each>
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>
