// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-8-1 4k/8k Static RAM board

****************************************************************************/

#include "emu.h"

#include "h_8_1.h"

namespace {

class h_8_1_device : public device_t, public device_h8bus_card_interface
{
public:

	h_8_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	bool m_installed;

	memory_share_creator<u8>    m_ram;
	required_ioport             m_jumpers;
	required_ioport             m_config;
};

h_8_1_device::h_8_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_H_8_1, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_ram(*this, "ram", 0x2000U, ENDIANNESS_LITTLE)
	, m_jumpers(*this, "JUMPERS")
	, m_config(*this, "CONFIG")
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
		ioport_value const config(m_config->read());

		u16 base_addr = (jumpers & 0x07) << 13;
		u16 top_addr = base_addr + (BIT(config, 0) ? 0x1fff : 0x0fff);

		h8bus().space(AS_PROGRAM).install_ram(base_addr, top_addr, m_ram);

		m_installed = true;
	}
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

	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x01, "Memory installed")
	PORT_CONFSETTING(   0x00, "4k")
	PORT_CONFSETTING(   0x01, "8k")

INPUT_PORTS_END

ioport_constructor h_8_1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h_8_1_jumpers);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_H_8_1, device_h8bus_card_interface, h_8_1_device, "h8_h_8_1", "Heath H-8-1 8k Static RAM");
