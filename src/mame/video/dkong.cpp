// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.


***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/dkong.h"

#define RADARSCP_BCK_COL_OFFSET         256
#define RADARSCP_GRID_COL_OFFSET        (RADARSCP_BCK_COL_OFFSET + 256)
#define RADARSCP_STAR_COL               (RADARSCP_GRID_COL_OFFSET + 8)

static const double cd4049_vl = 1.5/5.0;
static const double cd4049_vh = 3.5/5.0;
static const double cd4049_al = 0.01;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Donkey Kong has two 256x4 palette PROMs and one 256x4 PROM which contains
  the color codes to use for characters on a per row/column basis (groups of
  of 4 characters in the same column - actually row, since the display is
  rotated)
  The palette PROMs are connected to the RGB output this way:

    5V  -- 470 ohm resistor -- inverter  -- RED
  bit 3 -- 220 ohm resistor -- inverter  -- RED
        -- 470 ohm resistor -- inverter  -- RED
        -- 1  kohm resistor -- inverter  -- RED

   5V  -- 470 ohm resistor -- inverter  -- GREEN
  bit 0 -- 220 ohm resistor -- inverter  -- GREEN
  bit 3 -- 470 ohm resistor -- inverter  -- GREEN
        -- 1  kohm resistor -- inverter  -- GREEN
    5V  -- 680 ohm resistor -- inverter  -- BLUE
        -- 220 ohm resistor -- inverter  -- BLUE
  bit 0 -- 470 ohm resistor -- inverter  -- BLUE

  After the mixing stage there is a darlington circuit with approx. linear
  transfer function for red and green. Minimum output voltage is 0.9 Volt.
  Blue is transferred through just one transistor. Here we have to
  substract 0.7 Volts. This signal (0..5V) is inverted by a amplifier
  circuit with an emitter lowpass RC. The R is a variable resistor.
  So any calculations done here may be void in reality ...
***************************************************************************/

/*
    dkong color interface:

    All outputs are open-collector and pullup resistors are connected to 5V.
    Red and Green outputs are routed through a complimentary darlington
    whereas blue is routed through a 1:1 stage leading to a 0.7V cutoff.
*/

static const res_net_decode_info dkong_decode_info =
{
	2,      /*  there may be two proms needed to construct color */
	0,      /*  start at 0 */
	255,    /*  end at 255 */
	/*  R,   G,   B,   R,   G,   B */
	{ 256, 256,   0,   0,   0,   0},        /*  offsets */
	{   1,  -2,   0,   0,   2,   0},        /*  shifts */
	{0x07,0x04,0x03,0x00,0x03,0x00}         /*  masks */
};

static const res_net_info dkong_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  /*  dkong */
	}
};

static const res_net_info dkong_net_bck_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 0, { 0 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 0, { 0 } }
	}
};

static const res_net_decode_info dkong3_decode_info =
{
	1,      /*  one prom needed to contruct color */
	0,      /*  start at 0 */
	255,    /*  end at 255 */
	/*   R,   G,   B */
	{   0,   0, 512 },      /*  offsets */
	{   4,   0,   0 },      /*  shifts */
	{0x0F,0x0F,0x0F }           /*  masks */
};

static const res_net_info dkong3_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470,      0, 4, { 2200, 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470,      0, 4, { 2200, 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470,      0, 4, { 2200, 1000, 470, 220 } }
	}
};

/*
    radarscp interface

    All outputs are open-collector. The pullup resistors are connected to
    inverters (TTL). All outputs are routed through a complimentary
    darlington. The blue channel has a pulldown resistor (R8, 0M15) as well.
*/


#define TRS_J1  (1)         // (1) = Closed (0) = Open


static const res_net_info radarscp_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_TTL | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470 * TRS_J1, 470*(1-TRS_J1), 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470 * TRS_J1, 470*(1-TRS_J1), 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680 * TRS_J1, 680*(1-TRS_J1), 2, {  470, 220,   0 } }    /*  radarscp */
	}
};

static const res_net_info radarscp_net_bck_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_TTL | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 4700, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON, 470, 4700, 0, { 0 } },
		{ RES_NET_AMP_EMITTER,    470, 4700, 0, { 0 } }    /*  radarscp */
	}
};

/*
    radarscp1 interface

    All outputs are open-collector. They are followed by inverters which
    drive the resistor network. All outputs are routed through a complimentary
    darlington.
*/

static const res_net_info radarscp1_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 0,      0, 4, { 39000, 20000, 10000, 4990 } },
		{ RES_NET_AMP_DARLINGTON, 0,      0, 4, { 39000, 20000, 10000, 4990 } },
		{ RES_NET_AMP_EMITTER,    0,      0, 4, { 39000, 20000, 10000, 4990 } }
	}
};

/* Radarscp star color */

static const res_net_info radarscp_stars_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 4700, 470, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },    /*  dummy */
		{ RES_NET_AMP_EMITTER,       1,   0, 0, { 0 } },    /*  dummy */
	}
};

/* Dummy struct to generate background palette entries */

static const res_net_info radarscp_blue_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_VCC | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON,  470, 4700, 0, { 0 } },   /*  bias/gnd exist in schematics, readable in TKG3 schematics */
		{ RES_NET_AMP_DARLINGTON,  470, 4700, 0, { 0 } },   /*  bias/gnd exist in schematics, readable in TKG3 schematics */
		{ RES_NET_AMP_EMITTER,       0,    0, 8, { 128,64,32,16,8,4,2,1 } },    /*  dummy */
	}
};

/* Dummy struct to generate grid palette entries */

static const res_net_info radarscp_grid_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },    /*  dummy */
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },    /*  dummy */
		{ RES_NET_AMP_EMITTER,       0,   0, 1, { 1 } },    /*  dummy */
	}
};

PALETTE_INIT_MEMBER(dkong_state,dkong2b)
{
	const UINT8 *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;
	int i;

	compute_res_net_all(rgb, color_prom, dkong_decode_info, dkong_net_info);
	palette.set_pen_colors(0, rgb);

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			int r,g,b;
			r = compute_res_net( 1, 0, dkong_net_bck_info );
			g = compute_res_net( 1, 1, dkong_net_bck_info );
			b = compute_res_net( 1, 2, dkong_net_bck_info );
			palette.set_pen_color(i,r,g,b);
		}

	palette.palette()->normalize_range(0, 255);

	color_prom += 512;
	/* color_prom now points to the beginning of the character color codes */
	m_color_codes = color_prom; /* we'll need it later */
}

#ifdef UNUSED_FUNCTION
PALETTE_INIT_MEMBER(dkong_state,dkong4b)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int r,g,b;

	for (i = 0;i < 256;i++)
	{
		/* red component */
		r = compute_res_net( (color_prom[256]>>1) & 0x07, 0, radarscp_net_info );
		/* green component */
		g = compute_res_net( ((color_prom[256]<<2) & 0x04) | ((color_prom[0]>>2) & 0x03), 1, radarscp_net_info );
		/* blue component */
		b = compute_res_net( (color_prom[0]>>0) & 0x03, 2, radarscp_net_info );

		palette.set_pen_color(i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			r = compute_res_net( 1, 0, radarscp_net_bck_info );
			g = compute_res_net( 1, 1, radarscp_net_bck_info );
			b = compute_res_net( 1, 2, radarscp_net_bck_info );
			palette.set_pen_color(i,r,g,b);
		}

	palette.palette()->normalize_range(0, 255);

	color_prom += 256;
	/* color_prom now points to the beginning of the character color codes */
	m_color_codes = color_prom; /* we'll need it later */
}
#endif

PALETTE_INIT_MEMBER(dkong_state,radarscp)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int r,g,b;

	for (i = 0;i < 256;i++)
	{
		/* red component */
		r = compute_res_net( (color_prom[256]>>1) & 0x07, 0, radarscp_net_info );
		/* green component */
		g = compute_res_net( ((color_prom[256]<<2) & 0x04) | ((color_prom[0]>>2) & 0x03), 1, radarscp_net_info );
		/* blue component */
		b = compute_res_net( (color_prom[0]>>0) & 0x03, 2, radarscp_net_info );

		palette.set_pen_color(i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (m_vidhw != DKONG_RADARSCP_CONVERSION) && ( (i & 0x03) == 0x00 ))  /*  NOR => CS=1 => Tristate => real black */
		{
			r = compute_res_net( 1, 0, radarscp_net_bck_info );
			g = compute_res_net( 1, 1, radarscp_net_bck_info );
			b = compute_res_net( 1, 2, radarscp_net_bck_info );
			palette.set_pen_color(i,r,g,b);
		}

	/* Star color */
	r = compute_res_net( 1, 0, radarscp_stars_net_info );
	g = compute_res_net( 0, 1, radarscp_stars_net_info );
	b = compute_res_net( 0, 2, radarscp_stars_net_info );
	palette.set_pen_color(RADARSCP_STAR_COL,r,g,b);

	/* Oscillating background */
	for (i = 0;i < 256;i++)
	{
		r = compute_res_net( 0, 0, radarscp_blue_net_info );
		g = compute_res_net( 0, 1, radarscp_blue_net_info );
		b = compute_res_net( i, 2, radarscp_blue_net_info );

		palette.set_pen_color(RADARSCP_BCK_COL_OFFSET + i,r,g,b);
	}

	/* Grid */
	for (i = 0;i < 8;i++)
	{
		r = compute_res_net( i & 1, 0, radarscp_grid_net_info );
		g = compute_res_net( (i>>1) & 1, 1, radarscp_grid_net_info );
		b = compute_res_net( (i>>2) & 1, 2, radarscp_grid_net_info );

		palette.set_pen_color(RADARSCP_GRID_COL_OFFSET + i,r,g,b);
	}

	palette.palette()->normalize_range(0, RADARSCP_GRID_COL_OFFSET+7);

	color_prom += 256;
	/* color_prom now points to the beginning of the character color codes */
	m_color_codes = color_prom; /* we'll need it later */
}

PALETTE_INIT_MEMBER(dkong_state,radarscp1)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int r,g,b;

	for (i = 0;i < 256;i++)
	{
		/* red component */
		r = compute_res_net( color_prom[512], 0, radarscp1_net_info );
		/* green component */
		g = compute_res_net( color_prom[256], 1, radarscp1_net_info );
		/* blue component */
		b = compute_res_net( color_prom[0], 2, radarscp1_net_info );

		palette.set_pen_color(i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			r = compute_res_net( 0, 0, radarscp1_net_info );
			g = compute_res_net( 0, 1, radarscp1_net_info );
			b = compute_res_net( 0, 2, radarscp1_net_info );
			palette.set_pen_color(i,r,g,b);
		}

	/* Star color */
	r = compute_res_net( 1, 0, radarscp_stars_net_info );
	g = compute_res_net( 0, 1, radarscp_stars_net_info );
	b = compute_res_net( 0, 2, radarscp_stars_net_info );
	palette.set_pen_color(RADARSCP_STAR_COL,r,g,b);

	/* Oscillating background */
	for (i = 0;i < 256;i++)
	{
		r = compute_res_net( 0, 0, radarscp_blue_net_info );
		g = compute_res_net( 0, 1, radarscp_blue_net_info );
		b = compute_res_net( i, 2, radarscp_blue_net_info );

		palette.set_pen_color(RADARSCP_BCK_COL_OFFSET + i,r,g,b);
	}

	/* Grid */
	for (i = 0;i < 8;i++)
	{
		r = compute_res_net( i & 1, 0, radarscp_grid_net_info );
		g = compute_res_net( (i>>1) & 1, 1, radarscp_grid_net_info );
		b = compute_res_net( (i>>2) & 1, 2, radarscp_grid_net_info );

		palette.set_pen_color(RADARSCP_GRID_COL_OFFSET + i,r,g,b);
	}
	palette.palette()->normalize_range(0, RADARSCP_GRID_COL_OFFSET+7);

	color_prom += 512;
	/* color_prom now points to the beginning of the character color codes */
	m_color_codes = color_prom; /* we'll need it later */
}



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Donkey Kong 3 has two 512x8 palette PROMs and one 256x4 PROM which contains
  the color codes to use for characters on a per row/column basis (groups of
  of 4 characters in the same column - actually row, since the display is
  rotated)
  Interstingly, bytes 0-255 of the palette PROMs contain an inverted palette,
  as other Nintendo games like Donkey Kong, while bytes 256-511 contain a non
  inverted palette. This was probably done to allow connection to both the
  special Nintendo and a standard monitor.
  I don't know the exact values of the resistors between the PROMs and the
  RGB output, but they are probably the usual:

  Couriersud: Checked against the schematics.
              The proms are open collector proms and there is a pullup to
              +5V of 470 ohm.

  bit 7 -- 220 ohm resistor -- inverter  -- RED
        -- 470 ohm resistor -- inverter  -- RED
        -- 1  kohm resistor -- inverter  -- RED
        -- 2.2kohm resistor -- inverter  -- RED
        -- 220 ohm resistor -- inverter  -- GREEN
        -- 470 ohm resistor -- inverter  -- GREEN
        -- 1  kohm resistor -- inverter  -- GREEN
  bit 0 -- 2.2kohm resistor -- inverter  -- GREEN

  bit 3 -- 220 ohm resistor -- inverter  -- BLUE
        -- 470 ohm resistor -- inverter  -- BLUE
        -- 1  kohm resistor -- inverter  -- BLUE
  bit 0 -- 2.2kohm resistor -- inverter  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(dkong_state,dkong3)
{
	const UINT8 *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, dkong3_decode_info, dkong3_net_info);
	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 255);

	color_prom += 1024;
	/* color_prom now points to the beginning of the character color codes */
	m_color_codes = color_prom; /* we'll need it later */
}

TILE_GET_INFO_MEMBER(dkong_state::dkong_bg_tile_info)
{
	int code = m_video_ram[tile_index] + 256 * m_gfx_bank;
	int color = (m_color_codes[tile_index % 32 + 32 * (tile_index / 32 / 4)] & 0x0f) + 0x10 * m_palette_bank;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(dkong_state::radarscp1_bg_tile_info)
{
	int code = m_video_ram[tile_index] + 256 * m_gfx_bank;
	int color = (m_color_codes[tile_index % 32] & 0x0f);
	color = color | (m_palette_bank<<4);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

/***************************************************************************

  I/O Handling

***************************************************************************/

WRITE8_MEMBER(dkong_state::dkong_videoram_w)
{
	if (m_video_ram[offset] != data)
	{
		m_video_ram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset);
	}
}

WRITE8_MEMBER(dkong_state::dkongjr_gfxbank_w)
{
	if (m_gfx_bank != (data & 0x01))
	{
		m_gfx_bank = data & 0x01;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(dkong_state::dkong3_gfxbank_w)
{
	if (m_gfx_bank != (~data & 0x01))
	{
		m_gfx_bank = ~data & 0x01;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(dkong_state::dkong_palettebank_w)
{
	int newbank;

	newbank = m_palette_bank;

	if (data & 1)
		newbank |= 1 << offset;
	else
		newbank &= ~(1 << offset);

	if (m_palette_bank != newbank)
	{
		m_palette_bank = newbank;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(dkong_state::radarscp_grid_enable_w)
{
	m_grid_on = data & 0x01;
}

WRITE8_MEMBER(dkong_state::radarscp_grid_color_w)
{
	m_grid_col = (data & 0x07) ^ 0x07;
	/* popmessage("Gridcol: %d", m_grid_col); */
}

WRITE8_MEMBER(dkong_state::dkong_flipscreen_w)
{
	m_flip = ~data & 0x01;
}

WRITE8_MEMBER(dkong_state::dkong_spritebank_w)
{
	m_sprite_bank = data & 0x01;
}

/***************************************************************************

  Draw the game screen in the given bitmap_ind16.

***************************************************************************/

void dkong_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32 mask_bank, UINT32 shift_bits)
{
	int offs;
	int scanline_vf;    /* buffering scanline including flip */
	int scanline_vfc;   /* line buffering scanline including flip - this is the cached scanline_vf */
	int scanline;       /* current scanline */
	int add_y;
	int add_x;
	int num_sprt;

	/* Draw the sprites. There are two pecularities which have been mentioned by
	 * a Donkey Kong II author at CAX 2008:
	 * 1) On real hardware, sprites wrap around from the right to the left instead
	 *    of clipping.
	 * 2) On real hardware, there is a limit of 16 sprites per scanline.
	 *    Sprites after the 16th (starting from the left) simply don't show.
	 *
	 * 2) is in line with the real hardware which buffers the sprite data
	 * for one scanline. The ram is 64x9 and a sprite takes 4 bytes.
	 * ==> 16 sprites per scanline.
	 *
	 * TODO: 9th bit is not understood right now.
	 *
	 * 1) is due to limitation of signals to 8 bit.
	 *
	 * This is quite different from galaxian. The dkong hardware updates sprites
	 * only once every frame by dma. The number of sprites can not be processed
	 * directly, Thus the preselection. The buffering takes place during the
	 * active phase of the video signal. The scanline is than rendered into the linebuffer
	 * during HBLANK.
	 *
	 * A sprite will be drawn:
	 * a) FlipQ = 1 : (sprite_y + 0xF9 + scanline) & 0xF0 == 0xF0
	 * b) FlipQ = 0 : (sprite_y + 0xF7 + (scanline ^ 0xFF)) & 0xF0 == 0xF0
	 *
	 * FlipQ = 1 ("Normal Play"):
	 *
	 * sprite_y = 0x20
	 *
	 * scanline
	 * 0x10, 0xEF, 0x208, 0x00
	 * 0x18, 0xE7, 0x200, 0x00
	 * 0x19, 0xE6, 0x1FF, 0xF0
	 * 0x20, 0xDF, 0x1F8, 0xF0
	 *
	 */

	scanline_vf = (cliprect.max_y - 1) & 0xFF;
	scanline_vfc = (cliprect.max_y - 1) & 0xFF;
	scanline = cliprect.max_y & 0xFF;

	if (m_flip)
	{
		scanline_vf ^= 0xFF;
		scanline_vfc ^= 0xFF;
		add_y = 0xF7;
		add_x = 0xF7;
	}
	else
	{
		add_y = 0xF9;
		add_x = 0xF7;
	}

	for (offs = m_sprite_bank<<9, num_sprt=0; (num_sprt < 16) && (offs < (m_sprite_bank<<9) + 0x200) /* sprite_ram_size */; offs += 4)
	{
		int y = m_sprite_ram[offs];
		int do_draw = (((y + add_y + 1 + scanline_vf) & 0xF0) == 0xF0) ? 1 : 0;

		if (do_draw)
		{
			/* sprite_ram[offs + 2] & 0x40 is used by Donkey Kong 3 only */
			/* sprite_ram[offs + 2] & 0x30 don't seem to be used (they are */
			/* probably not part of the color code, since Mario Bros, which */
			/* has similar hardware, uses a memory mapped port to change */
			/* palette bank, so it's limited to 16 color codes) */

			int code = (m_sprite_ram[offs + 1] & 0x7f) + ((m_sprite_ram[offs + 2] & mask_bank) << shift_bits);
			int color = (m_sprite_ram[offs + 2] & 0x0f) + 16 * m_palette_bank;
			int flipx = m_sprite_ram[offs + 2] & 0x80;
			int flipy = m_sprite_ram[offs + 1] & 0x80;

			/* On the real board, the x and y are read inverted after the first
			 * buffer stage. This due to the fact that the 82S09 delivers complements
			 * of stored data on read!
			 */

			int x = (m_sprite_ram[offs + 3] + add_x + 1) & 0xFF;
			if (m_flip)
			{
				x = (x ^ 0xFF) - 15;
				flipx = !flipx;
			}
			y = scanline - ((y + add_y + 1 + scanline_vfc) & 0x0F);

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y, 0);

			// wraparound
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, color, flipx, flipy, m_flip ? x + 256 : x - 256, y, 0);
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y - 256, 0);

			num_sprt++;
		}
	}
}

/* The hardware designer must have been a real CD4049 fan
 * The blue signal is created by "abusing" the 4049
 * as an amplifier by operating it in the undefined area
 * between roughly 1.5 and 3.5V. The transfer function
 * below is an approximation but datasheets state
 * a wide range for the transfer function.
 *
 * SwitcherCad was not a real help since it can not
 * adequately model the 4049
 *
 * Sound02 will mix in noise into the periodic fluctuation
 * of the background: The 30Hz signal is used
 * to generate noise with an LS164. This signal is going
 * through a NAND (Signal RFLIP to video) and then XOR with 128V.
 * This should really be emulated using the discrete sound interface.
 * TODO: This should be part of the vblank routine
 */

inline double dkong_state::CD4049(double x)
{
	if (x>0)
		return exp(-m_cd4049_a * pow(x,m_cd4049_b));
	else
		return 1.0;
}

/* Actually the sound noise is a multivibrator with
 * a period of roughly 4.4 ms
 */

#define RC1     (2.2e3 * 22e-6) /*  22e-6; */
#define RC2     (10e3 * 33e-6)
#define RC31    (18e3 * 33e-6)
#define RC32    ((18e3 + 68e3) * 33e-6)
#define RC4     (90e3 * 0.47e-6)
#define dt      (1./60./(double) VTOTAL)
#define period2 (((INT64)(PIXEL_CLOCK) * ( 33L * 68L )) / (INT32)10000000L / 3)  /*  period/2 in pixel ... */

void dkong_state::radarscp_step(int line_cnt)
{
	/* Condensator is illegible in schematics for TRS2 board.
	 * TRS1 board states 3.3u.
	 */

	double vg3i;
	double diff;
	int sig;

	/* vsync is divided by 2 by a LS161
	 * The resulting 30 Hz signal clocks a LFSR (LS164) operating as a
	 * random number generator.
	 */

	if ( line_cnt == 0)
	{
		m_sig30Hz = (1-m_sig30Hz);
		if (m_sig30Hz)
			m_lfsr_5I = (machine().rand() > RAND_MAX/2);
	}

	/* sound2 mixes in a 30Hz noise signal.
	 * With the current model this has no real effect
	 * Included for completeness
	 */

	/* Now mix with SND02 (sound 2) line - on 74ls259, bit2 */
	address_space &space = machine().driver_data()->generic_space();
	m_rflip_sig = m_dev_6h->bit2_r(space, 0) & m_lfsr_5I;

	/* blue background generation */

	line_cnt += (256 - 8) + 1; // offset 8 needed to match monitor pictures
	if (line_cnt>511)
		line_cnt -= VTOTAL;

	sig = m_rflip_sig ^ ((line_cnt & 0x80)>>7);

	if (m_hardware_type == HARDWARE_TRS01)
		m_rflip_sig = !m_rflip_sig;

	if  (sig) /*  128VF */
		diff = (0.0 - m_cv1);
	else
		diff = (4.8 - m_cv1);
	diff = diff - diff*exp(0.0 - (1.0/RC1 * dt) );
	m_cv1 += diff;

	diff = (m_cv1 - m_cv2 - m_vg1);
	diff = diff - diff*exp(0.0 - (1.0/RC2 * dt) );
	m_cv2 += diff;

	// FIXME: use the inverse function
	// Solve the amplifier by iteration
	for (int j=1; j<=11; j++)// 11% = 1/75 / (1/75+1/10)
	{
		double f = (double) j / 100.0;
		m_vg1 = (m_cv1 - m_cv2)*(1-f) + f * m_vg2;
		m_vg2 = 5*CD4049(m_vg1/5);
	}
	// FIXME: use the inverse function
	// Solve the amplifier by iteration 50% = both resistors equal
	for (int j=10; j<=20; j++)
	{
		double f = (double) j / 40.0;
		vg3i = (1.0-f) * m_vg2 + f * m_vg3;
		m_vg3 = 5*CD4049(vg3i/5);
	}

#define RC17 (33e-6 * 1e3 * (0*4.7+1.0/(1.0/10.0+1.0/20.0+0.0/0.3)))
	diff = (m_vg3 - m_vc17);
	diff = diff - diff*exp(0.0 - (1.0/RC17 * dt) );
	m_vc17 += diff;

	double vo = (m_vg3 - m_vc17);
	vo = vo + 20.0 / (20.0+10.0) * 5;

	// Transistor is marked as OMIT in TRS-02 schems.
	//vo = vo - 0.7;


	//double vo = (vg3o - vg3)/4.7 + 5.0/16.0;
	//vo = vo / (1.0 / 4.7 + 1.0 / 16.0 + 1.0 / 30.0 );
	//printf("%f %f\n", vg3, vc17);

	m_blue_level = (int)(vo/5.0*255);
	//printf("%d\n", m_blue_level);

	/*
	 * Grid signal
	 *
	 * Mixed with ANS line (bit 5) from Port B of 8039
	 */
	if (m_grid_on && m_dev_vp2->bit5_r(space, 0))
	{
		diff = (0.0 - m_cv3);
		diff = diff - diff*exp(0.0 - (1.0/RC32 * dt) );
	}
	else
	{
		diff = (5.0 - m_cv3);
		diff = diff - diff*exp(0.0 - (1.0/RC31 * dt) );
	}
	m_cv3 += diff;

	diff = (m_vg2 - 0.8 * m_cv3 - m_cv4);
	diff = diff - diff*exp(0.0 - (1.0/RC4 * dt) );
	m_cv4 += diff;

	if (CD4049(CD4049((m_vg2 - m_cv4)/5.0))>2.4/5.0) /* TTL - Level */
		m_grid_sig = 0;
	else
		m_grid_sig = 1;

	/* stars */
	m_pixelcnt += HTOTAL;
	if (m_pixelcnt > period2 )
	{
		m_star_ff = !m_star_ff;
		m_pixelcnt = m_pixelcnt - period2;
	}

}

void dkong_state::radarscp_draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8     *htable = NULL;
	int             x,y;
	UINT8           draw_ok;
	UINT16          *pixel;

	if (m_hardware_type == HARDWARE_TRS01)
		htable = m_gfx4;

	y = cliprect.min_y;
	while (y <= cliprect.max_y)
	{
		x = cliprect.min_x;
		while (x <= cliprect.max_x)
		{
			pixel = &bitmap.pix16(y, x);
			draw_ok = !(*pixel & 0x01) && !(*pixel & 0x02);
			if (m_hardware_type == HARDWARE_TRS01) /*  Check again from schematics */
				draw_ok = draw_ok  && !((htable[ (!m_rflip_sig<<7) | (x>>2)] >>2) & 0x01);
			if (draw_ok)
				*pixel = *(&m_bg_bits.pix16(y, x));
			x++;
		}
		y++;
	}
}

void dkong_state::radarscp_scanline(int scanline)
{
	const UINT8 *table = m_gfx3;
	int         table_len = m_gfx3_len;
	int             x,y,offset;
	UINT16          *pixel;
	const rectangle &visarea = m_screen->visible_area();

	y = scanline;
	radarscp_step(y);
	if (y <= visarea.min_y || y > visarea.max_y)
		m_counter = 0;
	offset = (m_flip ^ m_rflip_sig) ? 0x000 : 0x400;
	x = 0;
	while (x < m_screen->width())
	{
		pixel = &m_bg_bits.pix16(y, x);
		if ((m_counter < table_len) && (x == 4 * (table[m_counter|offset] & 0x7f)))
		{
			if ( m_star_ff && (table[m_counter|offset] & 0x80) )    /* star */
				*pixel = RADARSCP_STAR_COL;
			else if (m_grid_sig && !(table[m_counter|offset] & 0x80))           /* radar */
				*pixel = RADARSCP_GRID_COL_OFFSET+m_grid_col;
			else
				*pixel = RADARSCP_BCK_COL_OFFSET + m_blue_level;
			m_counter++;
		}
		else
			*pixel = RADARSCP_BCK_COL_OFFSET + m_blue_level;
		x++;
	}
	while ((m_counter < table_len) && ( x < 4 * (table[m_counter|offset] & 0x7f)))
		m_counter++;
}

TIMER_CALLBACK_MEMBER(dkong_state::scanline_callback)
{
	int scanline = param;

	if ((m_hardware_type == HARDWARE_TRS02) || (m_hardware_type == HARDWARE_TRS01))
		radarscp_scanline(scanline);

	/* update any video up to the current scanline */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	scanline = (scanline+1) % VTOTAL;
	/* come back at the next appropriate scanline */
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

void dkong_state::check_palette()
{
	ioport_port *port;
	int newset;

	port = ioport("VIDHW");
	if (port != NULL)
	{
		newset = port->read();
		if (newset != m_vidhw)
		{
			m_vidhw = newset;
			switch (newset)
			{
				case DKONG_RADARSCP_CONVERSION:
					PALETTE_INIT_NAME(radarscp)(m_palette);
					break;
				case DKONG_BOARD:
					PALETTE_INIT_NAME(dkong2b)(m_palette);
					break;
			}
		}
	}
}

VIDEO_START_MEMBER(dkong_state,dkong_base)
{
	m_cd4049_b = (log(0.0 - log(cd4049_al)) - log(0.0 - log((1.0-cd4049_al))) ) / log(cd4049_vh/cd4049_vl);
	m_cd4049_a = log(0.0 - log(cd4049_al)) - m_cd4049_b * log(cd4049_vh);

	m_gfx_bank = 0;
	m_palette_bank = 0;
	m_sprite_bank = 0;
	m_vidhw = -1;

	save_item(NAME(m_vidhw));
	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_grid_on));

	save_item(NAME(m_grid_col));
	save_item(NAME(m_flip));

	// TRS01 TRS02
	save_item(NAME(m_sig30Hz));
	save_item(NAME(m_blue_level));
	save_item(NAME(m_cv1));
	save_item(NAME(m_cv2));
	save_item(NAME(m_vg1));
	save_item(NAME(m_vg2));
	save_item(NAME(m_vg3));
	save_item(NAME(m_cv3));
	save_item(NAME(m_cv4));

	save_item(NAME(m_lfsr_5I));
	save_item(NAME(m_grid_sig));
	save_item(NAME(m_rflip_sig));
	save_item(NAME(m_star_ff));
	save_item(NAME(m_counter));
	save_item(NAME(m_pixelcnt));
	save_item(NAME(m_bg_bits));

}

VIDEO_START_MEMBER(dkong_state,dkong)
{
	VIDEO_START_CALL_MEMBER(dkong_base);

	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dkong_state::scanline_callback),this));
	m_scanline_timer->adjust(m_screen->time_until_pos(0));

	switch (m_hardware_type)
	{
		case HARDWARE_TRS02:
			m_screen->register_screen_bitmap(m_bg_bits);
			m_gfx3 = memregion("gfx3")->base();
			m_gfx3_len = memregion("gfx3")->bytes();
			/* fall through */
		case HARDWARE_TKG04:
		case HARDWARE_TKG02:
			m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dkong_state::dkong_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
			break;
		case HARDWARE_TRS01:
			m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dkong_state::radarscp1_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);

			m_screen->register_screen_bitmap(m_bg_bits);
			m_gfx4 = memregion("gfx4")->base();
			m_gfx3 = memregion("gfx3")->base();
			m_gfx3_len = memregion("gfx3")->bytes();

			break;
		default:
			fatalerror("Invalid hardware type in dkong_video_start\n");
	}
}

UINT32 dkong_state::screen_update_dkong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	switch (m_hardware_type)
	{
		case HARDWARE_TKG02:
		case HARDWARE_TKG04:
			check_palette();
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			draw_sprites(bitmap, cliprect, 0x40, 1);
			break;
		case HARDWARE_TRS01:
		case HARDWARE_TRS02:
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			draw_sprites(bitmap, cliprect, 0x40, 1);
			radarscp_draw_background(bitmap, cliprect);
			break;
		default:
			fatalerror("Invalid hardware type in dkong_video_update\n");
	}
	return 0;
}

UINT32 dkong_state::screen_update_pestplce(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* Draw the sprites. */
	for (offs = 0;offs < m_sprite_ram.bytes();offs += 4)
	{
		if (m_sprite_ram[offs])
		{
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_sprite_ram[offs + 2],
					(m_sprite_ram[offs + 1] & 0x0f) + 16 * m_palette_bank,
					m_sprite_ram[offs + 1] & 0x80,m_sprite_ram[offs + 1] & 0x40,
					m_sprite_ram[offs + 3] - 8,240 - m_sprite_ram[offs] + 8,0);
		}
	}
	return 0;
}

UINT32 dkong_state::screen_update_spclforc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* it uses sprite_ram[offs + 2] & 0x10 for sprite bank */
	draw_sprites(bitmap, cliprect, 0x10, 3);
	return 0;
}
