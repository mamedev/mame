// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tasc Final ChessCard cartridge emulation

**********************************************************************/

/*

    TODO:

    629D ldx #$00
    629F stx $0e
    62A1 sta $df00
    62A4 inc $d020
    62A7 dec $d020
    62AA cpx $0e
    62AC beq $62a4 <-- eternal loop here
    62AE rts

*/

#include "fcc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define G65SC02P4_TAG   "g65sc02p4"


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
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x6000) AM_READWRITE(nvram_r, nvram_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(G65SC02P4_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_fcc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_fcc )
	MCFG_CPU_ADD(G65SC02P4_TAG, M65SC02, XTAL_5MHz)
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

static INPUT_PORTS_START( c64_fcc )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_expansion_slot_device, reset_w)
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

c64_final_chesscard_device::c64_final_chesscard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_FCC, "Final ChessCard", tag, owner, clock, "c64_fcc", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_maincpu(*this, G65SC02P4_TAG),
	m_bank(0),
	m_ramen(0)
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
	m_maincpu->reset();

	m_bank = 0;
	m_ramen = 0;
	m_game = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_final_chesscard_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		if (m_ramen)
		{
			data = m_nvram[offset & 0x1fff];
		}
		else
		{
			data = m_roml[(m_bank << 14) | (offset & 0x3fff)];
		}
	}
	else if (!romh)
	{
		data = m_roml[(m_bank << 14) | (offset & 0x3fff)];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_final_chesscard_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		if (m_ramen)
		{
			m_nvram[offset & 0x1fff] = data;
		}
	}
	else if (!io1)
	{
		/*

		    bit     description

		    0       ?
		    1
		    2
		    3
		    4
		    5
		    6
		    7

		*/

		printf("IO1 %04x %02x\n", offset, data);
		m_bank = BIT(data, 0);
	}
	else if (!io2)
	{
		/*

		    bit     description

		    0       ?
		    1
		    2
		    3
		    4
		    5
		    6
		    7       ?

		*/

		printf("IO2 %04x %02x\n", offset, data);
		m_ramen = BIT(data, 0);
		m_game = BIT(data, 7);
	}
}


//-------------------------------------------------
//  nvram_r - NVRAM read
//-------------------------------------------------

READ8_MEMBER( c64_final_chesscard_device::nvram_r )
{
	return m_nvram[offset & m_nvram.mask()];
}


//-------------------------------------------------
//  nvram_w - NVRAM write
//-------------------------------------------------

WRITE8_MEMBER( c64_final_chesscard_device::nvram_w )
{
	m_nvram[offset & m_nvram.mask()] = data;
}
