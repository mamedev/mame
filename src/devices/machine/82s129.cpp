// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    82S129/6 1K-bit TTL bipolar PROM

*****************************************************************************/

#include "emu.h"
#include "82s129.h"

const uint32_t prom82s129_base_device::PROM_SIZE = 256;

DEFINE_DEVICE_TYPE(PROM82S126, prom82s126_device, "82s126", "82S126 1K-bit bipolar PROM")
DEFINE_DEVICE_TYPE(PROM82S129, prom82s129_device, "82s129", "82S129 1K-bit bipolar PROM")

prom82s129_base_device::prom82s129_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_region(*this, DEVICE_SELF)
	, m_out_func(*this)
	, m_o1_func(*this)
	, m_o2_func(*this)
	, m_o3_func(*this)
	, m_o4_func(*this)
	, m_ce1(0)
	, m_ce2(0)
	, m_a(0)
	, m_out(0)
{
}

prom82s126_device::prom82s126_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: prom82s129_base_device(mconfig, PROM82S126, tag, owner, clock)
{
}

prom82s129_device::prom82s129_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: prom82s129_base_device(mconfig, PROM82S129, tag, owner, clock)
{
}

void prom82s129_base_device::device_start()
{
	init();

	m_data = std::make_unique<uint8_t[]>(PROM_SIZE);
	memcpy(&m_data[0], m_region->base(), PROM_SIZE);

	save_item(NAME(m_ce1));
	save_item(NAME(m_ce2));
	save_item(NAME(m_a));
	save_item(NAME(m_out));

	m_out_func.resolve_safe();
	m_o1_func.resolve_safe();
	m_o2_func.resolve_safe();
	m_o3_func.resolve_safe();
	m_o4_func.resolve_safe();
}

void prom82s129_base_device::device_reset()
{
	init();
}

void prom82s129_base_device::init()
{
	m_ce1 = 0;
	m_ce2 = 0;
	m_a = 0;
	m_out = 0;
}

void prom82s129_base_device::update()
{
	uint8_t last_out = m_out;

	if (m_ce1 == 0 && m_ce2 == 0)
		m_out = m_data[m_a];
	else
		m_out = 0;

	if (m_out != last_out)
	{
		m_out_func(m_out);

		for (int bit = 0; bit < 4; bit++)
		{
			if (BIT(m_out, bit) == BIT(last_out, bit))
				continue;

			switch(bit)
			{
				case 0: m_o1_func(BIT(m_out, bit)); break;
				case 1: m_o2_func(BIT(m_out, bit)); break;
				case 2: m_o3_func(BIT(m_out, bit)); break;
				case 3: m_o4_func(BIT(m_out, bit)); break;
			}
		}
	}
}

WRITE_LINE_MEMBER( prom82s129_base_device::ce1_w )
{
	uint8_t last_ce1 = m_ce1;
	m_ce1 = state;
	if (last_ce1 != m_ce1)
		update();
}

WRITE_LINE_MEMBER( prom82s129_base_device::ce2_w )
{
	uint8_t last_ce2 = m_ce2;
	m_ce2 = state;
	if (last_ce2 != m_ce2)
		update();
}

void prom82s129_base_device::a_w(uint8_t data)
{
	uint8_t last_a = m_a;
	m_a = data;
	if (last_a != m_a)
		update();
}

void prom82s129_base_device::write_line(uint8_t line, uint8_t state)
{
	uint8_t last_a = m_a;
	m_a &= ~(1 << line);
	m_a |= (state << line);
	if (last_a != m_a)
		update();
}

WRITE_LINE_MEMBER( prom82s129_base_device::a0_w ) { write_line(0, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a1_w ) { write_line(1, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a2_w ) { write_line(2, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a3_w ) { write_line(3, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a4_w ) { write_line(4, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a5_w ) { write_line(5, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a6_w ) { write_line(6, state); }
WRITE_LINE_MEMBER( prom82s129_base_device::a7_w ) { write_line(7, state); }

uint8_t prom82s129_base_device::output_r()
{
	return m_out;
}

READ_LINE_MEMBER( prom82s129_base_device::o1_r ) { return BIT(m_out, 0); }
READ_LINE_MEMBER( prom82s129_base_device::o2_r ) { return BIT(m_out, 1); }
READ_LINE_MEMBER( prom82s129_base_device::o3_r ) { return BIT(m_out, 2); }
READ_LINE_MEMBER( prom82s129_base_device::o4_r ) { return BIT(m_out, 3); }
