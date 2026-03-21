// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    NEC HuC6270 Video Display Controller

    The HuC6270 basically outputs a 9-bit stream of pixel data which
    holds a color index, a palette index, and an indication whether
    the pixel contains background data or from sprite data.

    This data can be used by a colour encoder to output graphics.

    A regular screen is displayed as follows:

        |<- HDS ->|<--       HDW       -->|<- HDE ->|<- HSW ->|
        |---------|-----------------------|---------|---------|
    VSW |                                                     |
        |---------|-----------------------|---------|---------|
    VDS |                                                     |
        |                  overscan                           |
        |---------|-----------------------|---------|---------|
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
    VDW | overscan|    active display     |      overscan     |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |---------|-----------------------|---------|---------|
    VCR |                  overscan                           |
        |                                                     |
        |---------|-----------------------|---------|---------|
        ^end hsync
         ^start vsync (30 cycles after hsync)


KNOWN ISSUES
  - Violent Soldier (probably connected):
    - In the intro some artefacts appear at the top of the
      screen every now and then.
  - In ccovell's splitres test not all sections seem to be aligned properly.
  - Side Arms: Seems to be totally broken.


TODO
  - Fix timing of VRAM-SATB DMA
  - Implement VRAM-VRAM DMA
  - DMA speeds differ depending on the dot clock selected in the huc6270
  - Convert VRAM bus to actual space address (optimization)

**********************************************************************/

#include "emu.h"
#include "huc6270.h"

//#define VERBOSE 1
#include "logmacro.h"


enum {
	MAWR = 0x00,
	MARR = 0x01,
	VxR = 0x02,
	CR = 0x05,
	RCR = 0x06,
	BXR = 0x07,
	BYR = 0x08,
	MWR = 0x09,
	HSR = 0x0a,
	HDR = 0x0b,
	VPR = 0x0c,
	VDW = 0x0d,
	VCR = 0x0e,
	DCR = 0x0f,
	SOUR = 0x10,
	DESR = 0x11,
	LENR = 0x12,
	DVSSR = 0x13
};

ALLOW_SAVE_TYPE(huc6270_device::v_state);
ALLOW_SAVE_TYPE(huc6270_device::h_state);


/* Bits in the VDC status register */
static constexpr u8 HUC6270_BSY = 0x40;    /* Set when the VDC accesses VRAM */
static constexpr u8 HUC6270_VD  = 0x20;    /* Set when in the vertical blanking period */
static constexpr u8 HUC6270_DV  = 0x10;    /* Set when a VRAM > VRAM DMA transfer is done */
static constexpr u8 HUC6270_DS  = 0x08;    /* Set when a VRAM > SATB DMA transfer is done */
static constexpr u8 HUC6270_RR  = 0x04;    /* Set when the current scanline equals the RCR register */
static constexpr u8 HUC6270_OR  = 0x02;    /* Set when there are more than 16 sprites on a line */
static constexpr u8 HUC6270_CR  = 0x01;    /* Set when sprite #0 overlaps with another sprite */


DEFINE_DEVICE_TYPE(HUC6270, huc6270_device, "huc6270", "Hudson HuC6270 VDC")


constexpr u8 huc6270_device::vram_increments[4];

huc6270_device::huc6270_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6270, tag, owner, clock)
	, m_vram_size(0)
	, m_irq_changed_cb(*this)
	, m_register_index(0)
	, m_mawr(0)
	, m_marr(0)
	, m_vrr(0)
	, m_vwr(0)
	, m_cr(0)
	, m_rcr(0)
	, m_bxr(0)
	, m_byr(0)
	, m_mwr(0)
	, m_hsr(0)
	, m_hdr(0)
	, m_vpr(0)
	, m_vdw(0)
	, m_vcr(0)
	, m_dcr(0)
	, m_sour(0)
	, m_desr(0)
	, m_lenr(0)
	, m_dvssr(0)
	, m_status(0)
	, m_hsync(false)
	, m_vsync(false)
	, m_vert_state(v_state::VSW)
	, m_horz_state(h_state::HDS)
	, m_vd_triggered(false)
	, m_vert_to_go(0)
	, m_horz_to_go(0)
	, m_horz_steps(0)
	, m_raster_count(0)
	, m_dvssr_written(false)
	, m_satb_countdown(0)
	, m_dma_enabled(false)
	, m_byr_latched(0)
	, m_bxr_latched(0)
	, m_bat_address(0)
	, m_bat_address_mask(0)
	, m_bat_row(0)
	, m_bat_column(0)
	, m_bat_tile_row{0}
	, m_sat{0}
	, m_sprites_this_line(0)
	, m_sprite_row_index(0)
	, m_sprite_row{0}
	, m_vram(nullptr)
	, m_vram_mask(0)

{
}


/*
  Read one row of tile data from video ram
*/
inline void huc6270_device::fetch_bat_tile_row()
{
	const u16 bat_data = m_vram[m_bat_address & m_vram_mask];
	const u16 tile_palette = (bat_data >> 8) & 0xf0;
	const u32 tile_address = ((bat_data & 0x0fff) << 4) + m_bat_row;
	u16 data1 = m_vram[(tile_address + 0) & m_vram_mask];
	u16 data2 = (data1 >> 7) & 0x1fe;
	u16 data3 = m_vram[(tile_address + 8) & m_vram_mask];
	u16 data4 = (data3 >> 5) & 0x7f8;
	data3 <<= 2;

	for (int i = 7; i >= 0; i--)
	{
		u16 c = (data1 & 0x01) | (data2 & 0x02) | (data3 & 0x04) | (data4 & 0x08);

		/* Colour 0 for background tiles is always taken from palette 0 */
		if (c)
			c |= tile_palette;

		m_bat_tile_row[i] = c;

		data1 >>= 1;
		data2 >>= 1;
		data3 >>= 1;
		data4 >>= 1;
	}
}


void huc6270_device::add_sprite(int index, int x, int pattern, int line, int flip_x, int palette, int priority, int sat_lsb)
{
	const int i = m_sprites_this_line;

	if (i < 16)
	{
		u32 b0, b1, b2, b3;

		if (flip_x)
			flip_x = 0x0f;

		pattern += ((line >> 4) << 1);
		const u32 pattern_address = (pattern << 6) + (line & 0x0f);

		if ((m_mwr & 0x0c) == 0x04)
		{
			if (!sat_lsb)
			{
				b0 = m_vram[(pattern_address + 0x00) & m_vram_mask];
				b1 = m_vram[(pattern_address + 0x10) & m_vram_mask] << 1;
			}
			else
			{
				b0 = m_vram[(pattern_address + 0x20) & m_vram_mask];
				b1 = m_vram[(pattern_address + 0x30) & m_vram_mask] << 1;
			}
			b2 = 0;
			b3 = 0;
		}
		else
		{
			b0 = m_vram[(pattern_address + 0x00) & m_vram_mask];
			b1 = m_vram[(pattern_address + 0x10) & m_vram_mask] << 1;
			b2 = m_vram[(pattern_address + 0x20) & m_vram_mask] << 2;
			b3 = m_vram[(pattern_address + 0x30) & m_vram_mask] << 3;
		}

		for (int j = 15; j >= 0; j--)
		{
			u8 data = (b3 & 0x08) | (b2 & 0x04) | (b1 & 0x02) | (b0 & 0x01);

			if (data)
			{
				data |= palette << 4;

				if (x + (j ^ flip_x) < 1024)
				{
					if (!m_sprite_row[x + (j ^ flip_x)])
					{
						m_sprite_row[x + (j ^ flip_x)] = (priority ? 0x4000 : 0x0000) | (index << 8) | data;
					}
					else
					{
						if (!(m_sprite_row[x + (j ^ flip_x)] & 0xff00))
						{
							/* Sprite 0 collision */
							m_sprite_row[x + (j ^ flip_x)] |= 0x8000;
						}
					}
				}
			}

			b0 >>= 1;
			b1 >>= 1;
			b2 >>= 1;
			b3 >>= 1;
		}

		m_sprites_this_line += 1;
	}
}


void huc6270_device::select_sprites()
{
	static const int cgy_table[4] = { 16, 32, 64, 64 };

	m_sprites_this_line = 0;
	memset(m_sprite_row, 0, sizeof(m_sprite_row));
	m_sprite_row_index = 0x20;

	for (int i = 0; i < 4 * 64; i += 4)
	{
		const int cgy = (m_sat[i + 3] >> 12) & 0x03;
		const int height = cgy_table[cgy];
		// TODO: we are one line off in alignment, is following compensation right?
		// cfr. rennybla & draculax (at least), they are otherwise offset by 1
		// compared to background.
		int sprite_line = m_raster_count - 1 - m_sat[i];

		if (sprite_line >= 0 && sprite_line < height)
		{
			const int sprite_index = i >> 2;
			const int xpos = m_sat[i + 1];
			int pattern = m_sat[i + 2] >> 1;
			const bool sat_lsb = BIT(m_sat[i + 2], 0);
			const int palette = m_sat[i + 3] & 0x0f;
			const bool priority = BIT(m_sat[i + 3], 7);
			const bool cgx = BIT(m_sat[i + 3], 8);

			/* If CGY is set to 1, bit 1 of the sprite pattern index is forced to 0 */
			if (BIT(cgy, 0))
				pattern &= ~0x0002;

			/* If CGY is set to 2 or 3, bits 1 and 2 of the sprite pattern index are forced to 0 */
			if (BIT(cgy, 1))
				pattern &= ~0x0006;

			/* Recalculate line index when sprite is flipped vertically */
			if (BIT(m_sat[i + 3], 15))
				sprite_line = (height - 1) - sprite_line;

			/* Is the sprite 32 pixels wide */
			if (cgx)
			{
				/* If CGX is set, bit 0 of the sprite pattern index is forced to 0 */
				pattern &= ~0x0001;

				/* Check for horizontal flip */
				if (BIT(m_sat[i + 3], 11))
				{
					/* Add to our list of sprites for this line */
					add_sprite(sprite_index, xpos, pattern + 1, sprite_line, 1, palette, priority, sat_lsb);
					add_sprite(sprite_index, xpos + 16, pattern, sprite_line, 1, palette, priority, sat_lsb);
				}
				else
				{
					/* Add to our list of sprites for this line */
					add_sprite(sprite_index, xpos, pattern, sprite_line, 0, palette, priority, sat_lsb);
					add_sprite(sprite_index, xpos + 16, pattern + 1, sprite_line, 0, palette, priority, sat_lsb);
				}
			}
			else
			{
				/* Add to our list of sprites for this line */
				add_sprite(sprite_index, xpos, pattern, sprite_line, BIT(m_sat[i + 3], 11), palette, priority, sat_lsb);
			}
		}
	}

	/* Check for sprite overflow */
	if (m_sprites_this_line >= 16)
	{
		/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
		if (BIT(m_cr, 1))
		{
			m_status |= HUC6270_OR;
			m_irq_changed_cb(ASSERT_LINE);
		}
	}
}


inline void huc6270_device::handle_vblank()
{
	if (!m_vd_triggered)
	{
		if (BIT(m_cr, 3))
		{
			m_status |= HUC6270_VD;
			m_irq_changed_cb(ASSERT_LINE);
		}

		/* Should we initiate a VRAM->SATB DMA transfer.
		   The timing for this is incorrect.
		 */
		if (m_dvssr_written || BIT(m_dcr, 4))
		{
			LOG("SATB transfer from %05x\n", m_dvssr << 1);
			for (int i = 0; i < 4 * 64; i += 4)
			{
				m_sat[i + 0] = m_vram[(m_dvssr + i + 0) & m_vram_mask] & 0x03ff;
				m_sat[i + 1] = m_vram[(m_dvssr + i + 1) & m_vram_mask] & 0x03ff;
				m_sat[i + 2] = m_vram[(m_dvssr + i + 2) & m_vram_mask] & 0x07ff;
				m_sat[i + 3] = m_vram[(m_dvssr + i + 3) & m_vram_mask];
			}
			m_dvssr_written = false;

			/* Generate SATB interrupt if requested */
			if (BIT(m_dcr, 0))
			{
				m_satb_countdown = 4;
//              m_status |= HUC6270_DS;
//              m_irq_changed_cb(ASSERT_LINE);
			}
		}

		m_vd_triggered = true;
	}
}


inline void huc6270_device::next_vert_state()
{
	switch (m_vert_state)
	{
	case v_state::VSW:
		m_vert_state = v_state::VDS;
		m_vert_to_go = ((m_vpr >> 8) & 0xff) + 2;
		break;

	case v_state::VDS:
		m_vert_state = v_state::VDW;
		m_vert_to_go = (m_vdw & 0x1ff) + 1;
		m_byr_latched = m_byr;
		m_vd_triggered = false;
		break;

	case v_state::VDW:
		m_vert_state = v_state::VCR;
		m_vert_to_go = (m_vcr & 0xff);
		handle_vblank();
		break;

	case v_state::VCR:
		m_vert_state = v_state::VSW;
		m_vert_to_go = (m_vpr & 0x1f) + 1;
		break;
	}
}


inline void huc6270_device::next_horz_state()
{
	switch (m_horz_state)
	{
	case h_state::HDS:
		m_bxr_latched = m_bxr;
		m_horz_state = h_state::HDW;
		m_horz_to_go = (m_hdr & 0x7f) + 1;
		{
			static const int width_shift[4] = { 5, 6, 7, 7 };
			const int width = width_shift[(m_mwr >> 4) & 0x03];

			const u16 v = m_byr_latched & (BIT(m_mwr, 6) ? 0x1ff : 0xff);
			m_bat_row = v & 7;
			m_bat_address_mask = (1 << width) - 1;
			m_bat_address = ((v >> 3) << width)
				| ((m_bxr_latched >> 3) & m_bat_address_mask);
			m_bat_column = m_bxr & 7;
			fetch_bat_tile_row();
		}
		break;

	case h_state::HDW:
		m_horz_state = h_state::HDE;
		m_horz_to_go = ((m_hdr >> 8) & 0x7f) + 1;
		break;

	case h_state::HDE:
		m_horz_state = h_state::HSW;
		m_horz_to_go = (m_hsr & 0x1f) + 1;
		break;

	case h_state::HSW:
		m_horz_state = h_state::HDS;
		m_horz_to_go = std::max(((m_hsr >> 8) & 0x7f), 2) + 1;

		/* If section has ended, advance to next vertical state */
		while (m_vert_to_go == 0)
			next_vert_state();

		/* Select sprites for the coming line */
		select_sprites();
		break;
	}
	m_horz_steps = 0;
}


u16 huc6270_device::next_pixel()
{
	u16 data = HUC6270_SPRITE;

	/* Check if we're on an active display line */
	if (m_vert_state == v_state::VDW)
	{
		/* Check if we're in active display area */
		if (m_horz_state == h_state::HDW)
		{
			const u8 sprite_data = m_sprite_row[m_sprite_row_index] & 0x00ff;
			const bool collision = BIT(m_sprite_row[m_sprite_row_index], 15);

			if (BIT(m_cr, 7))
			{
				data = HUC6270_BACKGROUND | m_bat_tile_row[m_bat_column];
				if (sprite_data && BIT(m_cr, 6))
				{
					if (BIT(m_sprite_row[m_sprite_row_index], 14))
					{
						data = HUC6270_SPRITE | sprite_data;
					}
					else
					{
						if (data == HUC6270_BACKGROUND)
						{
							data = HUC6270_SPRITE | sprite_data;
						}
					}
				}
			}
			else
			{
				if (BIT(m_cr, 6))
				{
					data = HUC6270_SPRITE | sprite_data;
				}
			}

			m_sprite_row_index = m_sprite_row_index + 1;
			m_bat_column += 1;
			if (m_bat_column >= 8)
			{
				m_bat_address = (m_bat_address & ~m_bat_address_mask)
					| ((m_bat_address + 1) & m_bat_address_mask);
				m_bat_column = 0;
				fetch_bat_tile_row();
			}

			if (collision && BIT(m_cr, 0))
			{
				m_status |= HUC6270_CR;
				m_irq_changed_cb(ASSERT_LINE);
			}
		}
	}

	m_horz_steps++;
	if (m_horz_steps == 8)
	{
		m_horz_to_go--;
		m_horz_steps = 0;
		while (m_horz_to_go == 0)
			next_horz_state();
	}
	return data;
}


//inline u16 huc6270_device::time_until_next_event()
//{
//  return m_horz_to_go * 8 + m_horz_steps;
//}


void huc6270_device::vsync_changed(int state)
{
	state &= 0x01;
	if (m_vsync != state)
	{
		/* Check for high->low VSYNC transition */
		if (!state)
		{
			m_vert_state = v_state::VCR;
			m_vert_to_go = 0;

			while (m_vert_to_go == 0)
				next_vert_state();
		}
		else
		{
			/* Check for low->high VSYNC transition */
			// VBlank IRQ happens at the beginning of HDW period after VDW ends
			handle_vblank();
		}
	}

	m_vsync = state;
}


void huc6270_device::hsync_changed(int state)
{
	state &= 0x01;

	if (m_hsync != state)
	{
		/* Check for low->high HSYNC transition */
		if (state)
		{
			if (m_satb_countdown)
			{
				m_satb_countdown--;

				if (m_satb_countdown == 0)
				{
					m_status |= HUC6270_DS;
					m_irq_changed_cb(ASSERT_LINE);
				}
			}

			m_horz_state = h_state::HSW;
			m_horz_to_go = 0;
			m_horz_steps = 0;
			m_byr_latched += 1;
			m_raster_count += 1;
			// raster count VSW latch happens one line earlier (cfr. +2 on assignment)
			// This has been confirmed on real HW, where the last possible RCR with
			// 240 VDW is 0x130 (i.e. 64 + 240). m_vert_to_go == 1 will also
			// cause several side effects, namely:
			// - draculax Stage 4' "all blue" Richter;
			// - faussete Stage 2 excessive slowdown;
			// - xwiber Stage 2 boss never spawning (MT#07384)
			if (m_vert_to_go == 2 && m_vert_state == v_state::VDS)
			{
				m_raster_count = 0x40;
			}

			m_vert_to_go--;

			while (m_horz_to_go == 0)
				next_horz_state();

			handle_dma();
		}
		else
		{
			/* Check for high->low HSYNC transition */
			// RCR IRQ happens near the end of the HDW period
			if (m_raster_count == m_rcr && BIT(m_cr, 2))
			{
				m_status |= HUC6270_RR;
				m_irq_changed_cb(ASSERT_LINE);
			}
		}
	}

	m_hsync = state;
}

inline void huc6270_device::handle_dma()
{
	/* Should we perform VRAM-VRAM dma.
	   The timing for this is incorrect.
	 */
	if (m_dma_enabled)
	{
		const int desr_inc = (m_dcr & 0x0008) ? -1 : +1;
		const int sour_inc = (m_dcr & 0x0004) ? -1 : +1;

		LOG("doing dma sour = %04x, desr = %04x, lenr = %04x\n", m_sour, m_desr, m_lenr);

		do {
			// area 0x8000-0xffff cannot be r/w (open bus)
			const u16 data = (m_sour <= m_vram_mask) ? m_vram[m_sour] : 0;

			if (m_desr <= m_vram_mask)
				m_vram[m_desr] = data;
			m_sour += sour_inc;
			m_desr += desr_inc;
			m_lenr--;
		} while (m_lenr != 0xffff);

		if (BIT(m_dcr, 1))
		{
			m_status |= HUC6270_DV;
			m_irq_changed_cb(ASSERT_LINE);
		}
		m_dma_enabled = false;
	}
}

u8 huc6270_device::read8(offs_t offset)
{
	u8 data = 0x00;

	switch (offset & 3)
	{
		case 0x00:  /* status */
			data = m_status;
			if (!machine().side_effects_disabled())
			{
				m_status &= ~(HUC6270_VD | HUC6270_DV | HUC6270_RR | HUC6270_CR | HUC6270_OR | HUC6270_DS);
				m_irq_changed_cb(CLEAR_LINE);
			}
			break;

		case 0x02:
			data = m_vrr & 0xff;
			break;

		case 0x03:
			data = m_vrr >> 8;
			if (!machine().side_effects_disabled())
			{
				if (m_register_index == VxR)
				{
					m_marr += vram_increments[(m_cr >> 11) & 3];

					if (m_marr <= m_vram_mask)
					{
						m_vrr = m_vram[m_marr];
					}
					else
					{
						// TODO: test with real HW
						m_vrr = 0;
						logerror("%s: Open Bus VRAM read (register read) %04x\n", machine().describe_context(), m_marr);
					}
				}
			}
			break;
	}
	return data;
}

u16 huc6270_device::read16(offs_t offset)
{
	u16 data = 0x0000;

	switch (offset & 1)
	{
		case 0x00:  /* status */
			data = m_status;
			if (!machine().side_effects_disabled())
			{
				m_status &= ~(HUC6270_VD | HUC6270_DV | HUC6270_RR | HUC6270_CR | HUC6270_OR | HUC6270_DS);
				m_irq_changed_cb(CLEAR_LINE);
			}
			break;

		case 0x01:
			data = m_vrr;
			if (!machine().side_effects_disabled())
			{
				if (m_register_index == VxR)
				{
					m_marr += vram_increments[(m_cr >> 11) & 3];

					if (m_marr <= m_vram_mask)
					{
						m_vrr = m_vram[m_marr];
					}
					else
					{
						// TODO: test with real HW
						m_vrr = 0;
						logerror("%s: Open Bus VRAM read (register read) %04x\n", machine().describe_context(), m_marr);
					}
				}
			}
			break;
	}
	return data;
}


void huc6270_device::write8(offs_t offset, u8 data)
{
	LOG("%s: huc6270 write %02x <- %02x\n", machine().describe_context(), offset, data);

	switch (offset & 3)
	{
		case 0x00:  /* VDC register select */
			m_register_index = data & 0x1f;
			break;

		case 0x02:  /* VDC data LSB */
			regs_w(m_register_index, data, 0x00ff);
			break;

		case 0x03:  /* VDC data MSB */
			regs_w(m_register_index, u16(data) << 8, 0xff00);
			break;
	}
}

void huc6270_device::write16(offs_t offset, u16 data)
{
	LOG("%s: huc6270 write %02x <- %04x\n", machine().describe_context(), offset, data);

	switch (offset & 1)
	{
		case 0x00:  /* VDC register select */
			m_register_index = data & 0x1f;
			break;

		case 0x01:  /* VDC data */
			regs_w(m_register_index, data);
			break;
	}
}

inline void huc6270_device::regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case MAWR:      /* memory address write register LSB */
			COMBINE_DATA(&m_mawr);
			break;

		case MARR:      /* memory address read register LSB */
			COMBINE_DATA(&m_marr);
			if (m_marr <= m_vram_mask)
				m_vrr = m_vram[m_marr];
			else
			{
				// TODO: test with real HW
				m_vrr = 0;
				if (ACCESSING_BITS_0_7)
					logerror("%s: Open Bus VRAM read (memory address) %04x\n", machine().describe_context(), m_marr);
			}
			break;

		case VxR:       /* vram write data LSB */
			COMBINE_DATA(&m_vwr);
			if (ACCESSING_BITS_8_15)
			{
				// area 0x8000-0xffff is NOP and cannot be written to.
				if (m_mawr <= m_vram_mask)
					m_vram[m_mawr] = m_vwr;
				m_mawr += vram_increments[(m_cr >> 11) & 3];
			}
			break;

		case CR:        /* control register LSB */
			COMBINE_DATA(&m_cr);
			break;

		case RCR:       /* raster compare register LSB */
			COMBINE_DATA(&m_rcr);
			m_rcr &= 0x3ff;
//logerror("%s: RCR set to %03x\n", machine().describe_context(), m_rcr);
//                  if (m_raster_count == m_rcr && m_cr & 0x04)
//                  {
//                      m_status |= HUC6270_RR;
//                      m_irq_changed_cb(ASSERT_LINE);
//                  }
			break;

		case BXR:       /* background x-scroll register LSB */
			COMBINE_DATA(&m_bxr);
			m_bxr &= 0x3ff;
			break;

		case BYR:       /* background y-scroll register LSB */
			COMBINE_DATA(&m_byr);
			m_byr &= 0x1ff;
			m_byr_latched = m_byr;
			break;

		case MWR:       /* memory width register LSB */
			COMBINE_DATA(&m_mwr);
			break;

		case HSR:       /* horizontal sync register LSB */
			COMBINE_DATA(&m_hsr);
			break;

		case HDR:       /* horizontal display register LSB */
			COMBINE_DATA(&m_hdr);
			break;

		case VPR:       /* vertical sync register LSB */
			COMBINE_DATA(&m_vpr);
			break;

		case VDW:       /* vertical display register LSB */
			COMBINE_DATA(&m_vdw);
			break;

		case VCR:       /* vertical display end position register LSB */
			COMBINE_DATA(&m_vcr);
			break;

		case DCR:       /* DMA control register LSB */
			COMBINE_DATA(&m_dcr);
			break;

		case SOUR:      /* DMA source address register LSB */
			COMBINE_DATA(&m_sour);
			break;

		case DESR:      /* DMA destination address register LSB */
			COMBINE_DATA(&m_desr);
			break;

		case LENR:      /* DMA length register LSB */
			COMBINE_DATA(&m_lenr);
			if (ACCESSING_BITS_8_15)
				m_dma_enabled = true;
			break;

		case DVSSR:     /* Sprite attribute table LSB */
			COMBINE_DATA(&m_dvssr);
			m_dvssr_written = true;
			break;
	}
}


void huc6270_device::device_start()
{
	assert(!(m_vram_size & (m_vram_size - 1)));
	m_vram = make_unique_clear<u16 []>(m_vram_size / sizeof(u16));
	m_vram_mask = (m_vram_size >> 1) - 1;

	save_pointer(NAME(m_vram), m_vram_size / sizeof(u16));

	save_item(NAME(m_register_index));
	save_item(NAME(m_mawr));
	save_item(NAME(m_marr));
	save_item(NAME(m_vrr));
	save_item(NAME(m_vwr));
	save_item(NAME(m_cr));
	save_item(NAME(m_rcr));
	save_item(NAME(m_bxr));
	save_item(NAME(m_byr));
	save_item(NAME(m_mwr));
	save_item(NAME(m_hsr));
	save_item(NAME(m_hdr));
	save_item(NAME(m_vpr));
	save_item(NAME(m_vdw));
	save_item(NAME(m_vcr));
	save_item(NAME(m_dcr));
	save_item(NAME(m_sour));
	save_item(NAME(m_desr));
	save_item(NAME(m_lenr));
	save_item(NAME(m_dvssr));
	save_item(NAME(m_status));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vert_state));
	save_item(NAME(m_horz_state));
	save_item(NAME(m_vd_triggered));
	save_item(NAME(m_vert_to_go));
	save_item(NAME(m_horz_to_go));
	save_item(NAME(m_horz_steps));
	save_item(NAME(m_raster_count));
	save_item(NAME(m_dvssr_written));
	save_item(NAME(m_satb_countdown));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_byr_latched));
	save_item(NAME(m_bxr_latched));
	save_item(NAME(m_bat_address));
	save_item(NAME(m_bat_address_mask));
	save_item(NAME(m_bat_row));
	save_item(NAME(m_bat_column));
	save_item(NAME(m_bat_tile_row));
	save_item(NAME(m_sat));
	save_item(NAME(m_sprites_this_line));
	save_item(NAME(m_sprite_row_index));
	save_item(NAME(m_sprite_row));
}


void huc6270_device::device_reset()
{
	m_mawr = 0;
	m_marr = 0;
	m_vrr = 0;
	m_vwr = 0;
	m_cr = 0;
	m_rcr = 0;
	m_bxr = 0;
	m_byr = 0;
	m_mwr = 0;
	m_hsr = 0x0202;     /* Take some defaults for horizontal timing */
	m_hdr = 0x041f;
	m_vpr = 0x0f02;     /* Take some defaults for vertical timing */
	m_vdw = 0x00ef;
	m_vcr = 0x0004;
	m_dcr = 0;
	m_sour = 0;
	m_lenr = 0;
	m_dvssr = 0;
	m_status = 0;
	m_vd_triggered = false;
	m_dvssr_written = false;
	m_satb_countdown = 0;
	m_raster_count = 0x4000;
	m_vert_to_go = 0;
	m_vert_state = v_state::VSW;
	m_horz_steps = 0;
	m_horz_to_go = 0;
	m_horz_state = h_state::HDS;
	m_hsync = false;
	m_vsync = false;
	m_dma_enabled = false;
	m_byr_latched = 0;

	memset(m_sat, 0, sizeof(m_sat));
}
