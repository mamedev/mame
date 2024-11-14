// license:BSD-3-Clause
// copyright-holders:

/*
Boo 1000 by JK Amusement

Components which could be identified:

Motorola MC68HC11A1 CPU
Winbond WF19054 sound chip (AY-8910 compatible)
HM6264ALP-12 SRAM
16 MHz XTAL
Altera Max EPM7064
Altera Acex EP1K50QC208-3
3x 8-DIP banks
4-DIP bank
*/

#include "emu.h"

#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class boo_state : public driver_device
{
public:
	boo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void boo(machine_config &config);

private:
	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t boo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void boo_state::program_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("maincpu", 0x0000);
}


static INPUT_PORTS_START(boo)
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
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
INPUT_PORTS_END


static GFXDECODE_START( gfx_boo ) // TODO
GFXDECODE_END


void boo_state::boo(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &boo_state::program_map);

	// TODO: everything
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(boo_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_boo);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay", 16_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "mono", 0.30); // divisor not verified
}


ROM_START( boo1000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr6.u14", 0x00000, 0x10000, CRC(83b1a60b) SHA1(becc1f7242e321aefbaf3d27c86b5b680f08c2d1) ) // 27C512
	// the 2 halves are almost identical (just 4 bytes differ)

	ROM_REGION( 0x80000, "reels", 0 ) // ??
	ROM_LOAD( "epr7.u16", 0x00000, 0x80000, CRC(5655b3aa) SHA1(91faf832ddd59ea59ab63142da1791cd0f4f15a8) ) // 27C4001

	ROM_REGION( 0x140000, "tiles", 0 ) // ??, all 27C2001
	ROM_LOAD( "epr1.u2", 0x000000, 0x40000, CRC(abc679c6) SHA1(48c35c9a432864d980d8a48ecfdea39dcf3a6954) )
	ROM_LOAD( "epr2.u3", 0x040000, 0x40000, CRC(a5755f8e) SHA1(453c2d2e518ef86e38483b28d424837ae4554ff5) )
	ROM_LOAD( "epr3.u4", 0x080000, 0x40000, CRC(f556920d) SHA1(36a6bfa9ff4b88e157e29e61f873379d5f9d6b2a) ) // BADADDR      xxxxxxxxxxxxxxxx--
	ROM_LOAD( "epr4.u5", 0x0c0000, 0x40000, CRC(9e255bc1) SHA1(6122432a99af7b768e70b4602209d3f38f6cd117) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "epr5.u6", 0x100000, 0x40000, CRC(038836cf) SHA1(c15cbef95c54cd38dcf6f3d790668710c1b6c400) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 2000, boo1000, 0, boo, boo, boo_state, empty_init, ROT0, "JK Amusement", "Boo 1000", MACHINE_IS_SKELETON )
