// license:BSD-3-Clause
// copyright-holders:

/*
Sega 834-12846 PCB (stickered 970729 0259 II)

The main components are:
MC68EC000FN16 CPU
32.000 MHz XTAL
Sega 315_5296 custom (I/O)
Battery
2x LH52B256N SRAM 32k x 8
2x Lattice pLSI 2032 High Density Programmable Logic
Lattice pLSI 1016 High Density Programmable Logic
NEC D71054GB programmable timer / counter
NEC D71055GB parallel interface unit
2x MB3771 power supply monitor
SLA7026M unipolar stepper motor
YM3438
NEC D7759GC
2x 8-dip banks
3-dip bank

Currently unknown which game this is. The M68K ROM contains several puck-related strings,
so it's supposed to be some kind of air hockey game.
epr codes point to 1996-97 timeframe. Possibly Hockey Stadium (1997)?
*/

#include "emu.h"

#include "315_5296.h"

#include "cpu/m68000/m68000.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "sound/upd7759.h"
#include "sound/ymopn.h"

#include "speaker.h"


namespace {

class unkhockey_state : public driver_device
{
public:
	unkhockey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_upd7759(*this, "upd")
	{ }

	void unkhockey(machine_config &config);

private:
	required_device<m68000_base_device> m_maincpu;
	required_device<upd7759_device> m_upd7759;

	void program_map(address_map &map) ATTR_COLD;
};


void unkhockey_state::program_map(address_map &map) // TODO: verify everything
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20001f).rw("io", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);
	map(0x700000, 0x700007).rw("ymsnd", FUNC(ym3438_device::read), FUNC(ym3438_device::write)).umask16(0x00ff);
	map(0xb00000, 0xb01fff).ram().share("nvram"); // TODO: size?
	//map(0x??0000, 0x??0001).w(FUNC(unkhockey_state::upd7759_w)).umask16(0x00ff);
	map(0xff0000, 0xffffff).ram();
}


static INPUT_PORTS_START( unkhockey )
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

	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("SW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("SW3") // only 3 switches
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
INPUT_PORTS_END


void unkhockey_state::unkhockey(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // divider unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &unkhockey_state::program_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIT8254(config, "d71054", 0); // NEC D71054GB

	I8255(config, "d071055"); // NEC D71055GB

	SEGA_315_5296(config, "io", 32_MHz_XTAL / 2); // divider unverified

	SPEAKER(config, "mono").front_center();

	ym3438_device &ymsnd(YM3438(config, "ymsnd", 32_MHz_XTAL / 4)); // divider unverified
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	UPD7759(config, m_upd7759).add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( unkhockey )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "epr-19513a.ic34", 0x00000, 0x80000, CRC(bf4228af) SHA1(817a96753bee081a9a34052f735403f9b8c167dc) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "epr-19514.ic2", 0x00000, 0x20000, CRC(71342fbc) SHA1(36c9f57da3749f9151e37a7799c23351e4313dea) ) // contents only up to ~ 0x18000
	ROM_IGNORE(                         0x60000 ) // 11xxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // Anonymous namespace

GAME( 199?, unkhockey, 0, unkhockey, unkhockey, unkhockey_state, empty_init, ROT0, "Sega", "unknown Sega air hockey game", MACHINE_IS_SKELETON_MECHANICAL )
