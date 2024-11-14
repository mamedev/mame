// license:BSD-3-Clause
// copyright-holders:

/*
Gu, Choki, Pa

PCB is marked 20282 and LC (stands for "lato componenti", so components side)
with a small riser board marked W 15482 plugged into one of the main CPU ROMs'
sockets

Main components are:
SGS Z80CPUB1 main CPU (clock measured 3.07 MHz)
18.432 MHz XTAL
SGS Z80CPUB1 audio CPU (clock measured 1.53 MHz)
AY-3-8910 sound chip
MK4802 RAM (near audio CPU) and snd chip Ay-3-8910
6x 2114 RAM (near GFX ROMs)
Bank of 8 switches

The riser board has a pair of HM4334 1K*4 static RAMs and a quad 2-input NAND gate.

TODO:
- sound
- colors
- is visible area correct?
- remaining dips
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class guchokipa_state : public driver_device
{
public:
	guchokipa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram")
	{ }

	void guchokipa(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void bgram_w(offs_t offset, uint8_t data);
	void fgram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_program_map(address_map &map) ATTR_COLD;
	void sound_program_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(guchokipa_state::get_bg_tile_info)
{
	int const code = m_bgram[tile_index];

	tileinfo.set(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(guchokipa_state::get_fg_tile_info)
{
	int code = m_fgram[tile_index];

	if (code == 0x00) code = 0x3ff; // why? is this another 'big sprite' thing?

	tileinfo.set(1, code, 0, 0);
}


void guchokipa_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(guchokipa_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(guchokipa_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void guchokipa_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void guchokipa_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint32_t guchokipa_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void guchokipa_state::main_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6400, 0x67ff).ram();
	map(0x7000, 0x73ff).ram().w(FUNC(guchokipa_state::bgram_w)).share(m_bgram);
	map(0x7400, 0x77ff).ram().w(FUNC(guchokipa_state::fgram_w)).share(m_fgram);
	map(0x7c00, 0x7fff).ram(); // only seems to be initialized with 0xff at start up
}

void guchokipa_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("SW");
	map(0x01, 0x01).portr("IN0");
	map(0x02, 0x02).portr("IN1");
	map(0x17, 0x17).lw8(NAME([this] (uint8_t data) { if (data & 0xfe) logerror("flip w: %02x\n", data); flip_screen_set(BIT(data, 0)); }));
	// map(0x30, 0x30).w() // lamps?
	// .w("soundlatch", FUNC(generic_latch_8_device::write));
}

void guchokipa_state::sound_program_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x2000, 0x23ff).ram();
}

void guchokipa_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	// .r("soundlatch", FUNC(generic_latch_8_device::read));
	// .w("ay", FUNC(ay8910_device::address_data_w));
	// .r("ay", FUNC(ay8910_device::data_r));
}


static INPUT_PORTS_START( guchokipa )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 100 in book-keeping
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // 500 in book-keeping
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // 1000 in book-keeping
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // only works without credits inserted
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Gu (rock)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Choki (scissors)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) // Pa (paper)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "A 1C / B 5C / C 10C" )
	PORT_DIPSETTING(    0x02, "A 2C / B 10C / C 20C" )
	PORT_DIPSETTING(    0x01, "A 4C / B 20C / C 40C" )
	PORT_DIPSETTING(    0x00, "A 5C / B 25C / C 50C" )
	PORT_DIPNAME( 0x04, 0x04, "Max Bet" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:4") // some combination of the following 3 seems to affect win probability
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_guchokipa )
	GFXDECODE_ENTRY( "tiles", 0,     gfx_8x8x4_planar, 0, 1 )
	GFXDECODE_ENTRY( "tiles", 0x800, gfx_8x8x4_planar, 0, 1 )
GFXDECODE_END


void guchokipa_state::guchokipa(machine_config &config)
{
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6); // 3.07 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &guchokipa_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &guchokipa_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(guchokipa_state::irq0_line_hold));

	Z80(config, m_audiocpu, 18.432_MHz_XTAL / 12); // 1.53 Mhz
	m_audiocpu->set_addrmap(AS_PROGRAM, &guchokipa_state::sound_program_map);
	m_audiocpu->set_addrmap(AS_IO, &guchokipa_state::sound_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(guchokipa_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_guchokipa);
	PALETTE(config, "palette").set_entries(0x10); // TODO

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay", 18.432_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.35);
}


ROM_START( guchokip )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "4210.bt1",  0x0000, 0x1000, CRC(6cf83735) SHA1(3d0a978adb1c1b4526fa00dedd08e2879f4af283) )
	ROM_LOAD( "4211.bg12", 0x1000, 0x1000, CRC(098bf9ff) SHA1(17da91457a6e8154e09361d0600b37156c05f7c2) )
	ROM_LOAD( "4212.bg13", 0x2000, 0x1000, CRC(6d02e421) SHA1(489a822a52db39f348282ba92fb1c1d3cbc68710) )
	ROM_LOAD( "4213.bg14", 0x3000, 0x0800, CRC(364891c8) SHA1(220cae58877cb6a80d01305375a5945c7dc5cfcc) )

	ROM_REGION( 0x800, "audiocpu", 0 )
	ROM_LOAD( "4209.bg11", 0x000, 0x800, CRC(44b2b7d1) SHA1(672931ff572ac6361b493dc9a49f6146bdc26b78) )

	ROM_REGION( 0x8000, "tiles", 0 )
	ROM_LOAD( "4201.b1", 0x0000, 0x1000, CRC(838726ab) SHA1(5bcfb3c6badc8f7b7bea17a228137e4bff39a0e5) )
	ROM_LOAD( "4205.b2", 0x1000, 0x1000, CRC(58efc253) SHA1(b3344df68c665da996f3332f43030a664931db80) )
	ROM_LOAD( "4202.g1", 0x2000, 0x1000, CRC(a45d5258) SHA1(9080c51b2dc5d6bc4d01cc29deed0e2a5ea78dbd) )
	ROM_LOAD( "4206.g2", 0x3000, 0x1000, CRC(e18d50f7) SHA1(c922a019c13c904701abe5a9e42be955d80a7ecb) )
	ROM_LOAD( "4203.r1", 0x4000, 0x1000, CRC(8769aad5) SHA1(71f3d22e8e0006ba89329ac4a48f09e11ab67875) )
	ROM_LOAD( "4207.r2", 0x5000, 0x1000, CRC(9b78e95e) SHA1(2f3fdcc3bb92b2eb3a967de39ffe4eee74cac8e0) )
	ROM_LOAD( "4204.t1", 0x6000, 0x1000, CRC(da77a765) SHA1(e8626548909b5e735cdb603964324482848ce476) )
	ROM_LOAD( "4208.t2", 0x7000, 0x1000, CRC(be97d733) SHA1(ff3b199c8d1203d9d6c0060f217bbd7de32a8152) )
ROM_END

} // anonymous namespace


GAME( 198?, guchokip, 0, guchokipa, guchokipa, guchokipa_state, empty_init, ROT0, "<unknown>", "Gu, Choki, Pa", MACHINE_IS_SKELETON )
