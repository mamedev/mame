// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_brushstore.cpp
    DPB-7000/1 - Brush Store Card

	TODO:
	- Code is currently a more or less direct translation of the board
	  schematic. It is highly inefficient, but accurate. An equally-
	  accurate, but faster, version can be made once better understanding
	  of the overall DPB-7000 system is had.

***************************************************************************/

#include "emu.h"
#include "dpb_brushstore.h"


/*****************************************************************************/

DEFINE_DEVICE_TYPE(DPB7000_BRUSHSTORE, dpb7000_brush_store_card_device, "dpb_brushstore", "Quantel DPB-7000 Brush Store Card")


//-------------------------------------------------
//  dpb7000_brush_store_card_device - constructor
//-------------------------------------------------

dpb7000_brush_store_card_device::dpb7000_brush_store_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DPB7000_BRUSHSTORE, tag, owner, clock)
	, m_pal_base(nullptr)
	, m_addr(0)
	, m_a0_chr(0)
	, m_bck(true)
	, m_lumen(false)
	, m_chren(false)
	, m_ca0(false)
	, m_ksel(false)
	, m_fcs(false)
	, m_func(0)
	, m_b_bus_a(false)
	, m_store_write_out(*this)
	, m_k_data_out(*this)
	, m_lum_data_out(*this)
	, m_chr_data_out(*this)
	, m_pal(*this, "pal")
{
}

ROM_START( dpb7000_brush_store )
	ROM_REGION(0x100, "pal", 0)
	ROM_LOAD("pb-02a-17421-ada.bin", 0x000, 0x100, CRC(84bf7029) SHA1(9d58322994f6f7e99a9c6478577559c8171670ed))
ROM_END

const tiny_rom_entry *dpb7000_brush_store_card_device::device_rom_region() const
{
	return ROM_NAME( dpb7000_brush_store );
}

void dpb7000_brush_store_card_device::device_start()
{
	save_item(NAME(m_addr));
	save_item(NAME(m_a0_chr));

	save_item(NAME(m_ras));
	save_item(NAME(m_cas));

	save_pointer(NAME(m_rav), STRIPE_COUNT);
	save_pointer(NAME(m_cav), STRIPE_COUNT);
	save_item(NAME(m_bck));

	save_item(NAME(m_lumen));
	save_item(NAME(m_chren));

	save_item(NAME(m_ca0));

	save_item(NAME(m_ksel));
	save_item(NAME(m_fcs));
	save_item(NAME(m_func));
	save_item(NAME(m_b_bus_a));

	m_store_write_out.resolve_safe();
	m_k_data_out.resolve_safe();
	m_lum_data_out.resolve_safe();
	m_chr_data_out.resolve_safe();

	m_pal_base = m_pal->base();

	for (size_t i = 0; i < STRIPE_COUNT; i++)
	{
		m_stripes[i] = make_unique_clear<uint8_t[]>(0x10000);
		save_pointer(NAME(m_stripes[i]), 0x10000, i);
	}
}

void dpb7000_brush_store_card_device::device_reset()
{
	m_addr = 0;
	m_a0_chr = 0;

	m_ras = false;
	m_cas = false;

	memset(m_rav, 0, STRIPE_COUNT);
	memset(m_cav, 0, STRIPE_COUNT);
	m_bck = true;

	m_lumen = false;
	m_chren = false;

	m_ca0 = false;

	m_ksel = false;
	m_fcs = false;
	m_func = 0;
	m_b_bus_a = false;

	for (size_t i = 0; i < STRIPE_COUNT; i++)
	{
		memset(&m_stripes[i][0], 0, 0x10000);
	}
}

uint16_t dpb7000_brush_store_card_device::read()
{
	return 0;
}

void dpb7000_brush_store_card_device::write(uint16_t data)
{
}


void dpb7000_brush_store_card_device::addr_w(uint8_t data)
{
	m_addr = data;
}

void dpb7000_brush_store_card_device::a0_chr_w(int state)
{
	m_a0_chr = (uint8_t)state;
}

void dpb7000_brush_store_card_device::ras_w(int state)
{
	m_ras = (bool)state;
}

void dpb7000_brush_store_card_device::cas_w(int state)
{
	m_cas = (bool)state;
}


void dpb7000_brush_store_card_device::lumen_w(int state)
{
	m_lumen = (bool)state;
}

void dpb7000_brush_store_card_device::chren_w(int state)
{
	m_chren = (bool)state;
}


void dpb7000_brush_store_card_device::ca0_w(int state)
{
	m_ca0 = (bool)state;
}

void dpb7000_brush_store_card_device::ksel_w(int state)
{
	m_ksel = (bool)state;
}


void dpb7000_brush_store_card_device::fcs_w(int state)
{
	m_fcs = (bool)state;
}

void dpb7000_brush_store_card_device::func_w(uint8_t data)
{
	m_func = data;
}

void dpb7000_brush_store_card_device::b_bus_a_w(int state)
{
	m_b_bus_a = (bool)state;
}
