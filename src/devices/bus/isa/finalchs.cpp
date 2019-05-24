// license:BSD-3-Clause
// copyright-holders:hap
/*

The Final ChessCard by Tasc

8-bit ISA card, comes with its own CPU (G65SC02P-4 @ 5MHz), and chess engine on 32KB ROM.
It is similar to the C64 version, actually not as impressive since a PC from around 1989
should be able to run a good chess game by itself.

Tasc later released The ChessMachine ISA card, not emulated yet.

*/

#include "emu.h"
#include "finalchs.h"


DEFINE_DEVICE_TYPE(ISA8_FINALCHS, isa8_finalchs_device, "isa_finalchs", "Final ChessCard")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

isa8_finalchs_device::isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_FINALCHS, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_mainlatch(*this, "mainlatch"),
	m_sublatch(*this, "sublatch")
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_finalchs_device::device_start()
{
	set_isa_device();
	m_installed = false;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_finalchs_device::device_reset()
{
	if (!m_installed)
	{
		// MAME doesn't allow reading ioport at device_start
		u16 port = ioport("DSW")->read() * 0x10 + 0x100;
		m_isa->install_device(port, port+1, read8_delegate(FUNC(isa8_finalchs_device::finalchs_r), this), write8_delegate(FUNC(isa8_finalchs_device::finalchs_w), this));

		m_installed = true;
	}
}



//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( finalchs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("finalchs.bin", 0x8000, 0x8000, CRC(c8e72dff) SHA1(f422b19a806cef4fadd580caefaaf8c32b644098) ) // v2.0
ROM_END

const tiny_rom_entry *isa8_finalchs_device::device_rom_region() const
{
	return ROM_NAME( finalchs );
}



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( finalchs )
	PORT_START("DSW") // DIP switch on the ISA card PCB
	PORT_DIPNAME( 0x0f, 0x07, "I/O Port Address" ) PORT_DIPLOCATION("SW1:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x00, "0x100" )
	PORT_DIPSETTING(    0x01, "0x110" )
	PORT_DIPSETTING(    0x02, "0x120" )
	PORT_DIPSETTING(    0x03, "0x130" )
	PORT_DIPSETTING(    0x04, "0x140" )
	PORT_DIPSETTING(    0x05, "0x150" )
	PORT_DIPSETTING(    0x06, "0x160" )
	PORT_DIPSETTING(    0x07, "0x170" )
	PORT_DIPSETTING(    0x08, "0x180" )
	PORT_DIPSETTING(    0x09, "0x190" )
	PORT_DIPSETTING(    0x0a, "0x1A0" )
	PORT_DIPSETTING(    0x0b, "0x1B0" )
	PORT_DIPSETTING(    0x0c, "0x1C0" )
	PORT_DIPSETTING(    0x0d, "0x1D0" )
	PORT_DIPSETTING(    0x0e, "0x1E0" )
	PORT_DIPSETTING(    0x0f, "0x1F0" )
INPUT_PORTS_END

ioport_constructor isa8_finalchs_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(finalchs);
}



//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_finalchs_device::device_add_mconfig(machine_config &config)
{
	M65SC02(config, m_maincpu, 5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &isa8_finalchs_device::finalchs_mem);

	GENERIC_LATCH_8(config, m_mainlatch);
	GENERIC_LATCH_8(config, m_sublatch);
	m_sublatch->data_pending_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}



/******************************************************************************
    I/O
******************************************************************************/

// External handlers

READ8_MEMBER(isa8_finalchs_device::finalchs_r)
{
	if (offset == 0)
		return m_mainlatch->read();
	else
		return !m_mainlatch->pending_r();
}

WRITE8_MEMBER(isa8_finalchs_device::finalchs_w)
{
	if (offset == 0)
		m_sublatch->write(data);
}


// internal (on-card CPU)

void isa8_finalchs_device::finalchs_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	//map(0x6000, 0x6000).noprw(); // ?
	map(0x7f00, 0x7f00).mirror(0x00ff).r(m_sublatch, FUNC(generic_latch_8_device::read)).w(m_mainlatch, FUNC(generic_latch_8_device::write));
	map(0x8000, 0xffff).rom();
}
