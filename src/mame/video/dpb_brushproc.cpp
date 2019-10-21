// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_brushproc.cpp
    DPB-7000/1 - Brush Processor Card

***************************************************************************/

#include "emu.h"
#include "dpb_brushproc.h"

#define VERBOSE (1)
#include "logmacro.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(DPB7000_BRUSHPROC, dpb7000_brushproc_card_device, "dpb_brushproc", "Quantel DPB-7000 Brush Processor Card")


//-------------------------------------------------
//  dpb7000_brushproc_card_device - constructor
//-------------------------------------------------

dpb7000_brushproc_card_device::dpb7000_brushproc_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DPB7000_BRUSHPROC, tag, owner, clock)
	, m_ext_in(0)
	, m_brush_in(0)
	, m_cbus_in(0)
	, m_k_in(0)
	, m_k_enable(0)
	, m_k_zero(false)
	, m_k_invert(false)
	, m_k_product(0)
	, m_ext_product(0)
	, m_final_product(0)
	, m_final_result(0)
	, m_brush_value(0)
	, m_final_brush_value(0)
	, m_compare_b_value(0)
	, m_func(0)
	, m_subtract_result(false)
	, m_sel_eh(false)
	, m_b_bus_ah(false)
	, m_fcs(false)
	, m_use_store1_for_brush(false)
	, m_use_store2_for_brush(false)
	, m_use_ext_for_brush(false)
	, m_use_store1_or_ext_for_brush(false)
	, m_use_store2_for_store1(false)
	, m_enable_store_ext_multiplicand(false)
	, m_output_16bit(false)
	, m_pal_proc_in(false)
	, m_disable_k_data(false)
	, m_prom_addr(0)
	, m_prom_base(nullptr)
	, m_prom_out(0)
	, m_pal_in(0)
	, m_pal_base(nullptr)
	, m_pal_data_out(0)
	, m_pal_bpinv_out(false)
	, m_pal_sel_out(false)
	, m_store1(*this)
	, m_store2(*this)
	, m_mult_fa(*this, "mult_fa")
	, m_mult_ga(*this, "mult_ga")
	, m_mult_gd(*this, "mult_gd")
	, m_alu_he(*this, "alu_he")
	, m_alu_ge(*this, "alu_ge")
	, m_alu_fe(*this, "alu_fe")
	, m_alu_ee(*this, "alu_ee")
	, m_prom(*this, "prom")
	, m_pal(*this, "pal")
{
}

void dpb7000_brushproc_card_device::device_start()
{
	save_item(NAME(m_store_in));
	save_item(NAME(m_store_out));
	save_item(NAME(m_ext_in));
	save_item(NAME(m_brush_in));
	save_item(NAME(m_cbus_in));

	save_item(NAME(m_k_in));
	save_item(NAME(m_k_enable));
	save_item(NAME(m_k_zero));
	save_item(NAME(m_k_invert));

	save_item(NAME(m_k_product));
	save_item(NAME(m_ext_product));
	save_item(NAME(m_final_product));
	save_item(NAME(m_final_result));

	save_item(NAME(m_brush_value));
	save_item(NAME(m_final_brush_value));
	save_item(NAME(m_compare_b_value));

	save_item(NAME(m_func));

	save_item(NAME(m_subtract_result));

	save_item(NAME(m_sel_luma));
	save_item(NAME(m_sel_eh));
	save_item(NAME(m_b_bus_ah));
	save_item(NAME(m_fcs));

	save_item(NAME(m_oe));
	save_item(NAME(m_use_store1_for_brush));
	save_item(NAME(m_use_store2_for_brush));
	save_item(NAME(m_use_ext_for_brush));
	save_item(NAME(m_use_store1_or_ext_for_brush));
	save_item(NAME(m_use_store2_for_store1));
	save_item(NAME(m_enable_store_ext_multiplicand));
	save_item(NAME(m_output_16bit));
	save_item(NAME(m_pal_proc_in));
	save_item(NAME(m_disable_k_data));

	save_item(NAME(m_prom_addr));
	save_item(NAME(m_prom_out));

	save_item(NAME(m_pal_in));
	save_item(NAME(m_pal_data_out));
	save_item(NAME(m_pal_bpinv_out));
	save_item(NAME(m_pal_sel_out));

	m_store1.resolve_safe();
	m_store2.resolve_safe();
}

void dpb7000_brushproc_card_device::device_reset()
{
	memset(m_store_in, 0, 2);
	memset(m_store_out, 0, 2);
	m_ext_in = 0;
	m_brush_in = 0;
	m_cbus_in = 0;

	m_k_in = 0;
	m_k_enable = false;
	m_k_zero = false;
	m_k_invert = false;

	m_k_product = 0;
	m_ext_product = 0;
	m_final_product = 0;
	m_final_result = 0;

	m_brush_value = 0;
	m_final_brush_value = 0;
	m_compare_b_value = 0;

	m_func = 0;

	m_subtract_result = false;

	memset(m_sel_luma, 0, 2);
	m_sel_eh = false;
	m_b_bus_ah = false;
	m_fcs = false;

	memset(m_oe, 0, 4);
	m_use_store1_for_brush = false;
	m_use_store2_for_brush = false;
	m_use_ext_for_brush = false;
	m_use_store1_or_ext_for_brush = false;
	m_use_store2_for_store1 = false;
	m_enable_store_ext_multiplicand = false;
	m_output_16bit = false;
	m_pal_proc_in = false;
	m_disable_k_data = false;

	m_prom_addr = 0;
	m_prom_base = m_prom->base();
	m_prom_out = 0;

	m_pal_in = 0;
	m_pal_base = m_pal->base();
	m_pal_data_out = 0;
	m_pal_bpinv_out = false;
	m_pal_sel_out = false;

	m_mult_fa->xm_w(0);
	m_mult_fa->ym_w(0);
	m_mult_fa->rs_w(0);
	m_mult_fa->ru_w(1);

	m_mult_ga->xm_w(0);
	m_mult_ga->ym_w(0);
	m_mult_ga->rs_w(0);
	m_mult_ga->ru_w(1);

	m_mult_gd->xm_w(0);
	m_mult_gd->ym_w(0);
	m_mult_gd->rs_w(0);
	m_mult_gd->ru_w(1);
}

void dpb7000_brushproc_card_device::device_add_mconfig(machine_config &config)
{
	AM25S558(config, m_mult_fa);
	AM25S558(config, m_mult_ga);
	AM25S558(config, m_mult_gd);

	SN74S381(config, m_alu_he);
	SN74S381(config, m_alu_ge);
	SN74S381(config, m_alu_fe);
	SN74S381(config, m_alu_ee);
}

ROM_START( dpb7000_brushproc )
	ROM_REGION(0x200, "prom", 0)
	ROM_LOAD("pb-02c-17593-baa.bin", 0x000, 0x200, CRC(a74cc1f5) SHA1(3b789d5a29c70c93dec56f44be8c14b41915bdef))

	ROM_REGION16_BE(0x800, "pal", 0)
	ROMX_LOAD("pb-02c-17593-hba.bin", 0x000, 0x800, CRC(76018e4f) SHA1(73d995e2e78410676061d45857756d5305a9984a), ROM_GROUPWORD)
ROM_END

const tiny_rom_entry *dpb7000_brushproc_card_device::device_rom_region() const
{
	return ROM_NAME( dpb7000_brushproc );
}

void dpb7000_brushproc_card_device::store1_w(uint8_t data)
{
	m_store_in[0] = data;
}

void dpb7000_brushproc_card_device::store2_w(uint8_t data)
{
	m_store_in[1] = data;
}

void dpb7000_brushproc_card_device::ext_w(uint8_t data)
{
	m_ext_in = data;
}

void dpb7000_brushproc_card_device::brush_w(uint8_t data)
{
	m_brush_in = data;
}

void dpb7000_brushproc_card_device::cbus_w(uint8_t data)
{
	m_cbus_in = data;
	update_k_product();
}

void dpb7000_brushproc_card_device::k_w(uint8_t data)
{
	m_k_in = data;
	if (m_k_enable && !m_disable_k_data)
		update_k_product();
}

void dpb7000_brushproc_card_device::k_en_w(int state)
{
	const bool old = m_k_enable;
	m_k_enable = (bool)state;
	if (!old && m_k_enable && !m_disable_k_data)
		update_k_product();
}

void dpb7000_brushproc_card_device::k_zero_w(int state)
{
	const bool old = m_k_zero;
	m_k_zero = (bool)state;
	if (old != m_k_zero)
		update_ext_product();
}

void dpb7000_brushproc_card_device::k_inv_w(int state)
{
	const bool old = m_k_invert;
	m_k_invert = (bool)state;
	if (old != m_k_invert)
		update_pal_signals();
}

void dpb7000_brushproc_card_device::func_w(uint8_t data)
{
	m_func = data;

	const uint16_t old = m_prom_addr;
	m_prom_addr &= 0x1f0;
	m_prom_addr |= BIT(data, 3);
	m_prom_addr |= BIT(data, 2) << 1;
	m_prom_addr |= BIT(data, 1) << 2;
	m_prom_addr |= BIT(data, 0) << 3;
	if (old != m_prom_addr)
		update_prom_signals();
}

void dpb7000_brushproc_card_device::sel_lum1_w(int state)
{
	m_sel_luma[0] = (bool)state;

	const uint16_t old = m_prom_addr;
	m_prom_addr &= ~0x20;
	m_prom_addr |= state << 5;
	if (old != m_prom_addr)
		update_prom_signals();
}

void dpb7000_brushproc_card_device::sel_lum2_w(int state)
{
	m_sel_luma[1] = (bool)state;

	const uint16_t old = m_prom_addr;
	m_prom_addr &= ~0x10;
	m_prom_addr |= state << 4;
	if (old != m_prom_addr)
		update_prom_signals();
}

void dpb7000_brushproc_card_device::sel_eh_w(int state)
{
	m_sel_eh = (bool)state;

	const uint16_t old = m_prom_addr;
	m_prom_addr &= ~0x40;
	m_prom_addr |= state << 6;
	if (old != m_prom_addr)
		update_prom_signals();
}

void dpb7000_brushproc_card_device::b_bus_ah_w(int state)
{
	m_b_bus_ah = (bool)state;

	const uint16_t old = m_prom_addr;
	m_prom_addr &= ~0x80;
	m_prom_addr |= state << 7;
	if (old != m_prom_addr)
		update_prom_signals();
}

void dpb7000_brushproc_card_device::fixed_col_select_w(int state)
{
	m_fcs = (bool)state;

	const uint16_t old = m_prom_addr;
	m_prom_addr &= ~0x80;
	m_prom_addr |= state << 8;
	if (old != m_prom_addr)
		update_prom_signals();
}

void dpb7000_brushproc_card_device::update_prom_signals()
{
	const uint8_t old = m_prom_out;
	m_prom_out = m_prom_base[m_prom_addr];
	if (BIT(old, 0) != BIT(m_prom_out, 0))
		set_oe1(BIT(m_prom_out, 0));
	if (BIT(old, 1) != BIT(m_prom_out, 1))
		set_oe2(BIT(m_prom_out, 1));
	if (BIT(old, 2) != BIT(m_prom_out, 2))
		set_oe3(BIT(m_prom_out, 2));
	if (BIT(old, 3) != BIT(m_prom_out, 3))
		set_mask_sel_h(BIT(m_prom_out, 3));
	if (BIT(old, 4) != BIT(m_prom_out, 4))
		set_16bit_h(BIT(m_prom_out, 4));
	if (BIT(old, 5) != BIT(m_prom_out, 5))
		set_proc_sel_h(BIT(m_prom_out, 5));
	if (BIT(old, 6) != BIT(m_prom_out, 6))
		set_k_eq_il(BIT(m_prom_out, 6));
	if (BIT(old, 7) != BIT(m_prom_out, 7))
		set_oe4(BIT(m_prom_out, 7));
	update_brush_value();
	update_compare_b_value();
}

void dpb7000_brushproc_card_device::update_pal_signals()
{
	const uint16_t old_data = m_pal_data_out;
	const bool old_bpinv = m_pal_bpinv_out;
	const uint16_t old_addr = m_pal_in;
	m_pal_in = m_ext_product | (m_pal_proc_in ? 0x100 : 0) | (m_k_invert ? 0x200 : 0);
	if (old_addr != m_pal_in)
	{
		const uint16_t shifted_addr = m_pal_in << 1;
		const uint8_t pal_msb = m_pal_base[shifted_addr];
		m_pal_data_out = m_pal_base[shifted_addr + 1];
		m_pal_bpinv_out = BIT(pal_msb, 0);
		m_pal_sel_out = BIT(pal_msb, 1);
		if (old_bpinv != m_pal_bpinv_out)
		{
			m_final_brush_value = m_brush_value ^ (m_pal_bpinv_out ? 0xff : 0x00);
		}
		if (old_data != m_pal_data_out)
		{
			update_final_product();
		}
	}
}

void dpb7000_brushproc_card_device::update_k_product()
{
	const uint16_t x = (uint16_t)m_cbus_in;
	const uint16_t y = (m_disable_k_data || !m_k_enable) ? 0x00ff : (uint16_t)m_k_in;
	const uint8_t old = m_k_product;
	m_k_product = (uint8_t)((x * y + 0x0080) >> 8);
	if (old != m_k_product)
		update_ext_product();
}

void dpb7000_brushproc_card_device::update_ext_product()
{
	const uint16_t y = (uint16_t)m_k_product;
	uint16_t x = 0;
	if (!m_k_zero)
	{
		if (m_enable_store_ext_multiplicand)
			x = (uint16_t)m_ext_in;
		else
			x = 0xff;
	}
	const uint16_t old = m_ext_product;
	m_ext_product = (uint8_t)((x * y + 0x0080) >> 8);
	if (old != m_ext_product)
		update_final_product();
}

void dpb7000_brushproc_card_device::update_final_product()
{
	const uint16_t x = (uint16_t)update_brush_alu_result();
	const uint16_t y = (uint16_t)m_pal_data_out;
	m_final_product = x * y + (m_output_16bit ? 0x80 : 0x00);
	uint16_t a = (m_compare_b_value << 8) | m_store_in[1];
	m_final_result = m_subtract_result ? (a - m_final_product) : (a + m_final_product);
	m_store_out[0] = m_pal_sel_out ? (uint8_t)(m_final_result >> 8) : m_final_brush_value;
	m_store_out[1] = m_output_16bit ? (uint8_t)m_final_result : m_store_out[0];
	m_store1(m_store_out[0]);
	m_store2(m_store_out[1]);
}

uint8_t dpb7000_brushproc_card_device::update_brush_alu_result()
{
	// Handled by a pair of 74S85 comparators (BB, CB), a pair of 74S381 ALUs (CD, DD) and a 74S182 carry lookahead generator (ED).
	const uint8_t a = m_brush_value;
	const uint8_t b = m_compare_b_value;
	if (a <= b)
	{
		m_subtract_result = false;
		return b - a;
	}
	else
	{
		m_subtract_result = true;
		return a - b;
	}
}

void dpb7000_brushproc_card_device::set_oe1(int state)
{
	// When 0, force Store I or Store Ext. data onto Brush Data lanes
	m_oe[0] = (bool)state;
	m_use_store1_or_ext_for_brush = !m_oe[0];
	m_use_store1_for_brush = m_use_store1_or_ext_for_brush && !m_oe[2];
	m_use_ext_for_brush = m_use_store1_or_ext_for_brush && m_oe[2];
}

void dpb7000_brushproc_card_device::set_oe2(int state)
{
	// When 0, force Store II data onto Brush Data lanes
	m_oe[1] = (bool)state;
	m_use_store2_for_brush = !m_oe[1];
}

void dpb7000_brushproc_card_device::set_oe3(int state)
{
	// When 0, force Store Ext. data onto Store I data lanes (disables Store I data input)
	m_oe[2] = (bool)state;
	m_use_store1_for_brush = m_use_store1_or_ext_for_brush && m_oe[2];
	m_use_ext_for_brush = m_use_store1_or_ext_for_brush && !m_oe[2];
}

void dpb7000_brushproc_card_device::update_brush_value()
{
	if (m_use_store1_for_brush)
	{
		m_brush_value = m_store_in[0];
	}
	else if (m_use_store2_for_brush)
	{
		m_brush_value = m_store_in[1];
	}
	else if (m_use_ext_for_brush)
	{
		m_brush_value = m_ext_in;
	}
	else
	{
		m_brush_value = m_brush_in;
	}
}

void dpb7000_brushproc_card_device::update_compare_b_value()
{
	if (m_use_store2_for_store1)
	{
		m_compare_b_value = m_store_in[1];
	}
	else if (m_oe[2])
	{
		m_compare_b_value = m_ext_in;
	}
	else
	{
		m_compare_b_value = m_store_in[0];
	}
}

void dpb7000_brushproc_card_device::set_oe4(int state)
{
	// When 0, force Store II data onto Store I data lanes (disables Store I data input)
	m_oe[3] = (bool)state;
	m_use_store2_for_store1 = !m_oe[3];
}

void dpb7000_brushproc_card_device::set_mask_sel_h(int state)
{
	// When 0, multiplies K product with 1.0 (0xff) instead of Store Ext. data.
	m_enable_store_ext_multiplicand = (bool)state;
}

void dpb7000_brushproc_card_device::set_16bit_h(int state)
{
	// When 0, enables Store II data output, enables Cx carry-out, and enables Ru pin on multiplier GD.
	m_output_16bit = (bool)(1 - state);
}

void dpb7000_brushproc_card_device::set_proc_sel_h(int state)
{
	// When 1, enables the PROC input pin on PAL 20L10 HB, not yet dumped.
	m_pal_proc_in = (bool)state;
	update_pal_signals();
}

void dpb7000_brushproc_card_device::set_k_eq_il(int state)
{
	// When 1, disables K data lanes, which collectively get pulled to 1.0 (0xff).
	m_disable_k_data = (bool)state;
	if (!m_disable_k_data && m_k_enable)
		update_k_product();
}

