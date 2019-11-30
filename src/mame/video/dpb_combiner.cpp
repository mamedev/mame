// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_combiner.cpp
    DPB-7000/1 - Combiner Card

***************************************************************************/

#include "emu.h"
#include "dpb_combiner.h"

#define VERBOSE (1)
#include "logmacro.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(DPB7000_COMBINER, dpb7000_combiner_card_device, "dpb_combiner", "Quantel DPB-7000 Combiner Card")


//-------------------------------------------------
//  dpb7000_combiner_card_device - constructor
//-------------------------------------------------

dpb7000_combiner_card_device::dpb7000_combiner_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DPB7000_COMBINER, tag, owner, clock)
	, m_latched_lum_sum(0)
	, m_latched_chr_sum(0)
	, m_lum_out(0)
	, m_chr_out(0)
	, m_chr_i_in(false)
	, m_chr_i(false)
	, m_palette_l(false)
	, m_cursor_enb(false)
	, m_cursor_col(false)
	, m_fsck(false)
	, m_cursor_y(0)
	, m_cursor_u(0)
	, m_cursor_v(0)
	, m_invert_mask(0)
	, m_lum(*this)
	, m_chr(*this)
	, m_fsck_timer(nullptr)
	, m_screen(*this, "screen")
	, m_mult_ge(*this, "mult_ge") // Lum I
	, m_mult_gd(*this, "mult_gd") // Lum II
	, m_mult_gc(*this, "mult_gc") // Chroma I
	, m_mult_gb(*this, "mult_gb") // Chroma II
	, m_mult_ga(*this, "mult_ga") // Ext I & II
{
}

void dpb7000_combiner_card_device::device_start()
{
	save_item(NAME(m_lum_in));
	save_item(NAME(m_latched_lum));
	save_item(NAME(m_selected_lum));
	save_item(NAME(m_chr_in));
	save_item(NAME(m_latched_chr));
	save_item(NAME(m_selected_chr));
	save_item(NAME(m_ext_in));

	save_item(NAME(m_blank));
	save_item(NAME(m_chr_i_in));
	save_item(NAME(m_chr_i));
	save_item(NAME(m_palette_l));
	save_item(NAME(m_cursor_enb));
	save_item(NAME(m_cursor_col));
	save_item(NAME(m_fsck));

	save_item(NAME(m_cursor_y));
	save_item(NAME(m_cursor_u));
	save_item(NAME(m_cursor_v));
	save_item(NAME(m_invert_mask));
	save_item(NAME(m_select_matte));
	save_item(NAME(m_matte_ext));
	save_item(NAME(m_matte_y));
	save_item(NAME(m_matte_u));
	save_item(NAME(m_matte_v));

	save_item(NAME(m_blank_or_suppress));
	save_item(NAME(m_output_matte_y));
	save_item(NAME(m_output_matte_u));
	save_item(NAME(m_output_matte_v));
	save_item(NAME(m_output_matte_ext));

	m_lum.resolve_safe();
	m_chr.resolve_safe();

	m_fsck_timer = timer_alloc(FSCK_TIMER);
	m_fsck_timer->adjust(attotime::never);
}

void dpb7000_combiner_card_device::device_reset()
{
	memset(m_lum_in, 0, 2);
	memset(m_latched_lum, 0, 2);
	memset(m_selected_lum, 0, 2);
	memset(m_chr_in, 0, 2);
	memset(m_latched_chr, 0, 2);
	memset(m_selected_lum, 0, 2);
	memset(m_ext_in, 0, 2);

	memset(m_blank, 0, 2);
	m_chr_i_in = false;
	m_chr_i = false;
	m_palette_l = false;
	m_cursor_enb = false;
	m_cursor_col = false;
	m_fsck = false;

	memset(m_blank_or_suppress, 0, 2);
	memset(m_output_matte_y, 0, 2);
	memset(m_output_matte_u, 0, 2);
	memset(m_output_matte_v, 0, 2);
	memset(m_output_matte_ext, 0, 2);

	m_cursor_y = 0;
	m_cursor_u = 0;
	m_cursor_v = 0;
	m_invert_mask = 0;
	memset(m_select_matte, 0, 2);
	memset(m_matte_ext, 0, 2);
	memset(m_matte_y, 0, 2);
	memset(m_matte_u, 0, 2);
	memset(m_matte_v, 0, 2);

	m_fsck_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}

void dpb7000_combiner_card_device::device_add_mconfig(machine_config &config)
{
	TMC28KU(config, m_mult_ge);
	TMC28KU(config, m_mult_gd);
	TMC28KU(config, m_mult_gc);
	TMC28KU(config, m_mult_gb);
	TMC28KU(config, m_mult_ga);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DERIVED_CLOCK(1, 1), 910, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(dpb7000_combiner_card_device::screen_update));
}

void dpb7000_combiner_card_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == FSCK_TIMER)
	{
		m_fsck = !m_fsck;
		if (m_fsck)
		{
			fsck_tick();
		}
	}
}

void dpb7000_combiner_card_device::fsck_tick()
{
	for (int i = 0; i < 2; i++)
	{
		m_latched_lum[i] = m_lum_in[i];
		m_latched_chr[i] = m_chr_in[i];
		m_selected_lum[i] = m_output_matte_y[i] ? m_matte_y[i] : m_latched_lum[i];
		m_selected_chr[i] = m_output_matte_u[i] ? m_matte_u[i] : (m_output_matte_v[i] ? m_matte_v[i] : m_latched_chr[i]);
		m_selected_ext[i] = m_output_matte_ext[i] ? m_matte_ext[i] : m_ext_in[i];
	}

	// MPY-8HUJ GA
	const uint8_t ext_product = m_palette_l ? (((uint16_t)m_selected_ext[0] * (uint16_t)m_selected_ext[1] + 0x80) >> 8) : 0xff;

	// 74LS175 FF and 74S157 FG
	const bool y1 = (bool)(m_palette_l ? BIT(m_invert_mask, 3) : BIT(m_invert_mask, 4));
	const bool y2 = !y1;

	// 74S86 EE/EF/FD/FE
	const uint8_t a = ext_product ^ (y1 ? 0xff : 0x00);
	const uint8_t b = ext_product ^ (y2 ? 0xff : 0x00);

	// MPY-8HUJ GE
	const uint8_t lum1_product = ((uint16_t)m_selected_lum[0] * (uint16_t)a + 0x80) >> 8;

	// MPY-8HUJ GD
	const uint8_t lum2_product = ((uint16_t)m_selected_lum[1] * (uint16_t)b + 0x80) >> 8;

	// MPY-8HUJ GC
	const uint8_t chr1_product = ((uint16_t)m_selected_chr[0] * (uint16_t)a + 0x80) >> 8;

	// MPY-8HUJ GB
	const uint8_t chr2_product = ((uint16_t)m_selected_chr[1] * (uint16_t)b + 0x80) >> 8;

	// 74S283 FC/ED, 74S374 DD
	m_latched_lum_sum = lum1_product + lum2_product;

	// 74S283 FB/EC, 74S374 DC
	m_latched_chr_sum = chr1_product + chr2_product;

	m_lum_out = m_cursor_enb ? (m_cursor_col ? (m_chr_i ? m_cursor_u : m_cursor_v) : 0x7f) : m_latched_lum_sum;
	m_chr_out = m_cursor_enb ? (m_cursor_col ? m_cursor_y : 0x7f) : m_latched_chr_sum;

	m_lum(m_lum_out);
	m_chr(m_chr_out);

	m_screen->update_now();
}

uint32_t dpb7000_combiner_card_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
		for (int x = cliprect.min_x; x <= cliprect.max_x && x < 256; x++)
		{
			*dest++ = 0xff000000 | (m_lum_out << 16) | (m_lum_out << 8) | m_lum_out;
		}
	}
	return 0;
}

void dpb7000_combiner_card_device::reg_w(uint16_t data)
{
	static const char* const s_const_names[16] = { "Y0", "CY", "CU", "CV", "ES", "EI", "EII", "Y7", "IS", "IY", "IU", "IV", "IIS", "IIY", "IIU", "IIV" };
	LOG("%s: CPU write to Combiner Card: %s = %02x\n", machine().describe_context(), s_const_names[(data >> 10) & 0xf], (uint8_t)data);
	switch ((data >> 10) & 0xf)
	{
		case 1: // CY
			m_cursor_y = (uint8_t)data;
			break;
		case 2: // CU
			m_cursor_u = (uint8_t)data;
			break;
		case 3: // CV
			m_cursor_v = (uint8_t)data;
			break;
		case 4: // ES
			m_invert_mask = (uint8_t)data;
			update_matte_selects();
			break;
		case 5: // EI
			m_matte_ext[0] = (uint8_t)data;
			break;
		case 6: // EII
			m_matte_ext[1] = (uint8_t)data;
			break;
		case 8: // IS
			m_select_matte[0] = BIT(data, 0);
			update_matte_selects();
			break;
		case 9: // IY
			m_matte_y[0] = (uint8_t)data;
			break;
		case 10: // IU
			m_matte_u[0] = (uint8_t)data;
			break;
		case 11: // IV
			m_matte_v[0] = (uint8_t)data;
			break;
		case 12: // IIS
			m_select_matte[1] = BIT(data, 0);
			update_matte_selects();
			break;
		case 13: // IIY
			m_matte_y[1] = (uint8_t)data;
			break;
		case 14: // IIU
			m_matte_u[1] = (uint8_t)data;
			break;
		case 15: // IIV
			m_matte_v[1] = (uint8_t)data;
			break;
	}
}

void dpb7000_combiner_card_device::lum1_w(uint8_t data)
{
	m_lum_in[0] = data;
}

void dpb7000_combiner_card_device::lum2_w(uint8_t data)
{
	m_lum_in[1] = data;
}

void dpb7000_combiner_card_device::blank1(int state)
{
	m_blank[0] = (bool)state;
	update_matte_selects();
}

void dpb7000_combiner_card_device::blank2(int state)
{
	m_blank[1] = (bool)state;
	update_matte_selects();
}

void dpb7000_combiner_card_device::chr1_w(uint8_t data)
{
	m_chr_in[0] = data;
}

void dpb7000_combiner_card_device::chr2_w(uint8_t data)
{
	m_chr_in[1] = data;
}

void dpb7000_combiner_card_device::chr_flag_w(int state)
{
	m_chr_i_in = (bool)state;
	update_matte_selects();
}

void dpb7000_combiner_card_device::ext1_w(uint8_t data)
{
	m_ext_in[0] = BIT(m_invert_mask, 4) ? (data ^ 0xff) : data;
}

void dpb7000_combiner_card_device::ext2_w(uint8_t data)
{
	m_ext_in[1] = BIT(m_invert_mask, 4) ? (data ^ 0xff) : data;
}

void dpb7000_combiner_card_device::palette_l_w(int state)
{
	m_palette_l = (bool)state;
	update_matte_selects();
}

void dpb7000_combiner_card_device::cursor_enb_w(int state)
{
	m_cursor_enb = (bool)state;
}

void dpb7000_combiner_card_device::cursor_col_w(int state)
{
	m_cursor_col = (bool)state;
}

void dpb7000_combiner_card_device::update_matte_selects()
{
	for (int i = 0; i < 2; i++)
	{
		m_blank_or_suppress[i] = m_select_matte[i] || m_blank[i];
		m_output_matte_y[i] = m_blank_or_suppress[i] && m_palette_l;
		m_output_matte_u[i] = m_output_matte_y[i] && m_chr_i;
		m_output_matte_v[i] = m_output_matte_y[i] && !m_chr_i;
		m_output_matte_ext[i] = !BIT(m_invert_mask, i) && m_blank[i];
	}
}
