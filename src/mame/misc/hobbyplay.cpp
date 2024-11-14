// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for early 80's slot machines manufactured by Hobby Play.
Hobby Play did about eight different machines, but they're mostly unknown.

Most Hobby Play slot machines (Gran Gol, etc.) use three 5x7 dot-matrix displays:

  o o o o o    o o o o o    o o o o o
  o o o o o    o o o o o    o o o o o
  o o o o o    o o o o o    o o o o o
  o o o o o    o o o o o    o o o o o
  o o o o o    o o o o o    o o o o o
  o o o o o    o o o o o    o o o o o
  o o o o o    o o o o o    o o o o o


PCB marked Hobby Play CIC 003 B

Z80 CPU
4'433'618 XTAL
2 x INS8255N (P8255)
1 x 8 dips bank
1 x NE555P (near the Z80)
2 x Intersil D2114 (SRAM)
various TTL chips
*/

#include "emu.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"


namespace {

class hobbyplay_state : public driver_device
{
public:
	hobbyplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void hobbyplay(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void prg_map(address_map &map) ATTR_COLD;
};

void hobbyplay_state::prg_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("maincpu", 0);
	map(0x0800, 0x0bff).ram();
	map(0x1000, 0x1003).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x1800, 0x1803).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

static INPUT_PORTS_START( hobbyplay )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")
INPUT_PORTS_END


void hobbyplay_state::hobbyplay(machine_config &config)
{
	Z80(config, m_maincpu, 4'433'618);
	m_maincpu->set_addrmap(AS_PROGRAM, &hobbyplay_state::prg_map);

	i8255_device &ppi0(I8255(config, "ppi0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("DSW0");
	//ppi0.out_pc_callback().set(hobbyplay_state::);

	I8255(config, "ppi1");
	//ppi1.out_pc_callback().set(hobbyplay_state::);

	SPEAKER(config, "mono").front_center();
	// sound? possibly very simple discrete
}


ROM_START( unkhpslt )
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD( "hobby_play_661.bin", 0x0000, 0x0800, CRC(e721d720) SHA1(23d84d2013f1ec42b1bcf6983ee28093071d4b8e) )
ROM_END

} // anonymous namespace


GAME( 198?, unkhpslt, 0, hobbyplay, hobbyplay, hobbyplay_state, empty_init, ROT0, "Hobby Play", "unknown Hobby Play slot machine", MACHINE_IS_SKELETON_MECHANICAL )
