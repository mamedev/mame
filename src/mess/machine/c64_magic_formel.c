/**********************************************************************

    Magic Formel cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_magic_formel.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6821_TAG		"mc6821"

#define PA4 	(m_pb7_ff ? m_ram_oe : 0)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MAGIC_FORMEL = &device_creator<c64_magic_formel_cartridge_device>;


//-------------------------------------------------
//  pia6821_interface pia_intf
//-------------------------------------------------

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

	logerror("PA %02x\n",data);
	m_rom_bank = data & 0x0f;

	m_rom_oe = BIT(data, 3);

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
        PB5
        PB6
        PB7     ROMH enable

    */
	logerror("PB %02x\n",data);
	m_ram_bank = data & 0x1f;

	m_pb7 = BIT(data, 7);

	if (!m_pb7)
	{
		m_pb7_ff = 1;
	}
}

WRITE_LINE_MEMBER( c64_magic_formel_cartridge_device::pia_cb2_w )
{
	if (!state)
	{
		m_cb2_ff = 1;
	}
}

static const pia6821_interface pia_intf =
{
	DEVCB_NULL,		// input A
	DEVCB_NULL,		// input B
	DEVCB_NULL,										// input CA1
	DEVCB_NULL,											// input CB1
	DEVCB_NULL,											// input CA2
	DEVCB_NULL,											// input CB2
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_magic_formel_cartridge_device, pia_pa_w),		// output A
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_magic_formel_cartridge_device, pia_pb_w),		// output B
	DEVCB_NULL,											// output CA2
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_magic_formel_cartridge_device, pia_cb2_w),		// output CB2
	DEVCB_NULL,											// irq A
	DEVCB_NULL											// irq B
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_magic_formel )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_magic_formel )
	MCFG_PIA6821_ADD(MC6821_TAG, pia_intf)
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
//  INPUT_PORTS( c64_magic_formel )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_magic_formel_cartridge_device::freeze )
{
    if (!newval && (m_pb7_ff & m_cb2_ff))
    {
        m_cb2_ff = 0;

        m_slot->nmi_w(ASSERT_LINE);
    }
    else
    {
        m_slot->nmi_w(CLEAR_LINE);
    }
}

static INPUT_PORTS_START( c64_magic_formel )
	PORT_START("FREEZE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Freeze") PORT_CHANGED_MEMBER(DEVICE_SELF, c64_magic_formel_cartridge_device, freeze, 0)
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

c64_magic_formel_cartridge_device::c64_magic_formel_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MAGIC_FORMEL, "C64 Magic Formel cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_pia(*this, MC6821_TAG),
	m_rom_bank(0),
	m_ram_bank(0),
	m_pb7_ff(0),
	m_cb2_ff(0),
	m_rom_oe(0),
	m_ram_oe(0),
	m_pb7(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_magic_formel_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_pb7_ff));
	save_item(NAME(m_cb2_ff));
	save_item(NAME(m_rom_oe));
	save_item(NAME(m_ram_oe));
	save_item(NAME(m_pb7));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_magic_formel_cartridge_device::device_reset()
{
	m_rom_bank = 0;
	m_ram_bank = 0;
	m_pb7_ff = 0;
	m_cb2_ff = 0;
	m_rom_oe = 0;
	m_ram_oe = 0;
	m_pb7 = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_magic_formel_cartridge_device::c64_cd_r(address_space &space, offs_t offset, int ba, int roml, int romh, int io1, int io2)
{
	UINT8 data = 0;

	if (!romh && !m_rom_oe)
	{
		UINT8 bank = m_pb7_ff ? m_rom_bank : 0;
		offs_t addr = (bank << 13) | (offset & 0x1fff);
		data = m_romh[addr];
	}
	else if (!io1 && !PA4)
	{
		offs_t addr = (m_ram_bank << 8) | (offset & 0xff);
		data = m_ram[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_magic_formel_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		offs_t addr = (m_ram_bank << 8) | (offset & 0xff);
		m_ram[addr] = data;
	}
	else if (!io2 && !(m_cb2_ff & PA4))
	{
		offs_t addr = (offset >> 6) & 0x03;
		UINT8 new_data = (BIT(data, 1) << 7) | (offset & 0x3f);
		m_pia->write(space, addr, new_data);
	}
	else if (offset == 0x0001)
	{
		m_pb7_ff = 0;
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_magic_formel_cartridge_device::c64_game_r(offs_t offset, int ba, int rw, int hiram)
{
	return !(ba & rw & ((offset & 0xe000) == 0xe000) & !(m_pb7 & m_cb2_ff));
}
