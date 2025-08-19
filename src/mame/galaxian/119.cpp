// license:BSD-3-Clause
// copyright-holders:David Haywood

// the hardware is quite galaxian-like, is it worth doing it as a derived class instead?

#include "emu.h"

#include "cpu/z80/z80.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class sega119_state : public driver_device
{
public:
	sega119_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void sega119(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<u8> m_fgram;
	required_shared_ptr<u8> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_fg_tilemap = nullptr;

	void unk_b000_w(u8 data);

	void fgram_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;

	u8 m_bankdata;
};


void sega119_state::sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int hoffset = 1;

	for (int sprnum = 7; sprnum >= 0; sprnum--)
	{
		const uint8_t *base = &m_spriteram[sprnum * 4];
		uint8_t sy = 240 - base[0];
		uint16_t code = base[1] & 0x3f;
		uint8_t flipx = base[1] & 0x40;
		uint8_t flipy = base[1] & 0x80;
		uint8_t color = base[2] & 7;
		uint8_t sx = base[3] + hoffset;

		/* draw */
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
		code + 0x40, color,
		flipx, flipy,
		sx, sy, 0);
	}
}


TILE_GET_INFO_MEMBER(sega119_state::get_fg_tile_info)
{
	int code = m_fgram[tile_index];
	tileinfo.set(0,
				 code + ((m_bankdata & 0x08) ? 0x200 : 0x000), // might be bit 0x04, 2 bits flip at the same time, one is probably sprite bank
				 0,
				 0);
}

void sega119_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sega119_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void sega119_state::fgram_w(offs_t offset, u8 data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

uint32_t sega119_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	sprites_draw(bitmap, cliprect);
	return 0;
}

void sega119_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(FUNC(sega119_state::fgram_w)).share(m_fgram);
	map(0x9000, 0x903f).ram(); // tile attribute ram
	map(0x9040, 0x907f).ram().share("spriteram");
	map(0x9080, 0x90ff).ram(); // 0x9080 / 0x9081 are accessed

	map(0xb000, 0xb000).portr("UNK").w(FUNC(sega119_state::unk_b000_w));
	map(0xb001, 0xb001).portr("UNK2");
	map(0xb002, 0xb002).portr("DSW1");
	map(0xb003, 0xb003).portr("DSW2");

	map(0xe000, 0xefff).ram();
}

void sega119_state::unk_b000_w(u8 data)
{
	//popmessage("%02x", data);
	m_bankdata = data;
	m_fg_tilemap->mark_all_dirty();
}

static INPUT_PORTS_START( sega119 )
	PORT_START("UNK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("UNK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // game speed is not driven by interrupts, polls bit in port

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "Free" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x38, "7" )
	PORT_DIPNAME( 0x40, 0x40, "DSW2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	16*16
};

static GFXDECODE_START( gfx_sega119 )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, spritelayout, 0, 16 )
GFXDECODE_END

void sega119_state::machine_start()
{
	m_bankdata = 0;
	save_item(NAME(m_bankdata));
}

void sega119_state::machine_reset()
{
}

void sega119_state::sega119(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4'000'000); // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &sega119_state::prg_map);
	//m_maincpu->set_vblank_int("screen", FUNC(sega119_state::nmi_line_pulse)); // NMI is just a retn, so might not be used

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-1);
	screen.set_screen_update(FUNC(sega119_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_sega119);
	PALETTE(config, "palette").set_entries(0x1000);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// MC1408P8 DAC
}

ROM_START( sega119 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "119_4.bin",   0x0000, 0x2000, CRC(b614229e) SHA1(06d0b17f5ff12222c74ff9325c21268bef25446e) )
	ROM_LOAD( "119_3.bin",   0x2000, 0x2000, CRC(d2a984bf) SHA1(d2d5a83deff894978394461f8779d296b855971f) )
	ROM_LOAD( "119_2.bin",   0x4000, 0x2000, CRC(d96611bc) SHA1(fffb516f00e747931941844b5358fe46d656bfb8) )
	ROM_LOAD( "119_1.bin",   0x6000, 0x2000, CRC(368723e2) SHA1(515724eed41138e2e852f53d63f9a226584126f5) )

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "119_5.bin",   0x0000, 0x2000, CRC(1b08c881) SHA1(b372d614ec41cff49d6ff1c2256170c15069bd55) )
	ROM_LOAD( "119_6.bin",   0x2000, 0x2000, CRC(1a7490a4) SHA1(e74141b04ffb63e5cc434fbce89ac0c51e79330f) )
	ROM_LOAD( "119_7.bin",   0x4000, 0x2000, CRC(fcff7f59) SHA1(87a4668ef0c28091c895b0aeae4d4c486396e549) )

	ROM_REGION( 0x6000, "audiocpu", 0 ) // another z80
	ROM_LOAD( "119_8.bin",   0x0000, 0x2000, CRC(6570149c) SHA1(b139edbe7bd2f965804b0c850f87e2ef8e418256) )

	ROM_REGION( 0x6000, "samples", 0 ) // samples for MC1408P8
	ROM_LOAD( "119_9.bin",   0x0000, 0x2000, BAD_DUMP CRC(b917e2c2) SHA1(8acd598b898204e18a4cfccc40720d149f401b42) ) //  FIXED BITS (xxxx1xxx)

	// colour PROMs?
ROM_END

} // anonymous namespace

// ROT180 is unusual, but all tiles are flipped in ROM
GAME( 1986, sega119, 0, sega119, sega119, sega119_state, empty_init, ROT180, "Sega / Coreland", "119", MACHINE_NOT_WORKING )
