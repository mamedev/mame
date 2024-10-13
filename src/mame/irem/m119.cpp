// license:BSD-3-Clause
// copyright-holders:

/*
Irem M119 PCB

Main components:

on main PCB:
HD6417708S (SH3)
2x 62265 RAM (for the CPU)
UPD65840GJ CMOS-8L ASIC
YMZ280BB-F
2x PLD's
4-dip bank

on sub PCB:
UPD94244-210 VDP
2x 62256 RAM (for the VDP)

Clocks:
UPD65840 - 15 MHz
UPD94244 - 26.6666 MHz
SH2 - ?? MHz
SG3032 XTAL (32.768 kHz) for the SH3 RTC
*/

#include "emu.h"

#include "cpu/sh/sh4.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class m119_state : public driver_device
{
public:
	m119_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void m119(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;

	required_device<sh3_device> m_maincpu;
};

uint32_t m119_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void m119_state::program_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().region("maincpu", 0);
	map(0x00400000, 0x0040ffff).ram(); // stack

//  map(0x04000080, 0x04000083) address/data pairs, likely YMZ
//  map(0x1000be01, 0x1000c1ff) filled as byte area
//  map(0x1000c000, 0x1000ffff) ^
//  0x18000096-0x18000097 writes before accessing byte area, banked view?
//  0x180000b5 always 0xaa, paired with WTCSR $5a
//  0x180000b4 r/w at PC=0x12b2 onward,
//             then loops expecting an external event filling work RAM buffer $401110 PC=12e6,
//             presumably for a thread dispatcher (observe R11 afterwards)

}

static INPUT_PORTS_START( m119 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // only 1 4-dip bank
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
INPUT_PORTS_END


void m119_state::m119(machine_config &config)
{
	// basic machine hardware
	SH3LE(config, m_maincpu, 60'000'000); // HD6417708S, according to the datasheet operation frequency is 60 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &m119_state::program_map);
//  m_maincpu->set_vblank_int("screen", FUNC(m119_state::irq2_line_hold));

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(m119_state::screen_update));

	// TODO: UPD94244-210 VDP

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16'934'400)); // internal?
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}


ROM_START( scumimon )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "scu1.a-pr00-c.ic43", 0x00000, 0x80000, CRC(18c17fcf) SHA1(500f69b65e0425c849833cfb58d5558862ac3c44) )

	ROM_REGION(0x400000, "vdp", 0) // TODO: ROM loading probably wrong
	ROM_LOAD16_BYTE( "scu1.b-c0.ic4", 0x000000, 0x200000, CRC(56402e4b) SHA1(b287f5424ebee8b9d68ef975de37bac900044ed2) )
	ROM_LOAD16_BYTE( "scu1.b-c1.ic2", 0x000001, 0x200000, CRC(adff81ba) SHA1(a176b9ab5b2f47abb89e817699d742dbf876a4c7) )

	ROM_REGION(0x200000, "ymz", 0)
	ROM_LOAD( "scu1.a-v0-.ic35", 0x000000, 0x100000, CRC(819e4bbd) SHA1(e0ca76a7b97b05bbffdb96866a8bdd460fc589b2) ) // FIXED BITS (xxxxxxxxxxxxx1xx)

	// TODO: 2x PLDs once identified
ROM_END

} // anonymous namespace


GAME( 2000, scumimon, 0, m119, m119, m119_state, empty_init, ROT0, "Irem", "Slotters Club: Umi Monogatari", MACHINE_IS_SKELETON ) // Ver PROGRAM 2000/09/14
