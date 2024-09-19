// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/***************************************************************************

    Mustache Boy
    (c)1987 March Electronics

    (there are also Seibu and Taito logos/copyrights in the ROMs)

 driver by Tomasz Slanina

 The hardware is similar to Knuckle Joe.

Oscillators:
. OSC1 - 14.318180 Mhz
. OSC2 - 18.432000 Mhz
. OSC3 - 12.000000 Mhz

Measured freq:

Z80:
. Pin  6 - 6001135.77 Hz  OSC3/2
. Pin 16 - 56.747 Hz  56 Hz

T5182:
. Pin 18 - 3577599.xx Hz  OSC1/4

YM2151:
. Pin 24 - 3577600.55 OSC1/4

***************************************************************************/

#include "emu.h"

#include "seibusound.h"    // for seibu_sound_decrypt on the MAIN cpu (not sound)
#include "t5182.h"

#include "cpu/z80/z80.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mustache_state : public driver_device
{
public:
	mustache_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_dswb(*this, "DSWB")
	{ }

	void mustache(machine_config &config);

	void init_mustache();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_ioport m_dswb;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_control_byte = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void video_control_w(uint8_t data);
	void scroll_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;
};


void mustache_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void mustache_state::video_control_w(uint8_t data)
{
	/* It is assumed that screen flipping is controlled by both
	   hardware (via a DIP switch, labeled "Hard SW" on the
	   operator's sheet) and software, as in some Irem games */
	flip_screen_set((data & 0x01) ^ BIT(~m_dswb->read(), 7));

	// tile bank
	if ((m_control_byte ^ data) & 0x08)
	{
		m_control_byte = data;
		machine().tilemap().mark_all_dirty();
	}
}

void mustache_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, 0x100 - data);
	m_bg_tilemap->set_scrollx(1, 0x100 - data);
	m_bg_tilemap->set_scrollx(2, 0x100 - data);
	m_bg_tilemap->set_scrollx(3, 0x100);
}

TILE_GET_INFO_MEMBER(mustache_state::get_bg_tile_info)
{
	int const attr = m_videoram[2 * tile_index + 1];
	int const code = m_videoram[2 * tile_index] + ((attr & 0x60) << 3) + ((m_control_byte & 0x08) << 7);
	int const color = attr & 0x0f;

	tileinfo.set(0, code, color, ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0));


}

void mustache_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mustache_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS_FLIP_X, 8, 8, 64, 32);

	m_bg_tilemap->set_scroll_rows(4);

	save_item(NAME(m_control_byte));
}

void mustache_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip = cliprect;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	const rectangle &visarea = m_screen->visible_area();

	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int sy = 240 - m_spriteram[offs];
		int sx = 240 - m_spriteram[offs + 3];
		int code = m_spriteram[offs + 2];
		int const attr = m_spriteram[offs + 1];
		int const color = (attr & 0xe0) >> 5;

		if (sy == 240) continue;

		code += (attr & 0x0c) << 6;

		if ((m_control_byte & 0xa))
			clip.max_y = visarea.max_y;
		else
			if (flip_screen())
				clip.min_y = visarea.min_y + 56;
			else
				clip.max_y = visarea.max_y - 56;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 232 - sy;
		}

		gfx->transpen(bitmap, clip,
				code,
				color,
				flip_screen(), flip_screen(),
				sx, sy, 0);
	}
}

uint32_t mustache_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void mustache_state::memmap(address_map &map)
{
	map(0x0000, 0x7fff).r("sei80bu", FUNC(sei80bu_device::data_r));
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram().w(FUNC(mustache_state::videoram_w)).share(m_videoram);
	map(0xd000, 0xd000).w("t5182", FUNC(t5182_device::sound_irq_w));
	map(0xd001, 0xd001).r("t5182", FUNC(t5182_device::sharedram_semaphore_snd_r));
	map(0xd002, 0xd002).w("t5182", FUNC(t5182_device::sharedram_semaphore_main_acquire_w));
	map(0xd003, 0xd003).w("t5182", FUNC(t5182_device::sharedram_semaphore_main_release_w));
	map(0xd400, 0xd4ff).rw("t5182", FUNC(t5182_device::sharedram_r), FUNC(t5182_device::sharedram_w));
	map(0xd800, 0xd800).portr("P1");
	map(0xd801, 0xd801).portr("P2");
	map(0xd802, 0xd802).portr("START");
	map(0xd803, 0xd803).portr("DSWA");
	map(0xd804, 0xd804).portr("DSWB");
	map(0xd806, 0xd806).w(FUNC(mustache_state::scroll_w));
	map(0xd807, 0xd807).w(FUNC(mustache_state::video_control_w));
	map(0xe800, 0xefff).writeonly().share(m_spriteram);
	map(0xf000, 0xffff).ram();
}

void mustache_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).r("sei80bu", FUNC(sei80bu_device::opcode_r));
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
}

/******************************************************************************/

static INPUT_PORTS_START( mustache )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("START")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xf9, IP_ACTIVE_LOW, IPT_UNUSED  )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW 2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW 2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW 2:4,5")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW 2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW 1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW 1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW 1:6")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW 1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW 1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3),RGN_FRAC(2,3)},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8
};
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4),RGN_FRAC(0,4),RGN_FRAC(2,4)},
	{STEP16(15,-1)},
	{STEP16(0,16)},
	16*16
};

static GFXDECODE_START( gfx_mustache )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,   0x00, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0x80, 8 )
GFXDECODE_END



void mustache_state::mustache(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mustache_state::memmap);
	m_maincpu->set_addrmap(AS_OPCODES, &mustache_state::decrypted_opcodes_map);

	SEI80BU(config, "sei80bu", 0).set_device_rom_tag("maincpu");

	t5182_device &t5182(T5182(config, "t5182", 14.318181_MHz_XTAL / 4));
	t5182.ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	t5182.ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(56.747);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(1*8, 31*8-1, 0, 31*8-1);
	m_screen->set_screen_update(FUNC(mustache_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // causes 2 interrupts per frame

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mustache);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 14.318181_MHz_XTAL / 4));
	ymsnd.irq_handler().set("t5182", FUNC(t5182_device::ym2151_irq_handler));
	ymsnd.add_route(0, "mono", 0.5);
	ymsnd.add_route(1, "mono", 0.5);
}

ROM_START( mustache )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mustache.h18", 0x0000, 0x8000, CRC(123bd9b8) SHA1(33a7cba5c3a54b0b1a15dd1e24d298b6f7274321) )
	ROM_LOAD( "mustache.h16", 0x8000, 0x4000, CRC(62552beb) SHA1(ee10991d7de0596608fa1db48805781cbfbbdb9f) )

	ROM_REGION( 0x8000, "t5182:external", 0 ) // Toshiba T5182 external ROM
	ROM_LOAD( "mustache.e5", 0x0000, 0x8000, CRC(efbb1943) SHA1(3320e9eaeb776d09ed63f7dedc79e720674e6718) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "mustache.a13", 0x0000,  0x4000, CRC(9baee4a7) SHA1(31bcec838789462e67e54ebe7256db9fc4e51b69) )
	ROM_LOAD( "mustache.a14", 0x4000,  0x4000, CRC(8155387d) SHA1(5f0a394c7671442519a831b0eeeaba4eecd5a406) )
	ROM_LOAD( "mustache.a16", 0x8000,  0x4000, CRC(4db4448d) SHA1(50a94fd65c263d95fd24b4009dbb87707929fdcb) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "mustache.a4", 0x00000,  0x8000, CRC(d5c3bbbf) SHA1(914e3feea54246476701f492c31bd094ad9cea10) )
	ROM_LOAD( "mustache.a7", 0x08000,  0x8000, CRC(e2a6012d) SHA1(4e4cd1a186870c8a88924d5bff917c6889da953d) )
	ROM_LOAD( "mustache.a5", 0x10000,  0x8000, CRC(c975fb06) SHA1(4d166bd79e19c7cae422673de3e095ad8101e013) )
	ROM_LOAD( "mustache.a8", 0x18000,  0x8000, CRC(2e180ee4) SHA1(a5684a25c337aeb4effeda7982164d35bc190af9) )

	ROM_REGION( 0x1300, "proms", 0 )
	ROM_LOAD( "mustache.c3",0x0000, 0x0100, CRC(68575300) SHA1(bc93a38df91ad8c2f335f9bccc98b52376f9b483) )
	ROM_LOAD( "mustache.c2",0x0100, 0x0100, CRC(eb008d62) SHA1(a370fbd1affaa489210ea36eb9e365263fb4e232) )
	ROM_LOAD( "mustache.c1",0x0200, 0x0100, CRC(65da3604) SHA1(e4874d4152a57944d4e47306250833ea5cd0d89b) )

	ROM_LOAD( "mustache.b6",0x0300, 0x1000, CRC(5f83fa35) SHA1(cb13e63577762d818e5dcbb52b8a53f66e284e8f) ) // 63S281N near SEI0070BU
ROM_END

ROM_START( mustachei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.h18", 0x0000, 0x8000, CRC(22893fbc) SHA1(724ea50642aec9be10547bd86fae5e1ebfe54685) )
	ROM_LOAD( "2.h16", 0x8000, 0x4000, CRC(ec70cfd3) SHA1(0476eab03b907778ea488c802b79da99bf376eb6) )

	ROM_REGION( 0x8000, "t5182:external", 0 ) // Toshiba T5182 external ROM
	ROM_LOAD( "10.e5", 0x0000, 0x8000, CRC(efbb1943) SHA1(3320e9eaeb776d09ed63f7dedc79e720674e6718) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "5.a13", 0x0000,  0x4000, CRC(9baee4a7) SHA1(31bcec838789462e67e54ebe7256db9fc4e51b69) )
	ROM_LOAD( "4.a15", 0x4000,  0x4000, CRC(8155387d) SHA1(5f0a394c7671442519a831b0eeeaba4eecd5a406) )
	ROM_LOAD( "3.a16", 0x8000,  0x4000, CRC(4db4448d) SHA1(50a94fd65c263d95fd24b4009dbb87707929fdcb) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "6.a4", 0x00000,  0x8000, CRC(4a95a89c) SHA1(b34ebbda9b0e591876988e42bd36fd505452f38c) )
	ROM_LOAD( "8.a7", 0x08000,  0x8000, CRC(3e6be0fb) SHA1(319ea59107e37953c31f59f5f635fc520682b09f) )
	ROM_LOAD( "7.a5", 0x10000,  0x8000, CRC(8ad38884) SHA1(e11f1e1db6d5d119afedbe6604d10a6fd6049f12) )
	ROM_LOAD( "9.a8", 0x18000,  0x8000, CRC(3568c158) SHA1(c3a2120086befe396a112bd62f032638011cb47a) )

	ROM_REGION( 0x1300, "proms", 0 )
	ROM_LOAD( "d.c3",0x0000, 0x0100, CRC(68575300) SHA1(bc93a38df91ad8c2f335f9bccc98b52376f9b483) )
	ROM_LOAD( "c.c2",0x0100, 0x0100, CRC(eb008d62) SHA1(a370fbd1affaa489210ea36eb9e365263fb4e232) )
	ROM_LOAD( "b.c1",0x0200, 0x0100, CRC(65da3604) SHA1(e4874d4152a57944d4e47306250833ea5cd0d89b) )

	ROM_LOAD( "a.b6",0x0300, 0x1000, CRC(5f83fa35) SHA1(cb13e63577762d818e5dcbb52b8a53f66e284e8f) ) // 63S281N near SEI0070BU
ROM_END

void mustache_state::init_mustache()
{
	int g1 = memregion("tiles")->bytes() / 3;
	int g2 = memregion("sprites")->bytes() / 2;
	uint8_t *gfx1 = memregion("tiles")->base();
	uint8_t *gfx2 = memregion("sprites")->base();
	std::vector<uint8_t> buf(g2 * 2);

	// BG data lines
	for (int i = 0; i < g1; i++)
	{
		buf[i] = bitswap<8>(gfx1[i], 0, 5, 2, 6, 4, 1, 7, 3);

		uint16_t w = (gfx1[i + g1] << 8) | gfx1[i + g1 * 2];
		w = bitswap<16>(w, 14, 1, 13, 5, 9, 2, 10, 6, 3, 8, 4, 15, 0, 11, 12, 7);

		buf[i + g1] = w >> 8;
		buf[i + g1 * 2] = w & 0xff;
	}

	// BG address lines
	for (int i = 0; i < 3 * g1; i++)
		gfx1[i] = buf[bitswap<16>(i, 15, 14, 13, 2, 1, 0, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3)];

	// SPR data lines
	for (int i = 0; i < g2; i++)
	{
		uint16_t w = (gfx2[i] << 8) | gfx2[i + g2];
		w = bitswap<16>(w, 5, 7, 11, 4, 15, 10, 3, 14, 9, 2, 13, 8, 1, 12, 0, 6);

		buf[i] = w >> 8;
		buf[i + g2] = w & 0xff;
	}

	// SPR address lines
	for (int i = 0; i < 2 * g2; i++)
		gfx2[i] = buf[bitswap<24>(i,23, 22, 21, 20, 19, 18, 17, 16, 15, 12, 11, 10, 9, 8, 7, 6, 5, 4, 13, 14, 3, 2, 1, 0)];
}

} // anonymous namespace


GAME( 1987, mustache,  0,        mustache, mustache, mustache_state, init_mustache, ROT90, "Seibu Kaihatsu (March license)",  "Mustache Boy (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, mustachei, mustache, mustache, mustache, mustache_state, init_mustache, ROT90, "Seibu Kaihatsu (IG SPA license)", "Mustache Boy (Italy)", MACHINE_SUPPORTS_SAVE )
