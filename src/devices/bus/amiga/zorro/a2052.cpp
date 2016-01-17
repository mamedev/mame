// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2052

    Zorro-II RAM Expansion (0.5, 1 or 2 MB)

***************************************************************************/

#include "a2052.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type A2052 = &device_creator<a2052_device>;

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( a2052 )
	PORT_START("config")
	PORT_CONFNAME(0x03, 0x02, "A2052 Installed RAM")
	PORT_CONFSETTING(0x00, "512 KB")
	PORT_CONFSETTING(0x01, "1 MB")
	PORT_CONFSETTING(0x02, "2 MB")
INPUT_PORTS_END

ioport_constructor a2052_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a2052 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2052_device - constructor
//-------------------------------------------------

a2052_device::a2052_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2052, "CBM A2052 Fast Memory", tag, owner, clock, "a2052", __FILE__),
	device_zorro2_card_interface(mconfig, *this),
	m_config(*this, "config")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2052_device::device_start()
{
	set_zorro_device();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void a2052_device::autoconfig_base_address(offs_t address)
{
	if (VERBOSE)
		logerror("%s('%s'): autoconfig_base_address received: 0x%06x\n", shortname().c_str(), basetag().c_str(), address);

	if (VERBOSE)
		logerror("-> installing a2052\n");

	// stop responding to default autoconfig
	m_slot->m_space->unmap_readwrite(0xe80000, 0xe8007f);

	// install access to the rom space
	m_slot->m_space->install_ram(address, address + m_ram.size()*2 - 1, &m_ram[0]);

	// we're done
	m_slot->cfgout_w(0);
}

WRITE_LINE_MEMBER( a2052_device::cfgin_w )
{
	if (VERBOSE)
		logerror("%s('%s'): configin_w (%d)\n", shortname().c_str(), basetag().c_str(), state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);

		// setup ram
		switch (m_config->read())
		{
		case 0:
			autoconfig_board_size(BOARD_SIZE_512K);
			m_ram.resize(0x080000/2);
			break;
		case 1:
			autoconfig_board_size(BOARD_SIZE_1M);
			m_ram.resize(0x100000/2);
			break;
		case 2:
			autoconfig_board_size(BOARD_SIZE_2M);
			m_ram.resize(0x200000/2);
			break;
		}

		autoconfig_product(0x0a);
		autoconfig_manufacturer(0x0202);
		autoconfig_serial(0x00000000);

		autoconfig_link_into_memory(true);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?

		// install autoconfig handler
		m_slot->m_space->install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
			write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);
	}
}
