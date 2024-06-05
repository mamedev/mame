// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/***************************************************************************
  GOINDOL

  Driver provided by Jarek Parchanski (jpdev@friko6.onet.pl)

Notes:
- byte at 7f87 controls region:
  0 = Japan
  1 = USA
  2 = World
  Regardless of the setting of this byte, the startup notice in Korean is
  always displayed.
  After the title screen, depending on the byte you get "for use only in Japan",
  "for use only in USA", or the Korean notice again! So 2 might actually mean
  Korea instead of World... but that version surely got to Europe since Gerald
  has three boards with this ROM.
- near the main Z80 there's a huge epoxy block, probably covering the protection
  logic or MCU.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PROTECTION     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PROTECTION)

#include "logmacro.h"

#define LOGPROTECTION(...)     LOGMASKED(LOG_PROTECTION,     __VA_ARGS__)


namespace {

class goindol_state : public driver_device
{
public:
	goindol_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_fg_scrolly(*this, "fg_scrolly"),
		m_fg_scrollx(*this, "fg_scrollx"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void goindol(machine_config &config);
	void init_goindol();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_fg_scrolly;
	required_shared_ptr<uint8_t> m_fg_scrollx;
	required_shared_ptr_array<uint8_t ,2> m_spriteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_memory_bank m_mainbank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint16_t m_char_bank;

	// misc
	uint8_t m_prot_toggle;

	void bankswitch_w(uint8_t data);
	uint8_t prot_f422_r();
	void prot_fc44_w(uint8_t data);
	void prot_fd99_w(uint8_t data);
	void prot_fc66_w(uint8_t data);
	void prot_fcb0_w(uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfxbank, uint8_t *sprite_ram);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(goindol_state::get_fg_tile_info)
{
	int code = m_fg_videoram[2 * tile_index + 1];
	int attr = m_fg_videoram[2 * tile_index];
	tileinfo.set(0,
			code | ((attr & 0x7) << 8) | (m_char_bank << 11),
			(attr & 0xf8) >> 3,
			0);
}

TILE_GET_INFO_MEMBER(goindol_state::get_bg_tile_info)
{
	int code = m_bg_videoram[2 * tile_index + 1];
	int attr = m_bg_videoram[2 * tile_index];
	tileinfo.set(1,
			code | ((attr & 0x7) << 8) | (m_char_bank << 11),
			(attr & 0xf8) >> 3,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void goindol_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goindol_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goindol_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void goindol_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void goindol_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void goindol_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfxbank, uint8_t *sprite_ram)
{
	for (int offs = 0; offs < m_spriteram[0].bytes(); offs += 4)
	{
		int sx = sprite_ram[offs];
		int sy = 240 - sprite_ram[offs + 1];

		if (flip_screen())
		{
			sx = 248 - sx;
			sy = 248 - sy;
		}

		if ((sprite_ram[offs + 1] >> 3) && (sx < 248))
		{
			int tile = ((sprite_ram[offs + 3]) + ((sprite_ram[offs + 2] & 7) << 8));
			tile += tile;
			int palette = sprite_ram[offs + 2] >> 3;


						m_gfxdecode->gfx(gfxbank)->transpen(bitmap, cliprect,
						tile,
						palette,
						flip_screen(), flip_screen(),
						sx, sy, 0);

						m_gfxdecode->gfx(gfxbank)->transpen(bitmap, cliprect,
						tile + 1,
						palette,
						flip_screen(), flip_screen(),
						sx, sy + (flip_screen() ? -8 : 8), 0);
		}
	}
}

uint32_t goindol_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->set_scrollx(0, *m_fg_scrollx);
	m_fg_tilemap->set_scrolly(0, *m_fg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 1, m_spriteram[0]);
	draw_sprites(bitmap, cliprect, 0, m_spriteram[1]);
	return 0;
}


void goindol_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x03);

	if (m_char_bank != ((data & 0x10) >> 4))
	{
		m_char_bank = (data & 0x10) >> 4;
		machine().tilemap().mark_all_dirty();
	}

	flip_screen_set(data & 0x20);
}



uint8_t goindol_state::prot_f422_r()
{
	// bit 7 = vblank?
	m_prot_toggle ^= 0x80;

	return m_prot_toggle;
}


void goindol_state::prot_fc44_w(uint8_t data)
{
	LOGPROTECTION("%04x: prot_fc44_w(%02x)\n", m_maincpu->pc(), data);
	m_ram[0x0419] = 0x5b;
	m_ram[0x041a] = 0x3f;
	m_ram[0x041b] = 0x6d;
}

void goindol_state::prot_fd99_w(uint8_t data)
{
	LOGPROTECTION("%04x: prot_fd99_w(%02x)\n", m_maincpu->pc(), data);
	m_ram[0x0421] = 0x3f;
}

void goindol_state::prot_fc66_w(uint8_t data)
{
	LOGPROTECTION("%04x: prot_fc66_w(%02x)\n", m_maincpu->pc(), data);
	m_ram[0x0423] = 0x06;
}

void goindol_state::prot_fcb0_w(uint8_t data)
{
	LOGPROTECTION("%04x: prot_fcb0_w(%02x)\n", m_maincpu->pc(), data);
	m_ram[0x0425] = 0x06;
}



void goindol_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc7ff).ram().share(m_ram);
	map(0xc800, 0xc800).nopr().w("soundlatch", FUNC(generic_latch_8_device::write)); // watchdog?
	map(0xc810, 0xc810).w(FUNC(goindol_state::bankswitch_w));
	map(0xc820, 0xc820).portr("DIAL").writeonly().share(m_fg_scrolly);
	map(0xc830, 0xc830).portr("P1").writeonly().share(m_fg_scrollx);
	map(0xc834, 0xc834).portr("P2");
	map(0xd000, 0xd03f).ram().share(m_spriteram[0]);
	map(0xd040, 0xd7ff).ram();
	map(0xd800, 0xdfff).ram().w(FUNC(goindol_state::bg_videoram_w)).share(m_bg_videoram);
	map(0xe000, 0xe03f).ram().share(m_spriteram[1]);
	map(0xe040, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().w(FUNC(goindol_state::fg_videoram_w)).share(m_fg_videoram);
	map(0xf000, 0xf000).portr("DSW1");
	map(0xf422, 0xf422).r(FUNC(goindol_state::prot_f422_r));
	map(0xf800, 0xf800).portr("DSW2");
	map(0xfc44, 0xfc44).w(FUNC(goindol_state::prot_fc44_w));
	map(0xfc66, 0xfc66).w(FUNC(goindol_state::prot_fc66_w));
	map(0xfcb0, 0xfcb0).w(FUNC(goindol_state::prot_fcb0_w));
	map(0xfd99, 0xfd99).w(FUNC(goindol_state::prot_fd99_w));
}

void goindol_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa001).w("ymsnd", FUNC(ym2203_device::write));
	map(0xc000, 0xc7ff).ram();
	map(0xd800, 0xd800).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( goindol )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("DIAL")      // spinner
	PORT_BIT( 0xff, 0x00, IPT_DIAL  ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x1c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x1c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x18, "Very Very Easy" )
	PORT_DIPSETTING(    0x14, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x04, "30k and every 50k" )
	PORT_DIPSETTING(    0x05, "50k and every 100k" )
	PORT_DIPSETTING(    0x06, "50k and every 200k" )
	PORT_DIPSETTING(    0x07, "100k and every 200k" )
	PORT_DIPSETTING(    0x01, "10000 only" )
	PORT_DIPSETTING(    0x02, "30000 only" )
	PORT_DIPSETTING(    0x03, "50000 only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( homo )
	PORT_INCLUDE( goindol )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{  RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_goindol )
	GFXDECODE_ENTRY( "fg_chars", 0, charlayout, 0, 32 )
	GFXDECODE_ENTRY( "bg_chars", 0, charlayout, 0, 32 )
GFXDECODE_END



void goindol_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_char_bank));
	save_item(NAME(m_prot_toggle));
}

void goindol_state::machine_reset()
{
	m_char_bank = 0;
	m_prot_toggle = 0;
}

void goindol_state::goindol(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2);  // XTAL confirmed, divisor is not
	m_maincpu->set_addrmap(AS_PROGRAM, &goindol_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(goindol_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(12'000'000) / 2)); // XTAL confirmed, divisor is not
	audiocpu.set_addrmap(AS_PROGRAM, &goindol_state::sound_map);
	audiocpu.set_periodic_int(FUNC(goindol_state::irq0_line_hold), attotime::from_hz(4*60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(goindol_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goindol);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ymsnd", XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.25);   // Confirmed pitch from recording
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goindol )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1w", 0x00000, 0x8000, CRC(df77c502) SHA1(15d111e38d63a8a800fbf5f15c4fb72efb0e5cf4) ) // Code
	ROM_LOAD( "r2",  0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) // Paged data
	ROM_LOAD( "r3",  0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) // Paged data

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "fg_chars", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) )
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "bg_chars", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) // palette red bits
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) // palette green bits
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) // palette blue bits
ROM_END

ROM_START( goindolu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1", 0x00000, 0x8000, CRC(3111c61b) SHA1(6cc3834f946566646f06efe0b65c4704574ec6f1) ) // Code
	ROM_LOAD( "r2", 0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) // Paged data
	ROM_LOAD( "r3", 0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) // Paged data

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "fg_chars", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) )
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "bg_chars", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) // palette red bits
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) // palette green bits
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) // palette blue bits
ROM_END

ROM_START( goindolj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1j", 0x00000, 0x8000, CRC(dde33ad3) SHA1(23cdb3494f5eeaeae2657a0101d5827aa32c526d) ) // Code
	ROM_LOAD( "r2",  0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) // Paged data
	ROM_LOAD( "r3",  0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) // Paged data

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "fg_chars", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) )
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "bg_chars", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) // palette red bits
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) // palette green bits
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) // palette blue bits
ROM_END

ROM_START( homo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "homo.01", 0x00000, 0x8000, CRC(28c539ad) SHA1(64e950a4238a5656a9e0d0a699a6545da8c59548) ) // Code
	ROM_LOAD( "r2", 0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) // Paged data
	ROM_LOAD( "r3", 0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) // Paged data

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "fg_chars", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) )
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "bg_chars", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) // palette red bits
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) // palette green bits
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) // palette blue bits
ROM_END



void goindol_state::init_goindol()
{
	uint8_t *rom = memregion("maincpu")->base();


	// I hope that's all patches to avoid protection

	rom[0x18e9] = 0x18; // ROM 1 check
	rom[0x1964] = 0x00; // ROM 9 error (MCU?)
	rom[0x1965] = 0x00; //
	rom[0x1966] = 0x00; //
//  rom[0x17c7] = 0x00; // c421 == 3f
//  rom[0x17c8] = 0x00; //
//  rom[0x16f0] = 0x18; // c425 == 06
//  rom[0x172c] = 0x18; // c423 == 06
//  rom[0x1779] = 0x00; // c419 == 5b 3f 6d
//  rom[0x177a] = 0x00; //
	rom[0x063f] = 0x18; //->fc55
	rom[0x0b30] = 0x00; // verify code at 0601-064b
	rom[0x1bdf] = 0x18; //->fc49

	rom[0x04a7] = 0xc9;
	rom[0x0831] = 0xc9;
	rom[0x3365] = 0x00; // verify code at 081d-0876
	rom[0x0c13] = 0xc9;
	rom[0x134e] = 0xc9;
	rom[0x333d] = 0xc9;
}

} // anonymous namespace


GAME( 1987, goindol,  0,       goindol, goindol, goindol_state, init_goindol, ROT90, "SunA",    "Goindol (SunA, World)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1987, goindolu, goindol, goindol, goindol, goindol_state, init_goindol, ROT90, "SunA",    "Goindol (SunA, US)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1987, goindolj, goindol, goindol, goindol, goindol_state, init_goindol, ROT90, "SunA",    "Goindol (SunA, Japan)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1987, homo,     goindol, goindol, homo,    goindol_state, empty_init,   ROT90, "bootleg", "Homo",                  MACHINE_SUPPORTS_SAVE )
