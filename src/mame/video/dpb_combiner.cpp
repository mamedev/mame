// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_combiner.cpp
    DPB-7000/1 - Combiner Card

	TODO:
	- Hook up clocked logic (multipliers, blanking, etc.)

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
	, m_chr_i_in(false)
	, m_chr_i(false)
	, m_palette_l(false)
	, m_cursor_enb(false)
	, m_cursor_col(false)
	, m_cursor_luma(0)
	, m_cursor_u(0)
	, m_cursor_v(0)
	, m_invert_mask(0)
	, m_lum(*this)
	, m_chr(*this)
	, m_fsck(nullptr)
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
	save_item(NAME(m_chr_in));
	save_item(NAME(m_latched_chr));
	save_item(NAME(m_ext_in));

	save_item(NAME(m_blank_lum));
	save_item(NAME(m_chr_i_in));
	save_item(NAME(m_chr_i));
	save_item(NAME(m_palette_l));
	save_item(NAME(m_cursor_enb));
	save_item(NAME(m_cursor_col));

	save_item(NAME(m_cursor_luma));
	save_item(NAME(m_cursor_u));
	save_item(NAME(m_cursor_v));
	save_item(NAME(m_invert_mask));
	save_item(NAME(m_select_matte));
	save_item(NAME(m_matte_ext));
	save_item(NAME(m_matte_y));
	save_item(NAME(m_matte_u));
	save_item(NAME(m_matte_v));

	m_lum.resolve_safe();
	m_chr.resolve_safe();
}

void dpb7000_combiner_card_device::device_reset()
{
	memset(m_lum_in, 0, 2);
	memset(m_latched_lum, 0, 2);
	memset(m_chr_in, 0, 2);
	memset(m_latched_chr, 0, 2);
	memset(m_ext_in, 0, 2);

	memset(m_blank_lum, 0, 2);
	m_chr_i_in = false;
	m_chr_i = false;
	m_palette_l = false;
	m_cursor_enb = false;
	m_cursor_col = false;

	m_cursor_luma = 0;
	m_cursor_u = 0;
	m_cursor_v = 0;
	m_invert_mask = 0;
	memset(m_select_matte, 0, 2);
	memset(m_matte_ext, 0, 2);
	memset(m_matte_y, 0, 2);
	memset(m_matte_u, 0, 2);
	memset(m_matte_v, 0, 2);
}

void dpb7000_combiner_card_device::device_add_mconfig(machine_config &config)
{
	TMC28KU(config, m_mult_ge);
	TMC28KU(config, m_mult_gd);
	TMC28KU(config, m_mult_gc);
	TMC28KU(config, m_mult_gb);
	TMC28KU(config, m_mult_ga);
}

void dpb7000_combiner_card_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == FSCK_TIMER)
	{
		fsck_tick();
	}
}

void dpb7000_combiner_card_device::fsck_tick()
{
}

void dpb7000_combiner_card_device::reg_w(uint16_t data)
{
	static const char* const s_const_names[16] = { "Y0", "CY", "CU", "CV", "ES", "EI", "EII", "Y7", "IS", "IY", "IU", "IV", "IIS", "IIY", "IIU", "IIV" };
	LOG("%s: CPU write to Combiner Card: %s = %02x\n", machine().describe_context(), s_const_names[(data >> 10) & 0xf], (uint8_t)data);
	switch ((data >> 10) & 0xf)
	{
		case 1: // CY
			m_cursor_luma = (uint8_t)data;
			break;
		case 2: // CU
			m_cursor_u = (uint8_t)data;
			break;
		case 3: // CV
			m_cursor_v = (uint8_t)data;
			break;
		case 4: // ES
			m_invert_mask = (uint8_t)data;
			break;
		case 5: // EI
			m_matte_ext[0] = (uint8_t)data;
			break;
		case 6: // EII
			m_matte_ext[1] = (uint8_t)data;
			break;
		case 8: // IS
			m_select_matte[0] = BIT(data, 0);
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

void dpb7000_combiner_card_device::blank_lum1(int state)
{
	m_blank_lum[0] = (bool)state;
}

void dpb7000_combiner_card_device::blank_lum2(int state)
{
	m_blank_lum[1] = (bool)state;
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
}

void dpb7000_combiner_card_device::ext1_w(uint8_t data)
{
	m_ext_in[0] = data;
}

void dpb7000_combiner_card_device::ext2_w(uint8_t data)
{
	m_ext_in[1] = data;
}

void dpb7000_combiner_card_device::palette_l_w(int state)
{
	m_palette_l = (bool)state;
}

void dpb7000_combiner_card_device::cursor_enb_w(int state)
{
	m_cursor_enb = (bool)state;
}

void dpb7000_combiner_card_device::cursor_col_w(int state)
{
	m_cursor_col = (bool)state;
}
