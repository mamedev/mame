// license:BSD-3-Clause
// copyright-holders:

/*
Crane games by SNK

SNK PCC008C

MC68B09P CPU
8.000 MHz XTAL
HM6116LP-3 RAM
TMP82C255AN-2 PPI
2x TD62803P stepping motor controller/driver
YM2413 sound chip
3.579545 MHz XTAL
2x bank of 8 switches
lots ot TTL


RPC-301 Rev.2
カクハンコントロール
(probably for controlling the turntable)

bank of 5 switches
lots of TTL
*/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/i8255.h"
#include "sound/ymopl.h"

#include "speaker.h"


namespace {

class neocrnvl_state : public driver_device
{
public:
	neocrnvl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void neocrnvl(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void neocrnvl_state::program_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	// map(0x0800, 0x0807) // probably the TMP82C255AN-2
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( neocrnvl )
INPUT_PORTS_END


void neocrnvl_state::neocrnvl(machine_config &config)
{
	MC6809(config, m_maincpu, 8_MHz_XTAL); // divider?
	m_maincpu->set_addrmap(AS_PROGRAM, &neocrnvl_state::program_map);

	// actually TMP82C255AN-2, which the datasheet describes as "almost equivalent to TMP82C55A x2"
	I8255(config, "ppi0");

	I8255(config, "ppi1");

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( neocrnvl )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "ic47",      0x00000, 0x08000, CRC(b1b67505) SHA1(2ed1fe041575183d5bcfe0cc11acbd9e473ec45f) )
	ROM_LOAD( "ns_2.ic46", 0x08000, 0x10000, CRC(b38dd048) SHA1(54b7825ac7d10c76be833533d6465e79ff51b91e) )
ROM_END

} // anonymous namespace


GAME( 1991?, neocrnvl, 0, neocrnvl, neocrnvl, neocrnvl_state, empty_init, ROT0, "SNK", "Neo Carnival", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
