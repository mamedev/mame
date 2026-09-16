// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD SuperCPU v2 + SuperRAM emulation

**********************************************************************/

#include "emu.h"
#include "supercpu.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define G65816_TAG  "g65816"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SUPERCPU, c64_supercpu_device, "c64_supercpu", "C64 SuperCPU v2 + SuperRAM")


//-------------------------------------------------
//  ROM( c64_supercpu )
//-------------------------------------------------

ROM_START( c64_supercpu )
	ROM_REGION( 0x20000, G65816_TAG, 0 )
	ROM_LOAD( "supercpu_dos_204.bin", 0x00000, 0x20000, CRC(f4151454) SHA1(6aa529a7b1b6de53e8979e407a77b4d5657727f5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_supercpu_device::device_rom_region() const
{
	return ROM_NAME( c64_supercpu );
}


//-------------------------------------------------
//  ADDRESS_MAP( c64_supercpu_map )
//-------------------------------------------------

void c64_supercpu_device::c64_supercpu_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram().share("sram");
	map(0x020000, 0xf7ffff).ram().share("dimm");
	map(0xf80000, 0xf9ffff).mirror(0x60000).rom().region(G65816_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_supercpu_device::device_add_mconfig(machine_config &config)
{
	G65816(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64_supercpu_device::c64_supercpu_map);

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
}


//-------------------------------------------------
//  INPUT_PORTS( c64_supercpu )
//-------------------------------------------------

static INPUT_PORTS_START( c64_supercpu )
	PORT_START("FRONT")
	PORT_DIPNAME( 0x01, 0x01, "Unit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "JiffyDOS" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Speed" )
	PORT_DIPSETTING(    0x04, "Normal" )
	PORT_DIPSETTING(    0x00, "Turbo" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_supercpu_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_supercpu );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_supercpu_device - constructor
//-------------------------------------------------

c64_supercpu_device::c64_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_SUPERCPU, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_maincpu(*this, G65816_TAG),
	m_exp(*this, "exp"),
	m_sram(*this, "sram"),
	m_dimm(*this, "dimm")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_supercpu_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_supercpu_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_supercpu_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);

	switch (offset)
	{
	case 0xd0b0:
		data = 0x40;
		break;

	case 0xd0b1:
		break;

	case 0xd0b2:
		break;

	case 0xd0b3:
	case 0xd0b4:
		break;

	case 0xd0b5:
		break;

	case 0xd0b6:
		break;

	case 0xd0b7:
		break;

	case 0xd0b8:
	case 0xd0b9:
		break;

	case 0xd0ba:
		break;

	case 0xd0bb:
		break;

	case 0xd0bc:
	case 0xd0bd:
	case 0xd0be:
	case 0xd0bf:
		break;
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_supercpu_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	switch (offset)
	{
	case 0xd071:
		break;

	case 0xd072:
		break;

	case 0xd073:
		break;

	case 0xd074:
	case 0xd075:
	case 0xd076:
	case 0xd077:
		break;

	case 0xd078:
		break;

	case 0xd07a:
		break;

	case 0xd079:
	case 0xd07b:
		break;

	case 0xd07c:
		break;

	case 0xd07d:
	case 0xd07f:
		break;

	case 0xd0b0:
	case 0xd0b1:
		break;

	case 0xd0b2:
		break;

	case 0xd0b3:
		break;

	case 0xd0b4:
		break;

	case 0xd0b5:
		break;

	case 0xd0b6:
		break;

	case 0xd0b7:
		break;

	case 0xd0b8:
		break;

	case 0xd0b9:
	case 0xd0ba:
	case 0xd0bb:
		break;

	case 0xd0bc:
		break;

	case 0xd0be:
		break;

	case 0xd0bd:
	case 0xd0bf:
		break;
	}

	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_supercpu_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_supercpu_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}
