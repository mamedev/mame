// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "latch.h"

const device_type OUTPUT_LATCH = &device_creator<output_latch_device>;

output_latch_device::output_latch_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, OUTPUT_LATCH, "Output Latch", tag, owner, clock, "output_latch", __FILE__),
	m_resolved(false),
	m_bit0(-1),
	m_bit1(-1),
	m_bit2(-1),
	m_bit3(-1),
	m_bit4(-1),
	m_bit5(-1),
	m_bit6(-1),
	m_bit7(-1),
	m_bit0_handler(*this),
	m_bit1_handler(*this),
	m_bit2_handler(*this),
	m_bit3_handler(*this),
	m_bit4_handler(*this),
	m_bit5_handler(*this),
	m_bit6_handler(*this),
	m_bit7_handler(*this)
{
}

void output_latch_device::device_start()
{
	save_item(NAME(m_bit0));
	save_item(NAME(m_bit1));
	save_item(NAME(m_bit2));
	save_item(NAME(m_bit3));
	save_item(NAME(m_bit4));
	save_item(NAME(m_bit5));
	save_item(NAME(m_bit6));
	save_item(NAME(m_bit7));
}

void output_latch_device::write(UINT8 data)
{
	if (!m_resolved)
	{
		// HACK: move to device_config_complete() when devcb supports that
		m_bit0_handler.resolve_safe();
		m_bit1_handler.resolve_safe();
		m_bit2_handler.resolve_safe();
		m_bit3_handler.resolve_safe();
		m_bit4_handler.resolve_safe();
		m_bit5_handler.resolve_safe();
		m_bit6_handler.resolve_safe();
		m_bit7_handler.resolve_safe();

		m_resolved = true;
	}

	int bit0 = (data >> 0) & 1;
	if (m_bit0 != bit0)
	{
		m_bit0 = bit0;
		if (!m_bit0_handler.isnull())
			m_bit0_handler(bit0);
	}

	int bit1 = (data >> 1) & 1;
	if (m_bit1 != bit1)
	{
		m_bit1 = bit1;
		if (!m_bit1_handler.isnull())
			m_bit1_handler(bit1);
	}

	int bit2 = (data >> 2) & 1;
	if (m_bit2 != bit2)
	{
		m_bit2 = bit2;
		if (!m_bit2_handler.isnull())
			m_bit2_handler(bit2);
	}

	int bit3 = (data >> 3) & 1;
	if (m_bit3 != bit3)
	{
		m_bit3 = bit3;
		if (!m_bit3_handler.isnull())
			m_bit3_handler(bit3);
	}

	int bit4 = (data >> 4) & 1;
	if (m_bit4 != bit4)
	{
		m_bit4 = bit4;
		if (!m_bit4_handler.isnull())
			m_bit4_handler(bit4);
	}

	int bit5 = (data >> 5) & 1;
	if (m_bit5 != bit5)
	{
		m_bit5 = bit5;
		if (!m_bit5_handler.isnull())
			m_bit5_handler(bit5);
	}

	int bit6 = (data >> 6) & 1;
	if (m_bit6 != bit6)
	{
		m_bit6 = bit6;
		if (!m_bit6_handler.isnull())
			m_bit6_handler(bit6);
	}

	int bit7 = (data >> 7) & 1;
	if (m_bit7 != bit7)
	{
		m_bit7 = bit7;
		if (!m_bit7_handler.isnull())
			m_bit7_handler(bit7);
	}
}
