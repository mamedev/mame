// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/***************************************************************************

  kc.c

  Functions to emulate the video hardware of the kc85/4,kc85/3

***************************************************************************/

#include "emu.h"
#include "includes/kc.h"

#include "screen.h"


// 3 bit colour value. bit 2->green, bit 1->red, bit 0->blue
static constexpr rgb_t kc85_pens[KC85_PALETTE_SIZE] =
{
	// foreground colours, "full" of each component
	{ 0x00, 0x00, 0x00 },   // black
	{ 0x00, 0x00, 0xd0 },   // blue
	{ 0xd0, 0x00, 0x00 },   // red
	{ 0xd0, 0x00, 0xd0 },   // magenta
	{ 0x00, 0xd0, 0x00 },   // green
	{ 0x00, 0xd0, 0xd0 },   // cyan
	{ 0xd0, 0xd0, 0x00 },   // yellow
	{ 0xd0, 0xd0, 0xd0 },   // white

	// full of each component + half of another component
	{ 0x00, 0x00, 0x00 },   // black
	{ 0x60, 0x00, 0xa0 },   // violet
	{ 0xa0, 0x60, 0x00 },   // brown
	{ 0xa0, 0x00, 0x60 },   // red/purple
	{ 0x00, 0xa0, 0x60 },   // pastel green
	{ 0x00, 0x60, 0xa0 },   // sky blue
	{ 0xa0, 0xa0, 0x60 },   // yellow/green
	{ 0xd0, 0xd0, 0xd0 },   // white

	// background colours are slightly darker than foreground colours
	{ 0x00, 0x00, 0x00 },   // black
	{ 0x00, 0x00, 0xa0 },   // dark blue
	{ 0xa0, 0x00, 0x00 },   // dark red
	{ 0xa0, 0x00, 0xa0 },   // dark magenta
	{ 0x00, 0xa0, 0x00 },   // dark green
	{ 0x00, 0xa0, 0xa0 },   // dark cyan
	{ 0xa0, 0xa0, 0x00 },   // dark yellow
	{ 0xa0, 0xa0, 0xa0 }    // dark white (grey)
};


// Initialise the palette
void kc_state::kc85_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, kc85_pens);
}

/* set new blink state */
WRITE_LINE_MEMBER( kc_state::video_toggle_blink_state )
{
	if (state)
	{
		m_screen->update_partial(m_screen->vpos());
		m_kc85_blink_state = !m_kc85_blink_state;
	}
}


/* draw 8 pixels */
void kc_state::video_draw_8_pixels(bitmap_ind16 &bitmap, int x, int y, uint8_t colour_byte, uint8_t gfx_byte)
{
	int pens[4];
	int px;

	if (m_high_resolution)
	{
		/* High resolution: 4 colors for block */

		pens[0] = 0;    // black
		pens[1] = 2;    // red
		pens[2] = 5;    // cyan
		pens[3] = 7;    // white

		px = x;

		for (int a=0; a<8; a++)
		{
			int pen = pens[((gfx_byte>>7) & 0x07) | ((colour_byte>>6) & 0x02)];

			if ((px >= 0) && (px < bitmap.width()) && (y >= 0) && (y < bitmap.height()))
			{
				bitmap.pix16(y, px) = pen;
			}

			px++;
			colour_byte <<= 1;
			gfx_byte <<= 1;
		}
	}
	else
	{
		/* Low resolution: 2 colors for block */
		/* 16 foreground colours, 8 background colours */

		/* bit 7 = 1: flash between foreground and background colour 0: no flash */
		/* bit 6: adjusts foreground colours by adding half of another component */
		/* bit 5,4,3 = foreground colour */
			/* bit 5: background colour -> Green */
			/* bit 4: background colour -> Red */
			/* bit 3: background colour -> Blue */
		/* bit 2,1,0 = background colour */
			/* bit 2: background colour -> Green */
			/* bit 1: background colour -> Red */
			/* bit 0: background colour -> Blue */

		int background_pen = (colour_byte&7) + 16;
		int foreground_pen = ((colour_byte>>3) & 0x0f);

		if ((colour_byte & 0x80) && m_kc85_blink_state && (m_pio_data[1] & 0x80))
		{
			foreground_pen = background_pen;
		}

		pens[0] = background_pen;
		pens[1] = foreground_pen;

		px = x;

		for (int a=0; a<8; a++)
		{
			int pen = pens[(gfx_byte>>7) & 0x01];

			if ((px >= 0) && (px < bitmap.width()) && (y >= 0) && (y < bitmap.height()))
			{
				bitmap.pix16(y, px) = pen;
			}
			px++;
			gfx_byte = gfx_byte<<1;
		}
	}
}


/***************************************************************************
 KC85/4 video hardware
***************************************************************************/

void kc85_4_state::video_start()
{
	m_video_ram = make_unique_clear<uint8_t[]>((KC85_4_SCREEN_COLOUR_RAM_SIZE*2) + (KC85_4_SCREEN_PIXEL_RAM_SIZE*2));
	m_display_video_ram = &m_video_ram[0];

	m_kc85_blink_state = 0;
}

void kc85_4_state::video_control_w(int data)
{
	/* calculate address of video ram to display */
	if (data & 1)
		m_display_video_ram = &m_video_ram[KC85_4_SCREEN_PIXEL_RAM_SIZE + KC85_4_SCREEN_COLOUR_RAM_SIZE];
	else
		m_display_video_ram = &m_video_ram[0];

	m_high_resolution = (data & 0x08) ? 0 : 1;
}


uint32_t kc85_4_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *pixel_ram = m_display_video_ram;
	uint8_t *colour_ram = pixel_ram + 0x04000;

	for (int y=cliprect.min_y; y<=cliprect.max_y; y++)
	{
		for (int x=0; x<(KC85_SCREEN_WIDTH>>3); x++)
		{
			uint16_t offset = y | (x<<8);

			uint8_t colour_byte = colour_ram[offset];
			uint8_t gfx_byte = pixel_ram[offset];

			video_draw_8_pixels(bitmap, (x<<3), y, colour_byte, gfx_byte);
		}
	}

	return 0;
}

/***************************************************************************
 KC85/3 video
***************************************************************************/

void kc_state::video_start()
{
	m_video_ram = make_unique_clear<uint8_t[]>(0x4000);

	m_kc85_blink_state = 0;
}

uint32_t kc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* colour ram takes up 0x02800 bytes */
	uint8_t *pixel_ram = &m_video_ram[0];
	uint8_t *colour_ram = &m_video_ram[0x2800];

	for (int y=cliprect.min_y; y<=cliprect.max_y; y++)
	{
		for (int x=0; x<(KC85_SCREEN_WIDTH>>3); x++)
		{
			int pixel_offset,colour_offset;

			if ((x & 0x020)==0)
			{
				pixel_offset = (x & 0x01f) | (((y>>2) & 0x03)<<5) |
				((y & 0x03)<<7) | (((y>>4) & 0x0f)<<9);

				colour_offset = (x & 0x01f) | (((y>>2) & 0x03f)<<5);
			}
			else
			{
				/* 1  0  1  0  0  V7 V6 V1  V0 V3 V2 V5 V4 H2 H1 H0 */
				/* 1  0  1  1  0  0  0  V7  V6 V3 V2 V5 V4 H2 H1 H0 */

				pixel_offset = 0x02000+((x & 0x07) | (((y>>4) & 0x03)<<3) |
					(((y>>2) & 0x03)<<5) | ((y & 0x03)<<7) | ((y>>6) & 0x03)<<9);

				colour_offset = 0x0800+((x & 0x07) | (((y>>4) & 0x03)<<3) |
					(((y>>2) & 0x03)<<5) | ((y>>6) & 0x03)<<7);
			}

			uint8_t colour_byte = colour_ram[colour_offset];
			uint8_t gfx_byte = pixel_ram[pixel_offset];

			video_draw_8_pixels(bitmap,(x<<3),y, colour_byte, gfx_byte);
		}
	}

	return 0;
}
