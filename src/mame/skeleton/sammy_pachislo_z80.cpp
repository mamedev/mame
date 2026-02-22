// license:BSD-3-Clause
// copyright-holders:

/*
Sammy mechanical pachislo. Z80-based.

Only hardware info available:

CPU   : Z-80
SOUND : YMF281-D ? (if confirmed, YM2413 compatbile with different presets)

Possibly only sound program?
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sammy_pachislo_z80_state : public driver_device
{
public:
	sammy_pachislo_z80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sammy_pachislo_z80(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void sammy_pachislo_z80_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x1fff).rom();
	map(0xfe00, 0xffff).ram();
}

void sammy_pachislo_z80_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	// map(0xfe, 0xfe).r();
	map(0xfe, 0xff).w("ym", FUNC(ym2413_device::write));
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


void sammy_pachislo_z80_state::sammy_pachislo_z80(machine_config &config)
{
	Z80(config, m_maincpu, 3'000'000); // clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &sammy_pachislo_z80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &sammy_pachislo_z80_state::io_map);
	m_maincpu->set_periodic_int(FUNC(sammy_pachislo_z80_state::nmi_line_pulse), attotime::from_hz(4*60)); // source?

	// TODO: other hw?

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3'000'000).add_route(ALL_OUTPUTS, "mono", 1.0); // should be YMF281-D, clock not verified
}


// ウルトラマン倶楽部3 (Ultraman Club 3)
ROM_START( ultracl3 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "ultracl3.bin", 0x0000, 0x2000, CRC(7f6ea8c6) SHA1(8a65a6e9ddd6b61405f41b4684f30a38829c5165) )
ROM_END

} // anonymous namespace


GAME( 1997, ultracl3, 0, sammy_pachislo_z80, pachislo, sammy_pachislo_z80_state, empty_init, ROT0, "Sammy", "Ultraman Club 3", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL ) // 1997/12/25
