// license:BSD-3-Clause
// copyright-holders:David Haywood

// Thanks to SynaMax & Dutchman2000

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class jammin_state : public driver_device
{
public:
	jammin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ym2151(*this, "ym2151"),
		m_bgram(*this, "bgram")
	{ }

	void jammin(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ym2151_device> m_ym2151;
	required_shared_ptr<uint8_t> m_bgram;

	tilemap_t *m_bg_tilemap = nullptr;

	void bgram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};

TILE_GET_INFO_MEMBER(jammin_state::get_bg_tile_info)
{
	int code = m_bgram[tile_index];

	tileinfo.set(0,
			code,
			0,
			0);
}

void jammin_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void jammin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jammin_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t jammin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void jammin_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();

	map(0x4000, 0x4001).ram().rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));

	map(0x5000, 0x50ff).ram();

	map(0x6000, 0x6bff).ram();

	map(0x7000, 0x73ff).ram(); // sprites?
	map(0x7400, 0x77ff).ram().w(FUNC(jammin_state::bgram_w)).share(m_bgram);

	map(0x7800, 0x7800).nopw(); // DMACTL
	map(0x7801, 0x7801).nopw();
	map(0x7802, 0x7802).nopw();
	map(0x7803, 0x7803).nopw();

	map(0x7808, 0x7808).nopw();

	map(0x7c00, 0x7c00).portr("IN0"); // player inputs
	map(0x7c80, 0x7c80).portr("IN1"); // player inputs
	map(0x7d07, 0x7d07).portr("IN2"); // Coin is in here
	map(0x7d80, 0x7d80).portr("IN3"); // unknown

	map(0x7d82, 0x7d82).nopw(); // VFLIP
	map(0x7d83, 0x7d83).nopw(); // MOCNTL
	map(0x7d84, 0x7d84).nopw(); // NMILAT
	map(0x7d85, 0x7d85).nopw(); // DMAGO
	map(0x7d86, 0x7d86).nopw(); // PAL1
	map(0x7d87, 0x7d87).nopw(); // PAL2

	map(0x8000, 0xbfff).rom();
}

static INPUT_PORTS_START( jammin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )  // acts as a drum, also start 2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // acts as a drum
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // acts as a drum
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // acts as a drum
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // acts as a drum, advances selection in menu
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // acts as a drum
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) // acts as a drum
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) // acts as a drum
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )  // acts as a drum, also start 1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tile_layout2 =
{
	8,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*8
};

// does't seem to be tile gfx?
static const gfx_layout tile_layout3 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,1 },
	{ 0,2,4,6,8,10,12,14 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16 },
	8*16
};

static GFXDECODE_START( gfx_jammin )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout, 0, 1 )
	GFXDECODE_ENTRY( "tiles2", 0, tile_layout2, 0, 1 )
	GFXDECODE_ENTRY( "unknown_data", 0, tile_layout3, 0, 1 )
GFXDECODE_END

void jammin_state::machine_start()
{
}

void jammin_state::machine_reset()
{
}

void jammin_state::jammin(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4'000'000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &jammin_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(jammin_state::nmi_line_pulse));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(jammin_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_jammin);

	// wrong
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 14'318'181 / 4); // ?? used sys2 clock
	m_ym2151->add_route(0, "speaker", 0.60, 0);
	m_ym2151->add_route(1, "speaker", 0.60, 1);
}

ROM_START( jammin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// rebuilt from sources
	ROM_LOAD( "jammin.bin", 0x00000, 0xb853, CRC(449ce727) SHA1(83a96284072cd5fc3aae5ad327fc95ad90346954) )

	ROM_REGION( 0x1000, "tiles", 0 ) // backgrounds?
	ROM_LOAD( "jambak.pl0", 0x0000, 0x0800, CRC(af808d29) SHA1(cad060ee4e529f9a2ffa9675682b9e17bed4dffe) )
	ROM_LOAD( "jambak.pl1", 0x0800, 0x0800, CRC(43eaccef) SHA1(37e032b60c0bbaea81b55b4d137cb2d9d047e521) )

	ROM_REGION( 0x2000, "tiles2", 0 ) // sprites?
	ROM_LOAD( "jammin.7c", 0x0000, 0x0800, CRC(82361b24) SHA1(ed070586296c329cb88e3dfc4741d591ec59fb8d) )
	ROM_LOAD( "jammin.7d", 0x0800, 0x0800, CRC(0f02eb55) SHA1(b97ee07dbd71bdc698ac775a721f220afd6bb7cc) )
	ROM_LOAD( "jammin.7e", 0x1000, 0x0800, CRC(9563c301) SHA1(3947af64f3becf36afaacdb8c962bfccc236525d) )
	ROM_LOAD( "jammin.7f", 0x1800, 0x0800, CRC(26c7f3b8) SHA1(a0a13ce692bcf40104099a4d9bb2c36aca885350) )

	ROM_REGION( 0x4000, "unknown_data", 0 ) // not tile based? (what is this?)
	ROM_LOAD( "jammin.int", 0x00000, 0x4000, CRC(0f9022de) SHA1(40f33dd7fcdc310c0eb93c3072b24f290247e974) )

	ROM_REGION( 0x100, "proms_col_n", 0 ) // lookup?
	ROM_LOAD( "col2n.bin", 0x000, 0x0100, CRC(c5ded6e3) SHA1(21d172952f5befafec6fa93be5023f1df0eceb7d) )

	ROM_REGION( 0x200, "proms_col_ef", 0 ) // colours?
	ROM_LOAD( "col2f.bin", 0x000, 0x0100, CRC(bf115ba7) SHA1(ecd12079c23ed73eed2056cad2c23e6bb19d803e) )
	ROM_LOAD( "col2e.bin", 0x100, 0x0100, CRC(d22fd797) SHA1(a21be0d280eb376dc600b28a15ece0f9d1cb6d42) )

	ROM_REGION( 0x100, "proms_mac_n", 0 ) // lookup?
	ROM_LOAD( "mac2n.bin", 0x000, 0x0100, CRC(e8198448) SHA1(20fc8da7858daa56be758148e5e80f5de30533f9) )

	ROM_REGION( 0x200, "proms_mac_ef", 0 ) // colours?
	ROM_LOAD( "mac2f.bin", 0x000, 0x0100, CRC(938955e5) SHA1(96accf365326e499898fb4d937d716df5792fade) )
	ROM_LOAD( "mac2e.bin", 0x100, 0x0100, CRC(65f57bc6) SHA1(8645c8291c7479ed093d64d3f9b19240d5cf8b4e) )
ROM_END

} // anonymous namespace


GAME( 1985, jammin,  0,    jammin, jammin,  jammin_state, empty_init, ROT90, "Atari", "Jammin' (prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
