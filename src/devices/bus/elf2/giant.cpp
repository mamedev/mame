// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Netronics Giant Board emulation

**********************************************************************/

/*

    TODO:
    - serial and parallel I/O

*/

#include "emu.h"
#include "giant.h"

#include "speaker.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELF2_GIANT_BOARD, elf2_giant_board_device, "elf2_giant", "Netronics Giant Board")



//-------------------------------------------------
//  ROM( giant )
//-------------------------------------------------

ROM_START( giant )
	ROM_REGION( 0x100, "rom", 0 )
	ROM_DEFAULT_BIOS( "v1" )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "giantboard.bin", 0x000, 0x100, CRC(ae6f5def) SHA1(4e34d5d430f9912b7541027d5c63f5a6b22bc3c2), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "giantboard.v2.bin", 0x000, 0x100, CRC(8f385779) SHA1(63ed5f06e7ce603bc72494496527f44d19fa3640), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *elf2_giant_board_device::device_rom_region() const
{
	return ROM_NAME( giant );
}


//-------------------------------------------------
//  INPUT_PORTS( giant )
//-------------------------------------------------

static INPUT_PORTS_START( giant )
	PORT_START("JUMPERS")
	PORT_CONFNAME( 0x01, 0x00, "Tape Polarity" )
	PORT_CONFSETTING(    0x00, "J1 and J3" )
	PORT_CONFSETTING(    0x01, "J2" )
INPUT_PORTS_END

ioport_constructor elf2_giant_board_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( giant );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void elf2_giant_board_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

elf2_giant_board_device::elf2_giant_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELF2_GIANT_BOARD, tag, owner, clock),
	device_elf2_expansion_card_interface(mconfig, *this),
	m_cassette(*this, "cassette"),
	m_rom(*this, "rom"),
	m_jumpers(*this, "JUMPERS"),
	m_tape_polarity(false)
{
}

void elf2_giant_board_device::device_start()
{
	m_bus->program().install_rom(0xf000, 0xf0ff, m_rom);

	save_item(NAME(m_tape_polarity));
}

void elf2_giant_board_device::device_reset()
{
	m_tape_polarity = BIT(m_jumpers->read(), 0);
}

int elf2_giant_board_device::elf2_ef2_r()
{
	return (m_cassette->input() < 0.0) != m_tape_polarity;
}

void elf2_giant_board_device::elf2_q_w(int state)
{
	m_cassette->output(state ? 1.0 : -1.0);
}
