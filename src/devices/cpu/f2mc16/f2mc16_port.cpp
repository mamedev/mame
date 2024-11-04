// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series I/O Port

***************************************************************************/

#include "emu.h"
#include "f2mc16.h"
#include "f2mc16_port.h"

DEFINE_DEVICE_TYPE(F2MC16_PORT, f2mc16_port_device, "f2mc16_port", "F2MC16 Port")

f2mc16_port_device::f2mc16_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_PORT, tag, owner, clock),
	m_cpu(nullptr),
	m_defval(0),
	m_mask(0),
	m_read_cb(*this, 0xff),
	m_write_cb(*this),
	m_pdr(0xff),
	m_output(0)
{
}

f2mc16_port_device::f2mc16_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t defval, uint8_t mask) :
	f2mc16_port_device(mconfig, tag, owner)
{
	m_cpu = downcast<f2mc16_device *>(owner);
	m_defval = defval;
	m_mask = mask;
}

void f2mc16_port_device::device_start()
{
	save_item(NAME(m_pdr));
	save_item(NAME(m_ddr));
	save_item(NAME(m_ader));
	save_item(NAME(m_output));
}

void f2mc16_port_device::device_reset()
{
	if (m_defval)
		m_pdr = m_defval;
	m_ddr = m_defval;
	m_ader = m_defval;

	update_output(true);
}

uint8_t f2mc16_port_device::pdr_r()
{
	uint8_t mask = ~m_ddr & m_mask;

	if (mask)
	{
		if (!m_read_cb.isunset())
			return (m_read_cb(0, mask) & mask) | (m_pdr & m_ddr);
		else if (!machine().side_effects_disabled())
			logerror("%s unmapped read %02x\n", machine().describe_context(), mask);
		return m_pdr | mask;
	}

	return m_pdr;
}

uint8_t f2mc16_port_device::pdr_adc_r()
{
	if (m_cpu->rmw())
		return m_pdr;

	uint8_t mask = ~m_ader & m_pdr & m_mask;

	if (mask)
	{
		if (!m_read_cb.isunset())
			return m_read_cb(0, mask) & mask;
		else if (!machine().side_effects_disabled())
			logerror("%s unmapped read (mask=0x%02x)\n", machine().describe_context(), mask);
	}

	return 0x00;
}

void f2mc16_port_device::pdr_w(uint8_t data)
{
	m_pdr = data & m_mask;
	update_output();
}

uint8_t f2mc16_port_device::ddr_r()
{
	return m_ddr;
}

void f2mc16_port_device::ddr_w(uint8_t data)
{
	m_ddr = data & m_mask;
	update_output();
}

uint8_t f2mc16_port_device::ader_r()
{
	return m_ader;
}

void f2mc16_port_device::ader_w(uint8_t data)
{
	m_ader = data;
}

void f2mc16_port_device::update_output(bool initial)
{
	uint8_t output = m_pdr | (~m_ddr & m_mask);

	if (m_output != output || initial)
	{
		m_output = output;

		if (!m_write_cb.isunset())
			m_write_cb(0, m_output, m_ddr & m_mask);
		else if (!initial)
			logerror("%s unmapped write %02x (mask=0x%02x)\n", machine().describe_context(), m_pdr & m_ddr, m_ddr);
	}
}
