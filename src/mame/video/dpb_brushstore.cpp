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
	, m_pal_addr(0)
	, m_pal_data(0)
	, m_addr(0)
	, m_a0_chr(0)
	, m_data(0)
	, m_is_read(false)
	, m_is_write(false)
	, m_lumen(false)
	, m_chren(false)
	, m_ca0(false)
	, m_ksel(false)
	, m_fcs(false)
	, m_func(0)
	, m_b_bus_a(false)
	, m_data_in(false)
	, m_fast_wipe(false)
	, m_store_write(false)
	, m_oe_brush(false)
	, m_brush_write(false)
	, m_store_write_out(*this)
	, m_data_out{{*this}, {*this}, {*this}}
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
	save_item(NAME(m_pal_addr));
	save_item(NAME(m_pal_data));

	save_item(NAME(m_addr));
	save_item(NAME(m_a0_chr));
	save_item(NAME(m_data));

	save_item(NAME(m_is_read));
	save_item(NAME(m_is_write));

	save_item(NAME(m_ras));
	save_item(NAME(m_cas));

	save_pointer(NAME(m_rav), STRIPE_COUNT);
	save_pointer(NAME(m_cav), STRIPE_COUNT);

	save_item(NAME(m_lumen));
	save_item(NAME(m_chren));

	save_item(NAME(m_ca0));

	save_item(NAME(m_ksel));
	save_item(NAME(m_fcs));
	save_item(NAME(m_func));
	save_item(NAME(m_b_bus_a));

	save_item(NAME(m_data_in));
	save_item(NAME(m_fast_wipe));
	save_item(NAME(m_store_write));
	save_item(NAME(m_oe));
	save_item(NAME(m_oe_brush));
	save_item(NAME(m_brush_write));

	save_item(NAME(m_write_enable));
	save_item(NAME(m_brush_latches));
	save_item(NAME(m_input_latches));

	m_store_write_out.resolve_safe();

	m_pal_base = m_pal->base();

	for (size_t i = 0; i < STRIPE_COUNT; i++)
	{
		m_data_out[i].resolve_safe();
		m_stripes[i] = make_unique_clear<uint8_t[]>(0x10000);
		save_pointer(NAME(m_stripes[i]), 0x10000, i);
	}
}

void dpb7000_brush_store_card_device::device_reset()
{
	m_pal_addr = 0;
	m_pal_data = 0;

	m_addr = 0;
	m_a0_chr = 0;
	m_data = 0;

	m_is_read = false;
	m_is_write = false;

	m_ras = false;
	m_cas = false;

	memset(m_rav, 0, STRIPE_COUNT);
	memset(m_cav, 0, STRIPE_COUNT);

	m_lumen = false;
	m_chren = false;

	m_ca0 = false;

	m_ksel = false;
	m_fcs = false;
	m_func = 0;
	m_b_bus_a = false;

	m_data_in = false;
	m_fast_wipe = false;
	m_store_write = false;
	memset(m_oe, 0, STRIPE_COUNT);
	m_oe_brush = false;
	m_brush_write = false;

	memset(m_write_enable, 0, STRIPE_COUNT);
	memset(m_brush_latches, 0, STRIPE_COUNT);
	memset(m_input_latches, 0, STRIPE_COUNT);

	for (size_t i = 0; i < STRIPE_COUNT; i++)
	{
		memset(&m_stripes[i][0], 0, 0x10000);
	}
}

uint16_t dpb7000_brush_store_card_device::read()
{
	m_is_read = true;
	m_is_write = false;
	uint8_t msb = m_brush_latches[STRIPE_CHR];
	uint8_t lsb = m_oe[STRIPE_K] ? m_brush_latches[STRIPE_K] : (m_oe[STRIPE_LUM] ? m_brush_latches[STRIPE_LUM] : 0);
	return (msb << 8) | lsb;
}

void dpb7000_brush_store_card_device::write(uint16_t data)
{
	m_is_read = false;
	m_is_write = true;
	m_data = data;
	update_input_latches();
	m_is_write = false;
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
	const bool old = m_ras;
	m_ras = (bool)state;
	if (old && !m_ras)
	{
		m_rav[STRIPE_CHR] = (m_addr & ~1) | m_a0_chr;
		m_rav[STRIPE_K] = m_addr;
		m_rav[STRIPE_LUM] = m_addr;
	}
}

void dpb7000_brush_store_card_device::cas_w(int state)
{
	const bool old = m_cas;
	m_cas = (bool)state;
	if (old && !m_cas)
	{
		m_cav[STRIPE_CHR] = (m_addr & ~1) | m_a0_chr;
		m_cav[STRIPE_K] = m_addr;
		m_cav[STRIPE_LUM] = m_addr;

		for (size_t i = 0; i < STRIPE_COUNT; i++)
		{
			const uint8_t addr = (m_rav[i] << 8) | m_cav[i];
			if (m_oe_brush && !m_write_enable[i])
			{
				m_brush_latches[i] = m_stripes[i][addr];
				m_data_out[i](m_brush_latches[i]);
			}
			else if (m_write_enable[i])
			{
				switch (i)
				{
				case STRIPE_CHR:
					m_stripes[i][addr] = m_oe[i] ? m_brush_latches[i] :
						((m_fast_wipe && m_ca0) ? m_input_latches[STRIPE_K] : m_input_latches[i]);
					break;
				case STRIPE_LUM:
					m_stripes[i][addr] = m_oe[i] ? m_brush_latches[i] : m_input_latches[i];
					break;
				case STRIPE_K:
					m_stripes[i][addr] = m_oe[STRIPE_LUM] ? m_brush_latches[STRIPE_LUM] : m_input_latches[STRIPE_LUM];
					break;
				}
			}
		}
	}
}


void dpb7000_brush_store_card_device::lumen_w(int state)
{
	const bool old = m_lumen;
	m_lumen = (bool)state;
	if (old != m_lumen)
	{
		update_write_enables();
	}
}

void dpb7000_brush_store_card_device::chren_w(int state)
{
	const bool old = m_chren;
	m_chren = (bool)state;
	if (old != m_lumen)
	{
		update_write_enables();
	}
}


void dpb7000_brush_store_card_device::ca0_w(int state)
{
	m_ca0 = (bool)state;
}

void dpb7000_brush_store_card_device::ksel_w(int state)
{
	const bool old = m_ksel;
	m_ksel = (bool)state;
	if (old != m_ksel)
	{
		update_pal_addr();
		update_write_enables();
	}
}

void dpb7000_brush_store_card_device::fcs_w(int state)
{
	const bool old = m_fcs;
	m_fcs = (bool)state;
	if (old != m_fcs)
	{
		update_pal_addr();
	}
}

void dpb7000_brush_store_card_device::func_w(uint8_t data)
{
	const uint8_t old = m_func;
	m_func = data;
	if (old != m_func)
	{
		update_pal_addr();
	}
}

void dpb7000_brush_store_card_device::b_bus_a_w(int state)
{
	const bool old = m_b_bus_a;
	m_b_bus_a = (bool)state;
	if (old != m_b_bus_a)
	{
		update_pal_addr();
	}
}

void dpb7000_brush_store_card_device::update_pal_addr()
{
	const uint8_t old = m_pal_addr;
	m_pal_addr = m_func;
	m_pal_addr |= m_ksel    ? 0x10 : 0x00;
	m_pal_addr |= m_b_bus_a ? 0x20 : 0x00;
	m_pal_addr |= m_fcs     ? 0x40 : 0x00;
	if (old != m_pal_addr)
	{
		update_pal_output();
	}
}

void dpb7000_brush_store_card_device::update_pal_output()
{
	const uint8_t old = m_pal_data;
	m_pal_data = m_pal_base[m_pal_addr];
	if (old != m_pal_data)
	{
		data_in_w(BIT(m_pal_data, 0));
		fast_wipe_w(BIT(m_pal_data, 1));
		store_write_w(BIT(m_pal_data, 2));
		oe_k_w(BIT(m_pal_data, 3));
		oe_chr_w(BIT(m_pal_data, 4));
		oe_lum_w(BIT(m_pal_data, 5));
		oe_brush_w(BIT(m_pal_data, 6));
		brush_write_w(BIT(m_pal_data, 7));
	}
}

void dpb7000_brush_store_card_device::data_in_w(bool state)
{
	m_data_in = state;
}

void dpb7000_brush_store_card_device::fast_wipe_w(bool state)
{
	m_fast_wipe = state;
}

void dpb7000_brush_store_card_device::store_write_w(bool state)
{
	const bool old = m_store_write;
	m_store_write = state;
	if (old != m_store_write)
	{
		m_store_write_out(!m_store_write);
	}
}

void dpb7000_brush_store_card_device::oe_k_w(bool state)
{
	m_oe[STRIPE_K] = state;
}

void dpb7000_brush_store_card_device::oe_chr_w(bool state)
{
	m_oe[STRIPE_CHR] = state;
}

void dpb7000_brush_store_card_device::oe_lum_w(bool state)
{
	m_oe[STRIPE_LUM] = state;
}

void dpb7000_brush_store_card_device::oe_brush_w(bool state)
{
	m_oe_brush = state;
}

void dpb7000_brush_store_card_device::brush_write_w(bool state)
{
	m_brush_write = state;
	update_write_enables();
}

void dpb7000_brush_store_card_device::update_write_enables()
{
	m_write_enable[STRIPE_CHR] = m_chren && m_brush_write && !m_ksel;
	m_write_enable[STRIPE_LUM] = m_lumen && m_brush_write && !m_ksel;
	m_write_enable[STRIPE_K]   = m_lumen && m_brush_write &&  m_ksel;
}

void dpb7000_brush_store_card_device::update_input_latches()
{
	if (m_is_write && m_data_in)
	{
		m_input_latches[STRIPE_LUM] = (uint8_t)m_data;
		if (!m_ksel)
		{
			m_input_latches[STRIPE_CHR] = (uint8_t)(m_data >> 8);
		}
		else
		{
			m_input_latches[STRIPE_K] = (uint8_t)(m_data >> 8);
		}
	}
}
