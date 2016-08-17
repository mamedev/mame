// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/cgc7900.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define OVERLAY_CUR                 BIT(cell, 31)   /* places a cursor in the cell if SET */
#define OVERLAY_BLK                 BIT(cell, 30)   /* blinks the foreground character in the cell if SET */
#define OVERLAY_VF                  BIT(cell, 28)   /* makes the foreground visible if SET (else transparent) */
#define OVERLAY_VB                  BIT(cell, 27)   /* makes the background visible if SET (else transparent) */
#define OVERLAY_PL                  BIT(cell, 24)   /* uses bits 0-7 as PLOT DOT descriptor if SET (else ASCII) */
#define OVERLAY_BR                  BIT(cell, 18)   /* turns on Red in background if SET */
#define OVERLAY_BG                  BIT(cell, 17)   /* turns on Green in background if SET */
#define OVERLAY_BB                  BIT(cell, 16)   /* turns on Blue in background if SET */
#define OVERLAY_FR                  BIT(cell, 10)   /* turns on Red in foreground if SET */
#define OVERLAY_FG                  BIT(cell, 9)    /* turns on Green in foreground if SET */
#define OVERLAY_FB                  BIT(cell, 8)    /* turns on Blue in background if SET */
#define OVERLAY_DATA                (cell & 0xff)   /* ASCII or Plot Dot character */

#define IMAGE_SELECT                BIT(m_roll_overlay[0], 13)
#define OVERLAY_CURSOR_BLINK        BIT(m_roll_overlay[0], 12)
#define OVERLAY_CHARACTER_BLINK     BIT(m_roll_overlay[0], 11)

PALETTE_INIT_MEMBER(cgc7900_state, cgc7900)
{
	palette.set_pen_color(0, rgb_t::black);
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0xff));
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00));
	palette.set_pen_color(3, rgb_t(0x00, 0xff, 0xff));
	palette.set_pen_color(4, rgb_t(0xff, 0x00, 0x00));
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0xff));
	palette.set_pen_color(6, rgb_t(0xff, 0xff, 0x00));
	palette.set_pen_color(7, rgb_t::white);
}

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    cgc7900_z_mode_r - Z mode read
-------------------------------------------------*/

READ16_MEMBER( cgc7900_state::z_mode_r )
{
	return 0;
}

/*-------------------------------------------------
    cgc7900_z_mode_w - Z mode write
-------------------------------------------------*/

WRITE16_MEMBER( cgc7900_state::z_mode_w )
{
}

/*-------------------------------------------------
    cgc7900_color_status_w - color status write
-------------------------------------------------*/

WRITE16_MEMBER( cgc7900_state::color_status_w )
{
}

/*-------------------------------------------------
    cgc7900_sync_r - sync information read
-------------------------------------------------*/

READ16_MEMBER( cgc7900_state::sync_r )
{
	/*

	    bit     signal      description

	     0      _VERT       vertical retrace (0=vblank)
	     1                  interlace (1=first field, 0=second field)
	     2      _HG         horizontal retrace (0=hblank)
	     3      1
	     4      1
	     5      1
	     6      1
	     7      1
	     8      1
	     9      1
	    10      1
	    11      1
	    12      1
	    13      1
	    14      1
	    15      1

	*/

	return 0xffff;
}

/***************************************************************************
    VIDEO
***************************************************************************/

/*-------------------------------------------------
    update_clut - update color lookup table
-------------------------------------------------*/

void cgc7900_state::update_clut()
{
	for (int i = 0; i < 256; i++)
	{
		UINT16 addr = i * 2;
		UINT32 data = (m_clut_ram[addr + 1] << 16) | m_clut_ram[addr];
		UINT8 b = data & 0xff;
		UINT8 g = (data >> 8) & 0xff;
		UINT8 r = (data >> 16) & 0xff;

		m_clut[i] = rgb_t(r, g, b);
	}
}

/*-------------------------------------------------
    draw_bitmap - draw bitmap image
-------------------------------------------------*/

void cgc7900_state::draw_bitmap(screen_device *screen, bitmap_rgb32 &bitmap)
{
}

/*-------------------------------------------------
    draw_overlay - draw text overlay
-------------------------------------------------*/

void cgc7900_state::draw_overlay(screen_device *screen, bitmap_rgb32 &bitmap)
{
	const pen_t *pen = m_palette->pens();
	for (int y = 0; y < 768; y++)
	{
		int sy = y / 8;
		int line = y % 8;

		for (int sx = 0; sx < 85; sx++)
		{
			UINT16 addr = (sy * 170) + (sx * 2);
			UINT32 cell = (m_overlay_ram[addr] << 16) | m_overlay_ram[addr + 1];
			UINT8 data = m_char_rom->base()[(OVERLAY_DATA << 3) | line];
			int fg = (cell >> 8) & 0x07;
			int bg = (cell >> 16) & 0x07;

			for (int x = 0; x < 8; x++)
			{
				if (OVERLAY_CUR)
				{
					if (!OVERLAY_CURSOR_BLINK || m_blink)
					{
						bitmap.pix32(y, (sx * 8) + x) = pen[7];
					}
				}
				else
				{
					if (BIT(data, x) && (!OVERLAY_CHARACTER_BLINK || m_blink))
					{
						if (OVERLAY_VF) bitmap.pix32(y, (sx * 8) + x) = pen[fg];
					}
					else
					{
						if (OVERLAY_VB) bitmap.pix32(y, (sx * 8) + x) = pen[bg];
					}
				}
			}
		}
	}
}

/*-------------------------------------------------
    TIMER_DEVICE_CALLBACK_MEMBER( blink_tick )
-------------------------------------------------*/

TIMER_DEVICE_CALLBACK_MEMBER(cgc7900_state::blink_tick)
{
	m_blink = !m_blink;
}

UINT32 cgc7900_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_clut();
	draw_bitmap(&screen, bitmap);
	draw_overlay(&screen, bitmap);

	return 0;
}

/*-------------------------------------------------
    gfx_layout cgc7900_charlayout
-------------------------------------------------*/

static const gfx_layout cgc7900_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

/*-------------------------------------------------
    GFXDECODE( cgc7900 )
-------------------------------------------------*/

static GFXDECODE_START( cgc7900 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cgc7900_charlayout, 0, 1 )
GFXDECODE_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    MACHINE_DRIVER( cgc7900_video )
-------------------------------------------------*/

MACHINE_CONFIG_FRAGMENT( cgc7900_video )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(cgc7900_state, screen_update)
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cgc7900)
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(cgc7900_state, cgc7900)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("blink", cgc7900_state, blink_tick, attotime::from_hz(XTAL_28_48MHz/7500000))
MACHINE_CONFIG_END
