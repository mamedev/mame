// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-8-1 4k/8k Static RAM board

****************************************************************************/

#include "emu.h"

#include "h_8_1.h"

#include "machine/ram.h"

namespace {

class h_8_1_device : public device_t, public device_h8bus_card_interface
{
public:

	h_8_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	h_8_1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	bool m_installed;

	required_device<ram_device> m_ram;
	required_ioport             m_jumpers;
};

class h_8_1_4k_device : public h_8_1_device
{
public:

	h_8_1_4k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


h_8_1_device::h_8_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	h_8_1_device(mconfig, H8BUS_H_8_1, tag, owner, 0)
{
}

h_8_1_device::h_8_1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, type, tag, owner, 0),
	device_h8bus_card_interface(mconfig, *this),
	m_ram(*this, "mem"),
	m_jumpers(*this, "JUMPERS")
{
}

void h_8_1_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
}

void h_8_1_device::device_reset()
{
	if (!m_installed)
	{
		ioport_value const jumpers(m_jumpers->read());

		u16 base_addr = (jumpers & 0x07) << 13;

		h8bus().space(AS_PROGRAM).install_readwrite_handler(base_addr, base_addr + 0x1fff,
			read8sm_delegate(m_ram, FUNC(ram_device::read)),
			write8sm_delegate(m_ram, FUNC(ram_device::write)));

		m_installed = true;
	}
}

void h_8_1_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_ram).set_default_size("8K").set_default_value(0x00);
}

static INPUT_PORTS_START( h_8_1_jumpers )
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x07, 0x01, "Memory base address" )
	PORT_CONFSETTING(   0x00, "0k")
	PORT_CONFSETTING(   0x01, "8k")
	PORT_CONFSETTING(   0x02, "16k")
	PORT_CONFSETTING(   0x03, "24k")
	PORT_CONFSETTING(   0x04, "32k")
	PORT_CONFSETTING(   0x05, "40k")
	PORT_CONFSETTING(   0x06, "48k")
	PORT_CONFSETTING(   0x07, "56k")
INPUT_PORTS_END

ioport_constructor h_8_1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h_8_1_jumpers);
}


h_8_1_4k_device::h_8_1_4k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	h_8_1_device(mconfig, H8BUS_H_8_1_4K, tag, owner, 0)
{
}

void h_8_1_4k_device::device_reset()
{
	if (!m_installed)
	{
		ioport_value const jumpers(m_jumpers->read());

		u16 base_addr = (jumpers & 0x07) << 13;

		h8bus().space(AS_PROGRAM).install_readwrite_handler(base_addr, base_addr + 0x0fff,
			read8sm_delegate(m_ram, FUNC(ram_device::read)),
			write8sm_delegate(m_ram, FUNC(ram_device::write)));

		m_installed = true;
	}
}

void h_8_1_4k_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_ram).set_default_size("4K").set_default_value(0x00);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_H_8_1,    device_h8bus_card_interface, h_8_1_device,    "h8_h_8_1",    "Heath H-8-1 8k Static RAM");
DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_H_8_1_4K, device_h8bus_card_interface, h_8_1_4k_device, "h8_h_8_1_4k", "Heath H-8-1 4k Static RAM");
