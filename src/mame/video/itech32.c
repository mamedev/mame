/***************************************************************************

    Incredible Technologies/Strata system
    (32-bit blitter variant)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/itech32.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define BLIT_LOGGING			0



/*************************************
 *
 *  Blitter constants
 *
 *************************************/

#define VIDEO_UNKNOWN00			state->m_video[0x00/2]	/* $0087 at startup */
#define VIDEO_STATUS			state->m_video[0x00/2]
#define VIDEO_INTSTATE			state->m_video[0x02/2]
#define VIDEO_INTACK			state->m_video[0x02/2]
#define VIDEO_TRANSFER			state->m_video[0x04/2]
#define VIDEO_TRANSFER_FLAGS	state->m_video[0x06/2]	/* $5080 at startup (kept at $1512) */
#define VIDEO_COMMAND			state->m_video[0x08/2]	/* $0005 at startup */
#define VIDEO_INTENABLE			state->m_video[0x0a/2]	/* $0144 at startup (kept at $1514) */
#define VIDEO_TRANSFER_HEIGHT	state->m_video[0x0c/2]
#define VIDEO_TRANSFER_WIDTH	state->m_video[0x0e/2]
#define VIDEO_TRANSFER_ADDRLO	state->m_video[0x10/2]
#define VIDEO_TRANSFER_X		state->m_video[0x12/2]
#define VIDEO_TRANSFER_Y		state->m_video[0x14/2]
#define VIDEO_SRC_YSTEP			state->m_video[0x16/2]	/* $0011 at startup */
#define VIDEO_SRC_XSTEP			state->m_video[0x18/2]
#define VIDEO_DST_XSTEP			state->m_video[0x1a/2]
#define VIDEO_DST_YSTEP			state->m_video[0x1c/2]
#define VIDEO_YSTEP_PER_X		state->m_video[0x1e/2]
#define VIDEO_XSTEP_PER_Y		state->m_video[0x20/2]
#define VIDEO_UNKNOWN22			state->m_video[0x22/2]	/* $0033 at startup */
#define VIDEO_LEFTCLIP			state->m_video[0x24/2]
#define VIDEO_RIGHTCLIP			state->m_video[0x26/2]
#define VIDEO_TOPCLIP			state->m_video[0x28/2]
#define VIDEO_BOTTOMCLIP		state->m_video[0x2a/2]
#define VIDEO_INTSCANLINE		state->m_video[0x2c/2]	/* $00ef at startup */
#define VIDEO_TRANSFER_ADDRHI	state->m_video[0x2e/2]	/* $0000 at startup */

#define VIDEO_UNKNOWN30			state->m_video[0x30/2]	/* $0040 at startup */
#define VIDEO_VTOTAL			state->m_video[0x32/2]	/* $0106 at startup */
#define VIDEO_VSYNC				state->m_video[0x34/2]	/* $0101 at startup */
#define VIDEO_VBLANK_START		state->m_video[0x36/2]	/* $00f3 at startup */
#define VIDEO_VBLANK_END		state->m_video[0x38/2]	/* $0003 at startup */
#define VIDEO_HTOTAL			state->m_video[0x3a/2]	/* $01fc at startup */
#define VIDEO_HSYNC				state->m_video[0x3c/2]	/* $01e4 at startup */
#define VIDEO_HBLANK_START		state->m_video[0x3e/2]	/* $01b2 at startup */
#define VIDEO_HBLANK_END		state->m_video[0x40/2]	/* $0032 at startup */
#define VIDEO_UNKNOWN42			state->m_video[0x42/2]	/* $0015 at startup */
#define VIDEO_DISPLAY_YORIGIN1	state->m_video[0x44/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_YORIGIN2	state->m_video[0x46/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_YSCROLL2	state->m_video[0x48/2]	/* $0000 at startup */
#define VIDEO_UNKNOWN4a			state->m_video[0x4a/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_XORIGIN1	state->m_video[0x4c/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_XORIGIN2	state->m_video[0x4e/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_XSCROLL2	state->m_video[0x50/2]	/* $0000 at startup */
#define VIDEO_UNKNOWN52			state->m_video[0x52/2]	/* $0000 at startup */
#define VIDEO_UNKNOWN54			state->m_video[0x54/2]	/* $0080 at startup */
#define VIDEO_UNKNOWN56			state->m_video[0x56/2]	/* $00c0 at startup */
#define VIDEO_UNKNOWN58			state->m_video[0x58/2]	/* $01c0 at startup */
#define VIDEO_UNKNOWN5a			state->m_video[0x5a/2]	/* $01c0 at startup */
#define VIDEO_UNKNOWN5c			state->m_video[0x5c/2]	/* $01cf at startup */
#define VIDEO_UNKNOWN5e			state->m_video[0x5e/2]	/* $01cf at startup */
#define VIDEO_UNKNOWN60			state->m_video[0x60/2]	/* $01e3 at startup */
#define VIDEO_UNKNOWN62			state->m_video[0x62/2]	/* $01cf at startup */
#define VIDEO_UNKNOWN64			state->m_video[0x64/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN66			state->m_video[0x66/2]	/* $0183 at startup */
#define VIDEO_UNKNOWN68			state->m_video[0x68/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN6a			state->m_video[0x6a/2]	/* $000f at startup */
#define VIDEO_UNKNOWN6c			state->m_video[0x6c/2]	/* $018f at startup */
#define VIDEO_UNKNOWN6e			state->m_video[0x6e/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN70			state->m_video[0x70/2]	/* $000f at startup */
#define VIDEO_UNKNOWN72			state->m_video[0x72/2]	/* $000f at startup */
#define VIDEO_UNKNOWN74			state->m_video[0x74/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN76			state->m_video[0x76/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN78			state->m_video[0x78/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN7a			state->m_video[0x7a/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN7c			state->m_video[0x7c/2]	/* $0820 at startup */
#define VIDEO_UNKNOWN7e			state->m_video[0x7e/2]	/* $0100 at startup */

#define VIDEO_STARTSTEP			state->m_video[0x80/2]	/* drivedge only? */
#define VIDEO_LEFTSTEPLO		state->m_video[0x82/2]	/* drivedge only? */
#define VIDEO_LEFTSTEPHI		state->m_video[0x84/2]	/* drivedge only? */
#define VIDEO_RIGHTSTEPLO		state->m_video[0x86/2]	/* drivedge only? */
#define VIDEO_RIGHTSTEPHI		state->m_video[0x88/2]	/* drivedge only? */

#define VIDEOINT_SCANLINE		0x0004
#define VIDEOINT_BLITTER		0x0040

#define XFERFLAG_TRANSPARENT	0x0001
#define XFERFLAG_XFLIP			0x0002
#define XFERFLAG_YFLIP			0x0004
#define XFERFLAG_DSTXSCALE		0x0008
#define XFERFLAG_DYDXSIGN		0x0010
#define XFERFLAG_DXDYSIGN		0x0020
#define XFERFLAG_UNKNOWN8		0x0100
#define XFERFLAG_CLIP			0x0400
#define XFERFLAG_UNKNOWN15		0x8000

#define XFERFLAG_KNOWNFLAGS		(XFERFLAG_TRANSPARENT | XFERFLAG_XFLIP | XFERFLAG_YFLIP | XFERFLAG_DSTXSCALE | XFERFLAG_DYDXSIGN | XFERFLAG_DXDYSIGN | XFERFLAG_CLIP)

#define VRAM_WIDTH				512



static TIMER_CALLBACK( scanline_interrupt );



/*************************************
 *
 *  Macros and inlines
 *
 *************************************/

#define ADJUSTED_HEIGHT(x) ((((x) >> 1) & 0x100) | ((x) & 0xff))

INLINE offs_t compute_safe_address(itech32_state *state, int x, int y)
{
	return ((y & state->m_vram_ymask) * 512) + (x & state->m_vram_xmask);
}

INLINE void disable_clipping(itech32_state *state)
{
	state->m_clip_save = state->m_clip_rect;
	state->m_clip_rect.set(0, 0xfff, 0, 0xfff);
	state->m_scaled_clip_rect.set(0, 0xfff << 8, 0, 0xfff << 8);
}

INLINE void enable_clipping(itech32_state *state)
{
	state->m_clip_rect = state->m_clip_save;

	state->m_scaled_clip_rect.min_x = state->m_clip_rect.min_x << 8;
	state->m_scaled_clip_rect.max_x = state->m_clip_rect.max_x << 8;
	state->m_scaled_clip_rect.min_y = state->m_clip_rect.min_y << 8;
	state->m_scaled_clip_rect.max_y = state->m_clip_rect.max_y << 8;
}



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( itech32 )
{
	itech32_state *state = machine.driver_data<itech32_state>();
	int i;

	/* allocate memory */
	state->m_videoram = auto_alloc_array(machine, UINT16, VRAM_WIDTH * (state->m_vram_height + 16) * 2);
	memset(state->m_videoram, 0xff, VRAM_WIDTH * (state->m_vram_height + 16) * 2 * 2);

	/* videoplane[0] is the foreground; videoplane[1] is the background */
	state->m_videoplane[0] = &state->m_videoram[0 * VRAM_WIDTH * (state->m_vram_height + 16) + 8 * VRAM_WIDTH];
	state->m_videoplane[1] = &state->m_videoram[1 * VRAM_WIDTH * (state->m_vram_height + 16) + 8 * VRAM_WIDTH];

	/* set the masks */
	state->m_vram_mask = VRAM_WIDTH * state->m_vram_height - 1;
	state->m_vram_xmask = VRAM_WIDTH - 1;
	state->m_vram_ymask = state->m_vram_height - 1;

	/* clear the planes initially */
	for (i = 0; i < VRAM_WIDTH * state->m_vram_height; i++)
		state->m_videoplane[0][i] = state->m_videoplane[1][i] = 0xff;

	/* fetch the GROM base */
	state->m_grom_base = machine.region("gfx1")->base();
	state->m_grom_size = machine.region("gfx1")->bytes();
	state->m_grom_bank = 0;
	state->m_grom_bank_mask = state->m_grom_size >> 24;
	if (state->m_grom_bank_mask == 2)
		state->m_grom_bank_mask = 3;

	/* reset statics */
	memset(state->m_video, 0, 0x80);

	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(scanline_interrupt));
	state->m_enable_latch[0] = 1;
	state->m_enable_latch[1] = (state->m_planes > 1) ? 1 : 0;
}



/*************************************
 *
 *  Latches
 *
 *************************************/

WRITE16_HANDLER( timekill_colora_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->m_enable_latch[0] = (~data >> 5) & 1;
		state->m_enable_latch[1] = (~data >> 7) & 1;
		state->m_color_latch[0] = (data & 0x0f) << 8;
	}
}


WRITE16_HANDLER( timekill_colorbc_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
		state->m_color_latch[1] = ((data & 0xf0) << 4) | 0x1000;
}


WRITE16_HANDLER( timekill_intensity_w )
{
	if (ACCESSING_BITS_0_7)
	{
		double intensity = (double)(data & 0xff) / (double)0x60;
		int i;
		for (i = 0; i < 8192; i++)
			palette_set_pen_contrast(space->machine(), i, intensity);
	}
}


WRITE16_HANDLER( bloodstm_color1_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
		state->m_color_latch[0] = (data & 0x7f) << 8;
}


WRITE16_HANDLER( bloodstm_color2_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
		state->m_color_latch[1] = (data & 0x7f) << 8;
}


WRITE16_HANDLER( bloodstm_plane_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->m_enable_latch[0] = (~data >> 1) & 1;
		state->m_enable_latch[1] = (~data >> 2) & 1;
	}
}


WRITE32_HANDLER( drivedge_color0_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_16_23)
		state->m_color_latch[0] = ((data >> 16) & 0x7f) << 8;
}


WRITE32_HANDLER( itech020_color1_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
		state->m_color_latch[1] = (data & 0x7f) << 8;
}


WRITE32_HANDLER( itech020_color2_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_0_7)
		state->m_color_latch[0] = (data & 0x7f) << 8;
}


WRITE32_HANDLER( itech020_plane_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (ACCESSING_BITS_8_15)
	{
		state->m_enable_latch[0] = (~data >> 9) & 1;
		state->m_enable_latch[1] = (~data >> 10) & 1;
		state->m_grom_bank = ((data >> 14) & state->m_grom_bank_mask) << 24;
	}
}



/*************************************
 *
 *  Palette I/O
 *
 *************************************/

WRITE16_HANDLER( timekill_paletteram_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	int r, g, b;

	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);

	r = state->m_generic_paletteram_16[offset & ~1] & 0xff;
	g = state->m_generic_paletteram_16[offset & ~1] >> 8;
	b = state->m_generic_paletteram_16[offset |  1] >> 8;

	palette_set_color(space->machine(), offset / 2, MAKE_RGB(r, g, b));
}


WRITE16_HANDLER( bloodstm_paletteram_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	int r, g, b;

	/* in test mode, the LSB is used; in game mode, the MSB is used */
	if (!ACCESSING_BITS_0_7 && (offset & 1))
		data >>= 8, mem_mask >>= 8;
	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);

	r = state->m_generic_paletteram_16[offset & ~1] & 0xff;
	g = state->m_generic_paletteram_16[offset & ~1] >> 8;
	b = state->m_generic_paletteram_16[offset |  1] & 0xff;

	palette_set_color(space->machine(), offset / 2, MAKE_RGB(r, g, b));
}


WRITE32_HANDLER( drivedge_paletteram_w )
{
	int r, g, b;

	itech32_state *state = space->machine().driver_data<itech32_state>();
	COMBINE_DATA(&state->m_generic_paletteram_32[offset]);

	r = state->m_generic_paletteram_32[offset] & 0xff;
	g = (state->m_generic_paletteram_32[offset] >> 8) & 0xff;
	b = (state->m_generic_paletteram_32[offset] >> 16) & 0xff;

	palette_set_color(space->machine(), offset, MAKE_RGB(r, g, b));
}


WRITE32_HANDLER( itech020_paletteram_w )
{
	int r, g, b;

	itech32_state *state = space->machine().driver_data<itech32_state>();
	COMBINE_DATA(&state->m_generic_paletteram_32[offset]);

	r = (state->m_generic_paletteram_32[offset] >> 16) & 0xff;
	g = (state->m_generic_paletteram_32[offset] >> 8) & 0xff;
	b = state->m_generic_paletteram_32[offset] & 0xff;

	palette_set_color(space->machine(), offset, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Debugging
 *
 *************************************/

static void logblit(running_machine &machine, const char *tag)
{
	itech32_state *state = machine.driver_data<itech32_state>();
	if (!machine.input().code_pressed(KEYCODE_L))
		return;
	if (state->m_is_drivedge && VIDEO_TRANSFER_FLAGS == 0x5490)
	{
		/* polygon drawing */
		logerror("%s: e=%d%d f=%04x s=(%03x-%03x,%03x) w=%03x h=%03x b=%02x%04x c=%02x%02x ss=%04x,%04x ds=%04x,%04x ls=%04x%04x rs=%04x%04x u80=%04x", tag,
			state->m_enable_latch[0], state->m_enable_latch[1],
			VIDEO_TRANSFER_FLAGS,
			VIDEO_TRANSFER_X, VIDEO_RIGHTCLIP, VIDEO_TRANSFER_Y, VIDEO_TRANSFER_WIDTH, VIDEO_TRANSFER_HEIGHT,
			VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
			state->m_color_latch[0] >> 8, state->m_color_latch[1] >> 8,
			VIDEO_SRC_XSTEP, VIDEO_SRC_YSTEP,
			VIDEO_DST_XSTEP, VIDEO_DST_YSTEP,
			VIDEO_LEFTSTEPHI, VIDEO_LEFTSTEPLO, VIDEO_RIGHTSTEPHI, VIDEO_RIGHTSTEPLO,
			VIDEO_STARTSTEP);
	}

	else if (state->m_video[0x16/2] == 0x100 && state->m_video[0x18/2] == 0x100 &&
		state->m_video[0x1a/2] == 0x000 && state->m_video[0x1c/2] == 0x100 &&
		state->m_video[0x1e/2] == 0x000 && state->m_video[0x20/2] == 0x000)
	{
		logerror("%s: e=%d%d f=%04x c=%02x%02x %02x%04x -> (%03x,%03x) %3dx%3dc=(%03x,%03x)-(%03x,%03x)", tag,
				state->m_enable_latch[0], state->m_enable_latch[1],
				VIDEO_TRANSFER_FLAGS,
				state->m_color_latch[0] >> 8, state->m_color_latch[1] >> 8,
				VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
				VIDEO_TRANSFER_X, VIDEO_TRANSFER_Y,
				VIDEO_TRANSFER_WIDTH, ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT),
				VIDEO_LEFTCLIP, VIDEO_TOPCLIP, VIDEO_RIGHTCLIP, VIDEO_BOTTOMCLIP);
	}
	else
	{
		logerror("%s: e=%d%d f=%04x c=%02x%02x %02x%04x -> (%03x,%03x) %3dx%3d c=(%03x,%03x)-(%03x,%03x) s=%04x %04x %04x %04x %04x %04x", tag,
				state->m_enable_latch[0], state->m_enable_latch[1],
				VIDEO_TRANSFER_FLAGS,
				state->m_color_latch[0] >> 8, state->m_color_latch[1] >> 8,
				VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
				VIDEO_TRANSFER_X, VIDEO_TRANSFER_Y,
				VIDEO_TRANSFER_WIDTH, ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT),
				VIDEO_LEFTCLIP, VIDEO_TOPCLIP, VIDEO_RIGHTCLIP, VIDEO_BOTTOMCLIP,
				state->m_video[0x16/2], state->m_video[0x18/2], state->m_video[0x1a/2],
				state->m_video[0x1c/2], state->m_video[0x1e/2], state->m_video[0x20/2]);
	}
	if (state->m_is_drivedge) logerror(" v0=%08x v1=%08x v2=%08x v3=%08x", state->m_drivedge_zbuf_control[0], state->m_drivedge_zbuf_control[1], state->m_drivedge_zbuf_control[2], state->m_drivedge_zbuf_control[3]);
	logerror("\n");
}



/*************************************
 *
 *  Video interrupts
 *
 *************************************/

static void update_interrupts(running_machine &machine, int fast)
{
	itech32_state *state = machine.driver_data<itech32_state>();
	int scanline_state = 0, blitter_state = 0;

	if (VIDEO_INTSTATE & VIDEO_INTENABLE & VIDEOINT_SCANLINE)
		scanline_state = 1;
	if (VIDEO_INTSTATE & VIDEO_INTENABLE & VIDEOINT_BLITTER)
		blitter_state = 1;

	itech32_update_interrupts(machine, -1, blitter_state, scanline_state);
}


static TIMER_CALLBACK( scanline_interrupt )
{
	itech32_state *state = machine.driver_data<itech32_state>();
	/* set timer for next frame */
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(VIDEO_INTSCANLINE));

	/* set the interrupt bit in the status reg */
	logerror("-------------- (DISPLAY INT @ %d) ----------------\n", machine.primary_screen->vpos());
	VIDEO_INTSTATE |= VIDEOINT_SCANLINE;

	/* update the interrupt state */
	update_interrupts(machine, 0);
}



/*************************************
 *
 *  Uncompressed blitter functions
 *
 *************************************/

static void draw_raw(itech32_state *state, UINT16 *base, UINT16 color)
{
	UINT8 *src = &state->m_grom_base[(state->m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % state->m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH << 8;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT) << 8;
	int xsrcstep = VIDEO_SRC_XSTEP;
	int ysrcstep = VIDEO_SRC_YSTEP;
	int sx, sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int startx = (VIDEO_TRANSFER_X & 0xfff) << 8;
	int xdststep = 0x100;
	int ydststep = VIDEO_DST_YSTEP;
	int x, y;

	/* adjust for (lack of) clipping */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		disable_clipping(state);

	/* adjust for scaling */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_DSTXSCALE)
		xdststep = VIDEO_DST_XSTEP;

	/* adjust for flipping */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_XFLIP)
		xdststep = -xdststep;
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP)
		ydststep = -ydststep;

	/* loop over Y in src pixels */
	for (y = 0; y < height; y += ysrcstep, sy += ydststep)
	{
		UINT8 *rowsrc = &src[(y >> 8) * (width >> 8)];

		/* simpler case: VIDEO_YSTEP_PER_X is zero */
		if (VIDEO_YSTEP_PER_X == 0)
		{
			/* clip in the Y direction */
			if (sy >= state->m_scaled_clip_rect.min_y && sy < state->m_scaled_clip_rect.max_y)
			{
				UINT32 dstoffs;

				/* direction matters here */
				sx = startx;
				if (xdststep > 0)
				{
					/* skip left pixels */
					for (x = 0; x < width && sx < state->m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep) ;

					/* compute the address */
					dstoffs = compute_safe_address(state, sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					for ( ; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
							base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
					}
				}
				else
				{
					/* skip right pixels */
					for (x = 0; x < width && sx >= state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep) ;

					/* compute the address */
					dstoffs = compute_safe_address(state, sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					for ( ; x < width && sx >= state->m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
							base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
					}
				}
			}
		}

		/* slow case: VIDEO_YSTEP_PER_X is non-zero */
		else
		{
			int ystep = (VIDEO_TRANSFER_FLAGS & XFERFLAG_DYDXSIGN) ? -VIDEO_YSTEP_PER_X : VIDEO_YSTEP_PER_X;
			int ty = sy;

			/* render all pixels */
			sx = startx;
			for (x = 0; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
				if (state->m_scaled_clip_rect.contains(sx, ty))
				{
					int pixel = rowsrc[x >> 8];
					if (pixel != transparent_pen)
						base[compute_safe_address(state, sx >> 8, ty >> 8)] = pixel | color;
				}
		}

		/* apply skew */
		if (VIDEO_TRANSFER_FLAGS & XFERFLAG_DXDYSIGN)
			startx += VIDEO_XSTEP_PER_Y;
		else
			startx -= VIDEO_XSTEP_PER_Y;
	}

	/* restore cliprects */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		enable_clipping(state);
}


static void draw_raw_drivedge(itech32_state *state, UINT16 *base, UINT16 *zbase, UINT16 color)
{
	UINT8 *src = &state->m_grom_base[(state->m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % state->m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH << 8;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT) << 8;
	int xsrcstep = VIDEO_SRC_XSTEP;
	int ysrcstep = VIDEO_SRC_YSTEP;
	int sx, sy = ((VIDEO_TRANSFER_Y & 0xfff) << 8) + 0x80;
	int startx = ((VIDEO_TRANSFER_X & 0xfff) << 8) + 0x80;
	int xdststep = 0x100;
	int ydststep = VIDEO_DST_YSTEP;
	INT32 z0 = state->m_drivedge_zbuf_control[2] & 0x7ff00;
	INT32 zmatch = (state->m_drivedge_zbuf_control[2] & 0x1f) << 11;
	INT32 srcdelta = 0;
	int x, y;

	/* adjust for (lack of) clipping */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		disable_clipping(state);

	/* adjust for scaling */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_DSTXSCALE)
		xdststep = VIDEO_DST_XSTEP;

	/* adjust for flipping */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_XFLIP)
		xdststep = -xdststep;
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP)
		ydststep = -ydststep;

	/* loop over Y in src pixels */
	for (y = 0; y < height; y += ysrcstep, sy += ydststep)
	{
		UINT8 *rowsrc = src + (srcdelta >> 8);

		/* in the polygon case, we don't factor in the Y */
		if (VIDEO_TRANSFER_FLAGS != 0x5490)
			rowsrc += (y >> 8) * (width >> 8);
		else
			width = 1000 << 8;

		/* simpler case: VIDEO_YSTEP_PER_X is zero */
		if (VIDEO_YSTEP_PER_X == 0)
		{
			/* clip in the Y direction */
			if (sy >= state->m_scaled_clip_rect.min_y && sy < state->m_scaled_clip_rect.max_y)
			{
				UINT32 dstoffs, zbufoffs;
				INT32 z = z0;

				/* direction matters here */
				sx = startx;
				if (xdststep > 0)
				{
					/* skip left pixels */
					for (x = 0; x < width && sx < state->m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						z += (INT32)state->m_drivedge_zbuf_control[0];

					/* compute the address */
					dstoffs = compute_safe_address(state, sx >> 8, sy >> 8) - (sx >> 8);
					zbufoffs = compute_safe_address(state, sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					if (state->m_drivedge_zbuf_control[3] & 0x8000)
					{
						for ( ; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen)
							{
								base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)state->m_drivedge_zbuf_control[0];
						}
					}
					else if (state->m_drivedge_zbuf_control[3] & 0x4000)
					{
						for ( ; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && zmatch == (zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] & (0x1f << 11)))
								base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
							z += (INT32)state->m_drivedge_zbuf_control[0];
						}
					}
					else
					{
						for ( ; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && ((z >> 8) <= (zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] & 0x7ff)))
							{
								base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)state->m_drivedge_zbuf_control[0];
						}
					}
				}
				else
				{
					/* skip right pixels */
					for (x = 0; x < width && sx >= state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						z += (INT32)state->m_drivedge_zbuf_control[0];

					/* compute the address */
					dstoffs = compute_safe_address(state, sx >> 8, sy >> 8) - (sx >> 8);
					zbufoffs = compute_safe_address(state, sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					if (state->m_drivedge_zbuf_control[3] & 0x8000)
					{
						for ( ; x < width && sx >= state->m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen)
							{
								base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)state->m_drivedge_zbuf_control[0];
						}
					}
					else if (state->m_drivedge_zbuf_control[3] & 0x4000)
					{
						for ( ; x < width && sx >= state->m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && zmatch == (zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] & (0x1f << 11)))
								base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
							z += (INT32)state->m_drivedge_zbuf_control[0];
						}
					}
					else
					{
						for ( ; x < width && sx >= state->m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && ((z >> 8) <= (zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] & 0x7ff)))
							{
								base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & state->m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)state->m_drivedge_zbuf_control[0];
						}
					}
				}
			}
		}

		/* slow case: VIDEO_YSTEP_PER_X is non-zero */
		else
		{
			int ystep = (VIDEO_TRANSFER_FLAGS & XFERFLAG_DYDXSIGN) ? -VIDEO_YSTEP_PER_X : VIDEO_YSTEP_PER_X;
			int ty = sy;
			INT32 z = z0;

			/* render all pixels */
			sx = startx;
			if (state->m_drivedge_zbuf_control[3] & 0x8000)
			{
				for (x = 0; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (state->m_scaled_clip_rect.contains(sx, ty))
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
						{
							base[compute_safe_address(state, sx >> 8, ty >> 8)] = pixel | color;
							zbase[compute_safe_address(state, sx >> 8, ty >> 8)] = (z >> 8) | zmatch;
						}
						z += (INT32)state->m_drivedge_zbuf_control[0];
					}
			}
			else if (state->m_drivedge_zbuf_control[3] & 0x4000)
			{
				for (x = 0; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (state->m_scaled_clip_rect.contains(sx, ty))
					{
						int pixel = rowsrc[x >> 8];
						UINT16 *zbuf = &zbase[compute_safe_address(state, sx >> 8, ty >> 8)];
						if (pixel != transparent_pen && zmatch == (*zbuf & (0x1f << 11)))
						{
							base[compute_safe_address(state, sx >> 8, ty >> 8)] = pixel | color;
							*zbuf = (z >> 8) | zmatch;
						}
						z += (INT32)state->m_drivedge_zbuf_control[0];
					}
			}
			else
			{
				for (x = 0; x < width && sx < state->m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (state->m_scaled_clip_rect.contains(sx, ty))
					{
						int pixel = rowsrc[x >> 8];
						UINT16 *zbuf = &zbase[compute_safe_address(state, sx >> 8, ty >> 8)];
						if (pixel != transparent_pen && ((z >> 8) <= (*zbuf & 0x7ff)))
						{
							base[compute_safe_address(state, sx >> 8, ty >> 8)] = pixel | color;
							*zbuf = (z >> 8) | zmatch;
						}
						z += (INT32)state->m_drivedge_zbuf_control[0];
					}
			}
		}

		/* apply skew */
		if (VIDEO_TRANSFER_FLAGS & XFERFLAG_DXDYSIGN)
			startx += VIDEO_XSTEP_PER_Y;
		else
			startx -= VIDEO_XSTEP_PER_Y;

		/* update the per-scanline parameters */
		if (VIDEO_TRANSFER_FLAGS == 0x5490)
		{
			startx += (INT32)((VIDEO_LEFTSTEPHI << 16) | VIDEO_LEFTSTEPLO);
			state->m_scaled_clip_rect.max_x += (INT32)((VIDEO_RIGHTSTEPHI << 16) | VIDEO_RIGHTSTEPLO);
			srcdelta += (INT16)VIDEO_STARTSTEP;
		}
		z0 += (INT32)state->m_drivedge_zbuf_control[1];
	}

	/* restore cliprects */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		enable_clipping(state);

	/* reflect the final values into registers */
	VIDEO_TRANSFER_X = (VIDEO_TRANSFER_X & ~0xfff) | (startx >> 8);
	VIDEO_RIGHTCLIP = (VIDEO_RIGHTCLIP & ~0xfff) | (state->m_scaled_clip_rect.max_x >> 8);
	VIDEO_TRANSFER_Y = (VIDEO_TRANSFER_Y & ~0xfff) | ((VIDEO_TRANSFER_Y + (y >> 8)) & 0xfff);
	VIDEO_TRANSFER_ADDRLO += srcdelta >> 8;

	state->m_drivedge_zbuf_control[2] = (state->m_drivedge_zbuf_control[2] & ~0x7ff00) | (z0 & 0x7ff00);
}



/*************************************
 *
 *  Compressed blitter macros
 *
 *************************************/

#define GET_NEXT_RUN(xleft, count, innercount, src)	\
do {												\
	/* load next RLE chunk if needed */				\
	if (!count)										\
	{												\
		count = *src++;								\
		val = (count & 0x80) ? -1 : *src++;			\
		count &= 0x7f;								\
	}												\
													\
	/* determine how much to bite off */			\
	innercount = (xleft > count) ? count : xleft;	\
	count -= innercount;							\
	xleft -= innercount;							\
} while (0)


#define SKIP_RLE(skip, xleft, count, innercount, src)\
do {												\
	/* scan RLE until done */						\
	for (xleft = skip; xleft > 0; )					\
	{												\
		/* load next RLE chunk if needed */			\
		GET_NEXT_RUN(xleft, count, innercount, src);\
													\
		/* skip past the data */					\
		if (val == -1) src += innercount;			\
	}												\
} while (0)



/*************************************
 *
 *  Fast compressed blitter functions
 *
 *************************************/

INLINE void draw_rle_fast(itech32_state *state, UINT16 *base, UINT16 color)
{
	UINT8 *src = &state->m_grom_base[(state->m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % state->m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int ydststep = VIDEO_DST_YSTEP;
	int lclip, rclip;

	/* determine clipping */
	lclip = state->m_clip_rect.min_x - sx;
	if (lclip < 0) lclip = 0;
	rclip = sx + width - state->m_clip_rect.max_x;
	if (rclip < 0) rclip = 0;
	width -= lclip + rclip;
	sx += lclip;

	/* adjust for flipping */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP)
		ydststep = -ydststep;

	/* loop over Y in src pixels */
	for (y = 0; y < height; y++, sy += ydststep)
	{
		UINT32 dstoffs;

		/* clip in the Y direction */
		if (sy < state->m_scaled_clip_rect.min_y || sy >= state->m_scaled_clip_rect.max_y)
		{
			SKIP_RLE(width + lclip + rclip, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		dstoffs = compute_safe_address(state, sx, sy >> 8);

		/* left clip */
		SKIP_RLE(lclip, xleft, count, innercount, src);

		/* loop until gone */
		for (xleft = width; xleft > 0; )
		{
			/* load next RLE chunk if needed */
			GET_NEXT_RUN(xleft, count, innercount, src);

			/* run of literals */
			if (val == -1)
				while (innercount--)
				{
					int pixel = *src++;
					if (pixel != transparent_pen)
						base[dstoffs & state->m_vram_mask] = color | pixel;
					dstoffs++;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				while (innercount--)
					base[dstoffs++ & state->m_vram_mask] = val;
			}

			/* run of transparent repeats */
			else
				dstoffs += innercount;
		}

		/* right clip */
		SKIP_RLE(rclip, xleft, count, innercount, src);
	}
}


INLINE void draw_rle_fast_xflip(itech32_state *state, UINT16 *base, UINT16 color)
{
	UINT8 *src = &state->m_grom_base[(state->m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % state->m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int ydststep = VIDEO_DST_YSTEP;
	int lclip, rclip;

	/* determine clipping */
	lclip = sx - state->m_clip_rect.max_x;
	if (lclip < 0) lclip = 0;
	rclip = state->m_clip_rect.min_x - (sx - width);
	if (rclip < 0) rclip = 0;
	width -= lclip + rclip;
	sx -= lclip;

	/* adjust for flipping */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP)
		ydststep = -ydststep;

	/* loop over Y in src pixels */
	for (y = 0; y < height; y++, sy += ydststep)
	{
		UINT32 dstoffs;

		/* clip in the Y direction */
		if (sy < state->m_scaled_clip_rect.min_y || sy >= state->m_scaled_clip_rect.max_y)
		{
			SKIP_RLE(width + lclip + rclip, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		dstoffs = compute_safe_address(state, sx, sy >> 8);

		/* left clip */
		SKIP_RLE(lclip, xleft, count, innercount, src);

		/* loop until gone */
		for (xleft = width; xleft > 0; )
		{
			/* load next RLE chunk if needed */
			GET_NEXT_RUN(xleft, count, innercount, src);

			/* run of literals */
			if (val == -1)
				while (innercount--)
				{
					int pixel = *src++;
					if (pixel != transparent_pen)
						base[dstoffs & state->m_vram_mask] = color | pixel;
					dstoffs--;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				while (innercount--)
					base[dstoffs-- & state->m_vram_mask] = val;
			}

			/* run of transparent repeats */
			else
				dstoffs -= innercount;
		}

		/* right clip */
		SKIP_RLE(rclip, xleft, count, innercount, src);
	}
}



/*************************************
 *
 *  Slow compressed blitter functions
 *
 *************************************/

INLINE void draw_rle_slow(itech32_state *state, UINT16 *base, UINT16 color)
{
	UINT8 *src = &state->m_grom_base[(state->m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % state->m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx, sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int xdststep = 0x100;
	int ydststep = VIDEO_DST_YSTEP;
	int startx = (VIDEO_TRANSFER_X & 0xfff) << 8;

	/* adjust for scaling */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_DSTXSCALE)
		xdststep = VIDEO_DST_XSTEP;

	/* adjust for flipping */
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_XFLIP)
		xdststep = -xdststep;
	if (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP)
		ydststep = -ydststep;

	/* loop over Y in src pixels */
	for (y = 0; y < height; y++, sy += ydststep)
	{
		UINT32 dstoffs;

		/* clip in the Y direction */
		if (sy < state->m_scaled_clip_rect.min_y || sy >= state->m_scaled_clip_rect.max_y)
		{
			SKIP_RLE(width, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		sx = startx;
		dstoffs = compute_safe_address(state, state->m_clip_rect.min_x, sy >> 8) - state->m_clip_rect.min_x;

		/* loop until gone */
		for (xleft = width; xleft > 0; )
		{
			/* load next RLE chunk if needed */
			GET_NEXT_RUN(xleft, count, innercount, src);

			/* run of literals */
			if (val == -1)
				for ( ; innercount--; sx += xdststep)
				{
					int pixel = *src++;
					if (pixel != transparent_pen)
						if (sx >= state->m_scaled_clip_rect.min_x && sx < state->m_scaled_clip_rect.max_x)
							base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = color | pixel;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				for ( ; innercount--; sx += xdststep)
					if (sx >= state->m_scaled_clip_rect.min_x && sx < state->m_scaled_clip_rect.max_x)
						base[(dstoffs + (sx >> 8)) & state->m_vram_mask] = val;
			}

			/* run of transparent repeats */
			else
				sx += xdststep * innercount;
		}

		/* apply skew */
		if (VIDEO_TRANSFER_FLAGS & XFERFLAG_DXDYSIGN)
			startx += VIDEO_XSTEP_PER_Y;
		else
			startx -= VIDEO_XSTEP_PER_Y;
	}
}



static void draw_rle(itech32_state *state, UINT16 *base, UINT16 color)
{
	/* adjust for (lack of) clipping */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		disable_clipping(state);

	/* if we have an X scale, draw it slow */
	if (((VIDEO_TRANSFER_FLAGS & XFERFLAG_DSTXSCALE) && VIDEO_DST_XSTEP != 0x100) || VIDEO_XSTEP_PER_Y)
		draw_rle_slow(state, base, color);

	/* else draw it fast */
	else if (VIDEO_TRANSFER_FLAGS & XFERFLAG_XFLIP)
		draw_rle_fast_xflip(state, base, color);
	else
		draw_rle_fast(state, base, color);

	/* restore cliprects */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		enable_clipping(state);
}



/*************************************
 *
 *  Shift register manipulation
 *
 *************************************/

static void shiftreg_clear(itech32_state *state, UINT16 *base, UINT16 *zbase)
{
	int ydir = (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP) ? -1 : 1;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = VIDEO_TRANSFER_Y & 0xfff;
	UINT16 *src;
	int y;

	/* first line is the source */
	src = &base[compute_safe_address(state, sx, sy)];
	sy += ydir;

	/* loop over height */
	for (y = 1; y < height; y++)
	{
		memcpy(&base[compute_safe_address(state, sx, sy)], src, 512*2);
		if (zbase)
		{
			UINT16 zval = ((state->m_drivedge_zbuf_control[2] >> 8) & 0x7ff) | ((state->m_drivedge_zbuf_control[2] & 0x1f) << 11);
			UINT16 *dst = &zbase[compute_safe_address(state, sx, sy)];
			int x;
			for (x = 0; x < 512; x++)
				*dst++ = zval;
		}
		sy += ydir;
	}
}



/*************************************
 *
 *  Video commands
 *
 *************************************/

static void handle_video_command(running_machine &machine)
{
	itech32_state *state = machine.driver_data<itech32_state>();
	/* only 6 known commands */
	switch (VIDEO_COMMAND)
	{
		/* command 1: blit raw data */
		case 1:
			g_profiler.start(PROFILER_USER1);
			if (BLIT_LOGGING) logblit(machine, "Blit Raw");

			if (state->m_is_drivedge)
			{
				if (state->m_enable_latch[0]) draw_raw_drivedge(state, state->m_videoplane[0], state->m_videoplane[1], state->m_color_latch[0]);
			}
			else
			{
				if (state->m_enable_latch[0]) draw_raw(state, state->m_videoplane[0], state->m_color_latch[0]);
				if (state->m_enable_latch[1]) draw_raw(state, state->m_videoplane[1], state->m_color_latch[1]);
			}

			g_profiler.stop();
			break;

		/* command 2: blit RLE-compressed data */
		case 2:
			g_profiler.start(PROFILER_USER2);
			if (BLIT_LOGGING) logblit(machine, "Blit RLE");

			if (state->m_enable_latch[0]) draw_rle(state, state->m_videoplane[0], state->m_color_latch[0]);
			if (state->m_enable_latch[1]) draw_rle(state, state->m_videoplane[1], state->m_color_latch[1]);

			g_profiler.stop();
			break;

		/* command 3: set up raw data transfer */
		case 3:
			if (BLIT_LOGGING) logblit(machine, "Raw Xfer");
			state->m_xfer_xcount = VIDEO_TRANSFER_WIDTH;
			state->m_xfer_ycount = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
			state->m_xfer_xcur = VIDEO_TRANSFER_X & 0xfff;
			state->m_xfer_ycur = VIDEO_TRANSFER_Y & 0xfff;
			break;

		/* command 4: flush? */
		case 4:
			break;

		/* command 5: reset? */
		case 5:
			break;

		/* command 6: perform shift register copy */
		case 6:
			g_profiler.start(PROFILER_USER3);
			if (BLIT_LOGGING) logblit(machine, "ShiftReg");

			if (state->m_is_drivedge)
			{
				if (state->m_enable_latch[0]) shiftreg_clear(state, state->m_videoplane[0], state->m_videoplane[1]);
			}
			else
			{
				if (state->m_enable_latch[0]) shiftreg_clear(state, state->m_videoplane[0], NULL);
				if (state->m_enable_latch[1]) shiftreg_clear(state, state->m_videoplane[1], NULL);
			}

			g_profiler.stop();
			break;

		default:
			if (BLIT_LOGGING) logerror("Unknown blit command %d\n", VIDEO_COMMAND);
			break;
	}

	/* tell the processor we're done */
	VIDEO_INTSTATE |= VIDEOINT_BLITTER;
	update_interrupts(machine, 1);
}



/*************************************
 *
 *  Video I/O
 *
 *************************************/

WRITE16_HANDLER( itech32_video_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	rectangle visarea;

	int old = state->m_video[offset];
	COMBINE_DATA(&state->m_video[offset]);

	switch (offset)
	{
		case 0x02/2:	/* VIDEO_INTACK */
			VIDEO_INTSTATE = old & ~data;
			update_interrupts(space->machine(), 1);
			break;

		case 0x04/2:	/* VIDEO_TRANSFER */
			if (VIDEO_COMMAND == 3 && state->m_xfer_ycount)
			{
				offs_t addr = compute_safe_address(state, state->m_xfer_xcur, state->m_xfer_ycur);
				if (state->m_enable_latch[0])
				{
					VIDEO_TRANSFER = state->m_videoplane[0][addr];
					state->m_videoplane[0][addr] = (data & 0xff) | state->m_color_latch[0];
				}
				if (state->m_enable_latch[1])
				{
					VIDEO_TRANSFER = state->m_videoplane[1][addr];
					state->m_videoplane[1][addr] = (data & 0xff) | state->m_color_latch[1];
				}
				if (--state->m_xfer_xcount)
					state->m_xfer_xcur++;
				else if (--state->m_xfer_ycount)
					state->m_xfer_xcur = VIDEO_TRANSFER_X, state->m_xfer_xcount = VIDEO_TRANSFER_WIDTH, state->m_xfer_ycur++;
			}
			break;

		case 0x08/2:	/* VIDEO_COMMAND */
			handle_video_command(space->machine());
			break;

		case 0x0a/2:	/* VIDEO_INTENABLE */
			update_interrupts(space->machine(), 1);
			break;

		case 0x24/2:	/* VIDEO_LEFTCLIP */
			state->m_clip_rect.min_x = VIDEO_LEFTCLIP;
			state->m_scaled_clip_rect.min_x = VIDEO_LEFTCLIP << 8;
			break;

		case 0x26/2:	/* VIDEO_RIGHTCLIP */
			state->m_clip_rect.max_x = VIDEO_RIGHTCLIP;
			state->m_scaled_clip_rect.max_x = VIDEO_RIGHTCLIP << 8;
			break;

		case 0x28/2:	/* VIDEO_TOPCLIP */
			state->m_clip_rect.min_y = VIDEO_TOPCLIP;
			state->m_scaled_clip_rect.min_y = VIDEO_TOPCLIP << 8;
			break;

		case 0x2a/2:	/* VIDEO_BOTTOMCLIP */
			state->m_clip_rect.max_y = VIDEO_BOTTOMCLIP;
			state->m_scaled_clip_rect.max_y = VIDEO_BOTTOMCLIP << 8;
			break;

		case 0x2c/2:	/* VIDEO_INTSCANLINE */
			state->m_scanline_timer->adjust(space->machine().primary_screen->time_until_pos(VIDEO_INTSCANLINE));
			break;

		case 0x32/2:	/* VIDEO_VTOTAL */
		case 0x36/2:	/* VIDEO_VBLANK_START */
		case 0x38/2:	/* VIDEO_VBLANK_END */
		case 0x3a/2:	/* VIDEO_HTOTAL */
		case 0x3e/2:	/* VIDEO_HBLANK_START */
		case 0x40/2:	/* VIDEO_HBLANK_END */
			/* do some sanity checks first */
			if ((VIDEO_HTOTAL > 0) && (VIDEO_VTOTAL > 0) &&
			    (VIDEO_VBLANK_START != VIDEO_VBLANK_END) &&
			    (VIDEO_HBLANK_START != VIDEO_HBLANK_END) &&
			    (VIDEO_HBLANK_START < VIDEO_HTOTAL) &&
			    (VIDEO_HBLANK_END < VIDEO_HTOTAL) &&
			    (VIDEO_VBLANK_START < VIDEO_VTOTAL) &&
			    (VIDEO_VBLANK_END < VIDEO_VTOTAL))
			{
				visarea.min_x = visarea.min_y = 0;

				if (VIDEO_HBLANK_START > VIDEO_HBLANK_END)
					visarea.max_x = VIDEO_HBLANK_START - VIDEO_HBLANK_END - 1;
				else
					visarea.max_x = VIDEO_HTOTAL - VIDEO_HBLANK_END + VIDEO_HBLANK_START - 1;

				if (VIDEO_VBLANK_START > VIDEO_VBLANK_END)
					visarea.max_y = VIDEO_VBLANK_START - VIDEO_VBLANK_END - 1;
				else
					visarea.max_y = VIDEO_VTOTAL - VIDEO_VBLANK_END + VIDEO_VBLANK_START - 1;

				logerror("Configure Screen: HTOTAL: %x  HBSTART: %x  HBEND: %x  VTOTAL: %x  VBSTART: %x  VBEND: %x\n",
					VIDEO_HTOTAL, VIDEO_HBLANK_START, VIDEO_HBLANK_END, VIDEO_VTOTAL, VIDEO_VBLANK_START, VIDEO_VBLANK_END);
				space->machine().primary_screen->configure(VIDEO_HTOTAL, VIDEO_VTOTAL, visarea, HZ_TO_ATTOSECONDS(VIDEO_CLOCK) * VIDEO_HTOTAL * VIDEO_VTOTAL);
			}
			break;
	}
}


READ16_HANDLER( itech32_video_r )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	if (offset == 0)
	{
		return (state->m_video[offset] & ~0x08) | 0x04 | 0x01;
	}
	else if (offset == 3)
	{
		return 0xef;/*space->machine().primary_screen->vpos() - 1;*/
	}

	return state->m_video[offset];
}



/*************************************
 *
 *  Alternate video I/O
 *
 *************************************/

WRITE16_HANDLER( bloodstm_video_w )
{
	itech32_video_w(space, offset / 2, data, mem_mask);
}


READ16_HANDLER( bloodstm_video_r )
{
	return itech32_video_r(space, offset / 2, mem_mask);
}


WRITE32_HANDLER( itech020_video_w )
{
	if (ACCESSING_BITS_16_31)
		itech32_video_w(space, offset, data >> 16, mem_mask >> 16);
	else
		itech32_video_w(space, offset, data, mem_mask);
}


WRITE32_HANDLER( drivedge_zbuf_control_w )
{
	itech32_state *state = space->machine().driver_data<itech32_state>();
	COMBINE_DATA(&state->m_drivedge_zbuf_control[offset]);
}


READ32_HANDLER( itech020_video_r )
{
	int result = itech32_video_r(space, offset, mem_mask);
	return (result << 16) | result;
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( itech32 )
{
	itech32_state *state = screen.machine().driver_data<itech32_state>();
	int y;

	/* loop over height */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *src1 = &state->m_videoplane[0][compute_safe_address(state, VIDEO_DISPLAY_XORIGIN1, VIDEO_DISPLAY_YORIGIN1 + y)];

		/* handle multi-plane case */
		if (state->m_planes > 1)
		{
			UINT16 *src2 = &state->m_videoplane[1][compute_safe_address(state, VIDEO_DISPLAY_XORIGIN2 + VIDEO_DISPLAY_XSCROLL2, VIDEO_DISPLAY_YORIGIN2 + VIDEO_DISPLAY_YSCROLL2 + y)];
			UINT16 scanline[384];
			int x;

			/* blend the pixels in the scanline; color xxFF is transparent */
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				UINT16 pixel = src1[x];
				if ((pixel & 0xff) == 0xff)
					pixel = src2[x];
				scanline[x] = pixel;
			}

			/* draw from the buffer */
			draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &scanline[cliprect.min_x], NULL);
		}

		/* otherwise, draw directly from VRAM */
		else
			draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &src1[cliprect.min_x], NULL);
	}
	return 0;
}
