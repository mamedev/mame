// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

Tasc Final ChessCard cartridge emulation

It expects a mouse in port 2, and/or joystick in port 1.

The cartridge includes its own CPU (G65SC02P-4 @ 5MHz), making a relatively
strong chess program possible on C64.
It was also released for IBM PC, with an ISA card.

**********************************************************************/

#include "emu.h"
#include "fcc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_FCC, c64_final_chesscard_device, "c64_fcc", "Final ChessCard")


//-------------------------------------------------
//  ADDRESS_MAP( c64_fcc_map )
//-------------------------------------------------

void c64_final_chesscard_device::c64_fcc_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(c64_final_chesscard_device::nvram_r), FUNC(c64_final_chesscard_device::nvram_w)); // A13-A15 = low
	//map(0x6000, 0x6000).noprw(); // N/C
	map(0x7f00, 0x7f00).mirror(0x00ff).r(m_sublatch, FUNC(generic_latch_8_device::read)).w(m_mainlatch, FUNC(generic_latch_8_device::write)); // A8-A14 = high
	map(0x8000, 0xffff).r(FUNC(c64_final_chesscard_device::rom_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_final_chesscard_device::device_add_mconfig(machine_config &config)
{
	M65SC02(config, m_maincpu, 5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64_final_chesscard_device::c64_fcc_map);

	config.m_perfect_cpu_quantum = subtag("maincpu");

	GENERIC_LATCH_8(config, m_mainlatch).data_pending_callback().set(FUNC(c64_final_chesscard_device::mainlatch_int));
	GENERIC_LATCH_8(config, m_sublatch).data_pending_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
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

c64_final_chesscard_device::c64_final_chesscard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_FCC, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_mainlatch(*this, "mainlatch"),
	m_sublatch(*this, "sublatch"),
	m_bank(0),
	m_hidden(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_final_chesscard_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_hidden));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_final_chesscard_device::device_reset()
{
	m_mainlatch->read();
	m_sublatch->read();
	m_maincpu->reset();

	m_bank = 0;
	m_hidden = 0;
	m_exrom = 0;
	m_game = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_final_chesscard_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh)
		data = m_roml[(m_bank << 14) | (offset & 0x3fff)];
	else if (!io2)
		data = m_mainlatch->read();

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_final_chesscard_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_hidden && !io1)
	{
		// d0: rom bank
		// d1: EXROM/GAME
		// d7: hide register
		m_bank = BIT(data, 0);
		m_exrom = BIT(data, 1);
		m_game = BIT(data, 1);
		m_hidden = BIT(data, 7);
	}
	else if (!io2)
		m_sublatch->write(data);
}
