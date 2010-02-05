/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.


***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/dkong.h"
#include "machine/latch8.h"

#define RADARSCP_BCK_COL_OFFSET			256
#define RADARSCP_GRID_COL_OFFSET		(RADARSCP_BCK_COL_OFFSET + 256)
#define RADARSCP_STAR_COL				(RADARSCP_GRID_COL_OFFSET + 8)

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
	2,		/*  there may be two proms needed to construct color */
	0,		/*  start at 0 */
	255,	/*  end at 255 */
	/*  R,   G,   B,   R,   G,   B */
	{ 256, 256,   0,   0,   0,   0},		/*  offsets */
	{   1,  -2,   0,   0,   2,   0},		/*  shifts */
	{0x07,0x04,0x03,0x00,0x03,0x00}		    /*  masks */
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
	1,		/*  one prom needed to contruct color */
	0,		/*  start at 0 */
	255,	/*  end at 255 */
	/*   R,   G,   B */
	{   0,   0, 512 },		/*  offsets */
	{   4,   0,   0 },		/*  shifts */
	{0x0F,0x0F,0x0F }		    /*  masks */
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

static const res_net_info radarscp_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_TTL | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470,      0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470,      0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 680, 150000, 2, {  470, 220,   0 } }    /*  radarscp */
	}
};

static const res_net_info radarscp_net_bck_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_TTL | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470,      0, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON, 470,      0, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON, 680, 150000, 0, { 0 } }    /*  radarscp */
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
		{ RES_NET_AMP_DARLINGTON, 0,      0, 4, { 39000, 20000, 10000, 4990 } }
	}
};

/* Radarscp star color */

static const res_net_info radarscp_stars_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 4700, 470, 0, { 0 } },
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },	/*  dummy */
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },	/*  dummy */
	}
};

/* Dummy struct to generate background palette entries */

static const res_net_info radarscp_blue_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_VCC | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON,  470, 4700, 0, { 0 } },	/*  bias/gnd exist in schematics, readable in TKG3 schematics */
		{ RES_NET_AMP_DARLINGTON,  470, 4700, 0, { 0 } },	/*  bias/gnd exist in schematics, readable in TKG3 schematics */
		{ RES_NET_AMP_DARLINGTON,    0,    0, 8, { 128,64,32,16,8,4,2,1 } },	/*  dummy */
	}
};

/* Dummy struct to generate grid palette entries */

static const res_net_info radarscp_grid_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },	/*  dummy */
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },	/*  dummy */
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },	/*  dummy */
	}
};

/***************************************************************************

  PALETTE_INIT

***************************************************************************/

PALETTE_INIT( dkong2b)
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	rgb_t	*rgb;
	int i;

	rgb = compute_res_net_all(machine, color_prom, &dkong_decode_info, &dkong_net_info);
	palette_set_colors(machine, 0, rgb, 256);

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			int r,g,b;
			r = compute_res_net( 1, 0, &dkong_net_bck_info );
			g = compute_res_net( 1, 1, &dkong_net_bck_info );
			b = compute_res_net( 1, 2, &dkong_net_bck_info );
			palette_set_color_rgb(machine,i,r,g,b);
		}

	palette_normalize_range(machine->palette, 0, 255, 0, 255);

	color_prom += 512;
	/* color_prom now points to the beginning of the character color codes */
	state->color_codes = color_prom;	/* we'll need it later */
	auto_free(machine, rgb);
}

#ifdef UNUSED_FUNCTION
PALETTE_INIT( dkong4b )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int i;
	int r,g,b;

	for (i = 0;i < 256;i++)
	{

		/* red component */
		r = compute_res_net( (color_prom[256]>>1) & 0x07, 0, &radarscp_net_info );
		/* green component */
		g = compute_res_net( ((color_prom[256]<<2) & 0x04) | ((color_prom[0]>>2) & 0x03), 1, &radarscp_net_info );
		/* blue component */
		b = compute_res_net( (color_prom[0]>>0) & 0x03, 2, &radarscp_net_info );

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			r = compute_res_net( 1, 0, &radarscp_net_bck_info );
			g = compute_res_net( 1, 1, &radarscp_net_bck_info );
			b = compute_res_net( 1, 2, &radarscp_net_bck_info );
			palette_set_color_rgb(machine,i,r,g,b);
		}

	palette_normalize_range(machine->palette, 0, 255, 0, 255);

	color_prom += 256;
	/* color_prom now points to the beginning of the character color codes */
	state->color_codes = color_prom;	/* we'll need it later */
}
#endif

PALETTE_INIT( radarscp )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int i;
	int r,g,b;

	for (i = 0;i < 256;i++)
	{

		/* red component */
		r = compute_res_net( (color_prom[256]>>1) & 0x07, 0, &radarscp_net_info );
		/* green component */
		g = compute_res_net( ((color_prom[256]<<2) & 0x04) | ((color_prom[0]>>2) & 0x03), 1, &radarscp_net_info );
		/* blue component */
		b = compute_res_net( (color_prom[0]>>0) & 0x03, 2, &radarscp_net_info );

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			r = compute_res_net( 1, 0, &radarscp_net_bck_info );
			g = compute_res_net( 1, 1, &radarscp_net_bck_info );
			b = compute_res_net( 1, 2, &radarscp_net_bck_info );
			palette_set_color_rgb(machine,i,r,g,b);
		}

	/* Star color */
	r = compute_res_net( 1, 0, &radarscp_stars_net_info );
	g = compute_res_net( 0, 1, &radarscp_stars_net_info );
	b = compute_res_net( 0, 2, &radarscp_stars_net_info );
	palette_set_color_rgb(machine,RADARSCP_STAR_COL,r,g,b);

	/* Oscillating background */
	for (i = 0;i < 256;i++)
	{
		r = compute_res_net( 0, 0, &radarscp_blue_net_info );
		g = compute_res_net( 0, 1, &radarscp_blue_net_info );
		b = compute_res_net( i, 2, &radarscp_blue_net_info );

		palette_set_color_rgb(machine,RADARSCP_BCK_COL_OFFSET + i,r,g,b);
	}

	/* Grid */
	for (i = 0;i < 8;i++)
	{
		r = compute_res_net( i & 1, 0, &radarscp_grid_net_info );
		g = compute_res_net( (i>>1) & 1, 1, &radarscp_grid_net_info );
		b = compute_res_net( (i>>2) & 1, 2, &radarscp_grid_net_info );

		palette_set_color_rgb(machine,RADARSCP_GRID_COL_OFFSET + i,r,g,b);
	}

	palette_normalize_range(machine->palette, 0, RADARSCP_GRID_COL_OFFSET+7, 0, 255);

	color_prom += 256;
	/* color_prom now points to the beginning of the character color codes */
	state->color_codes = color_prom;	/* we'll need it later */
}

PALETTE_INIT( radarscp1 )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int i;
	int r,g,b;

	for (i = 0;i < 256;i++)
	{

		/* red component */
		r = compute_res_net( color_prom[512], 0, &radarscp1_net_info );
		/* green component */
		g = compute_res_net( color_prom[256], 1, &radarscp1_net_info );
		/* blue component */
		b = compute_res_net( color_prom[0], 2, &radarscp1_net_info );

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  /*  NOR => CS=1 => Tristate => real black */
		{
			r = compute_res_net( 0, 0, &radarscp1_net_info );
			g = compute_res_net( 0, 1, &radarscp1_net_info );
			b = compute_res_net( 0, 2, &radarscp1_net_info );
			palette_set_color_rgb(machine,i,r,g,b);
		}

	/* Star color */
	r = compute_res_net( 1, 0, &radarscp_stars_net_info );
	g = compute_res_net( 0, 1, &radarscp_stars_net_info );
	b = compute_res_net( 0, 2, &radarscp_stars_net_info );
	palette_set_color_rgb(machine,RADARSCP_STAR_COL,r,g,b);

	/* Oscillating background */
	for (i = 0;i < 256;i++)
	{
		r = compute_res_net( 0, 0, &radarscp_blue_net_info );
		g = compute_res_net( 0, 1, &radarscp_blue_net_info );
		b = compute_res_net( i, 2, &radarscp_blue_net_info );

		palette_set_color_rgb(machine,RADARSCP_BCK_COL_OFFSET + i,r,g,b);
	}

	/* Grid */
	for (i = 0;i < 8;i++)
	{
		r = compute_res_net( i & 1, 0, &radarscp_grid_net_info );
		g = compute_res_net( (i>>1) & 1, 1, &radarscp_grid_net_info );
		b = compute_res_net( (i>>2) & 1, 2, &radarscp_grid_net_info );

		palette_set_color_rgb(machine,RADARSCP_GRID_COL_OFFSET + i,r,g,b);
	}
	palette_normalize_range(machine->palette, 0, RADARSCP_GRID_COL_OFFSET+7, 0, 255);

	color_prom += 512;
	/* color_prom now points to the beginning of the character color codes */
	state->color_codes = color_prom;	/* we'll need it later */
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

PALETTE_INIT( dkong3 )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	rgb_t	*rgb;

	rgb = compute_res_net_all(machine, color_prom, &dkong3_decode_info, &dkong3_net_info);
	palette_set_colors(machine, 0, rgb, 256);
	palette_normalize_range(machine->palette, 0, 255, 0, 255);
	auto_free(machine, rgb);

	color_prom += 1024;
	/* color_prom now points to the beginning of the character color codes */
	state->color_codes = color_prom;	/* we'll need it later */
}

static TILE_GET_INFO( dkong_bg_tile_info )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int code = state->video_ram[tile_index] + 256 * state->gfx_bank;
	int color = (state->color_codes[tile_index % 32 + 32 * (tile_index / 32 / 4)] & 0x0f) + 0x10 * state->palette_bank;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( radarscp1_bg_tile_info )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int code = state->video_ram[tile_index] + 256 * state->gfx_bank;
	int color = (state->color_codes[tile_index % 32] & 0x0f);
	color = color | (state->palette_bank<<4);

	SET_TILE_INFO(0, code, color, 0);
}

/***************************************************************************

  I/O Handling

***************************************************************************/

WRITE8_HANDLER( dkong_videoram_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	if (state->video_ram[offset] != data)
	{
		state->video_ram[offset] = data;
		tilemap_mark_tile_dirty(state->bg_tilemap, offset);
	}
}

WRITE8_HANDLER( dkongjr_gfxbank_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	if (state->gfx_bank != (data & 0x01))
	{
		state->gfx_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

WRITE8_HANDLER( dkong3_gfxbank_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	if (state->gfx_bank != (~data & 0x01))
	{
		state->gfx_bank = ~data & 0x01;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

WRITE8_HANDLER( dkong_palettebank_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;
	int newbank;

	newbank = state->palette_bank;

	if (data & 1)
		newbank |= 1 << offset;
	else
		newbank &= ~(1 << offset);

	if (state->palette_bank != newbank)
	{
		state->palette_bank = newbank;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

WRITE8_HANDLER( radarscp_grid_enable_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	state->grid_on = data & 0x01;
}

WRITE8_HANDLER( radarscp_grid_color_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	state->grid_col = (data & 0x07) ^ 0x07;
	/* popmessage("Gridcol: %d", state->grid_col); */
}

WRITE8_HANDLER( dkong_flipscreen_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	state->flip = ~data & 0x01;
}

WRITE8_HANDLER( dkong_spritebank_w )
{
	dkong_state *state = (dkong_state *)space->machine->driver_data;

	state->sprite_bank = data & 0x01;
}

/***************************************************************************

  Draw the game screen in the given bitmap_t.

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 mask_bank, UINT32 shift_bits)
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int offs;
	int scanline_vf;	/* buffering scanline including flip */
	int scanline_vfc;		/* line buffering scanline including flip - this is the cached scanline_vf*/
	int scanline;		/* current scanline */
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

	scanline_vf = (cliprect->max_y - 1) & 0xFF;
	scanline_vfc = (cliprect->max_y - 1) & 0xFF;
	scanline = cliprect->max_y & 0xFF;

	if (state->flip)
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

	for (offs = state->sprite_bank<<9, num_sprt=0; (num_sprt < 16) && (offs < (state->sprite_bank<<9) + 0x200) /* sprite_ram_size */; offs += 4)
	{
		int y = state->sprite_ram[offs];
		int do_draw = (((y + add_y + 1 + scanline_vf) & 0xF0) == 0xF0) ? 1 : 0;

		if (do_draw)
		{
			/* sprite_ram[offs + 2] & 0x40 is used by Donkey Kong 3 only */
			/* sprite_ram[offs + 2] & 0x30 don't seem to be used (they are */
			/* probably not part of the color code, since Mario Bros, which */
			/* has similar hardware, uses a memory mapped port to change */
			/* palette bank, so it's limited to 16 color codes) */

			int x = state->sprite_ram[offs + 3];

			/* On the real board, the x and y are read inverted after the first
             * buffer stage. This due to the fact that the 82S09 delivers complements
             * of stored data on read!
             */

			x = (x + add_x + 1) & 0xFF;
			if (state->flip)
				x ^= 0xFF;
			y = (y + add_y + 1 + scanline_vfc) & 0x0F;

			if (state->flip)
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						(state->sprite_ram[offs + 1] & 0x7f) + ((state->sprite_ram[offs + 2] & mask_bank) << shift_bits),
						(state->sprite_ram[offs + 2] & 0x0f) + 16 * state->palette_bank,
						!(state->sprite_ram[offs + 2] & 0x80),(state->sprite_ram[offs + 1] & 0x80),
						x-15, scanline-y,0);
			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						(state->sprite_ram[offs + 1] & 0x7f) + ((state->sprite_ram[offs + 2] & mask_bank) << shift_bits),
						(state->sprite_ram[offs + 2] & 0x0f) + 16 * state->palette_bank,
						(state->sprite_ram[offs + 2] & 0x80),(state->sprite_ram[offs + 1] & 0x80),
						x, scanline-y,0);
			}

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

INLINE double CD4049(running_machine *machine, double x)
{
	dkong_state *state = (dkong_state *)machine->driver_data;

	if (x>0)
		return exp(-state->cd4049_a * pow(x,state->cd4049_b));
	else
		return 1.0;
}

/* Actually the sound noise is a multivibrator with
 * a period of roughly 4.4 ms
 */

#define RC1		(2.2e3 * 22e-6) /*  22e-6; */
#define RC2		(10e3 * 33e-6)
#define RC31	(18e3 * 33e-6)
#define RC32	((18e3 + 68e3) * 33e-6)
#define RC4		(90e3 * 0.47e-6)
#define dt		(1./60./(double) VTOTAL)
#define period2 (((INT64)(PIXEL_CLOCK) * ( 33L * 68L )) / (INT32)10000000L / 3)  /*  period/2 in pixel ... */

static void radarscp_step(running_machine *machine, int line_cnt)
{
	dkong_state *state = (dkong_state *)machine->driver_data;

	/* Condensator is illegible in schematics for TRS2 board.
     * TRS1 board states 3.3u.
     */

	static double cv1=0,cv2=0,vg1=0,vg2=0,vg3=0,cv3=0,cv4=0;
	double vg3i;
	static int pixelcnt = 0;
	double diff;
	int sig;

	line_cnt += 256;
	if (line_cnt>511)
		line_cnt -= VTOTAL;

	/* sound2 mixes in a 30Hz noise signal.
     * With the current model this has no real effect
     * Included for completeness
     */

	line_cnt++;
	if (line_cnt>=512)
		line_cnt=512-VTOTAL;

	if ( ( !(line_cnt & 0x40) && ((line_cnt+1) & 0x40) ) && (mame_rand(machine) > RAND_MAX/2))
		state->sig30Hz = (1-state->sig30Hz);

	/* Now mix with SND02 (sound 2) line - on 74ls259, bit2 */
	state->rflip_sig = latch8_bit2_r(state->dev_6h, 0) & state->sig30Hz;

	sig = state->rflip_sig ^ ((line_cnt & 0x80)>>7);

	if (state->hardware_type == HARDWARE_TRS01)
		state->rflip_sig = !state->rflip_sig;

	if  (sig) /*  128VF */
		diff = (0.0 - cv1);
	else
		diff = (3.4 - cv1);
	diff = diff - diff*exp(0.0 - (1.0/RC1 * dt) );
	cv1 += diff;

	diff = (cv1 - cv2 - vg1);
	diff = diff - diff*exp(0.0 - (1.0/RC2 * dt) );
	cv2 += diff;

	vg1 = (cv1 - cv2)*0.9 + 0.1 * vg2;
	vg2 = 5*CD4049(machine, vg1/5);

	/* on the real hardware, the gain would be 1.
     * This will not work here.
     */
	vg3i = 0.9*vg2 + 0.1 * vg3;
	vg3 = 5*CD4049(machine, vg3i/5);

	state->blue_level = (int)(vg3/5.0*255);

	/*
     * Grid signal
     *
     * Mixed with ANS line (bit 5) from Port B of 8039
     */
	if (state->grid_on && latch8_bit5_r(state->dev_vp2, 0))
	{
		diff = (0.0 - cv3);
		diff = diff - diff*exp(0.0 - (1.0/RC32 * dt) );
	}
	else
	{
		diff = (5.0 - cv3);
		diff = diff - diff*exp(0.0 - (1.0/RC31 * dt) );
	}
	cv3 += diff;

	diff = (vg2 - 0.8 * cv3 - cv4);
	diff = diff - diff*exp(0.0 - (1.0/RC4 * dt) );
	cv4 += diff;

	if (CD4049(machine, CD4049(machine, vg2 - cv4))>2.4/5.0) /* TTL - Level */
		state->grid_sig = 0;
	else
		state->grid_sig = 1;

	/* stars */
	pixelcnt += HTOTAL;
	if (pixelcnt > period2 )
	{
		state->star_ff = !state->star_ff;
		pixelcnt = pixelcnt - period2;
	}

}

static void radarscp_draw_background(running_machine *machine, dkong_state *state, bitmap_t *bitmap, const rectangle *cliprect)
{
	const UINT8 	*htable = NULL;
	int 			x,y;
	UINT8			draw_ok;
	UINT16			*pixel;

	if (state->hardware_type == HARDWARE_TRS01)
		htable = state->gfx4;

	y = cliprect->min_y;
	while (y <= cliprect->max_y)
	{
		x = cliprect->min_x;
		while (x <= cliprect->max_x)
		{
			pixel = BITMAP_ADDR16(bitmap, y, x);
			draw_ok = !(*pixel & 0x01) && !(*pixel & 0x02);
			if (state->hardware_type == HARDWARE_TRS01) /*  Check again from schematics */
				draw_ok = draw_ok  && !((htable[ (!state->rflip_sig<<7) | (x>>2)] >>2) & 0x01);
			if (draw_ok)
				*pixel = *(BITMAP_ADDR16(state->bg_bits, y, x));
			x++;
		}
		y++;
	}
}

static void radarscp_scanline(running_machine *machine, int scanline)
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	const UINT8 *table = state->gfx3;
	int 		table_len = state->gfx3_len;
	int 			x,y,offset;
	UINT16			*pixel;
	static int		counter=0;
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	y = scanline;
	radarscp_step(machine, y);
	if (y <= visarea->min_y || y > visarea->max_y)
		counter = 0;
	offset = (state->flip ^ state->rflip_sig) ? 0x000 : 0x400;
	x = 0;
	while (x < video_screen_get_width(machine->primary_screen))
	{
		pixel = BITMAP_ADDR16(state->bg_bits, y, x);
		if ((counter < table_len) && (x == 4 * (table[counter|offset] & 0x7f)))
		{
			if ( state->star_ff && (table[counter|offset] & 0x80) )	/* star */
				*pixel = RADARSCP_STAR_COL;
			else if (state->grid_sig && !(table[counter|offset] & 0x80))			/* radar */
				*pixel = RADARSCP_GRID_COL_OFFSET+state->grid_col;
			else
				*pixel = RADARSCP_BCK_COL_OFFSET + state->blue_level;
			counter++;
		}
		else
			*pixel = RADARSCP_BCK_COL_OFFSET + state->blue_level;
		x++;
	}
	while ((counter < table_len) && ( x < 4 * (table[counter|offset] & 0x7f)))
		counter++;
}

static TIMER_CALLBACK( scanline_callback )
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	int scanline = param;

	if ((state->hardware_type == HARDWARE_TRS02) || (state->hardware_type == HARDWARE_TRS01))
		radarscp_scanline(machine, scanline);

	/* update any video up to the current scanline */
	video_screen_update_now(machine->primary_screen);

	scanline = (scanline+1) % VTOTAL;
	/* come back at the next appropriate scanline */
	timer_adjust_oneshot(state->scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline, 0), scanline);
}

static void check_palette(running_machine *machine)
{
	dkong_state *state = (dkong_state *)machine->driver_data;
	const input_port_config *port;
	int newset;

	port = machine->port("VIDHW");
	if (port != NULL)
	{
		newset = input_port_read_direct(port);
		if (newset != state->vidhw)
		{
			const UINT8 *color_prom;
			state->vidhw = newset;
			switch (newset)
			{
				case 0x00:
					color_prom = memory_region(machine, "proms");
					PALETTE_INIT_CALL(radarscp);
					break;
				case 0x01:
					color_prom = memory_region(machine, "proms");
					PALETTE_INIT_CALL(dkong2b);
					break;
			}
		}
	}
}

static VIDEO_START( dkong_base )
{
	dkong_state *state = (dkong_state *)machine->driver_data;

	state->cd4049_b = (log(0.0 - log(cd4049_al)) - log(0.0 - log((1.0-cd4049_al))) ) / log(cd4049_vh/cd4049_vl);
	state->cd4049_a = log(0.0 - log(cd4049_al)) - state->cd4049_b * log(cd4049_vh);

	state->gfx_bank = 0;
	state->palette_bank = 0;
	state->sprite_bank = 0;
	state->vidhw = -1;

	state_save_register_global(machine, state->gfx_bank);
	state_save_register_global(machine, state->palette_bank);
	state_save_register_global(machine, state->sprite_bank);
	state_save_register_global(machine, state->grid_on);

	state_save_register_global(machine, state->grid_col);
	state_save_register_global(machine, state->flip);
}

VIDEO_START( dkong )
{
	dkong_state *state = (dkong_state *)machine->driver_data;

	VIDEO_START_CALL(dkong_base);

	state->scanline_timer = timer_alloc(machine, scanline_callback, NULL);
	timer_adjust_oneshot(state->scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);

	switch (state->hardware_type)
	{
		case HARDWARE_TRS02:
			state->bg_bits = video_screen_auto_bitmap_alloc(machine->primary_screen);
			state->gfx3 = memory_region(machine, "gfx3");
			state->gfx3_len = memory_region_length(machine, "gfx3");
		    /* fall through */
		case HARDWARE_TKG04:
		case HARDWARE_TKG02:
			state->bg_tilemap = tilemap_create(machine, dkong_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
			tilemap_set_scrolldx(state->bg_tilemap, 0, 128);
			break;
		case HARDWARE_TRS01:
			state->bg_tilemap = tilemap_create(machine, radarscp1_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
			tilemap_set_scrolldx(state->bg_tilemap, 0, 128);

			state->bg_bits = video_screen_auto_bitmap_alloc(machine->primary_screen);
			state->gfx4 = memory_region(machine, "gfx4");
			state->gfx3 = memory_region(machine, "gfx3");
			state->gfx3_len = memory_region_length(machine, "gfx3");

			break;
		default:
			fatalerror("Invalid hardware type in dkong_video_start");
	}
}

VIDEO_UPDATE( dkong )
{
	dkong_state *state = (dkong_state *)screen->machine->driver_data;

	tilemap_set_flip_all(screen->machine, state->flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->flip ?  0 : 0);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->flip ? -8 : 0);

	switch (state->hardware_type)
	{
		case HARDWARE_TKG02:
		case HARDWARE_TKG04:
			check_palette(screen->machine);
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
			draw_sprites(screen->machine, bitmap, cliprect, 0x40, 1);
			break;
		case HARDWARE_TRS01:
		case HARDWARE_TRS02:
			tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
			draw_sprites(screen->machine, bitmap, cliprect, 0x40, 1);
			radarscp_draw_background(screen->machine, state, bitmap, cliprect);
			break;
		default:
			fatalerror("Invalid hardware type in dkong_video_update");
	}
	return 0;
}

VIDEO_UPDATE( pestplce )
{
	dkong_state *state = (dkong_state *)screen->machine->driver_data;
	int offs;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* Draw the sprites. */
	for (offs = 0;offs < state->sprite_ram_size;offs += 4)
	{
		if (state->sprite_ram[offs])
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
					state->sprite_ram[offs + 2],
					(state->sprite_ram[offs + 1] & 0x0f) + 16 * state->palette_bank,
					state->sprite_ram[offs + 1] & 0x80,state->sprite_ram[offs + 1] & 0x40,
					state->sprite_ram[offs + 3] - 8,240 - state->sprite_ram[offs] + 8,0);
		}
	}
	return 0;
}

VIDEO_UPDATE( spclforc )
{
	dkong_state *state = (dkong_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* it uses sprite_ram[offs + 2] & 0x10 for sprite bank */
	draw_sprites(screen->machine, bitmap, cliprect, 0x10, 3);
	return 0;
}
