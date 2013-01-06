/**********************************************************************

    Tasc Final ChessCard cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_fcc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define G65SC02P4_TAG	"g65sc02p4"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_FCC = &device_creator<c64_final_chesscard_device>;


//-------------------------------------------------
//  ROM( c64_fcc )
//-------------------------------------------------

ROM_START( c64_fcc )
	ROM_REGION( 0x8000, G65SC02P4_TAG, 0 )
	ROM_LOAD( "fcc_rom1", 0x0000, 0x8000, CRC(2949836a) SHA1(9e6283095df9e3f4802ed0c654101f8e37168bf6) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_final_chesscard_device::device_rom_region() const
{
	return ROM_NAME( c64_fcc );
}


//-------------------------------------------------
//  ADDRESS_MAP( c64_fcc_map )
//-------------------------------------------------

static ADDRESS_MAP_START( c64_fcc_map, AS_PROGRAM, 8, c64_final_chesscard_device )
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(G65SC02P4_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_fcc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_fcc )
	MCFG_CPU_ADD(G65SC02P4_TAG, M65SC02, 5000000)
	MCFG_CPU_PROGRAM_MAP(c64_fcc_map)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_final_chesscard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_fcc );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_fcc )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_final_chesscard_device::reset )
{
	if (!newval)
	{
		device_reset();
	}

	m_slot->reset_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( c64_fcc )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_CHANGED_MEMBER(DEVICE_SELF, c64_final_chesscard_device, reset, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_final_chesscard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_fcc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_final_chesscard_device - constructor
//-------------------------------------------------

c64_final_chesscard_device::c64_final_chesscard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_FCC, "Final ChessCard", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_maincpu(*this, G65SC02P4_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_final_chesscard_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_final_chesscard_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_final_chesscard_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[(m_bank << 13) | (offset & 0x1fff)];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_final_chesscard_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		 printf("IO1 %04x %02x\n", offset, data);
		m_bank = data;
	}

	if (!io2) printf("IO1 %04x %02x\n", offset, data);
}
