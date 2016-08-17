// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
Consolidation and enhancment of documentation by Manfred Schneider based on previous work from
 PeT mess@utanet.at and Paul Robson (autismuk@aol.com)

 Schematics, manuals and anything you can desire for at http://amigan.1emu.net/

 TODO: find a dump of the charactyer ROM
             convert the drawing code to tilemap

  emulation of signetics 2637 video/audio device

General
The UVI is capable of controlling 512 Bytes of RAM. It also generates a select signal
for a 128 byte wide area.
This whole addres space maps in the arcadia and compatible machines from $1800 - $1AFF.

1. Video Memory

The screen table is at 1800-CF and 1A00-CF. Each page has 13 lines of the
screen (16 bytes per line,26 lines in total, 208 scan lines). The 2 most
significant bits of each byte are colour data, the 6 least significant
are character data. The resolution of the Arcadia is 128 x 208 pixels.

It is possible to halve the screen resolution so 1A00..1ACF can
be used for code. This is controlled by bit 6 of $19F8.

The byte at location $18FF is the current Character Line address, lower
4 bits only. The start line goes from 1800 to 18C0 then from 1900 to 19C0.
The 4 least significant bits of this count 0123456789ABC0123456789ABC,
going to D when in vertical blank. The 4 most significant bits are always
'1'. Some games do use this for scrolling effects - a good example of this
is the routine at $010F in Alien Invaders which scrolls the various bits of
the screen about using the memory locations $18FF and $18FE.

The screen can be scrolled upto 7 pixels to the left using the high 3 bits
of $18FE. This is used in Alien Invaders.

A line beginning with $C0 contains block graphics. Each square contains
3 wide x 2 high pixels, coloured as normal. The 3 least significant bits
are the top, the next 3 bits are the bottom. Alien Invaders uses this for
shields. The graphics are returned to normal for the next line.

The VBlank signal (maybe VSYNC) is connected to the SENSE input. This is
logic '1' when the system is in VBLANK.

The Flag line does.... something graphical - it might make the sprites
half width/double height perhaps. Breakaway sets this when the bats are
double size in vertical mode.


2. Character codes

Character codes 00..37 to be in a ROM somewhere in the Emerson. These
are known, others may be discovered by comparing the screen snapshots
against the character tables. If the emulator displays an exclamation
mark you've found one. Get a snapshot to see what it looks like
normally and let me know. Codes 38..3F are taken from RAM.

00      (space)
01..0F  Graphic Characters
10..19  0..9
1A..33  A..Z
34      Decimal Point
35      comma
36        +
37         $
38..3F  User Defined Characters (8 off, from 1980..19FF)

Character data is stored 8 bits per character , as a single plane graphic
The 2nd and 3rd bits of palette data come from the screen tables, so there
are two colours per character and 4 possible palette selections for the
background.


3. Sound

fixed frequency sound and random Noise generator.
18FD pitch (lower 7 bits)
18FE volume  ( bits 0 - 2)
18fe bit 3 set means sound on
18fe bit 4 set means random noise on

Calculation of sound frequency is done as follows
 1/Freq = 2 (pitch +1) * (horizontal line period)


4. Sprites

Sprite pointers are at 18F0..18F7 (there are four of them). The graphics
used are the ones in the 1980..19BF UDG table (the first four).

Sprite addresses (x,y) are converted to offsets in the 128 x 208 as follows:

1) 1's complement the y coordinate
2) subtract 16 from the y coordinate
3) subtract 43 from the x coordinate

5. Palette

The Palette is encoded between 19F8-19FB. This section describes the method
by which colours are allocated. There are 8 colours, information is coded
3 bits per colour (usually 2 colours per byte)

 Colour Code    Name
 ------     -----   -------
  7             111     Black
  6             110     Blue
  5             101     Red
  4             100     Magenta
  3             011     Green
  2             010     Cyan
  1             001     Yellow
  0             000     White

Bits 0..2 of $19F9 are the screen colour
Bits 3..5 of $19F9 are the colours of characters
Bit 6 of $19f9 is for poti axis
Bit 7 of $19f9 is for character size (1 = 8x8; 0 = 8x16)
Bits 0..5 of $19FA are the colours of Sprites 2 & 3 (sprite 3 is low bits)
Bit 6 of $19FA is for size of sprite 3 (0 = 8x16; 1 = 8x8)
Bit 7 of $19FA is for size of sprite 2 (0 = 8x16; 1 = 8x8)
Bits 0..5 of $19FB are the colours of Sprites 0 & 1 (sprite 1 is low bits)
Bit 6 of $19FB is for size of sprite 1 (0 = 8x16; 1 = 8x8)
Bit 7 of $19FB is for size of sprite 0 (0 = 8x16; 1 = 8x8)


6. Collision Detection

Bits are set to zero on a collision - I think they are reset at the
frame start. There are two locations : one is for sprite/background
collisions, one is for sprite/sprite collisions.

19FC    bits 0..3 are collision between sprites 0..3 and the background.

19FD    bit 0 is sprite 0 / 1 collision
        bit 1 is sprite 0 / 2 collision
        bit 2 is sprite 0 / 3 collision
        bit 3 is sprite 1 / 2 collision
        bit 4 is sprite 1 / 3 collision (guess)
        bit 5 is sprite 2 / 3 collision (guess)


7. Graphic Mode
19f8 bit 7 graphics mode on (lower 6 bits descripe rectangles)
0xc0 in line switches to graphics mode in this line
0x40 in line switches to char mode in this line
 22211100
 22211100
 22211100
 22211100
 55544433
 55544433
 55544433
 55544433

 8. Memory Map
 The offsets in the following memory Map are from the view of the UVI.
 In the arcadia and compatibles the base offset is $1800 thats how the CPU sees the UVI

 0000 - 00FF    external RAM
    0000 - 00CF     upper screen character/graphics codes organised as 16 char x 13 rows
    00D0 - 00EF     RAM (not used by UVI, can be used by CPU)
    00F0            vertical offset object 0
    00F1            horizontal offset object 0
    00F2            vertical offset object 1
    00F3            horizontal offset object 1
    00F4            vertical offset object 2
    00F5            horizontal offset object 2
    00F6            vertical offset object 3
    00F7            horizontal offset object 3
    00F8 - 00FB RAM (not used by UVI, can be used by CPU)
    00FC            complement number of rows from the trailing edge of VRST
                to the start of character display
    00FD            bit0 - 6 sound frequeny,  bit 7 color mode
    00FE            bit0 - 2 loudness, bit 3 sound enable when set, bit 4 random noise enable when set
                bit 5 - 7 delay for row of characters
    00FF            bit 0 - 3 DMA number, bit 4 - 7 unused
0100 - 017F     CE area. In arcadia used for keypad.
0180 - 01FF     internal UVI registers
    0180 - 0187 image of object 0 or UDC-code $38
    0188 - 018F     image of object 1 or UDC-code $39
    0190 - 0197 image of object 2 or UDC-code $3A
    0198 - 019F     image of object 3 or UDC-code $3B
    01A0 - 01A7 image of UDC-code $3C
    01A8 - 01AF image of UDC-code $3D
    01B0 - 01B7 image of UDC-code $3E
    01B8 - 01BF     image of UDC-code $3F
    01C0 - 01F7     unused
    01F8            bit 0 - 2 alternate screen color, bit 3 - 5 alternate character color
                bit 6 Refresh mode when set entire character field will be display twice
                bit 7 Grahics mode
    01F9            bit 0 - 2 screen color, bit 3 - 5 character color
                bit 6 poti mux control
                bit 7 character size when set size=8x8 else size=8x16
    01FA            bit 0 - 2 color of object 3, bit 3 - 5 color of object 2
                bit 6 size of object 3 when set size=8x8 else size=8x16
                bit 7 size of object 2 when set size=8x8 else size=8x16
    01FB            bit 0 - 2 color of object 1, bit 3 - 5 color of object 0
                bit 6 size of object 1 when set size=8x8 else size=8x16
                bit 7 size of object 0 when set size=8x8 else size=8x16
    01FC            object - character collision bits
                bit 0 when low object 0 collided with character
                bit 1 when low object 1 collided with character
                bit 2 when low object 2 collided with character
                bit 3 when low object 3 collided with character
                bit 4 - 7 unused set to high
    01FD            inter object collision
                bit 0 - when low object 1 and 2 collide
                bit 1 - when low object 1 and 3 collide
                bit 2 - when low object 1 and 4 collide
                bit 3 - when low object 2 and 3 collide
                bit 4 - when low object 2 and 4 collide
                bit 5 - when low object 3 and 4 collide
                bit 6 - 7 unused set to high
    01FE            digital value of POT1 or POT3 input. Valid only during VRST
    01FF            digital value of POT2 or POT4 input. Valid only furing VRST
0200 - 02FF 2nd external RAM
    0200 - 02CF lower screen character/graphics codes organised as 16 char x 13 rows
    02D0 - 02FF RAM (not used by UVI, can be used by CPU)
*/


#include "includes/arcadia.h"

static const UINT8 chars[0x40][8]={
	// read from the screen generated from a palladium
	{ 0,0,0,0,0,0,0,0 },                        // 00 (space)
	{ 1,2,4,8,16,32,64,128 },                   // 01 (\)
	{ 128,64,32,16,8,4,2,1 },                   // 02 (/)
	{ 255,255,255,255,255,255,255,255 },        // 03 (solid block)
	{ 0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00 },// 04 (?)
	{ 3,3,3,3,3,3,3,3 },                        // 05 (half square right on)
	{ 0,0,0,0,0,0,255,255 },                    // 06 (horz lower line)
	{ 0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0 },// 07 (half square left on)
	{ 0xff,0xff,3,3,3,3,3,3 },                  // 08 (?)
	{ 0xff,0xff,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0 },// 09 (?)
	{ 192,192,192,192,192,192,255,255 },        // 0A (!_)
	{ 3,3,3,3,3,3,255,255 },                    // 0B (_!)
	{ 1,3,7,15,31,63,127,255 },                 // 0C (diagonal block)
	{ 128,192,224,240,248,252,254,255 },        // 0D (diagonal block)
	{ 255,254,252,248,240,224,192,128 },        // 0E (diagonal block)
	{ 255,127,63,31,15,7,3,1 },                 // 0F (diagonal block)
	{ 0x00,0x1c,0x22,0x26,0x2a,0x32,0x22,0x1c },// 10 0
	{ 0x00,0x08,0x18,0x08,0x08,0x08,0x08,0x1c },// 11 1
	{ 0x00,0x1c,0x22,0x02,0x0c,0x10,0x20,0x3e },// 12 2
	{ 0x00,0x3e,0x02,0x04,0x0c,0x02,0x22,0x1c },// 13 3
	{ 0x00,0x04,0x0c,0x14,0x24,0x3e,0x04,0x04 },// 14 4
	{ 0x00,0x3e,0x20,0x3c,0x02,0x02,0x22,0x1c },// 15 5
	{ 0x00,0x0c,0x10,0x20,0x3c,0x22,0x22,0x1c },// 16 6
	{ 0x00,0x7c,0x02,0x04,0x08,0x10,0x10,0x10 },// 17 7
	{ 0x00,0x1c,0x22,0x22,0x1c,0x22,0x22,0x1c },// 18 8
	{ 0x00,0x1c,0x22,0x22,0x3e,0x02,0x04,0x18 },// 19 9
	{ 0x00,0x08,0x14,0x22,0x22,0x3e,0x22,0x22 },// 1A A
	{ 0x00,0x3c,0x22,0x22,0x3c,0x22,0x22,0x3c },// 1B B
	{ 0x00,0x1c,0x22,0x20,0x20,0x20,0x22,0x1c },// 1C C
	{ 0x00,0x3c,0x22,0x22,0x22,0x22,0x22,0x3c },// 1D D
	{ 0x00,0x3e,0x20,0x20,0x3c,0x20,0x20,0x3e },// 1E E
	{ 0x00,0x3e,0x20,0x20,0x3c,0x20,0x20,0x20 },// 1F F
	{ 0x00,0x1e,0x20,0x20,0x20,0x26,0x22,0x1e },// 20 G
	{ 0x00,0x22,0x22,0x22,0x3e,0x22,0x22,0x22 },// 21 H
	{ 0x00,0x1c,0x08,0x08,0x08,0x08,0x08,0x1c },// 22 I
	{ 0x00,0x02,0x02,0x02,0x02,0x02,0x22,0x1c },// 23 J
	{ 0x00,0x22,0x24,0x28,0x30,0x28,0x24,0x22 },// 24 K
	{ 0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x3e },// 25 L
	{ 0x00,0x22,0x36,0x2a,0x2a,0x22,0x22,0x22 },// 26 M
	{ 0x00,0x22,0x22,0x32,0x2a,0x26,0x22,0x22 },// 27 N
	{ 0x00,0x1c,0x22,0x22,0x22,0x22,0x22,0x1c },// 28 O
	{ 0x00,0x3c,0x22,0x22,0x3c,0x20,0x20,0x20 },// 29 P
	{ 0x00,0x1c,0x22,0x22,0x22,0x2a,0x24,0x1a },// 2A Q
	{ 0x00,0x3c,0x22,0x22,0x3c,0x28,0x24,0x22 },// 2B R
	{ 0x00,0x1c,0x22,0x20,0x1c,0x02,0x22,0x1c },// 2C S
	{ 0x00,0x3e,0x08,0x08,0x08,0x08,0x08,0x08 },// 2D T
	{ 0x00,0x22,0x22,0x22,0x22,0x22,0x22,0x1c },// 2E U
	{ 0x00,0x22,0x22,0x22,0x22,0x22,0x14,0x08 },// 2F V
	{ 0x00,0x22,0x22,0x22,0x2a,0x2a,0x36,0x22 },// 30 W
	{ 0x00,0x22,0x22,0x14,0x08,0x14,0x22,0x22 },// 31 X
	{ 0x00,0x22,0x22,0x14,0x08,0x08,0x08,0x08 },// 32 Y
	{ 0x00,0x3e,0x02,0x04,0x08,0x10,0x20,0x3e },// 33 Z
	{ 0,0,0,0,0,0,0,8 },                        // 34 .
	{ 0,0,0,0,0,8,8,0x10 },                     // 35 ,
	{ 0,0,8,8,0x3e,8,8,0 },                     // 36 +
	{ 0,8,0x1e,0x28,0x1c,0xa,0x3c,8 },          // 37 $
	// 8x user defined
};

void arcadia_state::video_start()
{
	int i;
	memcpy(&m_chars, chars, sizeof(chars));
	for (i=0; i<0x40; i++)
	{
		m_rectangle[i][0]=0;
		m_rectangle[i][4]=0;
		if (i&1) m_rectangle[i][0]|=3;
		if (i&2) m_rectangle[i][0]|=0x1c;
		if (i&4) m_rectangle[i][0]|=0xe0;
		if (i&8) m_rectangle[i][4]|=3;
		if (i&0x10) m_rectangle[i][4]|=0x1c;
		if (i&0x20) m_rectangle[i][4]|=0xe0;
		m_rectangle[i][1]=m_rectangle[i][2]=m_rectangle[i][3]=m_rectangle[i][0];
		m_rectangle[i][5]=m_rectangle[i][6]=m_rectangle[i][7]=m_rectangle[i][4];
	}

	{
		int width = m_screen->width();
		int height = m_screen->height();
		m_bitmap = std::make_unique<bitmap_ind16>(width, height);
	}
}

READ8_MEMBER( arcadia_state::video_r )
{
	UINT8 data=0;
	switch (offset)
	{
	case 0xff: data = m_charline|0xf0;break;
	case 0x100: data = m_controller1_col1->read();break;
	case 0x101: data = m_controller1_col2->read();break;
	case 0x102: data = m_controller1_col3->read();break;
	case 0x103: data = m_controller1_extra->read();break;
	case 0x104: data = m_controller2_col1->read();break;
	case 0x105: data = m_controller2_col2->read();break;
	case 0x106: data = m_controller2_col3->read();break;
	case 0x107: data = m_controller2_extra->read();break;
	case 0x108: data = m_panel->read();break;
#if 0
	case 0x1fe:
		if (m_ad_select)
			data=ioport("controller1_joy_y")->read()<<3;
		else
			data=ioport("controller1_joy_x")->read()<<3;
		break;
	case 0x1ff:
		if (m_ad_select)
			data=ioport("controller2_joy_y")->read()<<3;
		else
			data=ioport("controller2_joy_x")->read()<<3;
		break;
#else
	case 0x1fe:
		data = 0x80;
		if (m_ad_select)
		{
			if (m_joysticks->read()&0x10) data=0;
			if (m_joysticks->read()&0x20) data=0xff;
		}
		else
		{
			if (m_joysticks->read()&0x40) data=0xff;
			if (m_joysticks->read()&0x80) data=0;
		}
		break;
	case 0x1ff:
		data = 0x6f; // 0x7f too big for alien invaders (move right)
		if (m_ad_select)
		{
			if (m_joysticks->read()&0x1) data=0;
			if (m_joysticks->read()&0x2) data=0xff;
		}
		else
		{
			if (m_joysticks->read()&0x4) data=0xff;
			if (m_joysticks->read()&0x8) data=0;
		}
		break;
#endif
	default:
		data = m_reg.data[offset];
	}
	return data;
}

WRITE8_MEMBER( arcadia_state::video_w )
{
	m_reg.data[offset]=data;
	switch (offset)
	{
		case 0xfc:
			m_ypos=255-data+YPOS;
			break;
		case 0xfd:
			m_custom->write(space, offset&3, data);
			m_multicolor = data & 0x80;
			break;
		case 0xfe:
			m_custom->write(space, offset&3, data);
			m_shift = (data>>5);
			break;
		case 0xf0:
		case 0xf2:
		case 0xf4:
		case 0xf6:
			m_pos[(offset>>1)&3].y = (data^0xff)+1;
			break;
		case 0xf1:
		case 0xf3:
		case 0xf5:
		case 0xf7:
			m_pos[(offset>>1)&3].x = data-43;
			break;
		case 0x180: case 0x181: case 0x182: case 0x183: case 0x184: case 0x185: case 0x186: case 0x187:
		case 0x188: case 0x189: case 0x18a: case 0x18b: case 0x18c: case 0x18d: case 0x18e: case 0x18f:
		case 0x190: case 0x191: case 0x192: case 0x193: case 0x194: case 0x195: case 0x196: case 0x197:
		case 0x198: case 0x199: case 0x19a: case 0x19b: case 0x19c: case 0x19d: case 0x19e: case 0x19f:
		case 0x1a0: case 0x1a1: case 0x1a2: case 0x1a3: case 0x1a4: case 0x1a5: case 0x1a6: case 0x1a7:
		case 0x1a8: case 0x1a9: case 0x1aa: case 0x1ab: case 0x1ac: case 0x1ad: case 0x1ae: case 0x1af:
		case 0x1b0: case 0x1b1: case 0x1b2: case 0x1b3: case 0x1b4: case 0x1b5: case 0x1b6: case 0x1b7:
		case 0x1b8: case 0x1b9: case 0x1ba: case 0x1bb: case 0x1bc: case 0x1bd: case 0x1be: case 0x1bf:
			m_chars[0x38|((offset>>3)&7)][offset&7] = data;
			break;
		case 0x1f8:
			m_lines26 = data & 0x40;
			m_graphics = data & 0x80;
			break;
		case 0x1f9:
			m_doublescan =! (data&0x80);
			m_ad_delay = 10;
			break;
	}
}

void arcadia_state::draw_char(UINT8 *ch, int charcode, int y, int x)
{
	int k,b,cc,sc, colour;
	if (m_multicolor)
	{
		if (charcode&0x40)
			cc=((m_reg.d.pal[1]>>3)&7);
		else
			cc=((m_reg.d.pal[0]>>3)&7);

		if (charcode&0x80)
			sc=(m_reg.d.pal[1]&7);
		else
			sc=(m_reg.d.pal[0]&7);
	}
	else
	{
		cc=((m_reg.d.pal[1]>>3)&1)|((charcode>>5)&6);
		sc=(m_reg.d.pal[1]&7);
	}
	colour = (((sc << 3) | cc) + 4);

	if (m_doublescan)
	{
		for (k=0; (k<8)&&(y<m_bitmap->height()); k++, y+=2)
		{
			b=ch[k];
			m_bg[y][x>>3]|=b>>(x&7);
			m_bg[y][(x>>3)+1]|=b<<(8-(x&7));

			if (y+1<m_bitmap->height())
			{
				m_bg[y+1][x>>3]|=b>>(x&7);
				m_bg[y+1][(x>>3)+1]|=b<<(8-(x&7));
				m_gfxdecode->gfx(0)->opaque(*m_bitmap,m_bitmap->cliprect(), b,colour, 0,0,x,y);
				m_gfxdecode->gfx(0)->opaque(*m_bitmap,m_bitmap->cliprect(), b,colour, 0,0,x,y+1);
			}
		}
	}
	else
	{
		for (k=0; (k<8)&&(y<m_bitmap->height()); k++, y++)
		{
			b=ch[k];
			m_bg[y][x>>3]|=b>>(x&7);
			m_bg[y][(x>>3)+1]|=b<<(8-(x&7));

			m_gfxdecode->gfx(0)->opaque(*m_bitmap,m_bitmap->cliprect(), b,colour, 0,0,x,y);
		}
	}
}


void arcadia_state::vh_draw_line(int y, UINT8 chars1[16])
{
	int x, ch, j, h;
	int graphics = m_graphics;
	h = m_doublescan ? 16 : 8 ;

	if (m_bitmap->height() - m_line < h)
		h = m_bitmap->height() - m_line;

	m_bitmap->plot_box(0, y, m_bitmap->width(), h, (m_reg.d.pal[1]&7));
	memset(m_bg[y], 0, sizeof(m_bg[0])*h);

	for (x=XPOS+m_shift, j=0; j<16;j++,x+=8)
	{
		ch=chars1[j];
// hangman switches with 0x40
// alien invaders shield lines start with 0xc0
		if ((ch&0x3f)==0)
		{
			switch (ch)
			{
				case 0xc0: graphics=TRUE;break;
				case 0x40: graphics=FALSE;break;
//        case 0x80:
//        alien invaders shields are empty 0x80
//            popmessage(5, "graphics code 0x80 used");
			}
		}
		if (graphics)
			draw_char(m_rectangle[ch&0x3f], ch, y, x);
		else
			draw_char(m_chars[ch&0x3f], ch, y, x);
	}
}


int arcadia_state::sprite_collision(int n1, int n2)
{
	int k, b1, b2, x;
	if (m_pos[n1].x+8<=m_pos[n2].x)
		return FALSE;
	if (m_pos[n1].x>=m_pos[n2].x+8)
		return FALSE;

	for (k=0; k<8; k++)
	{
		if (m_pos[n1].y+k<m_pos[n2].y)
			continue;
		if (m_pos[n1].y+k>=m_pos[n2].y+8)
			break;
		x=m_pos[n1].x-m_pos[n2].x;
		b1=m_reg.d.chars[n1][k];
		b2=m_reg.d.chars[n2][m_pos[n1].y+k-m_pos[n2].y];
		if (x<0)
			b2>>=-x;
		if (x>0)
			b1>>=x;
		if (b1&b2)
			return TRUE;
	}
	return FALSE;
}


void arcadia_state::draw_sprites()
{
	int i, k, x, y, color=0;
	UINT8 b;

	m_reg.d.collision_bg|=0xf;
	m_reg.d.collision_sprite|=0x3f;
	for (i=0; i<4; i++)
	{
		int doublescan = FALSE;
		if (m_pos[i].y<=-YPOS) continue;
		if (m_pos[i].y>=m_bitmap->height()-YPOS-8) continue;
		if (m_pos[i].x<=-XPOS) continue;
		if (m_pos[i].x>=128+XPOS-8) continue;

		switch (i)
		{
		case 0:
			color=(m_reg.d.pal[3]>>3)&7;
			doublescan=m_reg.d.pal[3]&0x80?FALSE:TRUE;
			break;
		case 1:
			color=m_reg.d.pal[3]&7;
			doublescan=m_reg.d.pal[3]&0x40?FALSE:TRUE;
			break;
		case 2:
			color=(m_reg.d.pal[2]>>3)&7;
			doublescan=m_reg.d.pal[2]&0x80?FALSE:TRUE;
			break;
		case 3:
			color=m_reg.d.pal[2]&7;
			doublescan=m_reg.d.pal[2]&0x40?FALSE:TRUE;
			break;
		}
		for (k=0; k<8; k++)
		{
			int j, m;
			b=m_reg.d.chars[i][k];
			x=m_pos[i].x+XPOS;
			if (!doublescan)
			{
				y=m_pos[i].y+YPOS+k;
				for (j=0,m=0x80; j<8; j++, m>>=1)
				{
					if (b & m)
						m_bitmap->pix16(y, x + j) = color;
				}
			}
			else
			{
				y=m_pos[i].y+YPOS+k*2;
				for (j=0,m=0x80; j<8; j++, m>>=1)
				{
					if (b & m)
					{
						m_bitmap->pix16(y, x + j) = color;
						m_bitmap->pix16(y+1, x + j) = color;
					}
				}
			}
			if (m_reg.d.collision_bg&(1<<i))
			{
				if ( (b<<(8-(x&7))) & ((m_bg[y][x>>3]<<8) | m_bg[y][(x>>3)+1]) )
				m_reg.d.collision_bg&=~(1<<i);
			}
		}
	}
	if (sprite_collision(0,1)) m_reg.d.collision_sprite&=~1;
	if (sprite_collision(0,2)) m_reg.d.collision_sprite&=~2;
	if (sprite_collision(0,3)) m_reg.d.collision_sprite&=~4;
	if (sprite_collision(1,2)) m_reg.d.collision_sprite&=~8;
	if (sprite_collision(1,3)) m_reg.d.collision_sprite&=~0x10; //guess
	if (sprite_collision(2,3)) m_reg.d.collision_sprite&=~0x20; //guess
}

INTERRUPT_GEN_MEMBER(arcadia_state::video_line)
{
	int width = m_screen->width();

	if (m_ad_delay<=0)
	m_ad_select=m_reg.d.pal[1]&0x40;
	else m_ad_delay--;

	m_line++;
	m_line%=262;
	// unbelievable, reflects only charline, but alien invaders uses it for
	// alien scrolling

	if (m_line<m_ypos)
	{
		m_bitmap->plot_box(0, m_line, width, 1, (m_reg.d.pal[1])&7);
		memset(m_bg[m_line], 0, sizeof(m_bg[0]));
	}
	else
	{
		int h=m_doublescan?16:8;

		m_charline=(m_line-m_ypos)/h;

		if (m_charline<13)
		{
			if (((m_line-m_ypos)&(h-1))==0)
			{
				vh_draw_line(m_charline*h+m_ypos, m_reg.d.chars1[m_charline]);
			}
		}
		else
		if (m_lines26 && (m_charline<26))
		{
			if (((m_line-m_ypos)&(h-1))==0)
			{
				vh_draw_line(m_charline*h+m_ypos, m_reg.d.chars2[m_charline-13]);
			}
		m_charline-=13;
		}
		else
		{
			m_charline=0xd;
			m_bitmap->plot_box(0, m_line, width, 1, (m_reg.d.pal[1])&7);
			memset(m_bg[m_line], 0, sizeof(m_bg[0]));
		}
	}
	if (m_line==261)
		draw_sprites();
}

READ8_MEMBER( arcadia_state::vsync_r )
{
	return m_line>=216 ? 0x80 : 0 ;
}

UINT32 arcadia_state::screen_update_arcadia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
