// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni

/***************************************************************************

Solomon's Key

driver by Mirko Buffoni

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class solomon_state : public driver_device
{
public:
	solomon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void solomon(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	uint8_t m_nmi_mask = 0;

	void sh_command_w(uint8_t data);
	uint8_t _0xe603_r();
	void nmi_mask_w(uint8_t data);
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};


template <uint8_t Which>
void solomon_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	Which ? m_bg_tilemap->mark_tile_dirty(offset) : m_fg_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void solomon_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	Which ? m_bg_tilemap->mark_tile_dirty(offset) : m_fg_tilemap->mark_tile_dirty(offset);
}

void solomon_state::flipscreen_w(uint8_t data)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(solomon_state::get_bg_tile_info)
{
	int const attr = m_colorram[1][tile_index];
	int const code = m_videoram[1][tile_index] + 256 * (attr & 0x07);
	int const color = ((attr & 0x70) >> 4);
	int const flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x08) ? TILE_FLIPY : 0);

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(solomon_state::get_fg_tile_info)
{
	int const attr = m_colorram[0][tile_index];
	int const code = m_videoram[0][tile_index] + 256 * (attr & 0x07);
	int const color = (attr & 0x70) >> 4;

	tileinfo.set(0, code, color, 0);
}

void solomon_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(solomon_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(solomon_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void solomon_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int const code = m_spriteram[offs] + 16 * (m_spriteram[offs + 1] & 0x10);
		int const color = (m_spriteram[offs + 1] & 0x0e) >> 1;
		int flipx = m_spriteram[offs + 1] & 0x40;
		int flipy = m_spriteram[offs + 1] & 0x80;
		int sx = m_spriteram[offs + 3];
		int sy = 241 - m_spriteram[offs + 2];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 242 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

uint32_t solomon_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void solomon_state::sh_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/* this is checked on the title screen and when you reach certain scores in the game
   it could be a form of protection.  the real board needs to be analysed to find out
   what really lives here */

uint8_t solomon_state::_0xe603_r()
{
	if (m_maincpu->pc() == 0x161) // all the time .. return 0 to act as before  for coin / startup etc.
	{
		return 0;
	}
	else if (m_maincpu->pc() == 0x4cf0) // stop it clearing the screen at certain scores
	{
		return (m_maincpu->state_int(Z80_BC) & 0x08);
	}
	else
	{
		osd_printf_debug("unhandled _0xe603_r %04x\n", m_maincpu->pc());
		return 0;
	}
}

void solomon_state::nmi_mask_w(uint8_t data)
{
	m_nmi_mask = data & 1;
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void solomon_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd3ff).ram().w(FUNC(solomon_state::colorram_w<0>)).share(m_colorram[0]);
	map(0xd400, 0xd7ff).ram().w(FUNC(solomon_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xd800, 0xdbff).ram().w(FUNC(solomon_state::colorram_w<1>)).share(m_colorram[1]);
	map(0xdc00, 0xdfff).ram().w(FUNC(solomon_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xe000, 0xe07f).ram().share(m_spriteram);
	map(0xe400, 0xe5ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe600, 0xe600).portr("P1");
	map(0xe601, 0xe601).portr("P2");
	map(0xe602, 0xe602).portr("SYSTEM");
	map(0xe603, 0xe603).r(FUNC(solomon_state::_0xe603_r));
	map(0xe604, 0xe604).portr("DSW1");
	map(0xe605, 0xe605).portr("DSW2");
	map(0xe606, 0xe606).nopr(); // watchdog?
	map(0xe600, 0xe600).w(FUNC(solomon_state::nmi_mask_w));
	map(0xe604, 0xe604).w(FUNC(solomon_state::flipscreen_w));
	map(0xe800, 0xe800).w(FUNC(solomon_state::sh_command_w));
	map(0xf000, 0xffff).rom();
}

void solomon_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xffff, 0xffff).nopw();    // watchdog?
}

void solomon_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x11).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x20, 0x21).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x30, 0x31).w("ay3", FUNC(ay8910_device::address_data_w));
}



static INPUT_PORTS_START( solomon )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x0c, 0x00, "Timer Speed" )       PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Faster" )
	PORT_DIPSETTING(    0x0c, "Fastest" )
	PORT_DIPNAME( 0x10, 0x00, "Extra" )         PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "30k 200k 500k" )
	PORT_DIPSETTING(    0x80, "100k 300k 800k" )
	PORT_DIPSETTING(    0x40, "30k 200k" )
	PORT_DIPSETTING(    0xc0, "100k 300k" )
	PORT_DIPSETTING(    0x20, "30k" )
	PORT_DIPSETTING(    0xa0, "100k" )
	PORT_DIPSETTING(    0x60, "200k" )
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )
INPUT_PORTS_END



static const gfx_layout spritelayout =
{
	16,16,  // 8*8 sprites
	512,    // 512 sprites
	4,      // 4 bits per pixel
	{ 0, 512*32*8, 2*512*32*8, 3*512*32*8 },    // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7,   // pretty straightforward layout
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    // every sprite takes 32 consecutive bytes
};

static GFXDECODE_START( gfx_solomon )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_packed_msb,     0, 8 )  // colors   0-127
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x4_packed_msb,   128, 8 )  // colors 128-255
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,             0, 8 )  // colors   0-127
GFXDECODE_END

void solomon_state::vblank_w(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}



void solomon_state::solomon(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4'000'000);   // 4.0 MHz (?????)
	m_maincpu->set_addrmap(AS_PROGRAM, &solomon_state::main_map);

	Z80(config, m_audiocpu, 3'072'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &solomon_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &solomon_state::sound_portmap);
	m_audiocpu->set_periodic_int(FUNC(solomon_state::irq0_line_hold), attotime::from_hz(2*60));   // ???
						// NMIs are caused by the main CPU

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(solomon_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(solomon_state::vblank_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_solomon);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	// bootlegs may use 3x AY-3-8913 instead
	YM2149(config, "ay1", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.12);
	YM2149(config, "ay2", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.12);
	YM2149(config, "ay3", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.12);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( solomon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6.3f",  0x00000, 0x4000, CRC(645eb0f3) SHA1(b911aa157ad94aa539dbe329a197d8902860c809) )
	ROM_LOAD( "7.3h",  0x08000, 0x4000, CRC(1bf5c482) SHA1(3b6a8dde72cddf95438539c5dc4622c95f932a04) )
	ROM_CONTINUE(      0x04000, 0x4000 )
	ROM_LOAD( "8.3jk", 0x0f000, 0x1000, CRC(0a6cdefc) SHA1(101acaa19b779cb8b4fffddbe63fe011c7d4b6e9) )
	ROM_IGNORE(                 0x7000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.3jk",  0x0000, 0x4000, CRC(fa6e562e) SHA1(713036c0a80b623086aa674bb5f8a135b6fedb01) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "12.3t",  0x00000, 0x08000, CRC(b371291c) SHA1(27302898c64330870c47025e61bd5acbd9483865) )
	ROM_LOAD( "11.3r",  0x08000, 0x08000, CRC(6f94d2af) SHA1(2e070c0fd5b9d7eb9b7e0d53f25b1a5063ef3095) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "10.3p",  0x00000, 0x08000, CRC(8310c2a1) SHA1(8cc87ab8faacdb1973791d207bb25ea9b444b66d) )
	ROM_LOAD( "9.3m",   0x08000, 0x08000, CRC(ab7e6c42) SHA1(0fc3b4a0bd2b17b79e2d1f7d4fe445c09ce4e730) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "2.5lm",  0x00000, 0x04000, CRC(80fa2be3) SHA1(8e7a78186473a6b5c42577ac9e4591ee2d1151f2) )
	ROM_LOAD( "3.6lm",  0x04000, 0x04000, CRC(236106b4) SHA1(8eaf3150568c407bd8dc1cdf874b8417e5cca3d2) )
	ROM_LOAD( "4.7lm",  0x08000, 0x04000, CRC(088fe5d9) SHA1(e29ffb9fcff50ce982d5e502e10a8e29a4c47390) )
	ROM_LOAD( "5.8lm",  0x0c000, 0x04000, CRC(8366232a) SHA1(1c7a01dab056ec7d787a6f55772b9fa6fe67305a) )
ROM_END


ROM_START( solomonj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "slmn_06.bin",  0x00000, 0x4000, CRC(e4d421ff) SHA1(9599fa6e2d42bf0cfe77d62c6b162f56eae5efff) )
	ROM_LOAD( "slmn_07.bin",  0x08000, 0x4000, CRC(d52d7e38) SHA1(8439eeeedd1e47d2b9719a05c85a05283c11d7a8) )
	ROM_CONTINUE(             0x04000, 0x4000 )
	ROM_LOAD( "slmn_08.bin",  0x0f000, 0x1000, CRC(b924d162) SHA1(6299b791ec874bc3ef0424b277ec8a736c8cdd9a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "slmn_01.bin",  0x0000, 0x4000, CRC(fa6e562e) SHA1(713036c0a80b623086aa674bb5f8a135b6fedb01) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "slmn_12.bin",  0x00000, 0x08000, CRC(aa26dfcb) SHA1(71748eaceeca878ae9f871e30d5951ca4dde37d6) )
	ROM_LOAD( "slmn_11.bin",  0x08000, 0x08000, CRC(6f94d2af) SHA1(2e070c0fd5b9d7eb9b7e0d53f25b1a5063ef3095) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "slmn_10.bin",  0x00000, 0x08000, CRC(8310c2a1) SHA1(8cc87ab8faacdb1973791d207bb25ea9b444b66d) )
	ROM_LOAD( "slmn_09.bin",  0x08000, 0x08000, CRC(ab7e6c42) SHA1(0fc3b4a0bd2b17b79e2d1f7d4fe445c09ce4e730) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "slmn_02.bin",  0x00000, 0x04000, CRC(80fa2be3) SHA1(8e7a78186473a6b5c42577ac9e4591ee2d1151f2) )
	ROM_LOAD( "slmn_03.bin",  0x04000, 0x04000, CRC(236106b4) SHA1(8eaf3150568c407bd8dc1cdf874b8417e5cca3d2) )
	ROM_LOAD( "slmn_04.bin",  0x08000, 0x04000, CRC(088fe5d9) SHA1(e29ffb9fcff50ce982d5e502e10a8e29a4c47390) )
	ROM_LOAD( "slmn_05.bin",  0x0c000, 0x04000, CRC(8366232a) SHA1(1c7a01dab056ec7d787a6f55772b9fa6fe67305a) )
ROM_END

} // anonymous namespace


GAME( 1986, solomon,  0,       solomon, solomon, solomon_state, empty_init, ROT0, "Tecmo", "Solomon's Key (US)",      MACHINE_SUPPORTS_SAVE )
GAME( 1986, solomonj, solomon, solomon, solomon, solomon_state, empty_init, ROT0, "Tecmo", "Solomon no Kagi (Japan)", MACHINE_SUPPORTS_SAVE )
