print '<?xml version="1.0"?>\n\
<mamelayout version="2">\n\
	<element name="dotmatrix5dot">\n\
	<dotmatrix5dot>\n\
		<color red="1.0" green="0" blue="0" />\n\
	</dotmatrix5dot>\n\
	</element>\n\
	<element name="background">\n\
		<rect>\n\
			<bounds left="0" top="0" right="1" bottom="1" />\n\
			<color red="0.3" green="0.3" blue="0.5" />\n\
		</rect>\n\
	</element>\n\
	<view name="DMD">\n\
		<bezel element="background">\n\
			<bounds left="00" top="00" right="70" bottom="7" />\n\
		</bezel>'

for x in range(14):
	for y in range(7):
		print '\t<bezel name="dmd_%d" element="dotmatrix5dot" state="0">\n\t\t<bounds x="%d" y="%d" width="5" height="1" />\n\t</bezel>' % (x*7 + y, x*5, y)

print "\t</view>\n</mamelayout>"
