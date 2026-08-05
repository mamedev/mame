// license:BSD-3-Clause
// copyright-holders:

/*
Universal mechanical pachislo. Z80-based.

Only hardware info available:

CPU   : Z-80
SOUND : YM2413

Possibly only sound programs?
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pachislo_z80_state : public driver_device
{
public:
	pachislo_z80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void pachislo_z80(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void pachislo_z80_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x4000, 0x4001).w("ym", FUNC(ym2413_device::write));
	map(0x6000, 0x6000).portr("IN0");
	// map(0x6001, 0x6001).w();
}


static INPUT_PORTS_START( pachislo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void pachislo_z80_state::pachislo_z80(machine_config &config)
{
	Z80(config, m_maincpu, 3'000'000); // clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &pachislo_z80_state::program_map);
	m_maincpu->set_periodic_int(FUNC(pachislo_z80_state::irq0_line_hold), attotime::from_hz(4*60)); // source?

	// TODO: other hw?

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3'000'000).add_route(ALL_OUTPUTS, "mono", 1.0); // clock not verified
}


ROM_START( crankyc )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "crankyc.bin", 0x0000, 0x2000, CRC(af7af60c) SHA1(40840030826089299de0ad1eb29ba1dfb7bb6a1c) )
ROM_END

ROM_START( thunderv )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "thunderv.bin", 0x0000, 0x2000, CRC(00b9f00c) SHA1(472aca09113efc11934ce94c68373fe1c15c9a5b) )
ROM_END

ROM_START( versus )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "versus.bin", 0x0000, 0x2000, CRC(2ed3eb61) SHA1(1d0b4c61ff44fd6447d4e1f529a8efed221af3c7) )
ROM_END

} // anonymous namespace


GAME( 1997, crankyc,  0, pachislo_z80, pachislo, pachislo_z80_state, empty_init, ROT0, "Universal", "Cranky Condor", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL ) // 1997.02.17
GAME( 1997, thunderv, 0, pachislo_z80, pachislo, pachislo_z80_state, empty_init, ROT0, "Universal", "Thunder V",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL ) // 1997.09.08
GAME( 1997, versus,   0, pachislo_z80, pachislo, pachislo_z80_state, empty_init, ROT0, "Universal", "Versus",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL ) // 1997.11.17
