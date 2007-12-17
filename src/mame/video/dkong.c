/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.


***************************************************************************/

#include "driver.h"
#include "video/resnet.h"
#include "includes/dkong.h"

#define RADARSCP_BCK_COL_OFFSET			256
#define RADARSCP_GRID_COL_OFFSET		(RADARSCP_BCK_COL_OFFSET + 256)
#define RADARSCP_STAR_COL				(RADARSCP_GRID_COL_OFFSET + 8)

#define HW_TRS01			1
#define HW_TRS02			2
#define HW_DK4				4

static tilemap *bg_tilemap;

static mame_bitmap *bg_bits;
static const UINT8 *color_codes;
static emu_timer *scanline_timer;

static const double cd4049_vl = 1.5/5.0;
static const double cd4049_vh = 3.5/5.0;
static const double cd4049_al = 0.01;

static UINT8	sig30Hz = 0;
static UINT8	grid_sig = 0;
static UINT8	rflip_sig = 0;
static UINT8	star_ff = 0;

static int	blue_level = 0;
static double cd4049_a, cd4049_b;

/* Save state relevant */

static UINT8	gfx_bank, palette_bank;
static UINT8	grid_on;
static UINT8	snd02_enable;
static UINT8	sig_ansn;
static UINT16	grid_col;
static UINT8	vid_hw;
static UINT8	sprite_bank;

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
	2,		// there may be two proms needed to construct color
	0,		// start at 0
	255,	// end at 255
	//  R,   G,   B,   R,   G,   B
	{ 256, 256,   0,   0,   0,   0},		// offsets
	{   1,  -2,   0,   0,   2,   0},		// shifts
	{0x07,0x04,0x03,0x00,0x03,0x00}		    // masks
};

static const res_net_info dkong_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 |  RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
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
	1,		// one prom needed to contruct color
	0,		// start at 0
	255,	// end at 255
	//  R,   G,   B
	{   0,   0, 512 },		// offsets
	{   4,   0,   0 },		// shifts
	{0x0F,0x0F,0x0F }		    // masks
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
		{ RES_NET_AMP_DARLINGTON, 680, 150000, 2, {  470, 220,   0 } }    // radarscp
	}
};

/*
    radarsc1 interface

    All outputs are open-collector. They are followed by inverters which
    drive the resistor network. All outputs are routed through a complimentary
    darlington.
*/

static const res_net_info radarsc1_net_info =
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
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },	// dummy
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },	// dummy
	}
};

/* Dummy struct to generate background palette entries */

static const res_net_info radarscp_bck_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_VCC | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },	// bias/gnd exist in schematics, but not readable
		{ RES_NET_AMP_DARLINGTON,    1,   0, 0, { 0 } },	// bias/gnd exist in schematics, but not readable
		{ RES_NET_AMP_DARLINGTON,    0,   0, 8, { 128,64,32,16,8,4,2,1 } },	// dummy
	}
};

/* Dummy struct to generate grid palette entries */

static const res_net_info radarscp_grid_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },	// dummy
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },	// dummy
		{ RES_NET_AMP_DARLINGTON,    0,   0, 1, { 1 } },	// dummy
	}
};

/***************************************************************************

  PALETTE_INIT

***************************************************************************/

PALETTE_INIT( dkong )
{
	rgb_t	*rgb;
	int i;

	rgb = compute_res_net_all(color_prom, &dkong_decode_info, &dkong_net_info);
	palette_set_colors(machine, 0, rgb, 256);

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  // NOR => CS=1 => Tristate => real black
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
	color_codes = color_prom;	/* we'll need it later */
	free(rgb);
}

PALETTE_INIT( radarscp )
{
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
		if ( (i & 0x03) == 0x00 )  // NOR => CS=1 => Tristate => real black
		{
			r = compute_res_net( 1, 0, &dkong_net_bck_info );
			g = compute_res_net( 1, 1, &dkong_net_bck_info );
			b = compute_res_net( 1, 2, &dkong_net_bck_info );
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
		r = compute_res_net( 0, 0, &radarscp_bck_net_info );
		g = compute_res_net( 0, 1, &radarscp_bck_net_info );
		b = compute_res_net( i, 2, &radarscp_bck_net_info );

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
	color_codes = color_prom;	/* we'll need it later */
}

PALETTE_INIT( radarsc1 )
{
	int i;
	int r,g,b;

	printf("Here\n");
	for (i = 0;i < 256;i++)
	{

		/* red component */
		r = compute_res_net( color_prom[512], 0, &radarsc1_net_info );
		/* green component */
		g = compute_res_net( color_prom[256], 1, &radarsc1_net_info );
		/* blue component */
		b = compute_res_net( color_prom[0], 2, &radarsc1_net_info );

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}

	/* Now treat tri-state black background generation */

	for (i=0;i<256;i++)
		if ( (i & 0x03) == 0x00 )  // NOR => CS=1 => Tristate => real black
		{
			r = compute_res_net( 0, 0, &radarsc1_net_info );
			g = compute_res_net( 0, 1, &radarsc1_net_info );
			b = compute_res_net( 0, 2, &radarsc1_net_info );
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
		r = compute_res_net( 0, 0, &radarscp_bck_net_info );
		g = compute_res_net( 0, 1, &radarscp_bck_net_info );
		b = compute_res_net( i, 2, &radarscp_bck_net_info );

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
	color_codes = color_prom;	/* we'll need it later */
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

	rgb_t	*rgb;

	rgb = compute_res_net_all(color_prom, &dkong3_decode_info, &dkong3_net_info);
	palette_set_colors(machine, 0, rgb, 256);
	palette_normalize_range(machine->palette, 0, 255, 0, 255);
	free(rgb);

	color_prom += 1024;
	/* color_prom now points to the beginning of the character color codes */
	color_codes = color_prom;	/* we'll need it later */
}

static TILE_GET_INFO( dkong_bg_tile_info )
{
	int code = videoram[tile_index] + 256 * gfx_bank;
	int color = (color_codes[tile_index % 32 + 32 * (tile_index / 32 / 4)] & 0x0f) + 0x10 * palette_bank;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( radarsc1_bg_tile_info )
{
	int code = videoram[tile_index] + 256 * gfx_bank;
	int color = (color_codes[tile_index % 32] & 0x0f);
	color = color | (palette_bank<<4);

	SET_TILE_INFO(0, code, color, 0);
}

/***************************************************************************

  I/O Handling

***************************************************************************/

WRITE8_HANDLER( dkong_videoram_w )
{
	if (videoram[offset] != data)
	{
		videoram[offset] = data;
		tilemap_mark_tile_dirty(bg_tilemap, offset);
	}
}

WRITE8_HANDLER( radarscp_snd02_w )
{
	snd02_enable = data & 0x01;
}

WRITE8_HANDLER( radarsc1_ansn_w )
{
	sig_ansn = data & 0x01;
}

WRITE8_HANDLER( dkongjr_gfxbank_w )
{
	if (gfx_bank != (data & 0x01))
	{
		gfx_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( dkong3_gfxbank_w )
{
	if (gfx_bank != (~data & 0x01))
	{
		gfx_bank = ~data & 0x01;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( dkong_palettebank_w )
{
	int newbank;

	newbank = palette_bank;

	if (data & 1)
		newbank |= 1 << offset;
	else
		newbank &= ~(1 << offset);

	if (palette_bank != newbank)
	{
		palette_bank = newbank;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( radarscp_grid_enable_w )
{
	grid_on = data & 0x01;
}

WRITE8_HANDLER( radarscp_grid_color_w )
{
	grid_col = (data & 0x07) ^ 0x07;
	popmessage("Gridcol: %d", grid_col);
}

WRITE8_HANDLER( dkong_flipscreen_w )
{
	flip_screen_set(~data & 0x01);
}

WRITE8_HANDLER( dkong_spritebank_w )
{
	sprite_bank = data & 0x01;
}

/***************************************************************************

  Draw the game screen in the given mame_bitmap.

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT32 mask_bank, UINT32 shift_bits)
{
	int offs;

	/* Draw the sprites. */
	for (offs = sprite_bank<<9;offs < (sprite_bank<<9) + spriteram_size; offs += 4)
	{
		if (spriteram[offs])
		{
			/* spriteram[offs + 2] & 0x40 is used by Donkey Kong 3 only */
			/* spriteram[offs + 2] & 0x30 don't seem to be used (they are */
			/* probably not part of the color code, since Mario Bros, which */
			/* has similar hardware, uses a memory mapped port to change */
			/* palette bank, so it's limited to 16 color codes) */

			int x,y;

			x = spriteram[offs + 3] - 8;
			y = 240 - spriteram[offs] + 7;

			if (flip_screen)
			{
				x = 240 - x;
				y = VTOTAL - VBEND - y;

				drawgfx(bitmap,machine->gfx[1],
						(spriteram[offs + 1] & 0x7f) + ((spriteram[offs + 2] & mask_bank) << shift_bits),
						(spriteram[offs + 2] & 0x0f) + 16 * palette_bank,
						!(spriteram[offs + 2] & 0x80),!(spriteram[offs + 1] & 0x80),
						x,y,
						cliprect,TRANSPARENCY_PEN,0);

				/* draw with wrap around - this fixes the 'beheading' bug */
				drawgfx(bitmap,machine->gfx[1],
						(spriteram[offs + 1] & 0x7f) + ((spriteram[offs + 2] & mask_bank) << shift_bits),
						(spriteram[offs + 2] & 0x0f) + 16 * palette_bank,
						(spriteram[offs + 2] & 0x80),(spriteram[offs + 1] & 0x80),
						x-256,y,
						cliprect,TRANSPARENCY_PEN,0);
			}
			else
			{
				drawgfx(bitmap,machine->gfx[1],
						(spriteram[offs + 1] & 0x7f) + ((spriteram[offs + 2] & mask_bank) << shift_bits),
						(spriteram[offs + 2] & 0x0f) + 16 * palette_bank,
						(spriteram[offs + 2] & 0x80),(spriteram[offs + 1] & 0x80),
						x,y,
						cliprect,TRANSPARENCY_PEN,0);

				/* draw with wrap around - this fixes the 'beheading' bug */
				drawgfx(bitmap,machine->gfx[1],
						(spriteram[offs + 1] & 0x7f) + ((spriteram[offs + 2] & mask_bank) << shift_bits),
						(spriteram[offs + 2] & 0x0f) + 16 * palette_bank,
						(spriteram[offs + 2] & 0x80),(spriteram[offs + 1] & 0x80),
						x+256,y,
						cliprect,TRANSPARENCY_PEN,0);
			}
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

INLINE double CD4049(double x)
{
	if (x>0)
	 	return exp(-cd4049_a * pow(x,cd4049_b));
	else
		return 1.0;
}

/* Actually the sound noise is a multivibrator with
 * a period of roughly 4.4 ms
 */

static void radarscp_step(int line_cnt)
{
	/* Condensator is illegible in schematics for TRS2 board.
     * TRS1 board states 3.3u.
     */
	const double RC1= 2.2e3 * 22e-6;// 22e-6;
	const double RC2= 10e3 * 33e-6;
	const double RC31= 18e3 * 33e-6;
	const double RC32= (18e3 + 68e3) * 33e-6;
	const double RC4= 90e3 * 0.47e-6;
	const double dt= 1./60./(double) VTOTAL;
	const int period2 = ((long long)(PIXEL_CLOCK) * ( 33L * 68L )) / (long)10000000L / 3;  // period/2 in pixel ...
	static double cv1=0,cv2=0,vg1=0,vg2=0,vg3=0,vg3i=0,cv3=0,cv4=0;
	static int pixelcnt = 0;
	double diff;
	int sig;

	line_cnt += 256;
	if (line_cnt>511)
		line_cnt -= VTOTAL;
	//FIXME: remove after testing
	//return;
	/* sound2 mixes in a 30Hz noise signal.
     * With the current model this has no real effect
     * Included for completeness
     */

	line_cnt++;
	if (line_cnt>=512)
		line_cnt=512-VTOTAL;
	if ( ( !(line_cnt & 0x40) && ((line_cnt+1) & 0x40) ) && (mame_rand(Machine) > RAND_MAX/2))
		sig30Hz = (1-sig30Hz);
	rflip_sig = snd02_enable & sig30Hz;
	sig = rflip_sig ^ ((line_cnt & 0x80)>>7);
	if (vid_hw & HW_TRS01)
		rflip_sig = !rflip_sig;
	if  (sig) // 128VF
		diff = (0.0 - cv1);
	else
		diff = (3.4 - cv1);
	diff = diff - diff*exp(0.0 - (1.0/RC1 * dt) );
	cv1 += diff;

	diff = (cv1 - cv2 - vg1);
	diff = diff - diff*exp(0.0 - (1.0/RC2 * dt) );
	cv2 += diff;

	vg1 = (cv1 - cv2)*0.9 + 0.1 * vg2;
	vg2 = 5*CD4049(vg1/5);

	/* on the real hardware, the gain would be 1.
     * This will not work here.
     */
	vg3i = 0.9*vg2 + 0.1 * vg3;
	vg3 = 5*CD4049(vg3i/5);

	blue_level = (int)(vg3/5.0*255);

	// Grid signal
	if (grid_on && sig_ansn)
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

	if (CD4049(CD4049(vg2 - cv4))>2.4/5.0) //TTL - Level
		grid_sig = 0;
	else
		grid_sig = 1;

	/* stars */
	pixelcnt += HTOTAL;
	if (pixelcnt > period2 )
	{
		star_ff = !star_ff;
		pixelcnt = pixelcnt - period2;
	}

}

static void radarscp_draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const UINT8 	*htable = NULL;
	int 			x,y;
	UINT8 			draw_ok;
	UINT16 			*pixel;

	if (vid_hw & HW_TRS01)
		htable = memory_region(REGION_GFX4);

	y = machine->screen[0].visarea.min_y;
	while (y <= machine->screen[0].visarea.max_y)
	{
		x = machine->screen[0].visarea.min_x;
		while (x <= machine->screen[0].visarea.max_x)
		{
			pixel = BITMAP_ADDR16(bitmap, y, x);
			draw_ok = !(*pixel & 0x01) && !(*pixel & 0x02);
			if (vid_hw & HW_TRS01) // Check again from schematics
				draw_ok = draw_ok  && !((htable[ (!rflip_sig<<7) | (x>>2)] >>2) & 0x01);
			if (draw_ok)
				*pixel = *(BITMAP_ADDR16(bg_bits, y, x));
			x++;
		}
		y++;
	}
}


static TIMER_CALLBACK( scanline_callback )
{
	const UINT8 *table = memory_region(REGION_GFX3);
	int 		table_len = memory_region_length(REGION_GFX3);
	int 			x,y,offset;
	UINT16 			*pixel;
	static int		counter=0;
	int scanline = param;

	y = scanline;
	radarscp_step(y);
	if (y <= machine->screen[0].visarea.min_y || y > machine->screen[0].visarea.max_y)
		counter = 0;
	offset = ((-flip_screen) ^ rflip_sig) ? 0x000 : 0x400;
	x = 0;
	while (x < machine->screen[0].width)
	{
		pixel = BITMAP_ADDR16(bg_bits, y, x);
		if ((counter < table_len) && (x == 4 * (table[counter|offset] & 0x7f)))
		{
			if ( star_ff && (table[counter|offset] & 0x80) )	/* star */
				*pixel = machine->pens[RADARSCP_STAR_COL];
			else if (grid_sig && !(table[counter|offset] & 0x80))			/* radar */
				*pixel = machine->pens[RADARSCP_GRID_COL_OFFSET+grid_col];
			else
				*pixel = machine->pens[RADARSCP_BCK_COL_OFFSET + blue_level];
			counter++;
		}
		else
			*pixel = machine->pens[RADARSCP_BCK_COL_OFFSET + blue_level];
		x++;
	}
	while ((counter < table_len) && ( x < 4 * (table[counter|offset] & 0x7f)))
		counter++;
	scanline = (scanline+1) % VTOTAL;
	/* come back at the next appropriate scanline */
	timer_adjust(scanline_timer, video_screen_get_time_until_pos(0, scanline, 0), scanline, attotime_zero);
}

static VIDEO_START( dkong_base )
{

	cd4049_b = (log(0.0 - log(cd4049_al)) - log(0.0 - log((1.0-cd4049_al))) ) / log(cd4049_vh/cd4049_vl);
	cd4049_a = log(0.0 - log(cd4049_al)) - cd4049_b * log(cd4049_vh);

	gfx_bank = 0;
	palette_bank = 0;
	sprite_bank = 0;
	sig_ansn = 1;
	vid_hw = HW_DK4;

	state_save_register_global(gfx_bank);
	state_save_register_global(palette_bank);
	state_save_register_global(sprite_bank);
	state_save_register_global(grid_on);

	state_save_register_global(snd02_enable);
	state_save_register_global(sig_ansn);
	state_save_register_global(grid_col);

	/* this must be registered here - hmmm */
	state_save_register_global(flip_screen);

}

VIDEO_START( dkong )
{

	video_start_dkong_base(machine);

	bg_tilemap = tilemap_create(dkong_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32);
	tilemap_set_scrolldx(bg_tilemap, 0, 128);
}

VIDEO_START( radarscp )
{

	video_start_dkong(machine);

	vid_hw = HW_TRS02;

	bg_bits = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

    scanline_timer = timer_alloc(scanline_callback, NULL);
    timer_adjust(scanline_timer, video_screen_get_time_until_pos(0, 0, 0), 0, attotime_zero);

}

VIDEO_START( radarsc1 )
{

	video_start_dkong_base(machine);

	vid_hw = HW_TRS01;

	bg_tilemap = tilemap_create(radarsc1_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32);
	tilemap_set_scrolldx(bg_tilemap, 0, 128);

	bg_bits = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

    scanline_timer = timer_alloc(scanline_callback, NULL);
    timer_adjust(scanline_timer, video_screen_get_time_until_pos(0, 0, 0), 0, attotime_zero);

}

VIDEO_UPDATE( radarscp )
{

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect, 0x40, 1);
	radarscp_draw_background(machine, bitmap, cliprect);

	return 0;
}

VIDEO_UPDATE( dkong )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect, 0x40, 1);
	return 0;
}

VIDEO_UPDATE( pestplce )
{
	int offs;

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* Draw the sprites. */
	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		if (spriteram[offs])
		{
			drawgfx(bitmap,machine->gfx[1],
					spriteram[offs + 2],
					(spriteram[offs + 1] & 0x0f) + 16 * palette_bank,
					spriteram[offs + 1] & 0x80,spriteram[offs + 1] & 0x40,
					spriteram[offs + 3] - 8,240 - spriteram[offs] + 8,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
	return 0;
}

VIDEO_UPDATE( spclforc )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* it uses spriteram[offs + 2] & 0x10 for sprite bank */
	draw_sprites(machine, bitmap, cliprect, 0x10, 3);
	return 0;
}
