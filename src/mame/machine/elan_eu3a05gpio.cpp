// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05gpio.h"

DEFINE_DEVICE_TYPE(RADICA6502_GPIO, radica6502_gpio_device, "radica6502gpio", "Elan EU3A05 GPIO")

radica6502_gpio_device::radica6502_gpio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RADICA6502_GPIO, tag, owner, clock)
	, m_space_read0_cb(*this)
	, m_space_read1_cb(*this)
	, m_space_read2_cb(*this)
{
}

void radica6502_gpio_device::device_start()
{
	m_space_read0_cb.resolve_safe(0xff);
	m_space_read1_cb.resolve_safe(0xff);
	m_space_read2_cb.resolve_safe(0xff);
}

void radica6502_gpio_device::device_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_ddr[i] = 0;
		m_unk[i] = 0;
	}
}

uint8_t radica6502_gpio_device::read_port_data(int which)
{
	//todo, actually use the direction registers
	switch (which)
	{
		case 0: return m_space_read0_cb();
		case 1: return m_space_read1_cb();
		case 2: return m_space_read2_cb();
	}

	return 0xff;
}

uint8_t radica6502_gpio_device::read_direction(int which)
{
	return m_ddr[which];
}

READ8_MEMBER(radica6502_gpio_device::gpio_r)
{

	int port = offset/2;
	if (!(offset&1)) return read_direction(port);
	else return read_port_data(port);
}

void radica6502_gpio_device::write_port_data(int which, uint8_t data)
{
	//todo, actually use the direction registers
	logerror("%s: write_port_data (port %d) %02x (direction register %02x)\n", machine().describe_context(), which, data, m_ddr[which]);
}

void radica6502_gpio_device::write_direction(int which, uint8_t data)
{
	logerror("%s: write_direction (port %d) %02x\n", machine().describe_context(), which, data);
	m_ddr[which] = data;
}

WRITE8_MEMBER(radica6502_gpio_device::gpio_w)
{

	int port = offset/2;
	if (!(offset&1)) return write_direction(port, data);
	else return write_port_data(port, data);
}

WRITE8_MEMBER(radica6502_gpio_device::gpio_unk_w)
{
	logerror("%s: gpio_unk_w (port %d) %02x (direction register %02x)\n", machine().describe_context(), offset, data, m_ddr[offset]);
}


