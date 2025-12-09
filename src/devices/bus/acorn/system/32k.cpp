// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 32K Dynamic RAM Board

    Part No. 200,010

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_32KDRAM.html

    The 32K Dynamic RAM Board is provided with soldered links which give
    the Dynamic RAM the addresses 2000 to 7FFF and C000 to DFFF, Acorn
    Memory Blocks 2 to 7, C and D.

    The 16K DRAM option can he equipped with 8 DRAM IC; in Bank B. The
    Address Selection Links are then required to give DRAM addresses 8000
    to BFFF.

**********************************************************************/

#include "emu.h"
#include "32k.h"


namespace {

class acorn_32k_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_32K, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_links(*this, "LINKS")
		, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_links;
	memory_share_creator<uint8_t> m_ram;
};

//-------------------------------------------------
//  INPUT_PORTS( 32k )
//-------------------------------------------------

INPUT_PORTS_START( 32k )
	PORT_START("LINKS")
	PORT_CONFNAME(0x07, 0x00, "Address Selection (RAM)")
	PORT_CONFSETTING(0x00, "System 32K: &2000-&7FFF, &C000-&DFFF")
	PORT_CONFSETTING(0x01, "System 16K: &8000-&BFFF")
	PORT_CONFSETTING(0x02,   "Atom 32K: &0000-&7FFF")
	PORT_CONFSETTING(0x03,   "Atom 32K: &0000-&7FFF excl. &0Axx")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor acorn_32k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 32k );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_32k_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_32k_device::device_reset()
{
	address_space &space = m_bus->memspace();

	switch (m_links->read())
	{
	case 0:
		space.install_ram(0x2000, 0x7fff, m_ram);
		space.install_ram(0xc000, 0xdfff, m_ram + 0x6000);
		break;

	case 1:
		space.install_ram(0x8000, 0xbfff, m_ram);
		break;

	case 2:
		space.install_ram(0x0000, 0x7fff, m_ram);
		break;

	case 3:
		space.install_ram(0x0000, 0x09ff, m_ram);
		space.install_ram(0x0b00, 0x7fff, m_ram + 0x0b00);
		break;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_32K, device_acorn_bus_interface, acorn_32k_device, "acorn_32k", "Acorn 32K Dynamic RAM Board")
