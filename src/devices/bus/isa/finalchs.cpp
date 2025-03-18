// license:BSD-3-Clause
// copyright-holders:hap
/*

The Final ChessCard by Tasc

8-bit ISA card, comes with its own CPU (G65SC02P-4 @ 5MHz), and chess engine on 32KB ROM.
It is similar to the C64 version, actually not as impressive since a PC from around 1989
should be able to run a good chess game by itself.

Tasc later released The ChessMachine ISA card, see chessm*.cpp.

Multiple ROM revisions were made. Version 2.0 is compatible with the initial 1989
software version. Version 3.6 works with the newer software package, including the
one that came with The ChessMachine.

*/

#include "emu.h"
#include "finalchs.h"


DEFINE_DEVICE_TYPE(ISA8_FINALCHS, isa8_finalchs_device, "isa_finalchs", "The Final ChessCard")

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
		m_isa->install_device(port, port+1, read8sm_delegate(*this, FUNC(isa8_finalchs_device::finalchs_r)), write8sm_delegate(*this, FUNC(isa8_finalchs_device::finalchs_w)));

		m_installed = true;
	}
}



//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( finalchs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v36", "ROM v3.6") // 22-05-90
	ROMX_LOAD("fcc_v36.bin", 0x8000, 0x8000, CRC(70800e6c) SHA1(fa8170606313adeaadad3bbf1ca18cae567e4207), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v20", "ROM v2.0") // 01-11-89
	ROMX_LOAD("fcc_v20.bin", 0x8000, 0x8000, CRC(c8e72dff) SHA1(f422b19a806cef4fadd580caefaaf8c32b644098), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *isa8_finalchs_device::device_rom_region() const
{
	return ROM_NAME( finalchs );
}



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( finalchs )
	PORT_START("DSW") // DIP switch on the ISA card PCB, this range is for the 1st release
	PORT_DIPNAME( 0x0f, 0x08, "I/O Port Address" ) PORT_DIPLOCATION("FCC_SW1:1,2,3,4")
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
	G65SC02(config, m_maincpu, 5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &isa8_finalchs_device::finalchs_mem);

	GENERIC_LATCH_8(config, m_mainlatch);
	GENERIC_LATCH_8(config, m_sublatch);
	m_sublatch->data_pending_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}



/******************************************************************************
    I/O
******************************************************************************/

// External handlers

uint8_t isa8_finalchs_device::finalchs_r(offs_t offset)
{
	if (offset == 0)
		return m_mainlatch->read();
	else
		return m_mainlatch->pending_r() ? 0 : 1;
}

void isa8_finalchs_device::finalchs_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_sublatch->write(data);
}


// Internal (on-card CPU)

void isa8_finalchs_device::finalchs_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	//map(0x6000, 0x6000).noprw(); // ?
	map(0x7f00, 0x7f00).mirror(0x00ff).r(m_sublatch, FUNC(generic_latch_8_device::read)).w(m_mainlatch, FUNC(generic_latch_8_device::write));
	map(0x8000, 0xffff).rom();
}
