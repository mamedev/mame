# coding: utf=8
header = '''<?xml version="1.0"?>
<mamelayout version="2">

<!--
	Front panel of the Patinho Feio mini-computer
	with clickable buttons, switches and lamps.

	Written by Felipe Sanches.
-->

	<element name="bit_lamp" defstate="0">
		<disk state="1">
			<color red="1.0" green="0.1" blue="0.1" />
		</disk>
		<disk state="0">
			<color red="0.3" green="0.02" blue="0.02" />
		</disk>
	</element>

        <element name="rectangle">
          <rect>
            <color red="0.2" green="0.8" blue="0.8" />
          </rect>
        </element>

        <element name="str_flags_t">
                <text string="TRANSBORDO">
                  <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_flags_v">
                <text string="VAI - UM">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_ula">
                <text string="UNIDADE ARITMETICA E LÓGICA" >
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_rc">
                <text string="DADOS   DO   PAINEL">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_pc">
                <text string="ENDEREÇO DE INSTRUÇÃO">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_mem_addr">
                <text string="ENDEREÇO DA MEMÓRIA">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_acc">
                <text string="ACUMULADOR">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_opcode">
                <text string="CÓDIGO DE INSTRUÇÃO">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_mem_data">
                <text string="DADOS DA MEMÓRIA">
                <color red="0" green="0" blue="0" /></text>
        </element>

        <element name="str_mem">
                <text string="MEMÓRIA">
                <color red="0" green="0" blue="0" /></text>
        </element>

<!-- define background -->

	<view name="Button Lamps">
		<screen index="0">
			<bounds left="0" top="0" right="%d" bottom="%d" />
		</screen>

<!-- define lamps -->
'''

lamp =\
'''
		<bezel name="%s%d" element="bit_lamp" inputtag="%s" inputmask="0x%03X">
			<bounds x="%d" y="%d" width="%d" height="%d" />
		</bezel>
'''

rect =\
'''
		<bezel element="rectangle">
                        <bounds x="%d" y="%d" width="%d" height="%d" />
		</bezel>
'''

text =\
'''
		<bezel element="%s">
                        <bounds x="%d" y="%d" width="%d" height="%d" />
		</bezel>
'''

footer = \
'''
	</view>
</mamelayout>
'''

width = 1000
height = 1000

x1 = 0.2 * width
w1 = (0.2/8) * width
x2 = 0.45 * width
w2 = (0.3/12) * width
yspace = 0.10 * width
R = 0.02 * width
y0 = (height - 5.0 * yspace)/2
panel_margin = 0.05 * width
text_height = 0.02 * width

def rectangle(x1, y1, x2, y2):
  global data
  data += rect % (x1, height - y1, x2-x1, y1-y2)

def text_string(str_id, x, y, w, h=text_height):
  global data
  data += text % (str_id, x, height - y, w, h)

def group(name, inputtag, N, x0, y, xspace, w=R, h=R):
  global data
  for i in range(N):
    x = x0 + i * xspace
    data += lamp % (name, N-1-i, inputtag, (1<<i), x-R/2, height - y - R/2, w, h)

data = header % (width, height)

rectangle(x1 - panel_margin,
          y0 + 4.0 * yspace + panel_margin,
          x2 + w2*11 + panel_margin,
          y0 + yspace * 0.5 - panel_margin)

group('mem_data', 'mem_data',  8, x1, y0 + yspace * 1, w1)
text_string('str_mem_data', x1, y0 + yspace * 0.8, 8*w1)

group(  'opcode',   'opcode',  8, x1, y0 + yspace * 2, w1)
text_string('str_opcode', x1, y0 + yspace * 1.8, 8*w1)

group(     'acc',      'ACC',  8, x1, y0 + yspace * 3, w1)
text_string('str_acc', x1, y0 + yspace * 2.8, 8*w1)

group(      'rc',       'RC', 12, x2, y0 + yspace * 0.5, w2)
text_string('str_rc', x2, y0 + yspace * 0.3, 12*w2)

group(   'flags',    'FLAGS',  2, x2 + w2, y0 + yspace * 2.3, 9*w2)
text_string('str_flags_t', x2 - 2*w2, y0 + yspace * 2.0, 8*w2)
text_string('str_flags_v', x2 + 8*w2, y0 + yspace * 2.0, 4*w2)

group(      'pc',       'PC', 12, x2, y0 + yspace * 3.0, w2)
text_string('str_pc', x2, y0 + yspace * 2.8, 12*w2)

group('mem_addr', 'mem_addr', 12, x2, y0 + yspace * 4.0, w2)
text_string('str_mem_addr', x2, y0 + yspace * 3.8, 12*w2)

data += footer

open("patinho.lay", 'w').write(data)
