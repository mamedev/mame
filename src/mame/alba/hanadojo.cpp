// license:BSD-3-Clause
// copyright-holders:

/*
花道場 (Hana Doujou) - Alba 1984
AAJ-11 PCB

Main components are:

NEC D780C CPU
12 MHz XTAL (near CPU)
NEC D449C RAM (near CPU ROMs)
unknown 40-pin chip (near CPU, stickered AN-001 on one PCB)
2x Toshiba TMM2009P-B RAM (near GFX ROMs)
HD46505SP CRTC
2x bank of 8 DIP switches
bank of 4 DIP switches
AY-3-8910
unknown 40-pin chip (stickered AN-002)
unknown 40-pin chip (stickered AN-003)
2x AX-014 epoxy covered chips
AX-013 epoxy covered chip

TODO: everything. Puts RAM N.G. in video RAM. Banked RAM or protection?
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hanadojo_state : public driver_device
{
public:
	hanadojo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram")
	{ }

	void hanadojo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void hanadojo_state::video_start()
{
}

uint32_t hanadojo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void hanadojo_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8800, 0x8fff).ram();
	map(0x9000, 0x97ff).ram().share(m_videoram);
}

void hanadojo_state::io_map(address_map &map)
{
	map(0x20, 0x20).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x21, 0x21).w("crtc", FUNC(hd6845s_device::register_w));
}


static INPUT_PORTS_START( hanadojo )
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
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
INPUT_PORTS_END


static GFXDECODE_START( gfx_hanadojo )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x3_planar, 0, 32 ) // TODO: wrong
GFXDECODE_END


void hanadojo_state::hanadojo(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &hanadojo_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &hanadojo_state::io_map);

	hd6845s_device &crtc(HD6845S(config, "crtc", 12_MHz_XTAL / 16)); // divider guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette").set_entries(0x20); // TODO: wrong

	GFXDECODE(config, m_gfxdecode, "palette", gfx_hanadojo);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(hanadojo_state::screen_update));
	screen.set_palette("palette");

	SPEAKER(config, "speaker").front_center();

	ay8910_device &ay(AY8910(config, "ay", 12_MHz_XTAL / 16)); // divider guessed
	ay.add_route(ALL_OUTPUTS, "speaker", 0.33);
}

ROM_START( hanadojo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n0_1.2l",  0x0000, 0x2000, CRC(fedc7a24) SHA1(75baed5c4f86185a24c8a9995a246246fda480d2) )
	ROM_LOAD( "n10_2.2k", 0x2000, 0x2000, CRC(e9fd71d5) SHA1(dbd5ed3bf81e507ca9a08c06fe22060a03bf3eed) )
	ROM_LOAD( "n10_3.2h", 0x4000, 0x2000, CRC(8dc494f5) SHA1(23df7c564f9b33c6c3cffb7be5dbf3c025468e3e) )
	ROM_LOAD( "n10_4.2f", 0x6000, 0x2000, CRC(ecbbfe7f) SHA1(0425e5b71a07f93d1562be158a7c33041f025fdc) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "no6.4f",  0x0000, 0x2000, CRC(f049bc57) SHA1(cb6baaa4eaf9306a54ec39689e3b16ab7acc82c7) )
	ROM_LOAD( "no7.4e",  0x4000, 0x2000, CRC(2ff0c6c7) SHA1(a92926d497189fadb11d0c8fd12d8e4a1506c4fd) )
	ROM_LOAD( "no8.4d",  0x8000, 0x2000, CRC(32ed9c86) SHA1(6614df331ab8e57327b27393d189cc538f1b9567) )
	ROM_LOAD( "no9.6f",  0x2000, 0x2000, CRC(1e5f720e) SHA1(e3d55fae723625fcdd78ee02fb10e47d8e0628f0) )
	ROM_LOAD( "no10.6e", 0x6000, 0x2000, CRC(bfb79118) SHA1(191c441b60fdec714733e326e7ad984b551e2cce) )
	ROM_LOAD( "no11.6d", 0xa000, 0x2000, CRC(a601d401) SHA1(ac10da18c6ef46d9c9da10e292dcb49554676885) )

	ROM_REGION( 0x120, "proms", 0 )
	ROM_LOAD( "n2.9d",  0x000, 0x100, CRC(e6812f63) SHA1(2286b43970e51d6cfbceaaf74bcb6d2f35620d3a) )
	ROM_LOAD( "n1.11d", 0x100, 0x020, CRC(1e6f668a) SHA1(6006ee30920e51581862b0e7f56ac724831b0034) )
ROM_END

ROM_START( hanadojoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n1_1.2l",  0x0000, 0x2000, CRC(25c6360f) SHA1(d38a01471264f00f5ed6b4bb831620b13e8be411) )
	ROM_LOAD( "n20_2.2k", 0x2000, 0x2000, CRC(e9fd71d5) SHA1(dbd5ed3bf81e507ca9a08c06fe22060a03bf3eed) )
	ROM_LOAD( "n20_3.2h", 0x4000, 0x2000, CRC(094b55c9) SHA1(a1735ed788af778a3da358069af8567a3724aa0d) )
	ROM_LOAD( "n20_4.2f", 0x6000, 0x2000, CRC(ca47a101) SHA1(b31488e5102cd7f576bfc3ee4253e0fb752e72c9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "no6.4f",  0x0000, 0x2000, CRC(f049bc57) SHA1(cb6baaa4eaf9306a54ec39689e3b16ab7acc82c7) )
	ROM_LOAD( "no7.4e",  0x2000, 0x2000, CRC(2ff0c6c7) SHA1(a92926d497189fadb11d0c8fd12d8e4a1506c4fd) )
	ROM_LOAD( "no8.4d",  0x4000, 0x2000, CRC(32ed9c86) SHA1(6614df331ab8e57327b27393d189cc538f1b9567) )
	ROM_LOAD( "no9.6f",  0x6000, 0x2000, CRC(1e5f720e) SHA1(e3d55fae723625fcdd78ee02fb10e47d8e0628f0) )
	ROM_LOAD( "no10.6e", 0x8000, 0x2000, CRC(bfb79118) SHA1(191c441b60fdec714733e326e7ad984b551e2cce) )
	ROM_LOAD( "no11.6d", 0xa000, 0x2000, CRC(a601d401) SHA1(ac10da18c6ef46d9c9da10e292dcb49554676885) )

	ROM_REGION( 0x120, "proms", 0 )
	ROM_LOAD( "n2.9d",  0x000, 0x100, CRC(e6812f63) SHA1(2286b43970e51d6cfbceaaf74bcb6d2f35620d3a) )
	ROM_LOAD( "n1.11d", 0x100, 0x020, CRC(1e6f668a) SHA1(6006ee30920e51581862b0e7f56ac724831b0034) )
ROM_END

} // anonymous namespace


GAME( 1984, hanadojo,  0,        hanadojo, hanadojo, hanadojo_state, empty_init, ROT0, "Alba", "Hana Doujou (set 1)", MACHINE_IS_SKELETON )
GAME( 1984, hanadojoa, hanadojo, hanadojo, hanadojo, hanadojo_state, empty_init, ROT0, "Alba", "Hana Doujou (set 2)", MACHINE_IS_SKELETON )
