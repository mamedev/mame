// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at
******************************************************************************/

#include "emu.h"
#include "includes/ssystem3.h"

void ssystem3_state::ssystem3_lcd_reset()
{
	m_lcd.count=0; m_lcd.clock=1;
}

void ssystem3_state::ssystem3_lcd_write(int clock, int data)
{
	if (clock&&!m_lcd.clock) {
		m_lcd.data[m_lcd.count/8]&=~(1<<(m_lcd.count&7));
		if (data) m_lcd.data[m_lcd.count/8]|=1<<(m_lcd.count&7);
		if (m_lcd.count+1==40) {
			logerror("%.4x lcd %02x%02x%02x%02x%02x\n",(int)m_maincpu->pc(),
				m_lcd.data[0], m_lcd.data[1], m_lcd.data[2], m_lcd.data[3], m_lcd.data[4]);
		}
		m_lcd.count=(m_lcd.count+1)%40;
	}
	m_lcd.clock=clock;
}



static const unsigned char ssystem3_palette[] =
{
	0,12,12,
	80,82,75,
	0,12,12
};


PALETTE_INIT_MEMBER(ssystem3_state, ssystem3)
{
	int i;

	for ( i = 0; i < sizeof(ssystem3_palette) / 3; i++ ) {
		m_palette->set_pen_color(i, ssystem3_palette[i*3], ssystem3_palette[i*3+1], ssystem3_palette[i*3+2]);
	}
}


void ssystem3_state::video_start()
{
	// artwork seams to need this
	m_videoram = auto_alloc_array(machine(), UINT8, 6 * 2 + 24);
}


static const char led[]={
	"    aaaaaaaaaaaa\r"
	"  f  aaaaaaaaaa  b\r"
	"  ff  aaaaaaaa  bb\r"
	"  fff  aaaaaa  bbb\r"
	"  ffff        bbbb\r"
	"  ffff        bbbb\r"
	"  ffff        bbbb\r"
	"  ffff        bbbb\r"
	" ffff        bbbb\r"
	" ffff        bbbb\r"
	" fff          bbb\r"
		" f   gggggggg   b\r"
		"   gggggggggggg\r"
	"   gggggggggggg\r"
	" e   gggggggg   c\r"
	" eee          ccc\r"
	" eeee        cccc\r"
	" eeee        cccc\r"
	"eeee        cccc\r"
	"eeee        cccc\r"
	"eeee        cccc\r"
	"eeee        cccc\r"
	"eee  dddddd  ccc\r"
	"ee  dddddddd  cc\r"
		"e  dddddddddd  c\r"
		"  dddddddddddd"
};

void ssystem3_state::ssystem3_draw_7segment(bitmap_ind16 &bitmap,int value, int x, int y)
{
	int i, xi, yi, mask, color;

	for (i=0, xi=0, yi=0; led[i]; i++) {
		mask=0;
		switch (led[i]) {
		case 'a': mask=0x80; break;
		case 'b': mask=0x40; break;
		case 'c': mask=0x20; break;
		case 'd': mask=0x10; break;
		case 'e': mask=8; break;
		case 'f': mask=4; break;
		case 'g': mask=2; break;
		case 'h':
			// this is more likely wired to the separate leds
			mask=1;
			break;
		}

		if (mask!=0) {
			color=(value&mask)?1:0;
			bitmap.pix16(y+yi, x+xi) = color;
		}
		if (led[i]!='\r') xi++;
		else { yi++, xi=0; }
	}
}

static const struct {
	int x,y;
} ssystem3_led_pos[5]={
	{150,123},
	{170,123},
	{200,123},
	{220,123},
	{125,123}
};

static const char single_led[]=
"          c                                                                                                           1 1\r"
"   bb   ccccc   bb                                                                                                    1 1\r"
"   bb     c     bb                                                                                                    1 1\r"
" bb  bb       bb  bb                                                                                                  1 1\r"
" bb  bbbbbbbbbbb  bb                                                                                                111 111\r"
"                                                                                                                      1 1\r"
"  99   9999999    99                                                                                                  1 1\r"
"  99              99                                                                                                  1 1\r"
"     88888888888 9                                                                                                    1 1\r"
"    88         8 9999\r"
"  8888 6666666 8 9999\r"
"  8888 6     6\r"
"       6 777 6\r"
"       6 777 6\r"
"       6 777 6\r"
"      6  777  6\r"
"      6 77777 6\r"
"     6  77777  6                                                 2     2                                             4\r"
"     6 7777777 6                                                 2     2                                             4\r"
"    6  7777777  6                                                 2   2                                              4\r"
"    6 777777777 6                                                 2   2                                            44444\r"
"   6  777777777  6                                                 2 2                                               4\r"
"   6 77777777777 6                                                2   2                                            33433\r"
"  6       7       6                                               2   2                                              4\r"
"  6666666 7 6666666                                              2     2                                             4\r"
"          7                                                      2     2                                             4\r"
"  77777777777777777\r"
"\r"
"\r"
"\r"
" 5555555555555555555          000000   000000   00        00  0000000   00    00  00000000  00  00    00   0000000\r"
"5                   5        0000000  00000000  000      000  00000000  00    00  00000000  00  000   00  00000000\r"
"5                   5        00       00    00  0000    0000  00    00  00    00     00     00  0000  00  00\r"
"5                   5        00       00    00  00 00  00 00  00000000  00    00     00     00  00 00 00  00  0000\r"
"5                   5        00       00    00  00 00  00 00  0000000   00    00     00     00  00  0000  00    00\r"
"5                   5        0000000  00000000  00  0000  00  00        00000000     00     00  00   000  00000000\r"
" 55555555   55555555          000000   000000   00   00   00  00         000000      00     00  00    00   0000000"
;

void ssystem3_state::ssystem3_draw_led(bitmap_ind16 &bitmap,INT16 color, int x, int y, int ch)
{
	int j, xi=0;
	for (j=0; single_led[j]; j++) {
		switch (single_led[j]) {
		default:
			if (ch==single_led[j]) {
				bitmap.pix16(y, x+xi) = color;
			}
			xi++;
			break;
		case ' ':
			xi++;
			break;
		case '\r':
			xi=0;
			y++;
			break;
		};
	}
}

UINT32 ssystem3_state::screen_update_ssystem3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i=0; i<4; i++) {
		ssystem3_draw_7segment(bitmap, m_lcd.data[1+i], ssystem3_led_pos[i].x, ssystem3_led_pos[i].y);
	}

	ssystem3_draw_led(bitmap, m_lcd.data[0]&1?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '0'); //?
	ssystem3_draw_led(bitmap, m_lcd.data[0]&2?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '5');
	ssystem3_draw_led(bitmap, m_lcd.data[0]&4?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '7');
	ssystem3_draw_led(bitmap, m_lcd.data[0]&8?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, 'b');
	ssystem3_draw_led(bitmap, m_lcd.data[0]&0x10?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '9');
	ssystem3_draw_led(bitmap, m_lcd.data[0]&0x20?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '8');
	ssystem3_draw_led(bitmap, m_lcd.data[0]&0x40?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, 'c');
	ssystem3_draw_led(bitmap, m_lcd.data[0]&0x80?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '6');
	ssystem3_draw_led(bitmap, m_lcd.data[1]&1?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '2');
	ssystem3_draw_led(bitmap, m_lcd.data[2]&1?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '1'); //?
	ssystem3_draw_led(bitmap, m_lcd.data[3]&1?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '3');
	ssystem3_draw_led(bitmap, m_lcd.data[4]&1?1:0, ssystem3_led_pos[4].x, ssystem3_led_pos[4].y, '4');

	if (m_configuration->read() & 1) { // playfield(optional device)
		static const int lcd_signs_on[]={
			0, // empty
			1, // bauer
			3, // springer
			0x11, // l??ufer
			7, // turm
			0x1f, // dame
			0x17, // k??nig
			0
		};
		int y, x;
		for (y=0; y<8; y++) {
			for (x=0; x<8; x++) {
				int figure, black;
				int xp=263+x*22;
				int yp=55+(y^7)*28;
				ssystem3_playfield_getfigure(x, y, &figure, &black);
				ssystem3_draw_led(bitmap, lcd_signs_on[figure]&1?1:0, xp, yp, '6');
				ssystem3_draw_led(bitmap, lcd_signs_on[figure]&2?1:0, xp, yp, '8');
				ssystem3_draw_led(bitmap, lcd_signs_on[figure]&4?1:0, xp, yp, '9');
				ssystem3_draw_led(bitmap, lcd_signs_on[figure]&8?1:0, xp, yp, 'b');
				ssystem3_draw_led(bitmap, lcd_signs_on[figure]&0x10?1:0, xp, yp, 'c');
				ssystem3_draw_led(bitmap, figure!=0 && black?1:0, xp, yp, '7');
			}
		}
	}

	return 0;
}
