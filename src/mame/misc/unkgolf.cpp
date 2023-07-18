// license:BSD-3-Clause
// copyright-holders:

/*
HSG-001A PCB

This is probably a golf game (or part of it), given the ROM labels.

Main components:
Sharp LH0080A Z80A-CPU-D
Sharp LH0082A Z80A-CTC-D
HM6264ALP-15 RAM
4x TMP82C55AP-2 (with 2 more empty spaces)
OKI M6376
20 Mhz XTAL
4.096 Mhz XTAL
8-DIP bank
1 push button
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "sound/okim6376.h"

#include "speaker.h"


namespace {

class unkgolf_state : public driver_device
{
public:
	unkgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void unkgolf(machine_config &config) ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void unkgolf_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void unkgolf_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	/* // TODO: determine which of the six ranges are used by the 4 actually present PPIs
	map(0x10, 0x13).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x30).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x50, 0x53).rw("ppi4", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x60, 0x63).rw("ppi5", FUNC(i8255_device::read), FUNC(i8255_device::write));
	*/
}


static INPUT_PORTS_START( unkgolf )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


void unkgolf_state::unkgolf(machine_config &config)
{
	Z80(config, m_maincpu, 20_MHz_XTAL / 5); // divisor unknown
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &unkgolf_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &unkgolf_state::io_map);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 20_MHz_XTAL / 5)); // divisor unknown
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8255(config, "ppi0");

	I8255(config, "ppi1");

	I8255(config, "ppi2");

	I8255(config, "ppi3");

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 4.096_MHz_XTAL / 32).add_route(ALL_OUTPUTS, "mono", 1.0); // divisor unknown
}


ROM_START( unkgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pinest_4096h.u3", 0x00000, 0x10000, CRC(0f97fd6c) SHA1(d0f8ebf3414929498a8a014252ce61974e3b5d77) ) // 1ST AND 2ND HALF IDENTICAL, handwritten label

	ROM_REGION( 0x100000, "oki", 0 ) // handwritten labels
	ROM_LOAD( "golf11", 0x00000, 0x80000, CRC(48234f9e) SHA1(bd2d0c17b532fe105485d64a04c76b7a9d6b2f26) )
	ROM_LOAD( "golf12", 0x80000, 0x80000, CRC(2ee904e5) SHA1(b7565f5a1eb677e0d05aa43f302a0c50be48b708) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 19??, unkgolf, 0, unkgolf, unkgolf, unkgolf_state, empty_init, ROT0, "<unknown>", "unknown golf game", MACHINE_IS_SKELETON_MECHANICAL )
