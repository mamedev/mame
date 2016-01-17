// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Magic Formel cartridge emulation

**********************************************************************/

/*

    TODO:

    - pia6821 port A DDR needs to reset to 0xff or this won't boot

*/

#include "magic_formel.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6821_TAG      "mc6821"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MAGIC_FORMEL = &device_creator<c64_magic_formel_cartridge_device>;


WRITE8_MEMBER( c64_magic_formel_cartridge_device::pia_pa_w )
{
	/*

	    bit     description

	    PA0     ROM A13
	    PA1     ROM A14
	    PA2     ROM A15
	    PA3     ROM _OE
	    PA4     RAM _OE
	    PA5
	    PA6
	    PA7

	*/

	m_rom_bank = data & 0x0f;

	m_ram_oe = BIT(data, 4);
}

WRITE8_MEMBER( c64_magic_formel_cartridge_device::pia_pb_w )
{
	/*

	    bit     description

	    PB0     RAM A10
	    PB1     RAM A11
	    PB2     RAM A9
	    PB3     RAM A8
	    PB4     RAM A12
	    PB5     U9A clr
	    PB6
	    PB7     ROMH enable

	*/

	m_ram_bank = data & 0x1f;

	if (!BIT(data, 5))
	{
		m_u9a = 0;
	}

	m_pb7 = BIT(data, 7);
}

WRITE_LINE_MEMBER( c64_magic_formel_cartridge_device::pia_cb2_w )
{
	if (!state)
	{
		m_u9b = 1;
	}
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_magic_formel )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_magic_formel )
	MCFG_DEVICE_ADD(MC6821_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(c64_magic_formel_cartridge_device, pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(c64_magic_formel_cartridge_device, pia_pb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(c64_magic_formel_cartridge_device, pia_cb2_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_magic_formel_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_magic_formel );
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( freeze )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_magic_formel_cartridge_device::freeze )
{
	if (newval && (!m_u9a && !m_u9b))
	{
		m_u9b = 1;

		m_slot->nmi_w(ASSERT_LINE);
	}
	else
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  INPUT_PORTS( c64_magic_formel )
//-------------------------------------------------

static INPUT_PORTS_START( c64_magic_formel )
	PORT_START("FREEZE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, c64_magic_formel_cartridge_device, freeze, nullptr)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_magic_formel_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_magic_formel );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_magic_formel_cartridge_device - constructor
//-------------------------------------------------

c64_magic_formel_cartridge_device::c64_magic_formel_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MAGIC_FORMEL, "C64 Magic Formel cartridge", tag, owner, clock, "c64_magic_formel", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_pia(*this, MC6821_TAG),
	m_ram(*this, "ram"),
	m_rom_bank(0),
	m_ram_bank(0),
	m_ram_oe(0),
	m_pb7(1),
	m_u9a(1),
	m_u9b(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_magic_formel_cartridge_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x2000);

	// state saving
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_oe));
	save_item(NAME(m_pb7));
	save_item(NAME(m_u9a));
	save_item(NAME(m_u9b));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_magic_formel_cartridge_device::device_reset()
{
	m_rom_bank = 0;
	m_ram_bank = 0;
	m_ram_oe = 0;
	m_pb7 = 0;
	m_u9a = 1;
	m_u9b = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_magic_formel_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!romh)
	{
		offs_t addr = (m_rom_bank << 13) | (offset & 0x1fff);
		data = m_romh[addr];
	}
	else if (!io1 && !m_ram_oe)
	{
		offs_t addr = (m_ram_bank << 8) | (offset & 0xff);
		data = m_ram[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_magic_formel_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1 && !m_ram_oe)
	{
		offs_t addr = (m_ram_bank << 8) | (offset & 0xff);
		m_ram[addr] = data;
	}
	else if (!io2 && !(!m_u9b && m_ram_oe))
	{
		offs_t addr = (offset >> 6) & 0x03;
		UINT8 new_data = (BIT(data, 1) << 7) | (offset & 0x3f);

		m_pia->write(space, addr, new_data);
	}
	else if (offset == 0x0001)
	{
		m_u9a = 1;
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_magic_formel_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return !(ba && rw && ((offset & 0xe000) == 0xe000) && !(!m_pb7 && !m_u9b));
}
