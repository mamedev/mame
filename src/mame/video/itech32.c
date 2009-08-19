/***************************************************************************

    Incredible Technologies/Strata system
    (32-bit blitter variant)

***************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "cpu/m68000/m68000.h"
#include "itech32.h"


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

#define VIDEO_UNKNOWN00			itech32_video[0x00/2] 	/* $0087 at startup */
#define VIDEO_STATUS			itech32_video[0x00/2]
#define VIDEO_INTSTATE			itech32_video[0x02/2]
#define VIDEO_INTACK			itech32_video[0x02/2]
#define VIDEO_TRANSFER			itech32_video[0x04/2]
#define VIDEO_TRANSFER_FLAGS	itech32_video[0x06/2]	/* $5080 at startup (kept at $1512) */
#define VIDEO_COMMAND			itech32_video[0x08/2]	/* $0005 at startup */
#define VIDEO_INTENABLE			itech32_video[0x0a/2]	/* $0144 at startup (kept at $1514) */
#define VIDEO_TRANSFER_HEIGHT	itech32_video[0x0c/2]
#define VIDEO_TRANSFER_WIDTH	itech32_video[0x0e/2]
#define VIDEO_TRANSFER_ADDRLO	itech32_video[0x10/2]
#define VIDEO_TRANSFER_X		itech32_video[0x12/2]
#define VIDEO_TRANSFER_Y		itech32_video[0x14/2]
#define VIDEO_SRC_YSTEP			itech32_video[0x16/2]	/* $0011 at startup */
#define VIDEO_SRC_XSTEP			itech32_video[0x18/2]
#define VIDEO_DST_XSTEP			itech32_video[0x1a/2]
#define VIDEO_DST_YSTEP			itech32_video[0x1c/2]
#define VIDEO_YSTEP_PER_X		itech32_video[0x1e/2]
#define VIDEO_XSTEP_PER_Y		itech32_video[0x20/2]
#define VIDEO_UNKNOWN22			itech32_video[0x22/2]	/* $0033 at startup */
#define VIDEO_LEFTCLIP			itech32_video[0x24/2]
#define VIDEO_RIGHTCLIP			itech32_video[0x26/2]
#define VIDEO_TOPCLIP			itech32_video[0x28/2]
#define VIDEO_BOTTOMCLIP		itech32_video[0x2a/2]
#define VIDEO_INTSCANLINE		itech32_video[0x2c/2]	/* $00ef at startup */
#define VIDEO_TRANSFER_ADDRHI	itech32_video[0x2e/2]	/* $0000 at startup */

#define VIDEO_UNKNOWN30			itech32_video[0x30/2]	/* $0040 at startup */
#define VIDEO_VTOTAL			itech32_video[0x32/2]	/* $0106 at startup */
#define VIDEO_VSYNC				itech32_video[0x34/2]	/* $0101 at startup */
#define VIDEO_VBLANK_START		itech32_video[0x36/2]	/* $00f3 at startup */
#define VIDEO_VBLANK_END		itech32_video[0x38/2]	/* $0003 at startup */
#define VIDEO_HTOTAL			itech32_video[0x3a/2]	/* $01fc at startup */
#define VIDEO_HSYNC				itech32_video[0x3c/2]	/* $01e4 at startup */
#define VIDEO_HBLANK_START		itech32_video[0x3e/2]	/* $01b2 at startup */
#define VIDEO_HBLANK_END		itech32_video[0x40/2]	/* $0032 at startup */
#define VIDEO_UNKNOWN42			itech32_video[0x42/2]	/* $0015 at startup */
#define VIDEO_DISPLAY_YORIGIN1	itech32_video[0x44/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_YORIGIN2	itech32_video[0x46/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_YSCROLL2	itech32_video[0x48/2]	/* $0000 at startup */
#define VIDEO_UNKNOWN4a			itech32_video[0x4a/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_XORIGIN1	itech32_video[0x4c/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_XORIGIN2	itech32_video[0x4e/2]	/* $0000 at startup */
#define VIDEO_DISPLAY_XSCROLL2	itech32_video[0x50/2]	/* $0000 at startup */
#define VIDEO_UNKNOWN52			itech32_video[0x52/2]	/* $0000 at startup */
#define VIDEO_UNKNOWN54			itech32_video[0x54/2]	/* $0080 at startup */
#define VIDEO_UNKNOWN56			itech32_video[0x56/2]	/* $00c0 at startup */
#define VIDEO_UNKNOWN58			itech32_video[0x58/2]	/* $01c0 at startup */
#define VIDEO_UNKNOWN5a			itech32_video[0x5a/2]	/* $01c0 at startup */
#define VIDEO_UNKNOWN5c			itech32_video[0x5c/2]	/* $01cf at startup */
#define VIDEO_UNKNOWN5e			itech32_video[0x5e/2]	/* $01cf at startup */
#define VIDEO_UNKNOWN60			itech32_video[0x60/2]	/* $01e3 at startup */
#define VIDEO_UNKNOWN62			itech32_video[0x62/2]	/* $01cf at startup */
#define VIDEO_UNKNOWN64			itech32_video[0x64/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN66			itech32_video[0x66/2]	/* $0183 at startup */
#define VIDEO_UNKNOWN68			itech32_video[0x68/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN6a			itech32_video[0x6a/2]	/* $000f at startup */
#define VIDEO_UNKNOWN6c			itech32_video[0x6c/2]	/* $018f at startup */
#define VIDEO_UNKNOWN6e			itech32_video[0x6e/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN70			itech32_video[0x70/2]	/* $000f at startup */
#define VIDEO_UNKNOWN72			itech32_video[0x72/2]	/* $000f at startup */
#define VIDEO_UNKNOWN74			itech32_video[0x74/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN76			itech32_video[0x76/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN78			itech32_video[0x78/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN7a			itech32_video[0x7a/2]	/* $01ff at startup */
#define VIDEO_UNKNOWN7c			itech32_video[0x7c/2]	/* $0820 at startup */
#define VIDEO_UNKNOWN7e			itech32_video[0x7e/2]	/* $0100 at startup */

#define VIDEO_STARTSTEP			itech32_video[0x80/2]	/* drivedge only? */
#define VIDEO_LEFTSTEPLO		itech32_video[0x82/2]	/* drivedge only? */
#define VIDEO_LEFTSTEPHI		itech32_video[0x84/2]	/* drivedge only? */
#define VIDEO_RIGHTSTEPLO		itech32_video[0x86/2]	/* drivedge only? */
#define VIDEO_RIGHTSTEPHI		itech32_video[0x88/2]	/* drivedge only? */

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



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT16 *itech32_video;
UINT32 *drivedge_zbuf_control;
UINT8 itech32_planes;
UINT16 itech32_vram_height;

static UINT16 xfer_xcount, xfer_ycount;
static UINT16 xfer_xcur, xfer_ycur;

static rectangle clip_rect, scaled_clip_rect;
static rectangle clip_save;

static emu_timer *scanline_timer;

static UINT8 *grom_base;
static UINT32 grom_size;
static UINT32 grom_bank;
static UINT32 grom_bank_mask;

static UINT16 color_latch[2];
static UINT8 enable_latch[2];

static UINT16 *videoplane[2];
static UINT32 vram_mask;
static UINT32 vram_xmask, vram_ymask;

static int is_drivedge;

static TIMER_CALLBACK( scanline_interrupt );



/*************************************
 *
 *  Macros and inlines
 *
 *************************************/

#define ADJUSTED_HEIGHT(x) ((((x) >> 1) & 0x100) | ((x) & 0xff))

INLINE offs_t compute_safe_address(int x, int y)
{
	return ((y & vram_ymask) * 512) + (x & vram_xmask);
}

INLINE void disable_clipping(void)
{
	clip_save = clip_rect;

	clip_rect.min_x = clip_rect.min_y = 0;
	clip_rect.max_x = clip_rect.max_y = 0xfff;

	scaled_clip_rect.min_x = scaled_clip_rect.min_y = 0;
	scaled_clip_rect.max_x = scaled_clip_rect.max_y = 0xfff << 8;
}

INLINE void enable_clipping(void)
{
	clip_rect = clip_save;

	scaled_clip_rect.min_x = clip_rect.min_x << 8;
	scaled_clip_rect.max_x = clip_rect.max_x << 8;
	scaled_clip_rect.min_y = clip_rect.min_y << 8;
	scaled_clip_rect.max_y = clip_rect.max_y << 8;
}



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( itech32 )
{
	int i;

	/* allocate memory */
	videoram16 = auto_alloc_array(machine, UINT16, VRAM_WIDTH * (itech32_vram_height + 16) * 2);
	memset(videoram16, 0xff, VRAM_WIDTH * (itech32_vram_height + 16) * 2 * 2);

	/* videoplane[0] is the foreground; videoplane[1] is the background */
	videoplane[0] = &videoram16[0 * VRAM_WIDTH * (itech32_vram_height + 16) + 8 * VRAM_WIDTH];
	videoplane[1] = &videoram16[1 * VRAM_WIDTH * (itech32_vram_height + 16) + 8 * VRAM_WIDTH];

	/* set the masks */
	vram_mask = VRAM_WIDTH * itech32_vram_height - 1;
	vram_xmask = VRAM_WIDTH - 1;
	vram_ymask = itech32_vram_height - 1;

	/* clear the planes initially */
	for (i = 0; i < VRAM_WIDTH * itech32_vram_height; i++)
		videoplane[0][i] = videoplane[1][i] = 0xff;

	/* fetch the GROM base */
	grom_base = memory_region(machine, "gfx1");
	grom_size = memory_region_length(machine, "gfx1");
	grom_bank = 0;
	grom_bank_mask = grom_size >> 24;
	if (grom_bank_mask == 2)
		grom_bank_mask = 3;

	/* reset statics */
	memset(itech32_video, 0, 0x80);

	scanline_timer = timer_alloc(machine, scanline_interrupt, NULL);
	enable_latch[0] = 1;
	enable_latch[1] = (itech32_planes > 1) ? 1 : 0;

	is_drivedge = 0;
}



/*************************************
 *
 *  Latches
 *
 *************************************/

WRITE16_HANDLER( timekill_colora_w )
{
	if (ACCESSING_BITS_0_7)
	{
		enable_latch[0] = (~data >> 5) & 1;
		enable_latch[1] = (~data >> 7) & 1;
		color_latch[0] = (data & 0x0f) << 8;
	}
}


WRITE16_HANDLER( timekill_colorbc_w )
{
	if (ACCESSING_BITS_0_7)
		color_latch[1] = ((data & 0xf0) << 4) | 0x1000;
}


WRITE16_HANDLER( timekill_intensity_w )
{
	if (ACCESSING_BITS_0_7)
	{
		double intensity = (double)(data & 0xff) / (double)0x60;
		int i;
		for (i = 0; i < 8192; i++)
			palette_set_pen_contrast(space->machine, i, intensity);
	}
}


WRITE16_HANDLER( bloodstm_color1_w )
{
	if (ACCESSING_BITS_0_7)
		color_latch[0] = (data & 0x7f) << 8;
}


WRITE16_HANDLER( bloodstm_color2_w )
{
	if (ACCESSING_BITS_0_7)
		color_latch[1] = (data & 0x7f) << 8;
}


WRITE16_HANDLER( bloodstm_plane_w )
{
	if (ACCESSING_BITS_0_7)
	{
		enable_latch[0] = (~data >> 1) & 1;
		enable_latch[1] = (~data >> 2) & 1;
	}
}


WRITE32_HANDLER( drivedge_color0_w )
{
	if (ACCESSING_BITS_16_23)
		color_latch[0] = ((data >> 16) & 0x7f) << 8;
}


WRITE32_HANDLER( itech020_color1_w )
{
	if (ACCESSING_BITS_0_7)
		color_latch[1] = (data & 0x7f) << 8;
}


WRITE32_HANDLER( itech020_color2_w )
{
	if (ACCESSING_BITS_0_7)
		color_latch[0] = (data & 0x7f) << 8;
}


WRITE32_HANDLER( itech020_plane_w )
{
	if (ACCESSING_BITS_8_15)
	{
		enable_latch[0] = (~data >> 9) & 1;
		enable_latch[1] = (~data >> 10) & 1;
		grom_bank = ((data >> 14) & grom_bank_mask) << 24;
	}
}



/*************************************
 *
 *  Palette I/O
 *
 *************************************/

WRITE16_HANDLER( timekill_paletteram_w )
{
	int r, g, b;

	COMBINE_DATA(&paletteram16[offset]);

	r = paletteram16[offset & ~1] & 0xff;
	g = paletteram16[offset & ~1] >> 8;
	b = paletteram16[offset |  1] >> 8;

	palette_set_color(space->machine, offset / 2, MAKE_RGB(r, g, b));
}


WRITE16_HANDLER( bloodstm_paletteram_w )
{
	int r, g, b;

	/* in test mode, the LSB is used; in game mode, the MSB is used */
	if (!ACCESSING_BITS_0_7 && (offset & 1))
		data >>= 8, mem_mask >>= 8;
	COMBINE_DATA(&paletteram16[offset]);

	r = paletteram16[offset & ~1] & 0xff;
	g = paletteram16[offset & ~1] >> 8;
	b = paletteram16[offset |  1] & 0xff;

	palette_set_color(space->machine, offset / 2, MAKE_RGB(r, g, b));
}


WRITE32_HANDLER( drivedge_paletteram_w )
{
	int r, g, b;

	COMBINE_DATA(&paletteram32[offset]);

	r = paletteram32[offset] & 0xff;
	g = (paletteram32[offset] >> 8) & 0xff;
	b = (paletteram32[offset] >> 16) & 0xff;

	palette_set_color(space->machine, offset, MAKE_RGB(r, g, b));
}


WRITE32_HANDLER( itech020_paletteram_w )
{
	int r, g, b;

	COMBINE_DATA(&paletteram32[offset]);

	r = (paletteram32[offset] >> 16) & 0xff;
	g = (paletteram32[offset] >> 8) & 0xff;
	b = paletteram32[offset] & 0xff;

	palette_set_color(space->machine, offset, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Debugging
 *
 *************************************/

static void logblit(const char *tag)
{
	if (!input_code_pressed(KEYCODE_L))
		return;
	if (is_drivedge && VIDEO_TRANSFER_FLAGS == 0x5490)
	{
		/* polygon drawing */
		logerror("%s: e=%d%d f=%04x s=(%03x-%03x,%03x) w=%03x h=%03x b=%02x%04x c=%02x%02x ss=%04x,%04x ds=%04x,%04x ls=%04x%04x rs=%04x%04x u80=%04x", tag,
			enable_latch[0], enable_latch[1],
			VIDEO_TRANSFER_FLAGS,
			VIDEO_TRANSFER_X, VIDEO_RIGHTCLIP, VIDEO_TRANSFER_Y, VIDEO_TRANSFER_WIDTH, VIDEO_TRANSFER_HEIGHT,
			VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
			color_latch[0] >> 8, color_latch[1] >> 8,
			VIDEO_SRC_XSTEP, VIDEO_SRC_YSTEP,
			VIDEO_DST_XSTEP, VIDEO_DST_YSTEP,
			VIDEO_LEFTSTEPHI, VIDEO_LEFTSTEPLO, VIDEO_RIGHTSTEPHI, VIDEO_RIGHTSTEPLO,
			VIDEO_STARTSTEP);
	}

	else if (itech32_video[0x16/2] == 0x100 && itech32_video[0x18/2] == 0x100 &&
		itech32_video[0x1a/2] == 0x000 && itech32_video[0x1c/2] == 0x100 &&
		itech32_video[0x1e/2] == 0x000 && itech32_video[0x20/2] == 0x000)
	{
		logerror("%s: e=%d%d f=%04x c=%02x%02x %02x%04x -> (%03x,%03x) %3dx%3dc=(%03x,%03x)-(%03x,%03x)", tag,
				enable_latch[0], enable_latch[1],
				VIDEO_TRANSFER_FLAGS,
				color_latch[0] >> 8, color_latch[1] >> 8,
				VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
				VIDEO_TRANSFER_X, VIDEO_TRANSFER_Y,
				VIDEO_TRANSFER_WIDTH, ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT),
				VIDEO_LEFTCLIP, VIDEO_TOPCLIP, VIDEO_RIGHTCLIP, VIDEO_BOTTOMCLIP);
	}
	else
	{
		logerror("%s: e=%d%d f=%04x c=%02x%02x %02x%04x -> (%03x,%03x) %3dx%3d c=(%03x,%03x)-(%03x,%03x) s=%04x %04x %04x %04x %04x %04x", tag,
				enable_latch[0], enable_latch[1],
				VIDEO_TRANSFER_FLAGS,
				color_latch[0] >> 8, color_latch[1] >> 8,
				VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
				VIDEO_TRANSFER_X, VIDEO_TRANSFER_Y,
				VIDEO_TRANSFER_WIDTH, ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT),
				VIDEO_LEFTCLIP, VIDEO_TOPCLIP, VIDEO_RIGHTCLIP, VIDEO_BOTTOMCLIP,
				itech32_video[0x16/2], itech32_video[0x18/2], itech32_video[0x1a/2],
				itech32_video[0x1c/2], itech32_video[0x1e/2], itech32_video[0x20/2]);
	}
	if (is_drivedge) logerror(" v0=%08x v1=%08x v2=%08x v3=%08x", drivedge_zbuf_control[0], drivedge_zbuf_control[1], drivedge_zbuf_control[2], drivedge_zbuf_control[3]);
	logerror("\n");
}



/*************************************
 *
 *  Video interrupts
 *
 *************************************/

static void update_interrupts(running_machine *machine, int fast)
{
	int scanline_state = 0, blitter_state = 0;

	if (VIDEO_INTSTATE & VIDEO_INTENABLE & VIDEOINT_SCANLINE)
		scanline_state = 1;
	if (VIDEO_INTSTATE & VIDEO_INTENABLE & VIDEOINT_BLITTER)
		blitter_state = 1;

	itech32_update_interrupts(machine, -1, blitter_state, scanline_state);
}


static TIMER_CALLBACK( scanline_interrupt )
{
	/* set timer for next frame */
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, VIDEO_INTSCANLINE, 0), 0);

	/* set the interrupt bit in the status reg */
	logerror("-------------- (DISPLAY INT @ %d) ----------------\n", video_screen_get_vpos(machine->primary_screen));
	VIDEO_INTSTATE |= VIDEOINT_SCANLINE;

	/* update the interrupt state */
	update_interrupts(machine, 0);
}



/*************************************
 *
 *  Uncompressed blitter functions
 *
 *************************************/

static void draw_raw(UINT16 *base, UINT16 color)
{
	UINT8 *src = &grom_base[(grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % grom_size];
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
		disable_clipping();

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
			if (sy >= scaled_clip_rect.min_y && sy < scaled_clip_rect.max_y)
			{
				UINT32 dstoffs;

				/* direction matters here */
				sx = startx;
				if (xdststep > 0)
				{
					/* skip left pixels */
					for (x = 0; x < width && sx < scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep) ;

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					for ( ; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
							base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
					}
				}
				else
				{
					/* skip right pixels */
					for (x = 0; x < width && sx >= scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep) ;

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					for ( ; x < width && sx >= scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
							base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
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
			for (x = 0; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
				if (ty >= scaled_clip_rect.min_y && ty < scaled_clip_rect.max_y &&
					sx >= scaled_clip_rect.min_x && sx < scaled_clip_rect.max_x)
				{
					int pixel = rowsrc[x >> 8];
					if (pixel != transparent_pen)
						base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
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
		enable_clipping();
}


static void draw_raw_drivedge(UINT16 *base, UINT16 *zbase, UINT16 color)
{
	UINT8 *src = &grom_base[(grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH << 8;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT) << 8;
	int xsrcstep = VIDEO_SRC_XSTEP;
	int ysrcstep = VIDEO_SRC_YSTEP;
	int sx, sy = ((VIDEO_TRANSFER_Y & 0xfff) << 8) + 0x80;
	int startx = ((VIDEO_TRANSFER_X & 0xfff) << 8) + 0x80;
	int xdststep = 0x100;
	int ydststep = VIDEO_DST_YSTEP;
	INT32 z0 = drivedge_zbuf_control[2] & 0x7ff00;
	INT32 zmatch = (drivedge_zbuf_control[2] & 0x1f) << 11;
	INT32 srcdelta = 0;
	int x, y;

	/* adjust for (lack of) clipping */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		disable_clipping();

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
			if (sy >= scaled_clip_rect.min_y && sy < scaled_clip_rect.max_y)
			{
				UINT32 dstoffs, zbufoffs;
				INT32 z = z0;

				/* direction matters here */
				sx = startx;
				if (xdststep > 0)
				{
					/* skip left pixels */
					for (x = 0; x < width && sx < scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						z += (INT32)drivedge_zbuf_control[0];

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);
					zbufoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					if (drivedge_zbuf_control[3] & 0x8000)
					{
						for ( ; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen)
							{
								base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)drivedge_zbuf_control[0];
						}
					}
					else if (drivedge_zbuf_control[3] & 0x4000)
					{
						for ( ; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && zmatch == (zbase[(zbufoffs + (sx >> 8)) & vram_mask] & (0x1f << 11)))
								base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
							z += (INT32)drivedge_zbuf_control[0];
						}
					}
					else
					{
						for ( ; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && ((z >> 8) <= (zbase[(zbufoffs + (sx >> 8)) & vram_mask] & 0x7ff)))
							{
								base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)drivedge_zbuf_control[0];
						}
					}
				}
				else
				{
					/* skip right pixels */
					for (x = 0; x < width && sx >= scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						z += (INT32)drivedge_zbuf_control[0];

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);
					zbufoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					if (drivedge_zbuf_control[3] & 0x8000)
					{
						for ( ; x < width && sx >= scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen)
							{
								base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)drivedge_zbuf_control[0];
						}
					}
					else if (drivedge_zbuf_control[3] & 0x4000)
					{
						for ( ; x < width && sx >= scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && zmatch == (zbase[(zbufoffs + (sx >> 8)) & vram_mask] & (0x1f << 11)))
								base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
							z += (INT32)drivedge_zbuf_control[0];
						}
					}
					else
					{
						for ( ; x < width && sx >= scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && ((z >> 8) <= (zbase[(zbufoffs + (sx >> 8)) & vram_mask] & 0x7ff)))
							{
								base[(dstoffs + (sx >> 8)) & vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)drivedge_zbuf_control[0];
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
			if (drivedge_zbuf_control[3] & 0x8000)
			{
				for (x = 0; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (ty >= scaled_clip_rect.min_y && ty < scaled_clip_rect.max_y &&
						sx >= scaled_clip_rect.min_x && sx < scaled_clip_rect.max_x)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
						{
							base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
							zbase[compute_safe_address(sx >> 8, ty >> 8)] = (z >> 8) | zmatch;
						}
						z += (INT32)drivedge_zbuf_control[0];
					}
			}
			else if (drivedge_zbuf_control[3] & 0x4000)
			{
				for (x = 0; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (ty >= scaled_clip_rect.min_y && ty < scaled_clip_rect.max_y &&
						sx >= scaled_clip_rect.min_x && sx < scaled_clip_rect.max_x)
					{
						int pixel = rowsrc[x >> 8];
						UINT16 *zbuf = &zbase[compute_safe_address(sx >> 8, ty >> 8)];
						if (pixel != transparent_pen && zmatch == (*zbuf & (0x1f << 11)))
						{
							base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
							*zbuf = (z >> 8) | zmatch;
						}
						z += (INT32)drivedge_zbuf_control[0];
					}
			}
			else
			{
				for (x = 0; x < width && sx < scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (ty >= scaled_clip_rect.min_y && ty < scaled_clip_rect.max_y &&
						sx >= scaled_clip_rect.min_x && sx < scaled_clip_rect.max_x)
					{
						int pixel = rowsrc[x >> 8];
						UINT16 *zbuf = &zbase[compute_safe_address(sx >> 8, ty >> 8)];
						if (pixel != transparent_pen && ((z >> 8) <= (*zbuf & 0x7ff)))
						{
							base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
							*zbuf = (z >> 8) | zmatch;
						}
						z += (INT32)drivedge_zbuf_control[0];
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
			scaled_clip_rect.max_x += (INT32)((VIDEO_RIGHTSTEPHI << 16) | VIDEO_RIGHTSTEPLO);
			srcdelta += (INT16)VIDEO_STARTSTEP;
		}
		z0 += (INT32)drivedge_zbuf_control[1];
	}

	/* restore cliprects */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		enable_clipping();

	/* reflect the final values into registers */
	VIDEO_TRANSFER_X = (VIDEO_TRANSFER_X & ~0xfff) | (startx >> 8);
	VIDEO_RIGHTCLIP = (VIDEO_RIGHTCLIP & ~0xfff) | (scaled_clip_rect.max_x >> 8);
	VIDEO_TRANSFER_Y = (VIDEO_TRANSFER_Y & ~0xfff) | ((VIDEO_TRANSFER_Y + (y >> 8)) & 0xfff);
	VIDEO_TRANSFER_ADDRLO += srcdelta >> 8;

	drivedge_zbuf_control[2] = (drivedge_zbuf_control[2] & ~0x7ff00) | (z0 & 0x7ff00);
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

INLINE void draw_rle_fast(UINT16 *base, UINT16 color)
{
	UINT8 *src = &grom_base[(grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int ydststep = VIDEO_DST_YSTEP;
	int lclip, rclip;

	/* determine clipping */
	lclip = clip_rect.min_x - sx;
	if (lclip < 0) lclip = 0;
	rclip = sx + width - clip_rect.max_x;
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
		if (sy < scaled_clip_rect.min_y || sy >= scaled_clip_rect.max_y)
		{
			SKIP_RLE(width + lclip + rclip, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		dstoffs = compute_safe_address(sx, sy >> 8);

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
						base[dstoffs & vram_mask] = color | pixel;
					dstoffs++;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				while (innercount--)
					base[dstoffs++ & vram_mask] = val;
			}

			/* run of transparent repeats */
			else
				dstoffs += innercount;
		}

		/* right clip */
		SKIP_RLE(rclip, xleft, count, innercount, src);
	}
}


INLINE void draw_rle_fast_xflip(UINT16 *base, UINT16 color)
{
	UINT8 *src = &grom_base[(grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int ydststep = VIDEO_DST_YSTEP;
	int lclip, rclip;

	/* determine clipping */
	lclip = sx - clip_rect.max_x;
	if (lclip < 0) lclip = 0;
	rclip = clip_rect.min_x - (sx - width);
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
		if (sy < scaled_clip_rect.min_y || sy >= scaled_clip_rect.max_y)
		{
			SKIP_RLE(width + lclip + rclip, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		dstoffs = compute_safe_address(sx, sy >> 8);

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
						base[dstoffs & vram_mask] = color | pixel;
					dstoffs--;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				while (innercount--)
					base[dstoffs-- & vram_mask] = val;
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

INLINE void draw_rle_slow(UINT16 *base, UINT16 color)
{
	UINT8 *src = &grom_base[(grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % grom_size];
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
		if (sy < scaled_clip_rect.min_y || sy >= scaled_clip_rect.max_y)
		{
			SKIP_RLE(width, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		sx = startx;
		dstoffs = compute_safe_address(clip_rect.min_x, sy >> 8) - clip_rect.min_x;

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
						if (sx >= scaled_clip_rect.min_x && sx < scaled_clip_rect.max_x)
							base[(dstoffs + (sx >> 8)) & vram_mask] = color | pixel;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				for ( ; innercount--; sx += xdststep)
					if (sx >= scaled_clip_rect.min_x && sx < scaled_clip_rect.max_x)
						base[(dstoffs + (sx >> 8)) & vram_mask] = val;
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



static void draw_rle(UINT16 *base, UINT16 color)
{
	/* adjust for (lack of) clipping */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		disable_clipping();

	/* if we have an X scale, draw it slow */
	if (((VIDEO_TRANSFER_FLAGS & XFERFLAG_DSTXSCALE) && VIDEO_DST_XSTEP != 0x100) || VIDEO_XSTEP_PER_Y)
		draw_rle_slow(base, color);

	/* else draw it fast */
	else if (VIDEO_TRANSFER_FLAGS & XFERFLAG_XFLIP)
		draw_rle_fast_xflip(base, color);
	else
		draw_rle_fast(base, color);

	/* restore cliprects */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		enable_clipping();
}



/*************************************
 *
 *  Shift register manipulation
 *
 *************************************/

static void shiftreg_clear(UINT16 *base, UINT16 *zbase)
{
	int ydir = (VIDEO_TRANSFER_FLAGS & XFERFLAG_YFLIP) ? -1 : 1;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = VIDEO_TRANSFER_Y & 0xfff;
	UINT16 *src;
	int y;

	/* first line is the source */
	src = &base[compute_safe_address(sx, sy)];
	sy += ydir;

	/* loop over height */
	for (y = 1; y < height; y++)
	{
		memcpy(&base[compute_safe_address(sx, sy)], src, 512*2);
		if (zbase)
		{
			UINT16 zval = ((drivedge_zbuf_control[2] >> 8) & 0x7ff) | ((drivedge_zbuf_control[2] & 0x1f) << 11);
			UINT16 *dst = &zbase[compute_safe_address(sx, sy)];
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

static void handle_video_command(running_machine *machine)
{
	/* only 6 known commands */
	switch (VIDEO_COMMAND)
	{
		/* command 1: blit raw data */
		case 1:
			profiler_mark_start(PROFILER_USER1);
			if (BLIT_LOGGING) logblit("Blit Raw");

			if (is_drivedge)
			{
				if (enable_latch[0]) draw_raw_drivedge(videoplane[0], videoplane[1], color_latch[0]);
			}
			else
			{
				if (enable_latch[0]) draw_raw(videoplane[0], color_latch[0]);
				if (enable_latch[1]) draw_raw(videoplane[1], color_latch[1]);
			}

			profiler_mark_end();
			break;

		/* command 2: blit RLE-compressed data */
		case 2:
			profiler_mark_start(PROFILER_USER2);
			if (BLIT_LOGGING) logblit("Blit RLE");

			if (enable_latch[0]) draw_rle(videoplane[0], color_latch[0]);
			if (enable_latch[1]) draw_rle(videoplane[1], color_latch[1]);

			profiler_mark_end();
			break;

		/* command 3: set up raw data transfer */
		case 3:
			if (BLIT_LOGGING) logblit("Raw Xfer");
			xfer_xcount = VIDEO_TRANSFER_WIDTH;
			xfer_ycount = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
			xfer_xcur = VIDEO_TRANSFER_X & 0xfff;
			xfer_ycur = VIDEO_TRANSFER_Y & 0xfff;
			break;

		/* command 4: flush? */
		case 4:
			break;

		/* command 5: reset? */
		case 5:
			break;

		/* command 6: perform shift register copy */
		case 6:
			profiler_mark_start(PROFILER_USER3);
			if (BLIT_LOGGING) logblit("ShiftReg");

			if (is_drivedge)
			{
				if (enable_latch[0]) shiftreg_clear(videoplane[0], videoplane[1]);
			}
			else
			{
				if (enable_latch[0]) shiftreg_clear(videoplane[0], NULL);
				if (enable_latch[1]) shiftreg_clear(videoplane[1], NULL);
			}

			profiler_mark_end();
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
	rectangle visarea;

	int old = itech32_video[offset];
	COMBINE_DATA(&itech32_video[offset]);

	switch (offset)
	{
		case 0x02/2:	/* VIDEO_INTACK */
			VIDEO_INTSTATE = old & ~data;
			update_interrupts(space->machine, 1);
			break;

		case 0x04/2:	/* VIDEO_TRANSFER */
			if (VIDEO_COMMAND == 3 && xfer_ycount)
			{
				offs_t addr = compute_safe_address(xfer_xcur, xfer_ycur);
				if (enable_latch[0])
				{
					VIDEO_TRANSFER = videoplane[0][addr];
					videoplane[0][addr] = (data & 0xff) | color_latch[0];
				}
				if (enable_latch[1])
				{
					VIDEO_TRANSFER = videoplane[1][addr];
					videoplane[1][addr] = (data & 0xff) | color_latch[1];
				}
				if (--xfer_xcount)
					xfer_xcur++;
				else if (--xfer_ycount)
					xfer_xcur = VIDEO_TRANSFER_X, xfer_xcount = VIDEO_TRANSFER_WIDTH, xfer_ycur++;
			}
			break;

		case 0x08/2:	/* VIDEO_COMMAND */
			handle_video_command(space->machine);
			break;

		case 0x0a/2:	/* VIDEO_INTENABLE */
			update_interrupts(space->machine, 1);
			break;

		case 0x24/2:	/* VIDEO_LEFTCLIP */
			clip_rect.min_x = VIDEO_LEFTCLIP;
			scaled_clip_rect.min_x = VIDEO_LEFTCLIP << 8;
			break;

		case 0x26/2:	/* VIDEO_RIGHTCLIP */
			clip_rect.max_x = VIDEO_RIGHTCLIP;
			scaled_clip_rect.max_x = VIDEO_RIGHTCLIP << 8;
			break;

		case 0x28/2:	/* VIDEO_TOPCLIP */
			clip_rect.min_y = VIDEO_TOPCLIP;
			scaled_clip_rect.min_y = VIDEO_TOPCLIP << 8;
			break;

		case 0x2a/2:	/* VIDEO_BOTTOMCLIP */
			clip_rect.max_y = VIDEO_BOTTOMCLIP;
			scaled_clip_rect.max_y = VIDEO_BOTTOMCLIP << 8;
			break;

		case 0x2c/2:	/* VIDEO_INTSCANLINE */
			timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(space->machine->primary_screen, VIDEO_INTSCANLINE, 0), 0);
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
				video_screen_configure(space->machine->primary_screen, VIDEO_HTOTAL, VIDEO_VTOTAL, &visarea, HZ_TO_ATTOSECONDS(VIDEO_CLOCK) * VIDEO_HTOTAL * VIDEO_VTOTAL);
			}
			break;
	}
}


READ16_HANDLER( itech32_video_r )
{
	if (offset == 0)
	{
		return (itech32_video[offset] & ~0x08) | 0x04 | 0x01;
	}
	else if (offset == 3)
	{
		return 0xef;/*video_screen_get_vpos(space->machine->primary_screen) - 1;*/
	}

	return itech32_video[offset];
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
	COMBINE_DATA(&drivedge_zbuf_control[offset]);
	is_drivedge = 1;
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

VIDEO_UPDATE( itech32 )
{
	int y;

	/* loop over height */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *src1 = &videoplane[0][compute_safe_address(VIDEO_DISPLAY_XORIGIN1, VIDEO_DISPLAY_YORIGIN1 + y)];

		/* handle multi-plane case */
		if (itech32_planes > 1)
		{
			UINT16 *src2 = &videoplane[1][compute_safe_address(VIDEO_DISPLAY_XORIGIN2 + VIDEO_DISPLAY_XSCROLL2, VIDEO_DISPLAY_YORIGIN2 + VIDEO_DISPLAY_YSCROLL2 + y)];
			UINT16 scanline[384];
			int x;

			/* blend the pixels in the scanline; color xxFF is transparent */
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT16 pixel = src1[x];
				if ((pixel & 0xff) == 0xff)
					pixel = src2[x];
				scanline[x] = pixel;
			}

			/* draw from the buffer */
			draw_scanline16(bitmap, cliprect->min_x, y, cliprect->max_x - cliprect->min_x + 1, &scanline[cliprect->min_x], NULL);
		}

		/* otherwise, draw directly from VRAM */
		else
			draw_scanline16(bitmap, cliprect->min_x, y, cliprect->max_x - cliprect->min_x + 1, &src1[cliprect->min_x], NULL);
	}
	return 0;
}
