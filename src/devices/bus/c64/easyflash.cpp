// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    EasyFlash cartridge emulation

**********************************************************************/

#include "emu.h"
#include "easyflash.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define AM29F040_0_TAG  "u3"
#define AM29F040_1_TAG  "u4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_EASYFLASH, c64_easyflash_cartridge_device, "c64_easyflash", "C64 EasyFlash cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_easyflash_cartridge_device::device_add_mconfig(machine_config &config)
{
	AMD_29F040(config, AM29F040_0_TAG);
	AMD_29F040(config, AM29F040_1_TAG);
}


//-------------------------------------------------
//  INPUT_PORTS( c64_easyflash )
//-------------------------------------------------

static INPUT_PORTS_START( c64_easyflash )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x00, "Boot" )
	PORT_DIPSETTING(    0x00, "Disable" )
	PORT_DIPSETTING(    0x01, "Boot" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))
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

c64_easyflash_cartridge_device::c64_easyflash_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_EASYFLASH, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_flash_roml(*this, AM29F040_0_TAG),
	m_flash_romh(*this, AM29F040_1_TAG),
	m_jp1(*this, "JP1"),
	m_ram(*this, "ram", 0x100, ENDIANNESS_LITTLE),
	m_bank(0),
	m_mode(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_easyflash_cartridge_device::device_start()
{
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

uint8_t c64_easyflash_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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

void c64_easyflash_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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

int c64_easyflash_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return !BIT(m_mode, 1);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_easyflash_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return (BIT(m_mode, 0) || !(BIT(m_mode, 2) || m_jp1->read())) ? 0 : 1;
}
