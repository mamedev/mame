// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles
/***************************************************************************

    Amiga hardware

    Driver by: Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "amiga.h"

#define LOG_SPRITE_DMA (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


/*************************************
 *
 *  Tables
 *
 *************************************/

/* expand an 8-bit bit pattern into 16 bits, every other bit */
const uint16_t amiga_state::s_expand_byte[256] =
{
	0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
	0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
	0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
	0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
	0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
	0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
	0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
	0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
	0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
	0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
	0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
	0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
	0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
	0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
	0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
	0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,

	0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
	0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
	0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
	0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
	0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
	0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
	0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
	0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
	0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
	0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
	0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
	0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
	0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
	0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
	0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
	0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};

/*************************************
 *
 *  4-4-4 palette init
 *
 *************************************/

void amiga_state::amiga_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x1000; i++)
		palette.set_pen_color(i, pal4bit(i >> 8), pal4bit(i >> 4), pal4bit(i));
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void amiga_state::video_start_common()
{
	/* reset the genlock color */
	m_genlock_color = 0xffff;

	m_sprite_ctl_written = 0;

	m_screen->register_screen_bitmap(m_flickerfixer);
	m_screen->register_screen_bitmap(m_scanline_bitmap);
}

VIDEO_START_MEMBER( amiga_state, amiga )
{
	video_start_common();

	/* generate tables that produce the correct playfield color for dual playfield mode */
	m_separate_bitplanes[0].resize(64);
	m_separate_bitplanes[1].resize(64);
	for (int j = 0; j < 64; j++)
	{
		int pf1pix = ((j >> 0) & 1) | ((j >> 1) & 2) | ((j >> 2) & 4);
		int pf2pix = ((j >> 1) & 1) | ((j >> 2) & 2) | ((j >> 3) & 4);

		m_separate_bitplanes[0][j] = (pf1pix || !pf2pix) ? pf1pix : (pf2pix + 8);
		m_separate_bitplanes[1][j] = pf2pix ? (pf2pix + 8) : pf1pix;
	}
	// TODO: verify usage of values in the 64-255 range on real HW
	// (should black out pf1 if j & 0x40, pf2 if j & 0x80)
}



/*************************************
 *
 *  Beam position
 *
 *************************************/

// TODO: sync writes (VPOSW, VHPOSW), strobe beams (STR* class regs at 0x38-0x3e), ECS/AGA BEAMCON0
// A good chunk of copy protected games uses this timing as RNG seed,
// optionally syncing the beam to a known state (TBD find examples of this).
// In case of pbprel_a (AGA), it uses this to check if system has AGA equipped chipset.
// We may also need a "temporary" screen beam disable until a VBLANK occurs:
// for instance is dubious that beams are in a known state if a strobe happens ...
uint32_t amiga_state::amiga_gethvpos()
{
	uint32_t hvpos = (m_last_scanline << 8) | (m_screen->hpos() >> 2);
	uint32_t latchedpos = m_hvpos.read_safe(0);

	/* if there's no latched position, or if we are in the active display area */
	/* but before the latching point, return the live HV position */
	if (!BIT(CUSTOM_REG(REG_BPLCON0), 3) || latchedpos == 0 || (m_last_scanline >= 20 && hvpos < latchedpos))
		return hvpos;

	/* otherwise, return the latched position (cfr. lightgun input in alg.cpp, lightpen) */
	return latchedpos;
}



/*************************************
 *
 *  Genlock interaction
 *
 *************************************/

void amiga_state::set_genlock_color(uint16_t color)
{
	m_genlock_color = color;
}

/*************************************
 *
 *  External sprite controls
 *
 *************************************/

void amiga_state::sprite_dma_reset(int which)
{
	LOGMASKED(LOG_SPRITE_DMA, "sprite %d dma reset\n", which );
	m_sprite_dma_reload_mask |= 1 << which;
	m_sprite_dma_live_mask |= 1 << which;
}


void amiga_state::sprite_enable_comparitor(int which, int enable)
{
	LOGMASKED(LOG_SPRITE_DMA, "sprite %d comparitor %sable\n", which, enable ? "en" : "dis" );
	if (enable)
	{
		m_sprite_comparitor_enable_mask |= 1 << which;
		m_sprite_dma_live_mask &= ~(1 << which);
	}
	else
	{
		m_sprite_comparitor_enable_mask &= ~(1 << which);
		m_sprite_ctl_written |= (1 << which);
	}
}



/*************************************
 *
 *  Per-scanline sprite fetcher
 *
 *************************************/

void amiga_state::fetch_sprite_data(int scanline, int sprite)
{
	CUSTOM_REG(REG_SPR0DATA + 4 * sprite) = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
	CUSTOM_REG(REG_SPR0DATB + 4 * sprite) = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
	CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
	LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d fetch: data=%04X-%04X\n", scanline, sprite, CUSTOM_REG(REG_SPR0DATA + 4 * sprite), CUSTOM_REG(REG_SPR0DATB + 4 * sprite));
}

void amiga_state::update_sprite_dma(int scanline, int num)
{
	int dmaenable = (CUSTOM_REG(REG_DMACON) & (DMACON_SPREN | DMACON_DMAEN)) == (DMACON_SPREN | DMACON_DMAEN);
	int maxdma;

	/* channels are limited by DDFSTART */
	maxdma = (CUSTOM_REG(REG_DDFSTRT) - 0x14) / 4;
	if (maxdma > 8)
		maxdma = 8;

	if (num >= maxdma)
		return;

	int bitmask = 1 << num;
	int vstart, vstop;

	/* if we are == VSTOP, fetch new control words */
	if (dmaenable && (m_sprite_dma_live_mask & bitmask) && (m_sprite_dma_reload_mask & bitmask))
	{
		/* disable the sprite */
		m_sprite_comparitor_enable_mask &= ~bitmask;
		m_sprite_dma_reload_mask &= ~bitmask;

		/* fetch data into the control words */
		CUSTOM_REG(REG_SPR0POS + 4 * num) = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 0);
		CUSTOM_REG(REG_SPR0CTL + 4 * num) = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 2);
		CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) += 4;
		LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d fetch: pos=%04X ctl=%04X\n", scanline, num, CUSTOM_REG(REG_SPR0POS + 4 * num), CUSTOM_REG(REG_SPR0CTL + 4 * num));
	}

	/* compute vstart/vstop */
	vstart = (CUSTOM_REG(REG_SPR0POS + 4 * num) >> 8) | ((CUSTOM_REG(REG_SPR0CTL + 4 * num) << 6) & 0x100);
	vstop = (CUSTOM_REG(REG_SPR0CTL + 4 * num) >> 8) | ((CUSTOM_REG(REG_SPR0CTL + 4 * num) << 7) & 0x100);

	/* if we hit vstart, enable the comparitor */
	if (scanline == vstart)
	{
		m_sprite_comparitor_enable_mask |= 1 << num;
		LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d comparitor enable\n", scanline, num);
	}

	/* if we hit vstop, disable the comparitor and trigger a reload for the next scanline */
	if (scanline == vstop)
	{
		m_sprite_ctl_written &= ~bitmask;
		m_sprite_comparitor_enable_mask &= ~bitmask;
		m_sprite_dma_reload_mask |= 1 << num;
		CUSTOM_REG(REG_SPR0DATA + 4 * num) = 0;     /* just a guess */
		CUSTOM_REG(REG_SPR0DATB + 4 * num) = 0;
		LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d comparitor disable, prepare for reload\n", scanline, num);
	}

	/* fetch data if this sprite is enabled */
	if (dmaenable && (m_sprite_dma_live_mask & bitmask) && (m_sprite_comparitor_enable_mask & bitmask))
	{
		fetch_sprite_data(scanline, num);
	}
}



/*************************************
 *
 *  Per-pixel sprite computations
 *
 *************************************/

uint32_t amiga_state::interleave_sprite_data(uint16_t lobits, uint16_t hibits)
{
	return (s_expand_byte[lobits & 0xff] << 0) | (s_expand_byte[lobits >> 8] << 16) |
			(s_expand_byte[hibits & 0xff] << 1) | (s_expand_byte[hibits >> 8] << 17);
}


int amiga_state::get_sprite_pixel(int x)
{
	int pixels = 0;
	int num, pair;

	/* loop over sprite channels */
	for (num = 0; num < 8; num++)
		if (m_sprite_comparitor_enable_mask & (1 << num))
		{
			/* if we're not currently clocking, check against hstart */
			if (m_sprite_remain[num] == 0)
			{
				int hstart = ((CUSTOM_REG(REG_SPR0POS + 4 * num) & 0xff) << 1) | (CUSTOM_REG(REG_SPR0CTL + 4 * num) & 1);
				if (hstart == x)
				{
					m_sprite_remain[num] = 16;
					m_sprite_shiftreg[num] = interleave_sprite_data(CUSTOM_REG(REG_SPR0DATA + 4 * num), CUSTOM_REG(REG_SPR0DATB + 4 * num));
				}
			}

			/* clock the next pixel if we're doing it */
			if (m_sprite_remain[num] != 0)
			{
				m_sprite_remain[num]--;
				pixels |= (m_sprite_shiftreg[num] & 0xc0000000) >> (16 + 2 * (7 - num));
				m_sprite_shiftreg[num] <<= 2;
			}
		}

	/* if we have pixels, determine the actual color and get out */
	if (pixels)
	{
		static const uint16_t ormask[16] =
		{
			0x0000, 0x000c, 0x00c0, 0x00cc, 0x0c00, 0x0c0c, 0x0cc0, 0x0ccc,
			0xc000, 0xc00c, 0xc0c0, 0xc0cc, 0xcc00, 0xcc0c, 0xccc0, 0xcccc
		};
		static const uint16_t spritecollide[16] =
		{
			0x0000, 0x0000, 0x0000, 0x0200, 0x0000, 0x0400, 0x1000, 0x1600,
			0x0000, 0x0800, 0x2000, 0x2a00, 0x4000, 0x4c00, 0x7000, 0x7e00
		};
		int collide;
		const int esprm = 0x10, osprm = 0x10;

		/* OR the two sprite bits together so we only have 1 bit per sprite */
		collide = pixels | (pixels >> 1);

		/* based on the CLXCON, merge even/odd sprite results */
		collide |= (collide & ormask[CUSTOM_REG(REG_CLXCON) >> 12]) >> 2;

		/* collapse down to a 4-bit final "sprite present" mask */
		collide = (collide & 1) | ((collide >> 3) & 2) | ((collide >> 6) & 4) | ((collide >> 9) & 8);

		/* compute sprite-sprite collisions */
		CUSTOM_REG(REG_CLXDAT) |= spritecollide[collide];

		/* now determine the actual color */
		for (pair = 0; pixels; pair++, pixels >>= 4)
			if (pixels & 0x0f)
			{
				/* final result is:
				    topmost sprite color in bits 0-5
				    sprite present bitmask in bits 6-9
				    topmost sprite pair index in bits 10-11
				*/
				uint32_t result = (collide << 6) | (pair << 10);

				/* attached case */
				if (CUSTOM_REG(REG_SPR1CTL + 8 * pair) & 0x0080)
					return (pixels & 0xf) | osprm | result;

				/* lower-numbered sprite of pair */
				else if (pixels & 3)
					return (pixels & 3) | esprm | (pair << 2) | result;

				/* higher-numbered sprite of pair */
				else
					return ((pixels >> 2) & 3) | osprm | (pair << 2) | result;
			}
	}

	return 0;
}



/*************************************
 *
 *  Bitplane assembly
 *
 *************************************/

uint8_t amiga_state::assemble_odd_bitplanes(int planes, int obitoffs)
{
	uint8_t pix = (CUSTOM_REG(REG_BPL1DAT) >> obitoffs) & 1;
	if (planes >= 3)
	{
		pix |= ((CUSTOM_REG(REG_BPL3DAT) >> obitoffs) & 1) << 2;
		if (planes >= 5)
			pix |= ((CUSTOM_REG(REG_BPL5DAT) >> obitoffs) & 1) << 4;
	}
	return pix;
}


uint8_t amiga_state::assemble_even_bitplanes(int planes, int ebitoffs)
{
	uint8_t pix = 0;
	if (planes >= 2)
	{
		pix |= ((CUSTOM_REG(REG_BPL2DAT) >> ebitoffs) & 1) << 1;
		if (planes >= 4)
		{
			pix |= ((CUSTOM_REG(REG_BPL4DAT) >> ebitoffs) & 1) << 3;
			if (planes >= 6)
				pix |= ((CUSTOM_REG(REG_BPL6DAT) >> ebitoffs) & 1) << 5;
		}
	}
	return pix;
}

void amiga_state::fetch_bitplane_data(int plane)
{
	CUSTOM_REG(REG_BPL1DAT + plane) = read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2));
	CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
}


/*************************************
 *
 *  Hold and modify pixel computations
 *
 *************************************/

int amiga_state::update_ham(int newpix)
{
	switch (newpix >> 4)
	{
		case 0:
			m_ham_color = CUSTOM_REG(REG_COLOR00 + (newpix & 0xf));
			break;

		case 1:
			m_ham_color = (m_ham_color & 0xff0) | ((newpix & 0xf) << 0);
			break;

		case 2:
			m_ham_color = (m_ham_color & 0x0ff) | ((newpix & 0xf) << 8);
			break;

		case 3:
			m_ham_color = (m_ham_color & 0xf0f) | ((newpix & 0xf) << 4);
			break;
	}
	return m_ham_color;
}


//**************************************************************************
//  DISPLAY WINDOW
//**************************************************************************

void amiga_state::update_display_window()
{
	int vstart = CUSTOM_REG(REG_DIWSTRT) >> 8;
	int vstop = CUSTOM_REG(REG_DIWSTOP) >> 8;
	int hstart = CUSTOM_REG(REG_DIWSTRT) & 0xff;
	int hstop = CUSTOM_REG(REG_DIWSTOP) & 0xff;

	if (m_diwhigh_valid)
	{
		vstart |= (CUSTOM_REG(REG_DIWHIGH) & 7) << 8;
		vstop  |= ((CUSTOM_REG(REG_DIWHIGH) >> 8) & 7) << 8;
		hstart |= ((CUSTOM_REG(REG_DIWHIGH) >> 5) & 1) << 8;
		hstop  |= ((CUSTOM_REG(REG_DIWHIGH) >> 13) & 1) << 8;
	}
	else
	{
		vstop |= ((~CUSTOM_REG(REG_DIWSTOP) >> 7) & 0x100);
		hstop |= 0x100;
	}

	if (hstop < hstart)
	{
		hstart = 0x00;
		hstop = 0x1ff;
	}

	m_diw.set(hstart, hstop, vstart, vstop);
}


/*************************************
 *
 *  Single scanline rasterizer
 *
 *************************************/

void amiga_state::render_scanline(bitmap_rgb32 &bitmap, int scanline)
{
	uint16_t save_color0 = CUSTOM_REG(REG_COLOR00);
	int ddf_start_pixel = 0, ddf_stop_pixel = 0;
	int hires = 0, dualpf = 0, ham = 0;
	int pf1pri = 0, pf2pri = 0;
	int planes = 0;
	int raw_scanline = 0;

	uint32_t *dst = nullptr;
	int ebitoffs = 0, obitoffs = 0;
	int ecolmask = 0, ocolmask = 0;
	int edelay = 0, odelay = 0;
	int next_copper_x;
	int pl;
	const int defbitoffs = 15;
	bool bitplane_dma_enabled = false;

	int save_scanline = scanline;

	// we need to do a bit more work on the first scanline
	if (scanline == 0)
	{
		m_previous_lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		// toggle lof if enabled
		if (CUSTOM_REG(REG_BPLCON0) & BPLCON0_LACE)
			CUSTOM_REG(REG_VPOSR) ^= VPOSR_LOF;

		// reset copper and ham color
		m_copper->vblank_sync(true);
		m_ham_color = CUSTOM_REG(REG_COLOR00);
	}

	// in visible area?
	if (bitmap.valid())
	{
		bool lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		if ((scanline & 1) ^ lof)
		{
			// lof matches? then render this scanline
			dst = &bitmap.pix(scanline);
		}
		else
		{
			// lof doesn't match, we don't render this scanline
			// if we didn't switch lof we have a full non-interlace screen,
			// so we fill the black gaps with the contents of the previous scanline
			// otherwise just render the contents of the previous frame's scanline
			int shift = (m_previous_lof == lof) ? 1 : 0;

			if ((scanline - shift) < 0)
				return;
			std::copy_n(&m_flickerfixer.pix(scanline - shift), amiga_state::SCREEN_WIDTH, &bitmap.pix(scanline));
			return;
		}
	}

	raw_scanline = scanline;

	scanline /= 2;

	// notify copper that we are not in vblank anymore
	if (scanline == get_screen_vblank_line())
		m_copper->vblank_sync(false);

	m_last_scanline = scanline;

	/* all sprites off at the start of the line */
	memset(m_sprite_remain, 0, sizeof(m_sprite_remain));

	/* temporary set color 0 to the genlock color */
	if (m_genlock_color != 0xffff)
		CUSTOM_REG(REG_COLOR00) = m_genlock_color;

	/* loop over the line */
	// TODO: copper runs on odd timeslots
	next_copper_x = m_copper->restore_offset();
	// FIXME: without the add this increment will skip bitplane ops
	// ddf_stop_pixel_max = 0xd8 * 2 = 432 + 17 + 15 + 1(*) = 465 > width / 2 (455)
	// (*) because there's a comparison with <= in the bitplane code.
	// There are various root causes about why this happens:
	// - no separation of video and logic models;
	// - the offsets we are applying to DDFSTRT and DDFSTOP, they mustn't be right (copper timings?);
	// - ditto for DIW related values, they are offset in far too many places;
	// - Twintris intro/suprfrog expects +11 on fast scrolling section (glitches at sides)
	for (int x = 0; x < (amiga_state::SCREEN_WIDTH / 2) + 11; x++)
	{
		int sprpix;
		const bool out_of_beam = x >= amiga_state::SCREEN_WIDTH / 2;

		/* time to execute the copper? */
		if (x == next_copper_x && !out_of_beam)
		{
			/* execute the next batch, restoring and re-saving color 0 around it */
			CUSTOM_REG(REG_COLOR00) = save_color0;

			planes = (CUSTOM_REG(REG_BPLCON0) & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) >> 12;

			next_copper_x = m_copper->execute_next(
				x,
				m_last_scanline & 0xff,
				bool(BIT(CUSTOM_REG(REG_DMACON), 14)), // BBUSY
				planes
			);
			save_color0 = CUSTOM_REG(REG_COLOR00);
			if (m_genlock_color != 0xffff)
				CUSTOM_REG(REG_COLOR00) = m_genlock_color;

			/* compute update-related register values */
			hires = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HIRES;
			ham = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HOMOD;
			dualpf = CUSTOM_REG(REG_BPLCON0) & BPLCON0_DBLPF;

			/* compute the pixel fetch parameters */
			// lastbtle sets 0x34-0xd0, lores
			// swordsod sets 0x38-0xd6 (on gameplay), lores
			// TODO: verify hires, fix mask for ECS (which can set bit 1 too)
//          ddf_start_pixel = (CUSTOM_REG(REG_DDFSTRT) & (hires ? 0xfc : 0xf8)) * 2;
			ddf_start_pixel = (CUSTOM_REG(REG_DDFSTRT) & 0xfc) * 2;
			ddf_start_pixel += hires ? 9 : 17;
//          ddf_stop_pixel = (CUSTOM_REG(REG_DDFSTOP) & (hires ? 0xfc : 0xf8)) * 2;
			ddf_stop_pixel = (CUSTOM_REG(REG_DDFSTOP) & 0xfc) * 2;
			ddf_stop_pixel += hires ? (9 + defbitoffs) : (17 + defbitoffs);

			// TODO: verify this one on actual hires mode
			// lastbtle definitely don't need this (enables bit 2 of ddfstrt while in lores mode)
			if ( ( CUSTOM_REG(REG_DDFSTRT) ^ CUSTOM_REG(REG_DDFSTOP) ) & 0x04 && hires )
				ddf_stop_pixel += 8;

			// display window
			update_display_window();

			/* extract playfield priorities */
			pf1pri = CUSTOM_REG(REG_BPLCON2) & 7;
			pf2pri = (CUSTOM_REG(REG_BPLCON2) >> 3) & 7;

			/* extract collision masks */
			ocolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x15;
			ecolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x2a;
		}

		/* update sprite data fetching */
		// ensure this happens once every two scanlines for the RAM manipulation, kickoff cares
		// this is also unaffected by LACE
		// Update: comparison is unnecessary, as per tomato and amiga_cd:bigred cursor pointers (both enabling hires)
		//if ((raw_scanline & 1) == 0)
		{
			const int min_x = 0x18 << 1;
			const int max_x = 0x34 << 1;
			// TODO: refine
			// Sprite DMA loads first two words at $14 + num * 4, other 2 words at $18 + num * 4
			// NOTE: position $28 for sprite 4 has a typo on HRM diagram
			// non-zero DMA fetches are required by:
			// - beast gameplay, abreed (top left counter)
			if (x >= min_x && x <= max_x && (x & 7) == 0)
			{
				int num = (x - min_x) >> 3;
				//printf("%d %02x\n", num, x);
				update_sprite_dma(raw_scanline >> 1, num);
			}
		}

		/* clear the target pixels to the background color as a starting point */
		if (dst != nullptr && !out_of_beam)
		{
			dst[x*2+0] =
			dst[x*2+1] = m_palette->pen(CUSTOM_REG(REG_COLOR00));
		}

		/* if we hit the first fetch pixel, reset the counters and latch the delays */
		if (x == ddf_start_pixel)
		{
			odelay = CUSTOM_REG(REG_BPLCON1) & 0xf;
			edelay = ( CUSTOM_REG(REG_BPLCON1) >> 4 ) & 0x0f;

			if ( hires )
			{
				obitoffs = defbitoffs + ( odelay << 1 );
				ebitoffs = defbitoffs + ( edelay << 1 );
			}
			else
			{
				if ( CUSTOM_REG(REG_DDFSTRT) & 0x04 )
				{
					odelay = ( odelay + 8 ) & 0x0f;
					edelay = ( edelay + 8 ) & 0x0f;
				}

				obitoffs = defbitoffs + odelay;
				ebitoffs = defbitoffs + edelay;
			}

			for (pl = 0; pl < 6; pl++)
				CUSTOM_REG(REG_BPL1DAT + pl) = 0;
		}

		/* need to run the sprite engine every pixel to ensure display */
		sprpix = get_sprite_pixel(x);

		bitplane_dma_enabled = (CUSTOM_REG(REG_DMACON) & (DMACON_BPLEN | DMACON_DMAEN)) == (DMACON_BPLEN | DMACON_DMAEN);

		/* to render, we must have bitplane DMA enabled, at least 1 plane, and be within the */
		/* vertical display window */
		// TODO: bitplane DMA enabled applies to fetch_bitplane_data only
		if (bitplane_dma_enabled && planes > 0 && scanline >= m_diw.top() && scanline < m_diw.bottom())
		{
			int pfpix0 = 0, pfpix1 = 0, collide;

			/* fetch the odd bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + odelay)
			{
				/* if we need to fetch more data, do it now */
				if (obitoffs == defbitoffs)
				{
					for (pl = 0; pl < planes; pl += 2)
					{
						fetch_bitplane_data(pl);
					}
				}

				/* now assemble the bits */
				pfpix0 |= assemble_odd_bitplanes(planes, obitoffs);
				obitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (obitoffs < 0)
					{
						obitoffs = defbitoffs;

						for (pl = 0; pl < planes; pl += 2)
						{
							fetch_bitplane_data(pl);
						}
					}

					pfpix1 |= assemble_odd_bitplanes(planes, obitoffs);
					obitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x15;

				/* reset bit offsets if needed */
				if (obitoffs < 0)
					obitoffs = defbitoffs;
			}

			/* fetch the even bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + edelay)
			{
				/* if we need to fetch more data, do it now */
				if (ebitoffs == defbitoffs)
				{
					for (pl = 1; pl < planes; pl += 2)
					{
						fetch_bitplane_data(pl);
					}
				}

				/* now assemble the bits */
				pfpix0 |= assemble_even_bitplanes(planes, ebitoffs);
				ebitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (ebitoffs < 0)
					{
						ebitoffs = defbitoffs;

						for (pl = 1; pl < planes; pl += 2)
						{
							fetch_bitplane_data(pl);
						}
					}

					pfpix1 |= assemble_even_bitplanes(planes, ebitoffs);
					ebitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x2a;

				/* reset bit offsets if needed */
				if (ebitoffs < 0)
					ebitoffs = defbitoffs;
			}

			/* compute playfield/sprite collisions for first pixel */
			collide = pfpix0 ^ CUSTOM_REG(REG_CLXCON);
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 5) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 1) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* compute playfield/sprite collisions for second pixel */
			collide = pfpix1 ^ CUSTOM_REG(REG_CLXCON);
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 5) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 1) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* if we are within the display region, render */
			if (dst != nullptr && x >= m_diw.left() && x < m_diw.right() && !out_of_beam)
			{
				int pix, pri;

				/* hold-and-modify mode -- assume low-res (hi-res not supported by the hardware) */
				if (ham)
				{
					/* update the HAM color */
					pfpix0 = update_ham(pfpix0);

					pix = sprpix & 0x1f;
					pri = (sprpix >> 10);

					/* sprite has priority */
					// TODO: verify if PF2Px priority applies to HAM too
					// (technically it's a non-dual too?)
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + pix));
					}

					/* playfield has priority */
					else
					{
						dst[x*2+0] =
						dst[x*2+1] = m_palette->pen(pfpix0);
					}
				}

				/* dual playfield mode */
				else if (dualpf)
				{
					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					pri = (sprpix >> 10);
					if (pix)
					{
						if ((pfpix0 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix0 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the left pixel */
					if (pix)
						dst[x*2+0] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + pix));
					else
						dst[x*2+0] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + m_separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix0]));

					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					if (pix)
					{
						if ((pfpix1 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix1 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the right pixel */
					if (pix)
						dst[x*2+1] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + pix));
					else
						dst[x*2+1] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + m_separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix1]));
				}

				/* single playfield mode */
				else
				{
					pix = sprpix & 0x1f;
					pri = (sprpix >> 10);

					/* sprite has priority */
					// alfred OCS won't draw player sprite if PF1Px is used here
					// (writes $0038 to bplcon2)
					// According to HRM PF2Px is used there for non-dual playfield
					if (sprpix && pf2pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + pix));
					}

					/* playfield has priority */
					else
					{
						// TODO: fix SWIV wrong colors for text layer
						// Abuses of an undocumented OCS/ECS HW bug where priority >= 5 (7 in the specific case)
						// makes the bitplanes to only output bit 4 discarding the other pixels
						dst[x*2+0] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + pfpix0));
						dst[x*2+1] = m_palette->pen(CUSTOM_REG(REG_COLOR00 + pfpix1));
					}
				}
			}
		}
	}

	// end of the line: time to add the modulos
	// NOTE: lweapon intro expects modulos to not be applied when bitplane DMA is disabled
	if (scanline >= m_diw.top() && scanline < m_diw.bottom() && bitplane_dma_enabled)
	{
		// update odd planes
		for (pl = 0; pl < planes; pl += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += CUSTOM_REG_SIGNED(REG_BPL1MOD);

		// update even planes
		for (pl = 1; pl < planes; pl += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += CUSTOM_REG_SIGNED(REG_BPL2MOD);
	}

	m_copper->suspend_offset(next_copper_x, amiga_state::SCREEN_WIDTH / 2);

	// restore color00
	CUSTOM_REG(REG_COLOR00) = save_color0;

	// save
	if (dst != nullptr)
		std::copy_n(dst, amiga_state::SCREEN_WIDTH, &m_flickerfixer.pix(save_scanline));
}



/*************************************
 *
 *  Update
 *
 *************************************/

uint32_t amiga_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_scanline_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

bool amiga_state::get_screen_standard()
{
	// we support dynamic switching between PAL and NTSC, determine mode from register
	if (m_agnus_id >= AGNUS_HR_PAL)
		return CUSTOM_REG(REG_BEAMCON0) & 0x20;

	// old agnus, agnus id determines PAL or NTSC
	return !(m_agnus_id & 0x10);
}

int amiga_state::get_screen_vblank_line()
{
	return get_screen_standard() ? amiga_state::VBLANK_PAL : amiga_state::VBLANK_NTSC;
}

void amiga_state::update_screenmode()
{
	bool pal = get_screen_standard();

	// basic height & vblank length
	int height = pal ? SCREEN_HEIGHT_PAL : SCREEN_HEIGHT_NTSC;
	int vblank = pal ? VBLANK_PAL : VBLANK_NTSC;

	// frame period
	attoseconds_t period = HZ_TO_ATTOSECONDS(m_screen->clock()) * SCREEN_WIDTH * height;

	// adjust visible area
	rectangle visarea = m_screen->visible_area();
	visarea.sety(vblank, height - 1);

	// finally set our new mode
	m_screen->configure(SCREEN_WIDTH, height, visarea, period);
}


//**************************************************************************
//  MACHINE DRIVER FRAGMENTS
//**************************************************************************

void amiga_state::pal_video(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw
	(
		(amiga_state::CLK_28M_PAL / 4) * 2 * 2,
		amiga_state::SCREEN_WIDTH, amiga_state::HBLANK, amiga_state::SCREEN_WIDTH,
		amiga_state::SCREEN_HEIGHT_PAL, amiga_state::VBLANK_PAL, amiga_state::SCREEN_HEIGHT_PAL
	);
	m_screen->set_screen_update(FUNC(amiga_state::screen_update));
}

void amiga_state::ntsc_video(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw
	(
		(amiga_state::CLK_28M_NTSC / 4) * 2 * 2,
		amiga_state::SCREEN_WIDTH, amiga_state::HBLANK, amiga_state::SCREEN_WIDTH,
		amiga_state::SCREEN_HEIGHT_NTSC, amiga_state::VBLANK_NTSC, amiga_state::SCREEN_HEIGHT_NTSC
	);
	m_screen->set_screen_update(FUNC(amiga_state::screen_update));
}
