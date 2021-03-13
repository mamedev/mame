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


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_32K, acorn_32k_device, "acorn_32k", "Acorn 32K Dynamic RAM Board")


//-------------------------------------------------
//  INPUT_PORTS( 32k )
//-------------------------------------------------

INPUT_PORTS_START( 32k )
	PORT_START("LINKS")
	PORT_CONFNAME(0x01, 0x00, "Address Selection (RAM)")
	PORT_CONFSETTING(0x00, "32K: &2000-&7FFF, &C000-&DFFF")
	PORT_CONFSETTING(0x01, "16K: &8000-&BFFF")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor acorn_32k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 32k );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_32k_device - constructor
//-------------------------------------------------

acorn_32k_device::acorn_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_32K, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_links(*this, "LINKS")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_32k_device::device_start()
{
	save_item(NAME(m_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_32k_device::device_reset()
{
	address_space &space = m_bus->memspace();

	if (m_links->read())
	{
		space.install_ram(0x8000, 0xbfff, m_ram);
	}
	else
	{
		space.install_ram(0x2000, 0x7fff, m_ram);
		space.install_ram(0xc000, 0xdfff, m_ram + 0x6000);
	}
}
