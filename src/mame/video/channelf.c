// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Frank Palazzolo, Sean Riddle
#include "includes/channelf.h"

static const rgb_t channelf_palette[] =
{
	rgb_t(0x10, 0x10, 0x10), /* black */
	rgb_t(0xfd, 0xfd, 0xfd), /* white */
	rgb_t(0xff, 0x31, 0x53), /* red   */
	rgb_t(0x02, 0xcc, 0x5d), /* green */
	rgb_t(0x4b, 0x3f, 0xf3), /* blue  */
	rgb_t(0xe0, 0xe0, 0xe0), /* ltgray  */
	rgb_t(0x91, 0xff, 0xa6), /* ltgreen */
	rgb_t(0xce, 0xd0, 0xff)  /* ltblue  */
};

#define BLACK   0
#define WHITE   1
#define RED     2
#define GREEN   3
#define BLUE    4
#define LTGRAY  5
#define LTGREEN 6
#define LTBLUE  7

static const UINT16 colormap[] = {
	BLACK,   WHITE, WHITE, WHITE,
	LTBLUE,  BLUE,  RED,   GREEN,
	LTGRAY,  BLUE,  RED,   GREEN,
	LTGREEN, BLUE,  RED,   GREEN,
};

/* Initialise the palette */
PALETTE_INIT_MEMBER(channelf_state, channelf)
{
	palette.set_pen_colors(0, channelf_palette, ARRAY_LENGTH(channelf_palette));
}

void channelf_state::video_start()
{
	m_p_videoram = memregion("vram")->base();
}

int channelf_state::recalc_palette_offset(int reg1, int reg2)
{
	/* Note: This is based on the decoding they used to   */
	/*       determine which palette this line is using   */

	return ((reg2&0x2)|(reg1>>1)) << 2;
}

UINT32 channelf_state::screen_update_channelf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,col;
	UINT16 ma=0,x;
	int palette_offset;

	for(y = 0; y < 64; y++ )
	{
		UINT16 *p = &bitmap.pix16(y);
		palette_offset = recalc_palette_offset(m_p_videoram[y*128+125]&3, m_p_videoram[y*128+126]&3);

		for (x = ma; x < ma + 128; x++)
		{
			col = palette_offset+(m_p_videoram[x|(y<<7)]&3);
			*p++ = colormap[col];
		}
		ma+=128;
	}
	return 0;
}
