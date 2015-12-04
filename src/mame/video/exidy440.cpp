// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Exidy 440 video system

***************************************************************************/

#include "emu.h"
#include "includes/exidy440.h"


#define PIXEL_CLOCK         (EXIDY440_MASTER_CLOCK / 2)
#define HTOTAL              (0x1a0)
#define HBEND               (0x000)
#define HBSTART             (0x140)
#define HSEND               (0x178)
#define HSSTART             (0x160)
#define VTOTAL              (0x104)
#define VBEND               (0x000)
#define VBSTART             (0x0f0)
#define VSEND               (0x0f8)
#define VSSTART             (0x0f0)

/* Top Secret has a larger VBLANK area */
#define TOPSECEX_VBSTART    (0x0ec)

#define SPRITE_COUNT        (0x28)

/*************************************
 *
 *  Initialize the video system
 *
 *************************************/

VIDEO_START_MEMBER(exidy440_state,exidy440)
{
	/* reset the system */
	m_firq_enable = 0;
	m_firq_select = 0;
	m_palettebank_io = 0;
	m_palettebank_vis = 0;
	m_firq_vblank = 0;
	m_firq_beam = 0;

	/* allocate a buffer for VRAM */
	m_local_videoram = auto_alloc_array(machine(), UINT8, 256 * 256 * 2);
	memset(m_local_videoram, 0, 256 * 256 * 2);

	/* allocate a buffer for palette RAM */
	m_local_paletteram = auto_alloc_array(machine(), UINT8, 512 * 2);
	memset(m_local_paletteram, 0, 512 * 2);
}


VIDEO_START_MEMBER(exidy440_state,topsecex)
{
	VIDEO_START_CALL_MEMBER(exidy440);

	*m_topsecex_yscroll = 0;
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

READ8_MEMBER(exidy440_state::exidy440_videoram_r)
{
	UINT8 *base = &m_local_videoram[(*m_scanline * 256 + offset) * 2];

	/* combine the two pixel values into one byte */
	return (base[0] << 4) | base[1];
}


WRITE8_MEMBER(exidy440_state::exidy440_videoram_w)
{
	UINT8 *base = &m_local_videoram[(*m_scanline * 256 + offset) * 2];

	/* expand the two pixel values into two bytes */
	base[0] = (data >> 4) & 15;
	base[1] = data & 15;
}



/*************************************
 *
 *  Palette RAM read/write
 *
 *************************************/

READ8_MEMBER(exidy440_state::exidy440_paletteram_r)
{
	return m_local_paletteram[m_palettebank_io * 512 + offset];
}


WRITE8_MEMBER(exidy440_state::exidy440_paletteram_w)
{
	/* update palette ram in the I/O bank */
	m_local_paletteram[m_palettebank_io * 512 + offset] = data;

	/* if we're modifying the active palette, change the color immediately */
	if (m_palettebank_io == m_palettebank_vis)
	{
		int word;

		/* combine two bytes into a word */
		offset = m_palettebank_vis * 512 + (offset & 0x1fe);
		word = (m_local_paletteram[offset] << 8) + m_local_paletteram[offset + 1];

		/* extract the 5-5-5 RGB colors */
		m_palette->set_pen_color(offset / 2, pal5bit(word >> 10), pal5bit(word >> 5), pal5bit(word >> 0));
	}
}



/*************************************
 *
 *  Horizontal/vertical positions
 *
 *************************************/

READ8_MEMBER(exidy440_state::exidy440_horizontal_pos_r)
{
	/* clear the FIRQ on a read here */
	m_firq_beam = 0;
	exidy440_update_firq();

	/* according to the schems, this value is only latched on an FIRQ
	 * caused by collision or beam */
	return m_latched_x;
}


READ8_MEMBER(exidy440_state::exidy440_vertical_pos_r)
{
	int result;

	/* according to the schems, this value is latched on any FIRQ
	 * caused by collision or beam, ORed together with CHRCLK,
	 * which probably goes off once per scanline; for now, we just
	 * always return the current scanline */
	result = m_screen->vpos();
	return (result < 255) ? result : 255;
}



/*************************************
 *
 *  Sprite RAM handler
 *
 *************************************/

WRITE8_MEMBER(exidy440_state::exidy440_spriteram_w)
{
	m_screen->update_partial(m_screen->vpos());
	m_spriteram[offset] = data;
}



/*************************************
 *
 *  Interrupt and I/O control regs
 *
 *************************************/

WRITE8_MEMBER(exidy440_state::exidy440_control_w)
{
	int oldvis = m_palettebank_vis;

	/* extract the various bits */
	exidy440_bank_select(data >> 4);
	m_firq_enable = (data >> 3) & 1;
	m_firq_select = (data >> 2) & 1;
	m_palettebank_io = (data >> 1) & 1;
	m_palettebank_vis = data & 1;

	/* update the FIRQ in case we enabled something */
	exidy440_update_firq();

	/* if we're swapping palettes, change all the colors */
	if (oldvis != m_palettebank_vis)
	{
		int i;

		/* pick colors from the visible bank */
		offset = m_palettebank_vis * 512;
		for (i = 0; i < 256; i++, offset += 2)
		{
			/* extract a word and the 5-5-5 RGB components */
			int word = (m_local_paletteram[offset] << 8) + m_local_paletteram[offset + 1];
			m_palette->set_pen_color(i, pal5bit(word >> 10), pal5bit(word >> 5), pal5bit(word >> 0));
		}
	}
}


WRITE8_MEMBER(exidy440_state::exidy440_interrupt_clear_w)
{
	/* clear the VBLANK FIRQ on a write here */
	m_firq_vblank = 0;
	exidy440_update_firq();
}



/*************************************
 *
 *  Interrupt generators
 *
 *************************************/

void exidy440_state::exidy440_update_firq()
{
	if (m_firq_vblank || (m_firq_enable && m_firq_beam))
		m_maincpu->set_input_line(1, ASSERT_LINE);
	else
		m_maincpu->set_input_line(1, CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(exidy440_state::exidy440_vblank_interrupt)
{
	/* set the FIRQ line on a VBLANK */
	m_firq_vblank = 1;
	exidy440_update_firq();
}



/*************************************
 *
 *  IRQ callback handlers
 *
 *************************************/

TIMER_CALLBACK_MEMBER(exidy440_state::beam_firq_callback)
{
	/* generate the interrupt, if we're selected */
	if (m_firq_select && m_firq_enable)
	{
		m_firq_beam = 1;
		exidy440_update_firq();
	}

	/* round the x value to the nearest byte */
	param = (param + 1) / 2;

	/* latch the x value; this convolution comes from the read routine */
	m_latched_x = (param + 3) ^ 2;
}


TIMER_CALLBACK_MEMBER(exidy440_state::collide_firq_callback)
{
	/* generate the interrupt, if we're selected */
	if (!m_firq_select && m_firq_enable)
	{
		m_firq_beam = 1;
		exidy440_update_firq();
	}

	/* round the x value to the nearest byte */
	param = (param + 1) / 2;

	/* latch the x value; this convolution comes from the read routine */
	m_latched_x = (param + 3) ^ 2;
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void exidy440_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int scroll_offset, int check_collision)
{
	/* get a pointer to the palette to look for collision flags */
	UINT8 *palette = &m_local_paletteram[m_palettebank_vis * 512];
	int count = 0;

	/* draw the sprite images, checking for collisions along the way */
	UINT8 *sprite = m_spriteram + (SPRITE_COUNT - 1) * 4;

	for (int i = 0; i < SPRITE_COUNT; i++, sprite -= 4)
	{
		int image = (~sprite[3] & 0x3f);
		int xoffs = (~((sprite[1] << 8) | sprite[2]) & 0x1ff);
		int yoffs = (~sprite[0] & 0xff) + 1;
		int x, y, sy;
		UINT8 *src;

		/* skip if out of range */
		if (yoffs < cliprect.min_y || yoffs >= cliprect.max_y + 16)
			continue;

		/* get a pointer to the source image */
		src = &m_imageram[image * 128];

		/* account for large positive offsets meaning small negative values */
		if (xoffs >= 0x1ff - 16)
			xoffs -= 0x1ff;

		/* loop over y */
		sy = yoffs + scroll_offset;
		for (y = 0; y < 16; y++, yoffs--, sy--)
		{
			/* wrap at the top and bottom of the screen */
			if (sy >= VBSTART)
				sy -= (VBSTART - VBEND);
			else if (sy < VBEND)
				sy += (VBSTART - VBEND);

			/* stop if we get before the current scanline */
			if (yoffs < cliprect.min_y)
				break;

			/* only draw scanlines that are in this cliprect */
			if (yoffs <= cliprect.max_y)
			{
				UINT8 *old = &m_local_videoram[sy * 512 + xoffs];
				int currx = xoffs;

				/* loop over x */
				for (x = 0; x < 8; x++, old += 2)
				{
					int ipixel = *src++;
					int left = ipixel & 0xf0;
					int right = (ipixel << 4) & 0xf0;
					int pen;

					/* left pixel */
					if (left && currx >= HBEND && currx < HBSTART)
					{
						/* combine with the background */
						pen = left | old[0];
						bitmap.pix16(yoffs, currx) = pen;

						/* check the collisions bit */
						if (check_collision && (palette[2 * pen] & 0x80) && (count++ < 128))
							screen.machine().scheduler().timer_set(screen.time_until_pos(yoffs, currx), timer_expired_delegate(FUNC(exidy440_state::collide_firq_callback), this), currx);
					}
					currx++;

					/* right pixel */
					if (right && currx >= HBEND && currx < HBSTART)
					{
						/* combine with the background */
						pen = right | old[1];
						bitmap.pix16(yoffs, currx) = pen;

						/* check the collisions bit */
						if (check_collision && (palette[2 * pen] & 0x80) && (count++ < 128))
							screen.machine().scheduler().timer_set(screen.time_until_pos(yoffs, currx), timer_expired_delegate(FUNC(exidy440_state::collide_firq_callback), this), currx);
					}
					currx++;
				}
			}
			else
				src += 8;
		}
	}
}



/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

void exidy440_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,  int scroll_offset, int check_collision)
{
	/* draw any dirty scanlines from the VRAM directly */
	int sy = scroll_offset + cliprect.min_y;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++, sy++)
	{
		/* wrap at the bottom of the screen */
		if (sy >= VBSTART)
			sy -= (VBSTART - VBEND);

		/* draw line */
		draw_scanline8(bitmap, 0, y, (HBSTART - HBEND), &m_local_videoram[sy * 512], nullptr);
	}

	/* draw the sprites */
	draw_sprites(screen, bitmap, cliprect, scroll_offset, check_collision);
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

UINT32 exidy440_state::screen_update_exidy440(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* redraw the screen */
	update_screen(screen, bitmap, cliprect, 0, TRUE);

	/* generate an interrupt once/frame for the beam */
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		int i;

		int beamx = ((ioport("AN0")->read() & 0xff) * (HBSTART - HBEND)) >> 8;
		int beamy = ((ioport("AN1")->read() & 0xff) * (VBSTART - VBEND)) >> 8;

		/* The timing of this FIRQ is very important. The games look for an FIRQ
		    and then wait about 650 cycles, clear the old FIRQ, and wait a
		    very short period of time (~130 cycles) for another one to come in.
		    From this, it appears that they are expecting to get beams over
		    a 12 scanline period, and trying to pick roughly the middle one.
		    This is how it is implemented. */
		attotime increment = screen.scan_period();
		attotime time = screen.time_until_pos(beamy, beamx) - increment * 6;
		for (i = 0; i <= 12; i++)
		{
			machine().scheduler().timer_set(time, timer_expired_delegate(FUNC(exidy440_state::beam_firq_callback),this), beamx);
			time += increment;
		}
	}

	return 0;
}


UINT32 exidy440_state::screen_update_topsecex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* redraw the screen */
	update_screen(screen, bitmap, cliprect, *m_topsecex_yscroll, FALSE);

	return 0;
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( exidy440_video )
	MCFG_VIDEO_START_OVERRIDE(exidy440_state,exidy440)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(exidy440_state, screen_update_exidy440)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( topsecex_video )
	MCFG_VIDEO_START_OVERRIDE(exidy440_state,topsecex)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VIDEO_ATTRIBUTES(0)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, TOPSECEX_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(exidy440_state, screen_update_topsecex)
MACHINE_CONFIG_END
