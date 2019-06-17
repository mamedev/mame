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
	, m_k_in(0)
	, m_k_enable(0)
	, m_k_zero(false)
	, m_k_invert(false)
	, m_func(0)
	, m_sel_eh(false)
	, m_b_bus_ah(false)
	, m_fcs(false)
	, m_store1(*this)
	, m_store2(*this)
	, m_cbus(*this)
	, m_pck(*this)
	, m_mult_fa(*this, "mult_fa")
	, m_mult_ga(*this, "mult_ga")
	, m_mult_gd(*this, "mult_gd")
{
}

void dpb7000_brushproc_card_device::device_start()
{
	save_item(NAME(m_store_in));
	save_item(NAME(m_ext_in));
	save_item(NAME(m_brush_in));

	save_item(NAME(m_k_in));
	save_item(NAME(m_k_enable));
	save_item(NAME(m_k_zero));
	save_item(NAME(m_k_invert));

	save_item(NAME(m_func));

	save_item(NAME(m_sel_luma));
	save_item(NAME(m_sel_eh));
	save_item(NAME(m_b_bus_ah));
	save_item(NAME(m_fcs));

	m_store1.resolve_safe();
	m_store2.resolve_safe();
	m_cbus.resolve_safe();
	m_pck.resolve_safe();
}

void dpb7000_brushproc_card_device::device_reset()
{
	memset(m_store_in, 0, 2);
	m_ext_in = 0;
	m_brush_in = 0;

	m_k_in = 0;
	m_k_enable = 0;
	m_k_zero = 0;
	m_k_invert = 0;

	m_func = 0;

	memset(m_sel_luma, 0, 2);
	m_sel_eh = 0;
	m_b_bus_ah = 0;
	m_fcs = 0;
}

void dpb7000_brushproc_card_device::device_add_mconfig(machine_config &config)
{
	AM25S558(config, m_mult_fa);
	AM25S558(config, m_mult_ga);
	AM25S558(config, m_mult_gd);
}

ROM_START( dpb7000_brushproc )
	ROM_REGION(0x200, "brushproc_prom", 0)
	ROM_LOAD("pb-02c-17593-baa.bin", 0x000, 0x200, CRC(a74cc1f5) SHA1(3b789d5a29c70c93dec56f44be8c14b41915bdef))
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


void dpb7000_brushproc_card_device::k_w(uint8_t data)
{
	m_k_in = data;
}

void dpb7000_brushproc_card_device::k_en_w(int state)
{
	m_k_enable = (bool)state;
}

void dpb7000_brushproc_card_device::k_zero_w(int state)
{
	m_k_zero = (bool)state;
}

void dpb7000_brushproc_card_device::k_inv_w(int state)
{
	m_k_invert = (bool)state;
}


void dpb7000_brushproc_card_device::func_w(uint8_t data)
{
	m_func = data;
}

void dpb7000_brushproc_card_device::sel_lum1_w(int state)
{
	m_sel_luma[0] = (bool)state;
}

void dpb7000_brushproc_card_device::sel_lum2_w(int state)
{
	m_sel_luma[1] = (bool)state;
}

void dpb7000_brushproc_card_device::sel_eh_w(int state)
{
	m_sel_eh = (bool)state;
}

void dpb7000_brushproc_card_device::b_bus_ah_w(int state)
{
	m_b_bus_ah = (bool)state;
}

void dpb7000_brushproc_card_device::fixed_col_select_w(int state)
{
	m_fcs = (bool)state;
}
