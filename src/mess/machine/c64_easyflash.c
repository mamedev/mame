/**********************************************************************

    EasyFlash cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_easyflash.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define AM29F040_0_TAG	"u3"
#define AM29F040_1_TAG	"u4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_EASYFLASH = &device_creator<c64_easyflash_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_easyflash )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_easyflash )
	MCFG_AMD_29F040_ADD(AM29F040_0_TAG)
	MCFG_AMD_29F040_ADD(AM29F040_1_TAG)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_easyflash_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_easyflash );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_easyflash )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_easyflash_cartridge_device::reset )
{
	if (!newval)
	{
		device_reset();
	}

	m_slot->reset_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( c64_easyflash )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x00, "Boot" )
	PORT_DIPSETTING(    0x00, "Disable" )
	PORT_DIPSETTING(    0x01, "Boot" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, c64_easyflash_cartridge_device, reset, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_easyflash_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_easyflash );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_easyflash_cartridge_device - constructor
//-------------------------------------------------

c64_easyflash_cartridge_device::c64_easyflash_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_EASYFLASH, "C64 EasyFlash cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_flash_roml(*this, AM29F040_0_TAG),
	m_flash_romh(*this, AM29F040_1_TAG),
	m_jp1(*this, "JP1"),
	m_bank(0),
	m_mode(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_easyflash_cartridge_device::device_start()
{
	// allocate memory
	c64_ram_pointer(machine(), 0x100);

	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_mode));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_easyflash_cartridge_device::device_reset()
{
	m_bank = 0;
	m_mode = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_easyflash_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		data = m_flash_roml->read(addr);
	}
	else if (!romh)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		data = m_flash_romh->read(addr);
	}
	else if (!io2)
	{
		data = m_ram[offset & 0xff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_easyflash_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		m_flash_roml->write(addr, data);
	}
	else if (!romh)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		m_flash_romh->write(addr, data);
	}
	else if (!io1)
	{
		if (!BIT(offset, 1))
		{
			/*

                bit     description

                0       BA13
                1       BA14
                2       BA15
                3       BA16
                4       BA17
                5       BA18
                6
                7

            */

			m_bank = data & 0x3f;
		}
		else
		{
			/*

                bit     description

                0       GAME
                1       EXROM
                2       MODE
                3
                4
                5
                6
                7       LED

            */

			m_mode = data;
		}
	}
	else if (!io2)
	{
		m_ram[offset & 0xff] = data;
	}
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_easyflash_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return !BIT(m_mode, 1);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_easyflash_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return !(BIT(m_mode, 0) | !(BIT(m_mode, 2) | m_jp1->read()));
}
