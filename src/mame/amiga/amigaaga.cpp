// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles
/**************************************************************************************************

    Amiga AGA hardware "Lisa"

    Driver by: Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles

Done:
    - palette
    - bitplane data fetching
    - support for up to 8 standard bitplanes
    - HAM8 mode
    - preliminary sprites

TODO:
- Merge with base OCS Denise video emulation, use virtual overrides where applicable;
- Downport ECS Denise features;
- High bits sprite collisions;
- SHRES video mode;
- Confirm diwstrt/diwstop values;
- Add custom screen geometry registers (ECS/AGA chipsets, $1c0-$1ef, most are shared
    with Agnus/Alice really);
- Implement SPRES0/1 (sprite resolutions)
- Remaining unemulated new registers (ZD pin, SOG pin, SSCAN2, BRDRSPRT, BPLAMx);

**************************************************************************************************/

#include "emu.h"
#include "amiga.h"

#define LOG_SPRITE_DMA (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


/*************************************
 *
 *  Palette
 *
 *************************************/

u16 amiga_state::aga_palette_read(offs_t color_reg)
{
	u8 pal_bank = (CUSTOM_REG(REG_BPLCON3) >> 13) & 0x07;

	int color = (pal_bank * 32) + color_reg;

	u8 cr = m_aga_palette[color].r();
	u8 cg = m_aga_palette[color].g();
	u8 cb = m_aga_palette[color].b();

	// LOCT
	if (BIT(CUSTOM_REG(REG_BPLCON3),9))
	{
		return ((cr & 0xf) << 8) | ((cg & 0xf) << 4) | ((cb & 0xf) << 0);
	}

	return ((cr & 0xf0) << 4) | (cg & 0xf0) | ((cb & 0xf0) >> 4);
}

void amiga_state::aga_palette_write(offs_t color_reg, uint16_t data)
{
	int r,g,b;
	int cr,cg,cb;
	int color;
	u8 pal_bank = (CUSTOM_REG(REG_BPLCON3) >> 13) & 0x07;

	color = (pal_bank * 32) + color_reg;
	r = (data & 0xf00) >> 8;
	g = (data & 0x0f0) >> 4;
	b = (data & 0x00f) >> 0;
	cr = m_aga_palette[color].r();
	cg = m_aga_palette[color].g();
	cb = m_aga_palette[color].b();
	// LOCT bit
	if (BIT(CUSTOM_REG(REG_BPLCON3),9))
	{
		// load low nibbles
		cr = (cr & 0xf0) | r;
		cg = (cg & 0xf0) | g;
		cb = (cb & 0xf0) | b;
	}
	else
	{
		cr = (r << 4) | r;
		cg = (g << 4) | g;
		cb = (b << 4) | b;
		// TODO: transparency, bit 15
	}

	m_aga_palette[color] = rgb_t(cr, cg, cb);
	// make a copy for Extra Half Brite mode
	if (pal_bank == 0)
	{
		m_aga_ehb_palette[color_reg] = rgb_t(cr, cg, cb);
		m_aga_ehb_palette[color_reg + 32] = rgb_t(cr >> 1, cg >> 1, cb >> 1);
	}
}

/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(amiga_state,amiga_aga)
{
	video_start_common();

	// fill the AGA dblpf table, taking bplcon3:pf2ofx into account for offset values
	m_separate_bitplanes[0].resize(256 * 8);
	m_separate_bitplanes[1].resize(256 * 8);

	static const int dblpfofs[] = { 0, 2, 4, 8, 16, 32, 64, 128 };

	for (int offset_index = 0; offset_index < 8; offset_index ++)
	{
		int offset_value = dblpfofs[offset_index];
		for (int j = 0; j < 256; j++)
		{
			int pf1pix = ((j >> 0) & 1) | ((j >> 1) & 2) | ((j >> 2) & 4) | ((j >> 3) & 8);
			int pf2pix = ((j >> 1) & 1) | ((j >> 2) & 2) | ((j >> 3) & 4) | ((j >> 4) & 8);

			m_separate_bitplanes[0][j + (offset_index << 8)] = (pf1pix || !pf2pix) ? pf1pix : (pf2pix + offset_value);
			m_separate_bitplanes[1][j + (offset_index << 8)] = pf2pix ? (pf2pix + offset_value) : pf1pix;
		}
	}

	m_aga_diwhigh_written = 0;
}



/*************************************
 *
 *  Per-scanline sprite fetcher
 *
 *************************************/

void amiga_state::aga_fetch_sprite_data(int scanline, int sprite)
{
	switch((CUSTOM_REG(REG_FMODE) >> 2) & 0x03)
	{
		case 0:
			m_aga_sprdata[sprite][0] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdatb[sprite][0] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprite_fetched_words = 1;
			LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d fetch: data=%04X-%04X\n", scanline, sprite, m_aga_sprdata[sprite][0], m_aga_sprdatb[sprite][0]);
			break;
		case 1:
		case 2:
			m_aga_sprdata[sprite][0] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdata[sprite][1] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprdatb[sprite][0] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdatb[sprite][1] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprite_fetched_words = 2;
			LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d fetch: data=%04X-%04X %04X-%04X\n", scanline, sprite, m_aga_sprdata[sprite][0], m_aga_sprdatb[sprite][0], m_aga_sprdata[sprite][1], m_aga_sprdatb[sprite][1] );
			break;
		case 3:
			m_aga_sprdata[sprite][0] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdata[sprite][1] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprdata[sprite][2] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdata[sprite][3] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprdatb[sprite][0] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdatb[sprite][1] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprdatb[sprite][2] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
			m_aga_sprdatb[sprite][3] = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
			m_aga_sprite_fetched_words = 4;
			LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d fetch: data=%04X-%04X %04X-%04X %04X-%04X %04X-%04X\n",
										scanline, sprite,
										m_aga_sprdata[sprite][0], m_aga_sprdatb[sprite][0],
										m_aga_sprdata[sprite][1], m_aga_sprdatb[sprite][1],
										m_aga_sprdata[sprite][2], m_aga_sprdatb[sprite][2],
										m_aga_sprdata[sprite][3], m_aga_sprdatb[sprite][3]);
			break;
	}
	m_aga_sprite_dma_used_words[sprite] = 0;
}

void amiga_state::aga_update_sprite_dma(int scanline, int num)
{
	int dmaenable = (CUSTOM_REG(REG_DMACON) & (DMACON_SPREN | DMACON_DMAEN)) == (DMACON_SPREN | DMACON_DMAEN);
	int maxdma;
	const u16 sprctl_offs[4] = {2, 4, 4, 8};
	const u16 spr_fmode_inc = sprctl_offs[(CUSTOM_REG(REG_FMODE) >> 2) & 0x03];

	/* channels are limited by DDFSTART */
	maxdma = (CUSTOM_REG(REG_DDFSTRT) - 0x14) / 4;
	if (maxdma > 8)
		maxdma = 8;

	if (num >= maxdma)
		return;

	int bitmask = 1 << num;
	int vstart, vstop;

	/* if we are == VSTOP, fetch new control words */
	if (dmaenable && (m_sprite_dma_live_mask & bitmask) && (m_sprite_dma_reload_mask & bitmask) && !(m_sprite_ctl_written & bitmask))
	{
		/* disable the sprite */
		m_sprite_comparitor_enable_mask &= ~bitmask;
		m_sprite_dma_reload_mask &= ~bitmask;

		/* fetch data into the control words */
		CUSTOM_REG(REG_SPR0POS + 4 * num) = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 0);
		// diggers AGA suggests that the fmode increments with ctl are interleaved,
		// otherwise no sprites are drawn.
		// (it enables sprite 0 only, and +8 for the vstop values)
		CUSTOM_REG(REG_SPR0CTL + 4 * num) = read_chip_ram(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + spr_fmode_inc);
		CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) += 2 * spr_fmode_inc;
		LOGMASKED(LOG_SPRITE_DMA, "%3d:sprite %d fetch: pos=%04X ctl=%04X\n", scanline, num, CUSTOM_REG(REG_SPR0POS + 4 * num), CUSTOM_REG(REG_SPR0CTL + 4 * num));
	}

	u16 spr0ctl = CUSTOM_REG(REG_SPR0CTL + 4 * num);
	/* compute vstart/vstop */
	// bits 6 and 5 are respectively vstart bit 9 and vstop bit 9
	// TODO: do they disable with non-AGA modes?
	vstart = (CUSTOM_REG(REG_SPR0POS + 4 * num) >> 8);
	vstart |= (spr0ctl & 0x04) ? 0x100 : 0;
	vstart |= (spr0ctl & 0x40) ? 0x200 : 0;
	vstop = (spr0ctl >> 8);
	vstop |= (spr0ctl & 0x02) ? 0x100 : 0;
	vstop |= (spr0ctl & 0x20) ? 0x200 : 0;

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
		aga_fetch_sprite_data(scanline, num);
	}
}



/*************************************
 *
 *  Per-pixel sprite computations
 *
 *************************************/

int amiga_state::aga_get_sprite_pixel(int x)
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
					m_sprite_shiftreg[num] = interleave_sprite_data(m_aga_sprdata[num][0], m_aga_sprdatb[num][0]);
					m_aga_sprite_dma_used_words[num] = 1;
				}
			}

			/* clock the next pixel if we're doing it */
			if (m_sprite_remain[num] != 0)
			{
				m_sprite_remain[num]--;
				pixels |= (m_sprite_shiftreg[num] & 0xc0000000) >> (16 + 2 * (7 - num));
				m_sprite_shiftreg[num] <<= 2;

				if (m_sprite_remain[num] == 0)
				{
					if (m_aga_sprite_dma_used_words[num] < m_aga_sprite_fetched_words)
					{
						int w = m_aga_sprite_dma_used_words[num];
						m_sprite_remain[num] = 16;
						m_sprite_shiftreg[num] = interleave_sprite_data(m_aga_sprdata[num][w], m_aga_sprdatb[num][w]);
						m_aga_sprite_dma_used_words[num]++;
					}
				}
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
		int esprm, osprm;

		esprm = CUSTOM_REG(REG_BPLCON4) & 0x00f0;
		osprm = (CUSTOM_REG(REG_BPLCON4) & 0x000f) << 4;

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
				    topmost sprite color in bits 0-7
				    sprite present bitmask in bits 8-9
				    topmost sprite pair index in bits 12-13
				*/
				uint32_t result = (collide << 8) | (pair << 12);

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

uint8_t amiga_state::aga_assemble_odd_bitplanes(int planes, int obitoffs)
{
	uint64_t *aga_bpldat = m_aga_bpldat;
	uint8_t pix = (aga_bpldat[0] >> obitoffs) & 1;
	if (planes >= 3)
	{
		pix |= ((aga_bpldat[2] >> obitoffs) & 1) << 2;
		if (planes >= 5)
		{
			pix |= ((aga_bpldat[4] >> obitoffs) & 1) << 4;
			if (planes >= 7)
				pix |= ((aga_bpldat[6] >> obitoffs) & 1) << 6;
		}
	}
	return pix;
}


uint8_t amiga_state::aga_assemble_even_bitplanes(int planes, int ebitoffs)
{
	uint8_t pix = 0;
	if (planes >= 2)
	{
		uint64_t *aga_bpldat = m_aga_bpldat;
		pix |= ((aga_bpldat[1] >> ebitoffs) & 1) << 1;
		if (planes >= 4)
		{
			pix |= ((aga_bpldat[3] >> ebitoffs) & 1) << 3;
			if (planes >= 6)
			{
				pix |= ((aga_bpldat[5] >> ebitoffs) & 1) << 5;
				if (planes >= 8)
					pix |= ((aga_bpldat[7] >> ebitoffs) & 1) << 7;
			}
		}
	}
	return pix;
}

void amiga_state::aga_fetch_bitplane_data(int plane, u8 bitplane_fmode)
{
	uint64_t *aga_bpldat = m_aga_bpldat;

	switch (bitplane_fmode)
	{
		case 0:
			aga_bpldat[plane] = (uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2));
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			break;
		case 1:
		case 2:
			aga_bpldat[plane] = (uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2)) << 16;
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			aga_bpldat[plane] |= ((uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2)));
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			break;
		case 3:
			aga_bpldat[plane] = (uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2)) << 48;
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			aga_bpldat[plane] |= ((uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2))) << 32;
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			aga_bpldat[plane] |= ((uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2))) << 16;
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			aga_bpldat[plane] |= (uint64_t)read_chip_ram(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2));
			CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
			break;
	}
}



/*************************************
 *
 *  Hold and modify pixel computations
 *
 *************************************/

rgb_t amiga_state::aga_update_ham(int newpix, int planes)
{
	// if not in AGA mode just return the legacy HAM6 mode
	// CD32 would otherwise have ... interesting result with CDTV townona
	if (!(planes & 8))
		return m_palette->pen(amiga_state::update_ham(newpix));

	// HAM8
	switch (newpix & 0x03)
	{
		case 0:
			m_ham_color = m_aga_palette[(newpix >> 2) & 0x3f];
			break;

		case 1:
			m_ham_color = rgb_t(m_ham_color.r(), m_ham_color.g(), (newpix & 0xfc) | (m_ham_color.b() & 0x03));
			break;

		case 2:
			m_ham_color = rgb_t((newpix & 0xfc) | (m_ham_color.r() & 0x03), m_ham_color.g(), m_ham_color.b());
			break;

		case 3:
			m_ham_color = rgb_t(m_ham_color.r(), (newpix & 0xfc) | (m_ham_color.g() & 0x03), m_ham_color.b());
			break;
	}
	return m_ham_color;
}



/*************************************
 *
 *  Single scanline rasterizer
 *
 *************************************/

void amiga_state::aga_render_scanline(bitmap_rgb32 &bitmap, int scanline)
{
	uint16_t save_color0 = CUSTOM_REG(REG_COLOR00);
	int ddf_start_pixel = 0, ddf_stop_pixel = 0;
	int hires = 0, dualpf = 0, ham = 0, ehb = 0;
	int pf1pri = 0, pf2pri = 0;
	int planes = 0;
	int raw_scanline = 0;
	u8 bplam = 0;
	u16 pf2ofx = 0;

	uint32_t *dst = nullptr;
	int ebitoffs = 0, obitoffs = 0;
	int ecolmask = 0, ocolmask = 0;
	int edelay = 0, odelay = 0;
	int next_copper_x;
	int pl;
	int defbitoffs = 0;
	rgb_t *aga_palette = m_aga_palette;
	bool bitplane_dma_enabled = false;

	int save_scanline = scanline;

	/* on the first scanline, reset the COPPER and HAM color */
	if (scanline == 0)
	{
		m_previous_lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		// toggle lof if enabled
		if (CUSTOM_REG(REG_BPLCON0) & BPLCON0_LACE)
			CUSTOM_REG(REG_VPOSR) ^= VPOSR_LOF;

		m_copper->vblank_sync(true);
		// TODO: shouldn't be raw color ...
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

	const bool ecsena = !!BIT(CUSTOM_REG(REG_BPLCON0), 0);
	// BRDRBLNK, 17bitlv6:3871 C42 Demo enables this
	const rgb_t border_color = ecsena && BIT(CUSTOM_REG(REG_BPLCON3), 5) ? rgb_t(0, 0, 0) : aga_palette[0];

	// TODO: verify where we're missing pixels here for the GFX pitch bitplane corruptions
	// Update 2025: check and resort all these entries
	// - wbenc30 scrolling in lores mode (fmode=3, expects a +58!, verify ddfstrt / delays)
	// - sockid_a, alfred gameplay (fmode=1)
	// - ssf2t (fmode=3, wants >+100, scrolling is very offset)
	// - watchtow gameplay (fmode=3, copper timings)
	// - cd32 cdtv:insidino copyright screen (fmode=3)
	// - cd32 cdtv:labytime intro/tutorial screens
	//   (swaps between fmode=1 and 3, verify ddfstrt / ddfstop)

	const int offset_hack[] = {
		11,
		// fmode 1: virocp_a (copyright) +26
		11,
		11,
		// fmode 3: dxgalaga (title) wants +20, fatman_a (title) +24
		24
	};
	const int default_bit_offset[] = { 15, 31, 31, 63 };

	const int ddf_start_offset_lores[] = { 17, 17, 17, 17 };
	const int ddf_stop_offset_lores[] = {
		17 + 15,
		// fmode 1:
		// TODO: roadkill gameplay expects an extra +15 somewhere for scrolling to work to the right
		// that breaks pbillusn main menu
		17 + 31,
		17 + 31,
		17 + 63
	};

	const int ddf_start_offset_hires[] = {
		9,
		9,
		9,
		// fmode 3: wbenc30/kangfu/pbillusn (F10 key)/cd32 (CD splash loading) all expects a +8 offset
		9 + 8
	};
	const int ddf_stop_offset_hires[] = {
		9 + 15,
		9 + 15,
		9 + 15,
		// fmode 3: kangfu/pballdmg wants an extra +16
		9 + 8 + 16
	};
	// TODO: move this to inner block as const
	// atm the assignment is here just for the hack to work
	// Alice seems to expect to change it in mid-scanline for bitplane pointers to work
	// - amiga_cd:bigred would otherwise offset the intro
	u8 bitplane_fmode = CUSTOM_REG(REG_FMODE) & 0x3;

	for (int x = 0; x < (amiga_state::SCREEN_WIDTH / 2) + offset_hack[bitplane_fmode]; x++)
	{
		int sprpix;
		const bool out_of_beam = x >= amiga_state::SCREEN_WIDTH / 2;

		/* time to execute the copper? */
		if (x == next_copper_x && !out_of_beam)
		{
			planes = (CUSTOM_REG(REG_BPLCON0) & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) >> 12;
			// TODO: verify number of planes that doesn't go beyond 8
			if ( CUSTOM_REG(REG_BPLCON0) & BPLCON0_BPU3 )
				planes |= 8;
			bitplane_fmode = CUSTOM_REG(REG_FMODE) & 0x3;

			/* execute the next batch, restoring and re-saving color 0 around it */
			CUSTOM_REG(REG_COLOR00) = save_color0;
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
			// TODO: emulate SHRES mode
			// cfr. a1200 -bios logica2,
			// press any key when prompted, select A1200 (5) -> Display Menu (9) -> Super HIRES HAM mode (8)
			// In theory it's simple: maps bitplanes in 35ns resolution, offsetting where needed.
			// In practice we need to separate bitplane delays & drawing first.
			//shres = CUSTOM_REG(REG_BPLCON0) & 0x0040;

			// offset table for pf2 when in dualpf
			// - alfred_a, gameplay background
			// - slamtilt, main menu cursor
			// NOTE: val << 8 so we don't need to adjust per-pixel
			pf2ofx = ((CUSTOM_REG(REG_BPLCON3) >> 10) & 7) << 8;

			// bplam applies xor to bitplane colors (i.e. acting as pal bank)
			// - aladdin, status bar in gameplay
			// TODO: implement for ham and dualpf, below
			bplam = CUSTOM_REG(REG_BPLCON4) >> 8;

			// In AGA Extra Half-Brite applies if this condition is satisfied
			// (bit 9 of BPLCON2 is KILLEHB)
			// cfr. bblow_a main menu
			// TODO: verify if it needs no hires and no shres too
			//ehb = !ham && !dualpf && planes == 6 && !bool(BIT(CUSTOM_REG(REG_BPLCON2), 9));
			ehb = (CUSTOM_REG(REG_BPLCON0) & 0x7c10) == 0x6000 && !bool(BIT(CUSTOM_REG(REG_BPLCON2), 9));

			/* get default bitoffset */
			defbitoffs = default_bit_offset[bitplane_fmode];

			/* compute the pixel fetch parameters */
			// TODO: ECS/AGA can put bit 1 in play here
			ddf_start_pixel = ( CUSTOM_REG(REG_DDFSTRT) & 0xf8 ) * 2;
			ddf_start_pixel += hires ? ddf_start_offset_hires[bitplane_fmode] : ddf_start_offset_lores[bitplane_fmode];
			ddf_stop_pixel = ( CUSTOM_REG(REG_DDFSTOP) & 0xf8 ) * 2;
			ddf_stop_pixel += hires ? ddf_stop_offset_hires[bitplane_fmode] : ddf_stop_offset_lores[bitplane_fmode];

			// - https://github.com/dirkwhoffmann/vAmigaTS/blob/master/Agnus/DDF/DDF/ddf1/ddf1_A500_ECS.JPG
			// - turbojam (gameplay) fmode 3 18 9a lores, particularly when scrolling left (+15 isn't enough).
			// - aladdin 38 ca fmode 3 lores
			// - fbglory (main menu) 28 a4 lores
			// - amigames:Burnout.lha 34 b8 fmode 3 hires
			// - amigames:Wendetta.lha 28 d4 fmode 1 lores
			if ( (CUSTOM_REG(REG_DDFSTRT) & 6) != (CUSTOM_REG(REG_DDFSTOP) & 6))
			{
				ddf_stop_pixel += defbitoffs;
			}

			// display window
			update_display_window();

			/* extract playfield priorities */
			pf1pri = CUSTOM_REG(REG_BPLCON2) & 7;
			pf2pri = (CUSTOM_REG(REG_BPLCON2) >> 3) & 7;

			/* extract collision masks */
			ocolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x15;
			ocolmask |= m_aga_clxcon2 & 0x40;
			ecolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x2a;
			ecolmask |= m_aga_clxcon2 & 0x80;
		}

		//if ((raw_scanline & 1) == 0)
		{
			const int min_x = 0x18 << 1;
			const int max_x = 0x34 << 1;

			// TODO: refine, merge with OCS version
			if (x >= min_x && x <= max_x && (x & 7) == 0)
			{
				int num = (x - min_x) >> 3;
				//printf("%d %02x\n", num, x);
				aga_update_sprite_dma(raw_scanline >> 1, num);
			}
		}

		/* clear the target pixels to the background color as a starting point */
		if (dst != nullptr && !out_of_beam)
			dst[x*2+0] =
			dst[x*2+1] = border_color;

		/* if we hit the first fetch pixel, reset the counters and latch the delays */
		if (x == ddf_start_pixel)
		{
			odelay = CUSTOM_REG(REG_BPLCON1) & 0xf;
			edelay = ( CUSTOM_REG(REG_BPLCON1) >> 4 ) & 0x0f;

			//printf("%04x %d %04x %d %d (%02x %02x)\n", CUSTOM_REG(REG_BPLCON1), scanline, bitplane_fmode, ddf_start_pixel, ddf_stop_pixel, CUSTOM_REG(REG_DDFSTRT), CUSTOM_REG(REG_DDFSTOP));
			// extended delays for AGA
			// FIXME: check above table for implications about this
			switch( bitplane_fmode )
			{
				case 1:
				case 2:
					odelay += (CUSTOM_REG(REG_BPLCON1) & 0x0400) >> 6;
					edelay += (CUSTOM_REG(REG_BPLCON1) & 0x4000) >> 10;

					// NOTE: breaks exile_a gameplay with this enabled
					//odelay ^= 0x10;
					//edelay ^= 0x10;
					// TODO: amiga_cd:finlodys offset intro, particularly here with 0xffff writes
					// (alternates fmode 1 and 3)
					break;
				case 3:
					odelay += (CUSTOM_REG(REG_BPLCON1) & 0x0400) >> 6;
					edelay += (CUSTOM_REG(REG_BPLCON1) & 0x4000) >> 10;
					if (CUSTOM_REG(REG_BPLCON1) & 0x0800)
						odelay ^= 0x20;
					if (CUSTOM_REG(REG_BPLCON1) & 0x8000)
						edelay ^= 0x20;

					break;
			}

			if ( hires )
			{
				obitoffs = (defbitoffs + ( odelay << 1 ));
				ebitoffs = (defbitoffs + ( edelay << 1 ));
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

			for (pl = 0; pl < 8; pl++)
				m_aga_bpldat[pl] = 0;
		}

		/* need to run the sprite engine every pixel to ensure display */
		sprpix = aga_get_sprite_pixel(x);

		bitplane_dma_enabled = (CUSTOM_REG(REG_DMACON) & (DMACON_BPLEN | DMACON_DMAEN)) == (DMACON_BPLEN | DMACON_DMAEN);

		/* to render, we must have bitplane DMA enabled, at least 1 plane, and be within the */
		/* vertical display window */
		if (planes > 0 && scanline >= m_diw.top() && scanline < m_diw.bottom())
		{
			int pfpix0 = 0, pfpix1 = 0, collide;

			/* fetch the odd bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + odelay)
			{
				/* if we need to fetch more data, do it now */
				if (obitoffs == defbitoffs && bitplane_dma_enabled)
				{
					for (pl = 0; pl < planes; pl += 2)
					{
						aga_fetch_bitplane_data(pl, bitplane_fmode);
					}
				}

				/* now assemble the bits */
				pfpix0 |= aga_assemble_odd_bitplanes(planes, obitoffs);
				obitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (obitoffs < 0 && bitplane_dma_enabled)
					{
						obitoffs = defbitoffs;

						for (pl = 0; pl < planes; pl += 2)
						{
							aga_fetch_bitplane_data(pl, bitplane_fmode);
						}
					}

					pfpix1 |= aga_assemble_odd_bitplanes(planes, obitoffs);
					obitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x55; // 0x15

				/* reset bit offsets if needed */
				if (obitoffs < 0)
					obitoffs = defbitoffs;
			}

			/* fetch the even bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + edelay)
			{
				/* if we need to fetch more data, do it now */
				if (ebitoffs == defbitoffs && bitplane_dma_enabled)
				{
					for (pl = 1; pl < planes; pl += 2)
					{
						aga_fetch_bitplane_data(pl, bitplane_fmode);
					}
				}

				/* now assemble the bits */
				pfpix0 |= aga_assemble_even_bitplanes(planes, ebitoffs);
				ebitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (ebitoffs < 0 && bitplane_dma_enabled)
					{
						ebitoffs = defbitoffs;

						for (pl = 1; pl < planes; pl += 2)
						{
							aga_fetch_bitplane_data(pl, bitplane_fmode);
						}
					}

					pfpix1 |= aga_assemble_even_bitplanes(planes, ebitoffs);
					ebitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0xaa;

				/* reset bit offsets if needed */
				if (ebitoffs < 0)
					ebitoffs = defbitoffs;
			}

			/* compute playfield/sprite collisions for first pixel */
			// NOTE: need to << 2 to please the upgraded get_sprite_pixel bitmask
			// - dxgalaga player sprite collisions
			// TODO: verify CLXCON2 match semantics
			// - roadkill writes 0xc0
			// - amigames:Wendetta*.lha writes 0x41 (planes offset below)
			const u16 clxcon_match = (CUSTOM_REG(REG_CLXCON) | (m_aga_clxcon2 & 0x3) << 6);
			collide = pfpix0 ^ clxcon_match;
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> (5 + 2)) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> (1 + 2)) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* compute playfield/sprite collisions for second pixel */
			collide = pfpix1 ^ clxcon_match;
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> (5 + 2)) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> (1 + 2)) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* if we are within the display region, render */
			if (dst != nullptr && x >= m_diw.left() && x < m_diw.right() && !out_of_beam)
			{
				int pix, pri;

				/* hold-and-modify mode -- hires and shres supported (cfr. roadkill) */
				if (ham)
				{
					/* update the HAM color */
					const rgb_t pfpix0_rgb = aga_update_ham(pfpix0, planes);
					const rgb_t pfpix1_rgb = (hires) ? aga_update_ham(pfpix1, planes) : pfpix0_rgb;

					pix = sprpix & 0xff;
					pri = (sprpix >> 12);

					/* sprite has priority */
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = aga_palette[pix];
					}

					/* playfield has priority */
					else
					{
						dst[x*2+0] = pfpix0_rgb;
						dst[x*2+1] = pfpix1_rgb;
					}
				}

				/* dual playfield mode */
				else if (dualpf)
				{
					// pf2pri really, overshadows above (i.e. pf1pri -> pf1p2:0)
					const u8 pf_layer_pri = BIT(CUSTOM_REG(REG_BPLCON2), 6);
					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0xff;
					pri = (sprpix >> 12);
					if (pix)
					{
						if ((pfpix0 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix0 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the left pixel */
					if (pix)
						dst[x*2+0] = aga_palette[pix];
					else
						dst[x*2+0] = aga_palette[m_separate_bitplanes[pf_layer_pri][pfpix0 | pf2ofx]];

					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0xff;
					if (pix)
					{
						if ((pfpix1 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix1 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the right pixel */
					if (pix)
						dst[x*2+1] = aga_palette[pix];
					else
						dst[x*2+1] = aga_palette[m_separate_bitplanes[pf_layer_pri][pfpix1 | pf2ofx]];
				}

				/* single playfield mode */
				else
				{
					pix = sprpix & 0xff;
					pri = (sprpix >> 12);

					/* sprite has priority */
					// TODO: porting OCS alfred fix to AGA will break OCS rodland status bar (!?)
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = aga_palette[pix];
					}

					/* playfield has priority */
					else
					{
						// TODO: does it applies to sprites too?
						rgb_t *dst_palette = ehb ? m_aga_ehb_palette : aga_palette;
						dst[x*2+0] = dst_palette[pfpix0 ^ bplam];
						dst[x*2+1] = dst_palette[pfpix1 ^ bplam];
					}
				}
			}
		}
	}

// TODO: move to debugger command
#if 0
	if ( m_screen->frame_number() % 16 == 0 && scanline == 250 )
	{
		const char *m_lores = "LORES";
		const char *m_hires = "HIRES";
		const char *m_ham = "HAM";
		const char *m_dualpf = "DUALPF";
		//const char *m_lace = "LACE";
		//const char *m_hilace = "HI-LACE";
		const char *p = m_lores;

		if ( hires ) p = m_hires;
		if ( ham ) p = m_ham;
		if ( dualpf ) p = m_dualpf;
		//if ( lace ) p = m_lace;

		//if ( hires && lace ) p = m_hilace;

		popmessage("%s(%d pl od=%02x ed=%02x start=%d stop=%d (%d %d H %d %d V) fetchbits=%d )", p, planes, odelay, edelay, ddf_start_pixel, ddf_stop_pixel, m_diw.min_x, m_diw.max_x, m_diw.min_y, m_diw.max_y, defbitoffs );
		//popmessage("%s(%d pl bplpt1=%06X, bpl1mod=%04x, offset=%x)", p, planes, CUSTOM_REG_LONG(REG_BPL1PTH), CUSTOM_REG(REG_BPL1MOD), hires_modulo_offset );
	}
#endif

	/* end of the line: time to add the modulos */
	if (scanline >= m_diw.top() && scanline < m_diw.bottom() && bitplane_dma_enabled)
	{
		int16_t odd_modulo = CUSTOM_REG_SIGNED(REG_BPL1MOD);
		int16_t even_modulo = CUSTOM_REG_SIGNED(REG_BPL2MOD);
		// bscan2, vkart AGA
		if (CUSTOM_REG(REG_FMODE) & 0x4000)
		{
			int vstart = CUSTOM_REG(REG_DIWSTRT) >> 8;
			vstart |= (CUSTOM_REG(REG_DIWHIGH) & 7) << 8;
			int16_t current_modulo = ((vstart ^ (scanline ^ 1)) & 1) ? odd_modulo : even_modulo;

			for (pl = 0; pl < planes; pl ++)
				CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += current_modulo;
		}
		else
		{
			/* update odd planes */
			for (pl = 0; pl < planes; pl += 2)
				CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += odd_modulo;

			/* update even planes */
			for (pl = 1; pl < planes; pl += 2)
				CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += even_modulo;
		}
	}

	m_copper->suspend_offset(next_copper_x, amiga_state::SCREEN_WIDTH / 2);

	/* restore color00 */
	CUSTOM_REG(REG_COLOR00) = save_color0;

	// save
	if (dst != nullptr)
		std::copy_n(dst, amiga_state::SCREEN_WIDTH, &m_flickerfixer.pix(save_scanline));
}
