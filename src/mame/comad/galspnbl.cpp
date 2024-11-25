// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

Hot Pinball
Gals Pinball

driver by Nicola Salmoria

Notes:
- to start a 2 or more players game, press the start button multiple times
- the sprite graphics contain a "(c) Tecmo", and the sprite system is
  indeed similar to other Tecmo games like Ninja Gaiden.
- Clearly based on Tecmo's Super Pinball Action (see tecmo/spbactn.cpp)
- There seems to be a bug in Hot Pinball's Demo Sounds. If you start the
  game normally you get no demo sounds. However, if you select the "Slide
  Show" and run all the way through, the game will start with demo sounds.
  Gals Pinball's Demo Sounds works as expected according to DSW setting.

TODO:
- scrolling is wrong.
- sprite/tile priority might be wrong. There is an unknown bit in the fg
  tilemap, marking some tiles that I'm not currently drawing.

Manuals for both games define the controls as 4 push buttons:

   Left Push Buttons                     Right Push Buttons

|   o    |      o       |             |    o    |      o       |
|--------+--------------+-------------+------------------------|
| Select |     Set      | Select Mode |                        |
|  Stage |    Stage     |             |                        |
|--------+--------------+-------------+---------+--------------|
|Flipper | Shot & Shake |  Game Mode  | Flipper | Shot & Shake |
| Left   |    Left      |             |  Right  |    Right     |
|--------+--------------+-------------+---------+--------------|


  CPU: MC68000P10, Z80B
  OSC: 22.1184MHz by 68K CPU, 4MHz? OSC by Z80 & OKI 6295
Sound: Oki 6295, K-665 (rebadged YM3812)
Other: Actel A1020B 84pin PLCC, Cypress CY7384A 84pin PLCC
       2 8-switch dipswitches

Known undumped games/sets:

  Star Pinball
  Star Pinball II (non nude Hot Pinball with alternate title)
  Hot Pinball (non nude version)

***************************************************************************/

#include "emu.h"

#include "tecmo_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class galspnbl_state : public driver_device
{
public:
	galspnbl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_scroll(*this, "scroll"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void galspnbl(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_colorram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_scroll;

	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	void soundcommand_w(offs_t offset, uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	bitmap_ind16 m_sprite_bitmap;

	void mix_sprite_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};



void galspnbl_state::palette(palette_device &palette) const
{
	// initialize 555 RGB lookup
	for (int i = 0; i < 32768; i++)
		palette.set_pen_color(i + 1024, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}



void galspnbl_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  int const screenscroll = 4 - (m_scroll[0] & 0xff);

	for (offs_t offs = 0; offs < 0x20000; offs++)
	{
		int const y = offs >> 9;
		int const x = offs & 0x1ff;

		bitmap.pix(y, x) = 1024 + (m_bgvideoram[offs] >> 1);
	}
}

void galspnbl_state::video_start()
{
	// allocate bitmaps
	m_screen->register_screen_bitmap(m_sprite_bitmap);
}

void galspnbl_state::mix_sprite_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dd = &bitmap.pix(y);
		uint16_t const *const sd2 = &m_sprite_bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint16_t sprpixel = (sd2[x]);
			//uint16_t const sprpri = (sprpixel >> 8) & 3;
			uint16_t const sprpri = (sprpixel >> 9) & 1; // only upper priority bit matters on the bootleg hw?

			sprpixel &= 0xff;

			if (sprpixel & 0xf)
			{
				if (sprpri == pri)
					dd[x] = sprpixel;
			}

			//  uint16_t sprbln = (sprpixel >> 10) & 1; // we handle 'blending' from the original as a simple on/off flicker in the bootleg sprite function, I don't think the bootleg hw can blend
		}
	}
}

uint32_t galspnbl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprite_bitmap.fill(0, cliprect);
	m_sprgen->gaiden_draw_sprites(screen, m_sprite_bitmap, cliprect, m_spriteram, 0, 0, flip_screen());


	draw_background(bitmap, cliprect);

	mix_sprite_layer(screen, bitmap, cliprect, 0);

	for (int offs = 0; offs < 0x1000 / 2; offs++)
	{
		int const code = m_videoram[offs];
		int const attr = m_colorram[offs];
		int const color = (attr & 0x00f0) >> 4;
		int const sx = offs % 64;
		int const sy = offs / 64;

		// What is this? A priority/half transparency marker? */ // leftover blend flags from original spbactn game
		if (!(attr & 0x0008))
		{
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					color,
					0, 0,
//                  16 * sx + screenscroll, 8 * sy,
					16 * sx, 8 * sy, 0);
		}
	}

	mix_sprite_layer(screen, bitmap, cliprect, 1);


	return 0;
}


void galspnbl_state::soundcommand_w(offs_t offset, uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void galspnbl_state::main_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x700000, 0x703fff).ram();     // galspnbl work RAM
	map(0x708000, 0x70ffff).ram();     // galspnbl work RAM, bitmaps are decompressed here
	map(0x800000, 0x803fff).ram();     // hotpinbl work RAM
	map(0x808000, 0x80ffff).ram();     // hotpinbl work RAM, bitmaps are decompressed here
	map(0x880000, 0x880fff).ram().share(m_spriteram);
	map(0x8ff400, 0x8fffff).nopw();    // ???
	map(0x900000, 0x900fff).ram().share(m_colorram);
	map(0x901000, 0x903fff).nopw();    // ???
	map(0x904000, 0x904fff).ram().share(m_videoram);
	map(0x905000, 0x907fff).nopw();    // ???
	map(0x980000, 0x9bffff).ram().share(m_bgvideoram);
	map(0xa00000, 0xa00fff).nopw();    // more palette ?
	map(0xa01000, 0xa017ff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa01800, 0xa027ff).nopw();    // more palette ?
	map(0xa80000, 0xa80001).portr("IN0");
	map(0xa80010, 0xa80011).portr("IN1");
	map(0xa80011, 0xa80011).w(FUNC(galspnbl_state::soundcommand_w));
	map(0xa80020, 0xa80021).portr("SYSTEM").nopw();     // w - could be watchdog, but causes resets when picture is shown
	map(0xa80030, 0xa80031).portr("DSW1").nopw();       // w - irq ack?
	map(0xa80040, 0xa80041).portr("DSW2");
	map(0xa80050, 0xa80051).writeonly().share(m_scroll);    // ???
}

void galspnbl_state::audio_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf810, 0xf811).w("ymsnd", FUNC(ym3812_device::write));
	map(0xfc00, 0xfc00).noprw(); // irq ack ??
	map(0xfc20, 0xfc20).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( galspnbl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )            // tested at 0x0016c6 - doesn't seem tilt related
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            // tested at 0x0016d2 - doesn't seem tilt related
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flippers" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball / Shake (Left Side)" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )            // uncalled (?) code at 0x007ed2 ('hotpinbl') or 0x008106 ('galspnbl')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Launch Ball / Shake (Right Side)" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )            // uncalled (?) code at 0x007e90 ('hotpinbl') or 0x0080c4 ('galspnbl')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start" )  // needed to avoid confusion with # of players. Press multiple times for multiple players
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  // 0x700018.b
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2") // see code at 0x0085c6 ('hotpinbl') or 0x008994 ('galspnbl')
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Ball" )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "100K 500K" )
	PORT_DIPSETTING(    0x0c, "200K 800K" )
	PORT_DIPSETTING(    0x08, "200K only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, "Hit Difficulty" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Slide Show" )        PORT_DIPLOCATION("SW2:6") // Listed as unused on manuals for both games
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW2")  // 0x700019.b
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Balls" )         PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( hotpinbl )
	PORT_INCLUDE( galspnbl )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Launch Ball / Shake (Right Side)" )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4), STEP8(4*8*8, 4) },
	{ STEP8(0, 4*8) },
	16*8*4
};

static GFXDECODE_START( gfx_galspnbl )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,   512, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_galspnbl_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_packed_msb, 0, 16 )
GFXDECODE_END


void galspnbl_state::galspnbl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(22'118'400) / 2);    // 11.0592 MHz??? - NEEDS VERIFICATION!!
	m_maincpu->set_addrmap(AS_PROGRAM, &galspnbl_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(galspnbl_state::irq3_line_hold)); // also has vector for 6, but it does nothing

	Z80(config, m_audiocpu, XTAL(4'000'000));       // 4 MHz ??? - Use value from Tecmo's Super Pinball Action - NEEDS VERIFICATION!!
	m_audiocpu->set_addrmap(AS_PROGRAM, &galspnbl_state::audio_map);    // NMI is caused by the main CPU

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(galspnbl_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galspnbl);
	PALETTE(config, m_palette, FUNC(galspnbl_state::palette)).set_format(palette_device::xBGR_444, 1024 + 32768);

	TECMO_SPRITE(config, m_sprgen, 0, m_palette, gfx_galspnbl_spr);
	m_sprgen->set_bootleg(true);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(4'000'000))); // Use value from Super Pinball Action - NEEDS VERIFICATION!!
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	// Use value from Super Pinball Action - clock frequency & pin 7 not verified
	okim6295_device &oki(OKIM6295(config, "oki", XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galspnbl )
	ROM_REGION( 0x400000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "7.rom",        0x000000, 0x80000, CRC(ce0189bf) SHA1(06d8cc6f5b819fe2ca536ce553db6986547a15ba) )
	ROM_LOAD16_BYTE( "3.rom",        0x000001, 0x80000, CRC(9b0a8744) SHA1(ac80f22b8b2f4c559c225bf203af698bf59699e7) )
	ROM_LOAD16_BYTE( "8.rom",        0x100000, 0x80000, CRC(eee2f087) SHA1(37285ae7b49c9d20ad92b3971db89ba593975154) )
	ROM_LOAD16_BYTE( "4.rom",        0x100001, 0x80000, CRC(56298489) SHA1(6dce296d752fa5579e8a3c1db3e563c4f98b6bae) )
	ROM_LOAD16_BYTE( "9.rom",        0x200000, 0x80000, CRC(d9e4964c) SHA1(0bb63384ed57a6ee1d45d1f2c181172a3f0b39df) )
	ROM_LOAD16_BYTE( "5.rom",        0x200001, 0x80000, CRC(a5e71ee4) SHA1(f41ea4ab07be263157d6c1f4a96ef45c9bccfe39) )
	ROM_LOAD16_BYTE( "10.rom",       0x300000, 0x80000, CRC(3a20e1e5) SHA1(850be621547bc9c7519055211392f2684e440462) )
	ROM_LOAD16_BYTE( "6.rom",        0x300001, 0x80000, CRC(94927d20) SHA1(0ea1a179956ad9a93a99cccec92f0490044ad1d3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "2.rom",        0x0000, 0x10000, CRC(fae688a7) SHA1(e1ef7abd18f6a820d1a7f0ceb9a9b1a2c7de41f0) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "17.rom",       0x00000, 0x40000, CRC(7d435701) SHA1(f6a2241c95f101d09b18f550689d125abd3ea9c4) )
	ROM_LOAD16_BYTE( "18.rom",       0x00001, 0x40000, CRC(136adaac) SHA1(5f5e70a66d256cad9fcdbc5a7fff035f9a3279b9) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD16_BYTE( "15.rom",       0x00000, 0x20000, CRC(4beb840d) SHA1(351cd8da361a55794595d2cf7b0fed9233d0a5a0) )
	ROM_LOAD16_BYTE( "16.rom",       0x00001, 0x20000, CRC(93d3c610) SHA1(0cf1f311ec2646a436c37e121634731646c06437) )

	ROM_REGION( 0x40000, "oki", 0 ) // samples
	ROM_LOAD( "1.rom",        0x00000, 0x40000, CRC(93c06d3d) SHA1(8620d274ca7824e7e72a1ad1da3eaa804d550653) )
ROM_END

ROM_START( hotpinbl ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 950804 MADE IN KOREA
	ROM_REGION( 0x400000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "hp_07.bin",    0x000000, 0x80000, CRC(978cc13e) SHA1(0060aaf7259fdeeacb07e9ced01bdf69c27bdfb6) )
	ROM_LOAD16_BYTE( "hp_03.bin",    0x000001, 0x80000, CRC(68388726) SHA1(d8dca9050403be70097a0f833ba189bd2fa87e80) )
	ROM_LOAD16_BYTE( "hp_08.bin",    0x100000, 0x80000, CRC(bd16be12) SHA1(36e64705efba8ecdc96a62f55d68e959022fb98f) )
	ROM_LOAD16_BYTE( "hp_04.bin",    0x100001, 0x80000, CRC(655b0cf0) SHA1(4348d774155c61b1a29c2618f472a0032f4be9c0) )
	ROM_LOAD16_BYTE( "hp_09.bin",    0x200000, 0x80000, CRC(a6368624) SHA1(fa0b3dc4eb33a4b2d00e30676e59feec2fa64c3d) )
	ROM_LOAD16_BYTE( "hp_05.bin",    0x200001, 0x80000, CRC(48efd028) SHA1(8c968c4a73069a8f64e7bc339bab0eeb5254c580) )
	ROM_LOAD16_BYTE( "hp_10.bin",    0x300000, 0x80000, CRC(a5c63e34) SHA1(de27cfe20e09e8c13ee28d6eb42bfab1ebe33149) )
	ROM_LOAD16_BYTE( "hp_06.bin",    0x300001, 0x80000, CRC(513eda91) SHA1(14a43c00ad1f55bff525a14cd53913dd78e80f0c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "hp_02.bin",    0x0000, 0x10000, CRC(82698269) SHA1(5e27e89f1bdd7c3793d40867c50981f5fac0a7fb) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "hp_13.bin",    0x00000, 0x40000, CRC(d53b64b9) SHA1(347b6ec908e23f848e98eed46fb34b49b728556b) )
	ROM_LOAD16_BYTE( "hp_14.bin",    0x00001, 0x40000, CRC(2fe3fcee) SHA1(29f96aa3dded6cb0b2fe3d9507fb5638e9778ef3) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD16_BYTE( "hp_11.bin",    0x00000, 0x20000, CRC(deecd7f1) SHA1(752c944d941bfe8f21d32881f32676999ebc5a7f) )
	ROM_LOAD16_BYTE( "hp_12.bin",    0x00001, 0x20000, CRC(5fd603c2) SHA1(864686cd1ba5beb6cebfd394b60620106c929abd) )

	ROM_REGION( 0x40000, "oki", 0 ) // samples
	ROM_LOAD( "hp_01.bin",    0x00000, 0x40000, CRC(93c06d3d) SHA1(8620d274ca7824e7e72a1ad1da3eaa804d550653) )
ROM_END

} // anonymous namespace


GAME( 1995, hotpinbl, 0, galspnbl, hotpinbl, galspnbl_state, empty_init, ROT90, "Comad & New Japan System", "Hot Pinball",  MACHINE_SUPPORTS_SAVE )
GAME( 1996, galspnbl, 0, galspnbl, galspnbl, galspnbl_state, empty_init, ROT90, "Comad",                    "Gals Pinball", MACHINE_SUPPORTS_SAVE )
