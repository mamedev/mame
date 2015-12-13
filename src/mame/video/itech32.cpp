// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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

#define BLIT_LOGGING            0



/*************************************
 *
 *  Blitter constants
 *
 *************************************/

#define VIDEO_UNKNOWN00         m_video[0x00/2] /* $0087 at startup */
#define VIDEO_STATUS            m_video[0x00/2]
#define VIDEO_INTSTATE          m_video[0x02/2]
#define VIDEO_INTACK            m_video[0x02/2]
#define VIDEO_TRANSFER          m_video[0x04/2]
#define VIDEO_TRANSFER_FLAGS    m_video[0x06/2] /* $5080 at startup (kept at $1512) */
#define VIDEO_COMMAND           m_video[0x08/2] /* $0005 at startup */
#define VIDEO_INTENABLE         m_video[0x0a/2] /* $0144 at startup (kept at $1514) */
#define VIDEO_TRANSFER_HEIGHT   m_video[0x0c/2]
#define VIDEO_TRANSFER_WIDTH    m_video[0x0e/2]
#define VIDEO_TRANSFER_ADDRLO   m_video[0x10/2]
#define VIDEO_TRANSFER_X        m_video[0x12/2]
#define VIDEO_TRANSFER_Y        m_video[0x14/2]
#define VIDEO_SRC_YSTEP         m_video[0x16/2] /* $0011 at startup */
#define VIDEO_SRC_XSTEP         m_video[0x18/2]
#define VIDEO_DST_XSTEP         m_video[0x1a/2]
#define VIDEO_DST_YSTEP         m_video[0x1c/2]
#define VIDEO_YSTEP_PER_X       m_video[0x1e/2]
#define VIDEO_XSTEP_PER_Y       m_video[0x20/2]
#define VIDEO_UNKNOWN22         m_video[0x22/2] /* $0033 at startup */
#define VIDEO_LEFTCLIP          m_video[0x24/2]
#define VIDEO_RIGHTCLIP         m_video[0x26/2]
#define VIDEO_TOPCLIP           m_video[0x28/2]
#define VIDEO_BOTTOMCLIP        m_video[0x2a/2]
#define VIDEO_INTSCANLINE       m_video[0x2c/2] /* $00ef at startup */
#define VIDEO_TRANSFER_ADDRHI   m_video[0x2e/2] /* $0000 at startup */

#define VIDEO_UNKNOWN30         m_video[0x30/2] /* $0040 at startup */
#define VIDEO_VTOTAL            m_video[0x32/2] /* $0106 at startup */
#define VIDEO_VSYNC             m_video[0x34/2] /* $0101 at startup */
#define VIDEO_VBLANK_START      m_video[0x36/2] /* $00f3 at startup */
#define VIDEO_VBLANK_END        m_video[0x38/2] /* $0003 at startup */
#define VIDEO_HTOTAL            m_video[0x3a/2] /* $01fc at startup */
#define VIDEO_HSYNC             m_video[0x3c/2] /* $01e4 at startup */
#define VIDEO_HBLANK_START      m_video[0x3e/2] /* $01b2 at startup */
#define VIDEO_HBLANK_END        m_video[0x40/2] /* $0032 at startup */
#define VIDEO_UNKNOWN42         m_video[0x42/2] /* $0015 at startup */
#define VIDEO_DISPLAY_YORIGIN1  m_video[0x44/2] /* $0000 at startup */
#define VIDEO_DISPLAY_YORIGIN2  m_video[0x46/2] /* $0000 at startup */
#define VIDEO_DISPLAY_YSCROLL2  m_video[0x48/2] /* $0000 at startup */
#define VIDEO_UNKNOWN4a         m_video[0x4a/2] /* $0000 at startup */
#define VIDEO_DISPLAY_XORIGIN1  m_video[0x4c/2] /* $0000 at startup */
#define VIDEO_DISPLAY_XORIGIN2  m_video[0x4e/2] /* $0000 at startup */
#define VIDEO_DISPLAY_XSCROLL2  m_video[0x50/2] /* $0000 at startup */
#define VIDEO_UNKNOWN52         m_video[0x52/2] /* $0000 at startup */
#define VIDEO_UNKNOWN54         m_video[0x54/2] /* $0080 at startup */
#define VIDEO_UNKNOWN56         m_video[0x56/2] /* $00c0 at startup */
#define VIDEO_UNKNOWN58         m_video[0x58/2] /* $01c0 at startup */
#define VIDEO_UNKNOWN5a         m_video[0x5a/2] /* $01c0 at startup */
#define VIDEO_UNKNOWN5c         m_video[0x5c/2] /* $01cf at startup */
#define VIDEO_UNKNOWN5e         m_video[0x5e/2] /* $01cf at startup */
#define VIDEO_UNKNOWN60         m_video[0x60/2] /* $01e3 at startup */
#define VIDEO_UNKNOWN62         m_video[0x62/2] /* $01cf at startup */
#define VIDEO_UNKNOWN64         m_video[0x64/2] /* $01ff at startup */
#define VIDEO_UNKNOWN66         m_video[0x66/2] /* $0183 at startup */
#define VIDEO_UNKNOWN68         m_video[0x68/2] /* $01ff at startup */
#define VIDEO_UNKNOWN6a         m_video[0x6a/2] /* $000f at startup */
#define VIDEO_UNKNOWN6c         m_video[0x6c/2] /* $018f at startup */
#define VIDEO_UNKNOWN6e         m_video[0x6e/2] /* $01ff at startup */
#define VIDEO_UNKNOWN70         m_video[0x70/2] /* $000f at startup */
#define VIDEO_UNKNOWN72         m_video[0x72/2] /* $000f at startup */
#define VIDEO_UNKNOWN74         m_video[0x74/2] /* $01ff at startup */
#define VIDEO_UNKNOWN76         m_video[0x76/2] /* $01ff at startup */
#define VIDEO_UNKNOWN78         m_video[0x78/2] /* $01ff at startup */
#define VIDEO_UNKNOWN7a         m_video[0x7a/2] /* $01ff at startup */
#define VIDEO_UNKNOWN7c         m_video[0x7c/2] /* $0820 at startup */
#define VIDEO_UNKNOWN7e         m_video[0x7e/2] /* $0100 at startup */

#define VIDEO_STARTSTEP         m_video[0x80/2] /* drivedge only? */
#define VIDEO_LEFTSTEPLO        m_video[0x82/2] /* drivedge only? */
#define VIDEO_LEFTSTEPHI        m_video[0x84/2] /* drivedge only? */
#define VIDEO_RIGHTSTEPLO       m_video[0x86/2] /* drivedge only? */
#define VIDEO_RIGHTSTEPHI       m_video[0x88/2] /* drivedge only? */

#define VIDEOINT_SCANLINE       0x0004
#define VIDEOINT_BLITTER        0x0040

#define XFERFLAG_TRANSPARENT    0x0001
#define XFERFLAG_XFLIP          0x0002
#define XFERFLAG_YFLIP          0x0004
#define XFERFLAG_DSTXSCALE      0x0008
#define XFERFLAG_DYDXSIGN       0x0010
#define XFERFLAG_DXDYSIGN       0x0020
#define XFERFLAG_UNKNOWN8       0x0100
#define XFERFLAG_CLIP           0x0400
#define XFERFLAG_UNKNOWN15      0x8000

#define XFERFLAG_KNOWNFLAGS     (XFERFLAG_TRANSPARENT | XFERFLAG_XFLIP | XFERFLAG_YFLIP | XFERFLAG_DSTXSCALE | XFERFLAG_DYDXSIGN | XFERFLAG_DXDYSIGN | XFERFLAG_CLIP)

#define VRAM_WIDTH              512







/*************************************
 *
 *  Macros and inlines
 *
 *************************************/

#define ADJUSTED_HEIGHT(x) ((((x) >> 1) & 0x100) | ((x) & 0xff))

inline offs_t itech32_state::compute_safe_address(int x, int y)
{
	return ((y & m_vram_ymask) * 512) + (x & m_vram_xmask);
}

inline void itech32_state::disable_clipping()
{
	m_clip_save = m_clip_rect;
	m_clip_rect.set(0, 0xfff, 0, 0xfff);
	m_scaled_clip_rect.set(0, 0xfff << 8, 0, 0xfff << 8);
}

inline void itech32_state::enable_clipping()
{
	m_clip_rect = m_clip_save;

	m_scaled_clip_rect.min_x = m_clip_rect.min_x << 8;
	m_scaled_clip_rect.max_x = m_clip_rect.max_x << 8;
	m_scaled_clip_rect.min_y = m_clip_rect.min_y << 8;
	m_scaled_clip_rect.max_y = m_clip_rect.max_y << 8;
}



/*************************************
 *
 *  Video start
 *
 *************************************/

void itech32_state::video_start()
{
	int i;

	/* allocate memory */
	m_videoram = auto_alloc_array(machine(), UINT16, VRAM_WIDTH * (m_vram_height + 16) * 2);
	memset(m_videoram, 0xff, VRAM_WIDTH * (m_vram_height + 16) * 2 * 2);

	/* videoplane[0] is the foreground; videoplane[1] is the background */
	m_videoplane[0] = &m_videoram[0 * VRAM_WIDTH * (m_vram_height + 16) + 8 * VRAM_WIDTH];
	m_videoplane[1] = &m_videoram[1 * VRAM_WIDTH * (m_vram_height + 16) + 8 * VRAM_WIDTH];

	/* set the masks */
	m_vram_mask = VRAM_WIDTH * m_vram_height - 1;
	m_vram_xmask = VRAM_WIDTH - 1;
	m_vram_ymask = m_vram_height - 1;

	/* clear the planes initially */
	for (i = 0; i < VRAM_WIDTH * m_vram_height; i++)
		m_videoplane[0][i] = m_videoplane[1][i] = 0xff;

	/* fetch the GROM base */
	m_grom_base = memregion("gfx1")->base();
	m_grom_size = memregion("gfx1")->bytes();
	m_grom_bank = 0;
	m_grom_bank_mask = m_grom_size >> 24;
	if (m_grom_bank_mask == 2)
		m_grom_bank_mask = 3;

	/* reset statics */
	memset(m_video, 0, 0x80);

	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(itech32_state::scanline_interrupt),this));
	m_enable_latch[0] = 1;
	m_enable_latch[1] = (m_planes > 1) ? 1 : 0;
}



/*************************************
 *
 *  Latches
 *
 *************************************/

WRITE16_MEMBER(itech32_state::timekill_colora_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_enable_latch[0] = (~data >> 5) & 1;
		m_enable_latch[1] = (~data >> 7) & 1;
		m_color_latch[0] = (data & 0x0f) << 8;
	}
}


WRITE16_MEMBER(itech32_state::timekill_colorbc_w)
{
	if (ACCESSING_BITS_0_7)
		m_color_latch[1] = ((data & 0xf0) << 4) | 0x1000;
}


WRITE16_MEMBER(itech32_state::timekill_intensity_w)
{
	if (ACCESSING_BITS_0_7)
	{
		double intensity = (double)(data & 0xff) / (double)0x60;
		int i;
		for (i = 0; i < 8192; i++)
			m_palette->set_pen_contrast(i, intensity);
	}
}


WRITE16_MEMBER(itech32_state::bloodstm_color1_w)
{
	if (ACCESSING_BITS_0_7)
		m_color_latch[0] = (data & 0x7f) << 8;
}


WRITE16_MEMBER(itech32_state::bloodstm_color2_w)
{
	if (ACCESSING_BITS_0_7)
		m_color_latch[1] = (data & 0x7f) << 8;
}


WRITE16_MEMBER(itech32_state::bloodstm_plane_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_enable_latch[0] = (~data >> 1) & 1;
		m_enable_latch[1] = (~data >> 2) & 1;
	}
}


WRITE32_MEMBER(itech32_state::drivedge_color0_w)
{
	if (ACCESSING_BITS_16_23)
		m_color_latch[0] = ((data >> 16) & 0x7f) << 8;
}


WRITE32_MEMBER(itech32_state::itech020_color1_w)
{
	if (ACCESSING_BITS_0_7)
		m_color_latch[1] = (data & 0x7f) << 8;
}


WRITE32_MEMBER(itech32_state::itech020_color2_w)
{
	if (ACCESSING_BITS_0_7)
		m_color_latch[0] = (data & 0x7f) << 8;
}


WRITE32_MEMBER(itech32_state::itech020_plane_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_enable_latch[0] = (~data >> 9) & 1;
		m_enable_latch[1] = (~data >> 10) & 1;
		m_grom_bank = ((data >> 14) & m_grom_bank_mask) << 24;
	}
}



/*************************************
 *
 *  Palette I/O
 *
 *************************************/

WRITE16_MEMBER(itech32_state::bloodstm_paletteram_w)
{
	/* in test mode, the LSB is used; in game mode, the MSB is used */
	if (!ACCESSING_BITS_0_7 && (offset & 1))
	{
		data >>= 8;
		mem_mask >>= 8;
	}

	m_palette->write(space, offset, data, mem_mask);
}



/*************************************
 *
 *  Debugging
 *
 *************************************/

void itech32_state::logblit(const char *tag)
{
	if (!machine().input().code_pressed(KEYCODE_L))
		return;
	if (m_is_drivedge && VIDEO_TRANSFER_FLAGS == 0x5490)
	{
		/* polygon drawing */
		logerror("%s: e=%d%d f=%04x s=(%03x-%03x,%03x) w=%03x h=%03x b=%02x%04x c=%02x%02x ss=%04x,%04x ds=%04x,%04x ls=%04x%04x rs=%04x%04x u80=%04x", tag,
			m_enable_latch[0], m_enable_latch[1],
			VIDEO_TRANSFER_FLAGS,
			VIDEO_TRANSFER_X, VIDEO_RIGHTCLIP, VIDEO_TRANSFER_Y, VIDEO_TRANSFER_WIDTH, VIDEO_TRANSFER_HEIGHT,
			VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
			m_color_latch[0] >> 8, m_color_latch[1] >> 8,
			VIDEO_SRC_XSTEP, VIDEO_SRC_YSTEP,
			VIDEO_DST_XSTEP, VIDEO_DST_YSTEP,
			VIDEO_LEFTSTEPHI, VIDEO_LEFTSTEPLO, VIDEO_RIGHTSTEPHI, VIDEO_RIGHTSTEPLO,
			VIDEO_STARTSTEP);
	}

	else if (m_video[0x16/2] == 0x100 && m_video[0x18/2] == 0x100 &&
		m_video[0x1a/2] == 0x000 && m_video[0x1c/2] == 0x100 &&
		m_video[0x1e/2] == 0x000 && m_video[0x20/2] == 0x000)
	{
		logerror("%s: e=%d%d f=%04x c=%02x%02x %02x%04x -> (%03x,%03x) %3dx%3dc=(%03x,%03x)-(%03x,%03x)", tag,
				m_enable_latch[0], m_enable_latch[1],
				VIDEO_TRANSFER_FLAGS,
				m_color_latch[0] >> 8, m_color_latch[1] >> 8,
				VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
				VIDEO_TRANSFER_X, VIDEO_TRANSFER_Y,
				VIDEO_TRANSFER_WIDTH, ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT),
				VIDEO_LEFTCLIP, VIDEO_TOPCLIP, VIDEO_RIGHTCLIP, VIDEO_BOTTOMCLIP);
	}
	else
	{
		logerror("%s: e=%d%d f=%04x c=%02x%02x %02x%04x -> (%03x,%03x) %3dx%3d c=(%03x,%03x)-(%03x,%03x) s=%04x %04x %04x %04x %04x %04x", tag,
				m_enable_latch[0], m_enable_latch[1],
				VIDEO_TRANSFER_FLAGS,
				m_color_latch[0] >> 8, m_color_latch[1] >> 8,
				VIDEO_TRANSFER_ADDRHI, VIDEO_TRANSFER_ADDRLO,
				VIDEO_TRANSFER_X, VIDEO_TRANSFER_Y,
				VIDEO_TRANSFER_WIDTH, ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT),
				VIDEO_LEFTCLIP, VIDEO_TOPCLIP, VIDEO_RIGHTCLIP, VIDEO_BOTTOMCLIP,
				m_video[0x16/2], m_video[0x18/2], m_video[0x1a/2],
				m_video[0x1c/2], m_video[0x1e/2], m_video[0x20/2]);
	}
	if (m_is_drivedge) logerror(" v0=%08x v1=%08x v2=%08x v3=%08x", m_drivedge_zbuf_control[0], m_drivedge_zbuf_control[1], m_drivedge_zbuf_control[2], m_drivedge_zbuf_control[3]);
	logerror("\n");
}



/*************************************
 *
 *  Video interrupts
 *
 *************************************/

void itech32_state::update_interrupts(int fast)
{
	int scanline_state = 0, blitter_state = 0;

	if (VIDEO_INTSTATE & VIDEO_INTENABLE & VIDEOINT_SCANLINE)
		scanline_state = 1;
	if (VIDEO_INTSTATE & VIDEO_INTENABLE & VIDEOINT_BLITTER)
		blitter_state = 1;

	itech32_update_interrupts(-1, blitter_state, scanline_state);
}


TIMER_CALLBACK_MEMBER(itech32_state::scanline_interrupt)
{
	/* set timer for next frame */
	m_scanline_timer->adjust(m_screen->time_until_pos(VIDEO_INTSCANLINE));

	/* set the interrupt bit in the status reg */
	logerror("-------------- (DISPLAY INT @ %d) ----------------\n", m_screen->vpos());
	VIDEO_INTSTATE |= VIDEOINT_SCANLINE;

	/* update the interrupt state */
	update_interrupts(0);
}



/*************************************
 *
 *  Uncompressed blitter functions
 *
 *************************************/

void itech32_state::draw_raw(UINT16 *base, UINT16 color)
{
	UINT8 *src = &m_grom_base[(m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % m_grom_size];
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
			if (sy >= m_scaled_clip_rect.min_y && sy < m_scaled_clip_rect.max_y)
			{
				UINT32 dstoffs;

				/* direction matters here */
				sx = startx;
				if (xdststep > 0)
				{
					/* skip left pixels */
					for (x = 0; x < width && sx < m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep) ;

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					for ( ; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
							base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
					}
				}
				else
				{
					/* skip right pixels */
					for (x = 0; x < width && sx >= m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep) ;

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					for ( ; x < width && sx >= m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
							base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
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
			for (x = 0; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
				if (m_scaled_clip_rect.contains(sx, ty))
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


void itech32_state::draw_raw_drivedge(UINT16 *base, UINT16 *zbase, UINT16 color)
{
	UINT8 *src = &m_grom_base[(m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH << 8;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT) << 8;
	int xsrcstep = VIDEO_SRC_XSTEP;
	int ysrcstep = VIDEO_SRC_YSTEP;
	int sx, sy = ((VIDEO_TRANSFER_Y & 0xfff) << 8) + 0x80;
	int startx = ((VIDEO_TRANSFER_X & 0xfff) << 8) + 0x80;
	int xdststep = 0x100;
	int ydststep = VIDEO_DST_YSTEP;
	INT32 z0 = m_drivedge_zbuf_control[2] & 0x7ff00;
	INT32 zmatch = (m_drivedge_zbuf_control[2] & 0x1f) << 11;
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
			if (sy >= m_scaled_clip_rect.min_y && sy < m_scaled_clip_rect.max_y)
			{
				UINT32 dstoffs, zbufoffs;
				INT32 z = z0;

				/* direction matters here */
				sx = startx;
				if (xdststep > 0)
				{
					/* skip left pixels */
					for (x = 0; x < width && sx < m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						z += (INT32)m_drivedge_zbuf_control[0];

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);
					zbufoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					if (m_drivedge_zbuf_control[3] & 0x8000)
					{
						for ( ; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen)
							{
								base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)m_drivedge_zbuf_control[0];
						}
					}
					else if (m_drivedge_zbuf_control[3] & 0x4000)
					{
						for ( ; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && zmatch == (zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] & (0x1f << 11)))
								base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
							z += (INT32)m_drivedge_zbuf_control[0];
						}
					}
					else
					{
						for ( ; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && ((z >> 8) <= (zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] & 0x7ff)))
							{
								base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)m_drivedge_zbuf_control[0];
						}
					}
				}
				else
				{
					/* skip right pixels */
					for (x = 0; x < width && sx >= m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep)
						z += (INT32)m_drivedge_zbuf_control[0];

					/* compute the address */
					dstoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);
					zbufoffs = compute_safe_address(sx >> 8, sy >> 8) - (sx >> 8);

					/* render middle pixels */
					if (m_drivedge_zbuf_control[3] & 0x8000)
					{
						for ( ; x < width && sx >= m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen)
							{
								base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)m_drivedge_zbuf_control[0];
						}
					}
					else if (m_drivedge_zbuf_control[3] & 0x4000)
					{
						for ( ; x < width && sx >= m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && zmatch == (zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] & (0x1f << 11)))
								base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
							z += (INT32)m_drivedge_zbuf_control[0];
						}
					}
					else
					{
						for ( ; x < width && sx >= m_scaled_clip_rect.min_x; x += xsrcstep, sx += xdststep)
						{
							int pixel = rowsrc[x >> 8];
							if (pixel != transparent_pen && ((z >> 8) <= (zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] & 0x7ff)))
							{
								base[(dstoffs + (sx >> 8)) & m_vram_mask] = pixel | color;
								zbase[(zbufoffs + (sx >> 8)) & m_vram_mask] = (z >> 8) | zmatch;
							}
							z += (INT32)m_drivedge_zbuf_control[0];
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
			if (m_drivedge_zbuf_control[3] & 0x8000)
			{
				for (x = 0; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (m_scaled_clip_rect.contains(sx, ty))
					{
						int pixel = rowsrc[x >> 8];
						if (pixel != transparent_pen)
						{
							base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
							zbase[compute_safe_address(sx >> 8, ty >> 8)] = (z >> 8) | zmatch;
						}
						z += (INT32)m_drivedge_zbuf_control[0];
					}
			}
			else if (m_drivedge_zbuf_control[3] & 0x4000)
			{
				for (x = 0; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (m_scaled_clip_rect.contains(sx, ty))
					{
						int pixel = rowsrc[x >> 8];
						UINT16 *zbuf = &zbase[compute_safe_address(sx >> 8, ty >> 8)];
						if (pixel != transparent_pen && zmatch == (*zbuf & (0x1f << 11)))
						{
							base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
							*zbuf = (z >> 8) | zmatch;
						}
						z += (INT32)m_drivedge_zbuf_control[0];
					}
			}
			else
			{
				for (x = 0; x < width && sx < m_scaled_clip_rect.max_x; x += xsrcstep, sx += xdststep, ty += ystep)
					if (m_scaled_clip_rect.contains(sx, ty))
					{
						int pixel = rowsrc[x >> 8];
						UINT16 *zbuf = &zbase[compute_safe_address(sx >> 8, ty >> 8)];
						if (pixel != transparent_pen && ((z >> 8) <= (*zbuf & 0x7ff)))
						{
							base[compute_safe_address(sx >> 8, ty >> 8)] = pixel | color;
							*zbuf = (z >> 8) | zmatch;
						}
						z += (INT32)m_drivedge_zbuf_control[0];
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
			m_scaled_clip_rect.max_x += (INT32)((VIDEO_RIGHTSTEPHI << 16) | VIDEO_RIGHTSTEPLO);
			srcdelta += (INT16)VIDEO_STARTSTEP;
		}
		z0 += (INT32)m_drivedge_zbuf_control[1];
	}

	/* restore cliprects */
	if (!(VIDEO_TRANSFER_FLAGS & XFERFLAG_CLIP))
		enable_clipping();

	/* reflect the final values into registers */
	VIDEO_TRANSFER_X = (VIDEO_TRANSFER_X & ~0xfff) | (startx >> 8);
	VIDEO_RIGHTCLIP = (VIDEO_RIGHTCLIP & ~0xfff) | (m_scaled_clip_rect.max_x >> 8);
	VIDEO_TRANSFER_Y = (VIDEO_TRANSFER_Y & ~0xfff) | ((VIDEO_TRANSFER_Y + (y >> 8)) & 0xfff);
	VIDEO_TRANSFER_ADDRLO += srcdelta >> 8;

	m_drivedge_zbuf_control[2] = (m_drivedge_zbuf_control[2] & ~0x7ff00) | (z0 & 0x7ff00);
}



/*************************************
 *
 *  Compressed blitter macros
 *
 *************************************/

#define GET_NEXT_RUN(xleft, count, innercount, src) \
do {                                                \
	/* load next RLE chunk if needed */             \
	if (!count)                                     \
	{                                               \
		count = *src++;                             \
		val = (count & 0x80) ? -1 : *src++;         \
		count &= 0x7f;                              \
	}                                               \
													\
	/* determine how much to bite off */            \
	innercount = (xleft > count) ? count : xleft;   \
	count -= innercount;                            \
	xleft -= innercount;                            \
} while (0)


#define SKIP_RLE(skip, xleft, count, innercount, src)\
do {                                                \
	/* scan RLE until done */                       \
	for (xleft = skip; xleft > 0; )                 \
	{                                               \
		/* load next RLE chunk if needed */         \
		GET_NEXT_RUN(xleft, count, innercount, src);\
													\
		/* skip past the data */                    \
		if (val == -1) src += innercount;           \
	}                                               \
} while (0)



/*************************************
 *
 *  Fast compressed blitter functions
 *
 *************************************/

inline void itech32_state::draw_rle_fast(UINT16 *base, UINT16 color)
{
	UINT8 *src = &m_grom_base[(m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int ydststep = VIDEO_DST_YSTEP;
	int lclip, rclip;

	/* determine clipping */
	lclip = m_clip_rect.min_x - sx;
	if (lclip < 0) lclip = 0;
	rclip = sx + width - m_clip_rect.max_x;
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
		if (sy < m_scaled_clip_rect.min_y || sy >= m_scaled_clip_rect.max_y)
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
						base[dstoffs & m_vram_mask] = color | pixel;
					dstoffs++;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				while (innercount--)
					base[dstoffs++ & m_vram_mask] = val;
			}

			/* run of transparent repeats */
			else
				dstoffs += innercount;
		}

		/* right clip */
		SKIP_RLE(rclip, xleft, count, innercount, src);
	}
}


inline void itech32_state::draw_rle_fast_xflip(UINT16 *base, UINT16 color)
{
	UINT8 *src = &m_grom_base[(m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % m_grom_size];
	int transparent_pen = (VIDEO_TRANSFER_FLAGS & XFERFLAG_TRANSPARENT) ? 0xff : -1;
	int width = VIDEO_TRANSFER_WIDTH;
	int height = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
	int sx = VIDEO_TRANSFER_X & 0xfff;
	int sy = (VIDEO_TRANSFER_Y & 0xfff) << 8;
	int xleft, y, count = 0, val = 0, innercount;
	int ydststep = VIDEO_DST_YSTEP;
	int lclip, rclip;

	/* determine clipping */
	lclip = sx - m_clip_rect.max_x;
	if (lclip < 0) lclip = 0;
	rclip = m_clip_rect.min_x - (sx - width);
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
		if (sy < m_scaled_clip_rect.min_y || sy >= m_scaled_clip_rect.max_y)
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
						base[dstoffs & m_vram_mask] = color | pixel;
					dstoffs--;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				while (innercount--)
					base[dstoffs-- & m_vram_mask] = val;
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

inline void itech32_state::draw_rle_slow(UINT16 *base, UINT16 color)
{
	UINT8 *src = &m_grom_base[(m_grom_bank | ((VIDEO_TRANSFER_ADDRHI & 0xff) << 16) | VIDEO_TRANSFER_ADDRLO) % m_grom_size];
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
		if (sy < m_scaled_clip_rect.min_y || sy >= m_scaled_clip_rect.max_y)
		{
			SKIP_RLE(width, xleft, count, innercount, src);
			continue;
		}

		/* compute the address */
		sx = startx;
		dstoffs = compute_safe_address(m_clip_rect.min_x, sy >> 8) - m_clip_rect.min_x;

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
						if (sx >= m_scaled_clip_rect.min_x && sx < m_scaled_clip_rect.max_x)
							base[(dstoffs + (sx >> 8)) & m_vram_mask] = color | pixel;
				}

			/* run of non-transparent repeats */
			else if (val != transparent_pen)
			{
				val |= color;
				for ( ; innercount--; sx += xdststep)
					if (sx >= m_scaled_clip_rect.min_x && sx < m_scaled_clip_rect.max_x)
						base[(dstoffs + (sx >> 8)) & m_vram_mask] = val;
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



void itech32_state::draw_rle(UINT16 *base, UINT16 color)
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

void itech32_state::shiftreg_clear(UINT16 *base, UINT16 *zbase)
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
			UINT16 zval = ((m_drivedge_zbuf_control[2] >> 8) & 0x7ff) | ((m_drivedge_zbuf_control[2] & 0x1f) << 11);
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

void itech32_state::handle_video_command()
{
	/* only 6 known commands */
	switch (VIDEO_COMMAND)
	{
		/* command 1: blit raw data */
		case 1:
			g_profiler.start(PROFILER_USER1);
			if (BLIT_LOGGING) logblit("Blit Raw");

			if (m_is_drivedge)
			{
				if (m_enable_latch[0]) draw_raw_drivedge(m_videoplane[0], m_videoplane[1], m_color_latch[0]);
			}
			else
			{
				if (m_enable_latch[0]) draw_raw(m_videoplane[0], m_color_latch[0]);
				if (m_enable_latch[1]) draw_raw(m_videoplane[1], m_color_latch[1]);
			}

			g_profiler.stop();
			break;

		/* command 2: blit RLE-compressed data */
		case 2:
			g_profiler.start(PROFILER_USER2);
			if (BLIT_LOGGING) logblit("Blit RLE");

			if (m_enable_latch[0]) draw_rle(m_videoplane[0], m_color_latch[0]);
			if (m_enable_latch[1]) draw_rle(m_videoplane[1], m_color_latch[1]);

			g_profiler.stop();
			break;

		/* command 3: set up raw data transfer */
		case 3:
			if (BLIT_LOGGING) logblit("Raw Xfer");
			m_xfer_xcount = VIDEO_TRANSFER_WIDTH;
			m_xfer_ycount = ADJUSTED_HEIGHT(VIDEO_TRANSFER_HEIGHT);
			m_xfer_xcur = VIDEO_TRANSFER_X & 0xfff;
			m_xfer_ycur = VIDEO_TRANSFER_Y & 0xfff;
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
			if (BLIT_LOGGING) logblit("ShiftReg");

			if (m_is_drivedge)
			{
				if (m_enable_latch[0]) shiftreg_clear(m_videoplane[0], m_videoplane[1]);
			}
			else
			{
				if (m_enable_latch[0]) shiftreg_clear(m_videoplane[0], nullptr);
				if (m_enable_latch[1]) shiftreg_clear(m_videoplane[1], nullptr);
			}

			g_profiler.stop();
			break;

		default:
			if (BLIT_LOGGING) logerror("Unknown blit command %d\n", VIDEO_COMMAND);
			break;
	}

	/* tell the processor we're done */
	VIDEO_INTSTATE |= VIDEOINT_BLITTER;
	update_interrupts(1);
}



/*************************************
 *
 *  Video I/O
 *
 *************************************/

WRITE16_MEMBER(itech32_state::itech32_video_w)
{
	rectangle visarea;

	int old = m_video[offset];
	COMBINE_DATA(&m_video[offset]);

	switch (offset)
	{
		case 0x02/2:    /* VIDEO_INTACK */
			VIDEO_INTSTATE = old & ~data;
			update_interrupts(1);
			break;

		case 0x04/2:    /* VIDEO_TRANSFER */
			if (VIDEO_COMMAND == 3 && m_xfer_ycount)
			{
				offs_t addr = compute_safe_address(m_xfer_xcur, m_xfer_ycur);
				if (m_enable_latch[0])
				{
					VIDEO_TRANSFER = m_videoplane[0][addr];
					m_videoplane[0][addr] = (data & 0xff) | m_color_latch[0];
				}
				if (m_enable_latch[1])
				{
					VIDEO_TRANSFER = m_videoplane[1][addr];
					m_videoplane[1][addr] = (data & 0xff) | m_color_latch[1];
				}
				if (--m_xfer_xcount)
					m_xfer_xcur++;
				else if (--m_xfer_ycount)
					m_xfer_xcur = VIDEO_TRANSFER_X, m_xfer_xcount = VIDEO_TRANSFER_WIDTH, m_xfer_ycur++;
			}
			break;

		case 0x08/2:    /* VIDEO_COMMAND */
			handle_video_command();
			break;

		case 0x0a/2:    /* VIDEO_INTENABLE */
			update_interrupts(1);
			break;

		case 0x24/2:    /* VIDEO_LEFTCLIP */
			m_clip_rect.min_x = VIDEO_LEFTCLIP;
			m_scaled_clip_rect.min_x = VIDEO_LEFTCLIP << 8;
			break;

		case 0x26/2:    /* VIDEO_RIGHTCLIP */
			m_clip_rect.max_x = VIDEO_RIGHTCLIP;
			m_scaled_clip_rect.max_x = VIDEO_RIGHTCLIP << 8;
			break;

		case 0x28/2:    /* VIDEO_TOPCLIP */
			m_clip_rect.min_y = VIDEO_TOPCLIP;
			m_scaled_clip_rect.min_y = VIDEO_TOPCLIP << 8;
			break;

		case 0x2a/2:    /* VIDEO_BOTTOMCLIP */
			m_clip_rect.max_y = VIDEO_BOTTOMCLIP;
			m_scaled_clip_rect.max_y = VIDEO_BOTTOMCLIP << 8;
			break;

		case 0x2c/2:    /* VIDEO_INTSCANLINE */
			m_scanline_timer->adjust(m_screen->time_until_pos(VIDEO_INTSCANLINE));
			break;

		case 0x32/2:    /* VIDEO_VTOTAL */
		case 0x36/2:    /* VIDEO_VBLANK_START */
		case 0x38/2:    /* VIDEO_VBLANK_END */
		case 0x3a/2:    /* VIDEO_HTOTAL */
		case 0x3e/2:    /* VIDEO_HBLANK_START */
		case 0x40/2:    /* VIDEO_HBLANK_END */
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
				m_screen->configure(VIDEO_HTOTAL, VIDEO_VTOTAL, visarea, HZ_TO_ATTOSECONDS(VIDEO_CLOCK) * VIDEO_HTOTAL * VIDEO_VTOTAL);
			}
			break;
	}
}


READ16_MEMBER(itech32_state::itech32_video_r)
{
	if (offset == 0)
	{
		return (m_video[offset] & ~0x08) | 0x04 | 0x01;
	}
	else if (offset == 3)
	{
		return 0xef;/*m_screen->vpos() - 1;*/
	}

	return m_video[offset];
}



/*************************************
 *
 *  Alternate video I/O
 *
 *************************************/

WRITE16_MEMBER(itech32_state::bloodstm_video_w)
{
	itech32_video_w(space, offset / 2, data, mem_mask);
}


READ16_MEMBER(itech32_state::bloodstm_video_r)
{
	return itech32_video_r(space, offset / 2, mem_mask);
}


WRITE32_MEMBER(itech32_state::itech020_video_w)
{
	if (ACCESSING_BITS_16_31)
		itech32_video_w(space, offset, data >> 16, mem_mask >> 16);
	else
		itech32_video_w(space, offset, data, mem_mask);
}


WRITE32_MEMBER(itech32_state::drivedge_zbuf_control_w)
{
	COMBINE_DATA(&m_drivedge_zbuf_control[offset]);
}


READ32_MEMBER(itech32_state::itech020_video_r)
{
	int result = itech32_video_r(space, offset, mem_mask);
	return (result << 16) | result;
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 itech32_state::screen_update_itech32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	/* loop over height */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *src1 = &m_videoplane[0][compute_safe_address(VIDEO_DISPLAY_XORIGIN1, VIDEO_DISPLAY_YORIGIN1 + y)];

		/* handle multi-plane case */
		if (m_planes > 1)
		{
			UINT16 *src2 = &m_videoplane[1][compute_safe_address(VIDEO_DISPLAY_XORIGIN2 + VIDEO_DISPLAY_XSCROLL2, VIDEO_DISPLAY_YORIGIN2 + VIDEO_DISPLAY_YSCROLL2 + y)];
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
			draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &scanline[cliprect.min_x], nullptr);
		}

		/* otherwise, draw directly from VRAM */
		else
			draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &src1[cliprect.min_x], nullptr);
	}
	return 0;
}
