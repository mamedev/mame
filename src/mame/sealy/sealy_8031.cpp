// license:BSD-3-Clause
// copyright-holders:

/*
Jin Sanse (Sealy)

PCB is marked 1998 SEALY

Main components:
P8031AH CPU
12.0000 MHz XTAL (near CPU)
HM6116LP-4 RAM
MC68B21P PIA
MC6821P PIA
UM6845EA CRTC
KC89C502 (probably a clone of one the YM sound chips)
U6295 sample player (Oki M6295 compatible)
XLS28C16AP-150 parallel EEPROM
VCA625971 HONG KONG 9523 A N (84-pin PLCC)
unmarked chip with NCA64523 on the bottom side (84-pin PLCC)
10.240 MHz XTAL
unmarked chip at C6 (RAM?)
3 banks of 8 DIP switches
*/

#include "emu.h"

#include "cpu/mcs51/i8051.h"
#include "machine/6821pia.h"
#include "machine/at28c16.h"
#include "sound/okim6295.h"
// #include "sound/ymop?.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PORTS (1U << 1)

// #define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)


namespace {

class sealy_8031_state : public driver_device
{
public:
	sealy_8031_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void jinsanse(machine_config &config) ATTR_COLD;

	void init_jinsanse() ATTR_COLD;

private:
	required_device<i8031_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


uint32_t sealy_8031_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void sealy_8031_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void sealy_8031_state::io_map(address_map &map)
{
	//map(0x0000, 0x0000).w() // writes 0xff at start up
}


static INPUT_PORTS_START( jinsanse )
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
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx ) // TODO: decoding below is just for testing
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x2_planar, 0, 16 )
GFXDECODE_END


void sealy_8031_state::jinsanse(machine_config &config)
{
	I8031(config, m_maincpu, 10.240_MHz_XTAL); // P8031AH. Is this the correct XTAL or the other one? Divider?
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_8031_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &sealy_8031_state::io_map);
	m_maincpu->port_in_cb<0>().set([this] () { LOGPORTS("%s: 8031 port 0 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->port_in_cb<1>().set([this] () { LOGPORTS("%s: 8031 port 1 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->port_in_cb<2>().set([this] () { LOGPORTS("%s: 8031 port 2 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->port_in_cb<3>().set([this] () { LOGPORTS("%s: 8031 port 3 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->port_out_cb<0>().set([this] (uint8_t data) { LOGPORTS("%s: 8031 port 0 out %02x\n", machine().describe_context(), data); });
	m_maincpu->port_out_cb<1>().set([this] (uint8_t data) { LOGPORTS("%s: 8031 port 1 out %02x\n", machine().describe_context(), data); });
	m_maincpu->port_out_cb<2>().set([this] (uint8_t data) { LOGPORTS("%s: 8031 port 2 out %02x\n", machine().describe_context(), data); });
	m_maincpu->port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS("%s: 8031 port 3 out %02x\n", machine().describe_context(), data); });

	AT28C16(config, "at28c16", 0);

	PIA6821(config, "pia0");

	PIA6821(config, "pia1");

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(sealy_8031_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100); // wrong

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	mc6845_device &crtc(MC6845(config, "crtc", 10.240_MHz_XTAL)); // Is this the correct XTAL or the other one? Divider?
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8); // TODO

	SPEAKER(config, "mono").front_center();

	// TODO: KC89C502

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // divider and pin 7 not verified
}


ROM_START( jinsanse )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mbm27c256.c1", 0x0000, 0x8000, CRC(3ba2f257) SHA1(c435b43febc2d883d982bcad1b480e1678d67155) ) // possibly scrambled address lines?

	ROM_REGION( 0x800, "at28c16", 0 )
	ROM_LOAD( "xls28c16ap.c2", 0x000, 0x800, CRC(63604df6) SHA1(f9a0a540641d9e997189991b9df7baa59e8627b9) ) // TODO: verify if it contains important data or just settings

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "am27128a.c7", 0x0000, 0x4000, CRC(4aa26f0f) SHA1(d281118cea871776301173a25178f213b1c0da40) )
	ROM_LOAD( "d27128d.c8",  0x4000, 0x4000, CRC(7c4e9821) SHA1(612a3380d21767305abe793250fdca689c0222cf) )
	ROM_LOAD( "am27c128.c9", 0x8000, 0x4000, CRC(bd300ac6) SHA1(0bbd8860b8b1507558590d5bb13d9a8fae424d04) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "nm27c0100.c1", 0x00000, 0x20000, CRC(d82067d3) SHA1(396c61584d1b56260d18e09c3a8c795a05f763c5) )
ROM_END

void sealy_8031_state::init_jinsanse()
{
	uint8_t *rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x8000);

	memcpy(&buffer[0], rom, 0x8000);

	// TODO: descramble address
	for (int i = 0; i < 0x8000; i++)
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)];
}

} // anonymous namespace


GAME( 1998?, jinsanse, 0, jinsanse, jinsanse, sealy_8031_state, init_jinsanse, ROT0, "Sealy", "Jin Sanse", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
