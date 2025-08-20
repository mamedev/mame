// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Coreland / Sega 119 (C) 1986, limited release

Main PCB:
- 18.432MHz XTAL, NEC D780-C
- 4*2764 ROM (program data), 3*2764 ROM (gfx data), 2*6331-1N (PROM)
- 3*TC5517AP (2Kx8), 2*NMC2148 (1Kx4)
- unpopulated Alpha Denshi 8302 protection MCU with support chips
- 2*8-pos DSW

Sound PCB:
- 8MHz XTAL, NEC D780-C
- AY-3-8910, DAC0808LCN MC1408P8
- 2*2764 ROM, HM6116LP-4 (2Kx8)
- 8-pos DSW (why?)

Dumped from a bootleg PCB (no Sega labels anywhere), although it's possible that
the ROMs were unmodified. The MCU appears to be unpopulated on the genuine PCB too:
If you look closely at the PCB photo ( https://www.higenekodo.jp/untiku/119.htm ),
the protusions of 3 of the support chips aren't there.

The hardware is quite Galaxian-like (background + sprites + bullets) but tiles
are 3bpp, not 2bpp, and there is extra sound hardware. The hardware is outdated
for 1986, it's rumoured that the game is actually from 1983 (see Sega PCB label).

TODO:
- Sometimes killed by something invisible at the top-right of the 1st stage? It's
  not a hidden sprite, and whatever's there can't be defeated by spraying water.
  Suspected BTANB considering the game's unpolished state, needs PCB verification.
- bullets sometimes get stuck on screen (and need verifying)
- remaining dipswitches

*/

#include "emu.h"
#include "galaxian.h"

#include "cpu/z80/z80.h"

#include "speaker.h"

namespace {

class _119_state : public galaxian_state
{
public:
	_119_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaxian_state(mconfig, type, tag),
		m_dac(*this, "dac")
	{ }

	void _119(machine_config &config);

	void init_119();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void sprites_clip(screen_device &screen, rectangle &cliprect) override;

private:
	required_device<mc1408_device> m_dac;

	u8 m_bankdata = 0;
	bool m_sound_nmi_enable = false;

	void extend_sprite_info(const u8 *base, u8 *sx, u8 *sy, u8 *flipx, u8 *flipy, u16 *code, u8 *color);
	void extend_tile_info(u16 *code, u8 *color, u8 attrib, u8 x, u8 y);

	void tilebanks_flipscreen_w(u8 data);
	void sound_nmi_enable_w(u8 data);

	void prg_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(sound_nmi);
};

void _119_state::machine_start()
{
	galaxian_state::machine_start();

	save_item(NAME(m_bankdata));
	save_item(NAME(m_sound_nmi_enable));
}

void _119_state::machine_reset()
{
	galaxian_state::machine_reset();

	m_sound_nmi_enable = false;
}

INTERRUPT_GEN_MEMBER(_119_state::sound_nmi)
{
	if (m_sound_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void _119_state::extend_sprite_info(const u8 *base, u8 *sx, u8 *sy, u8 *flipx, u8 *flipy, u16 *code, u8 *color)
{
	if (m_flipscreen_x)
		*sx -= 2;

	if (m_bankdata & 0x04)
		*code |= 0xc0;
	else
		*code |= 0x40;
}

void _119_state::extend_tile_info(u16 *code, u8 *color, u8 attrib, u8 x, u8 y)
{
	if (m_bankdata & 0x08)
		*code |= 0x200;
}

void _119_state::sprites_clip(screen_device &screen, rectangle &cliprect)
{
	rectangle clip = screen.visible_area();
	if (m_flipscreen_x)
		clip.min_x += (64 * m_x_scale);
	else
		clip.max_x -= (64 * m_x_scale);

	cliprect &= clip;
}

void _119_state::tilebanks_flipscreen_w(u8 data)
{
	// ---- bbff
	// bb = sprite and tile banks, uncertain which is which
	// ff = flipscreen (could be separate sprite/tile flip, or separate x/y flip, we treat it as the latter)

	if ((data ^ m_bankdata) & 0x08)
		m_bg_tilemap->mark_all_dirty();
	m_bankdata = data;

	m_flipscreen_x = data & 0x01;
	m_flipscreen_y = data & 0x02;
	m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
}

void _119_state::sound_nmi_enable_w(u8 data)
{
	m_sound_nmi_enable = BIT(data, 6);
}

void _119_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(galaxian_state::galaxian_videoram_w)).share("videoram");
	map(0x9000, 0x90ff).ram().w(FUNC(galaxian_state::galaxian_objram_w)).share("spriteram");

	map(0xa000, 0xa000).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xb000, 0xb000).portr("IN0").w(FUNC(_119_state::tilebanks_flipscreen_w));
	map(0xb001, 0xb001).portr("IN1");
	map(0xb002, 0xb002).portr("DSW1");
	map(0xb003, 0xb003).portr("DSW2");

	map(0xe000, 0xefff).ram();
}

void _119_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x83ff).ram();
}

void _119_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(m_ay8910[0], FUNC(ay8910_device::data_address_w));
	map(0x02, 0x02).w(FUNC(_119_state::sound_nmi_enable_w)); // same as 0x04?
	map(0x03, 0x03).w(m_dac, FUNC(mc1408_device::write));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(FUNC(_119_state::sound_nmi_enable_w));
}

static INPUT_PORTS_START( 119 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // game speed is not driven by interrupts, polls bit in port

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	16*16
};

static GFXDECODE_START( gfx_119 )
	GFXDECODE_SCALE( "tiles", 0, gfx_8x8x3_planar, 0, 4, GALAXIAN_XSCALE, 1)
	GFXDECODE_SCALE( "tiles", 0, spritelayout, 0, 4, GALAXIAN_XSCALE, 1)
GFXDECODE_END

void _119_state::_119(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, GALAXIAN_PIXEL_CLOCK / 3 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &_119_state::prg_map);
	// nmi is unused (just returns), timing is done by polling vblank

	Z80(config, m_audiocpu, 8_MHz_XTAL/2);
	m_audiocpu->set_periodic_int(FUNC(_119_state::sound_nmi), attotime::from_hz(8_MHz_XTAL / 0x800));
	m_audiocpu->set_addrmap(AS_PROGRAM, &_119_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &_119_state::sound_io_map);

	//WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8); // does it exist on this hardware?

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_119);
	PALETTE(config, m_palette, FUNC(_119_state::galaxian_palette), 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(GALAXIAN_PIXEL_CLOCK, GALAXIAN_HTOTAL, GALAXIAN_HBEND, GALAXIAN_HBSTART, GALAXIAN_VTOTAL, GALAXIAN_VBEND, GALAXIAN_VBSTART);
	m_screen->set_screen_update(FUNC(_119_state::screen_update_galaxian));
	m_screen->screen_vblank().set(FUNC(_119_state::vblank_interrupt_w));

	// sound hardware
	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	SPEAKER(config, "speaker").front_center();

	AY8910(config, m_ay8910[0], 8_MHz_XTAL / 4);
	m_ay8910[0]->add_route(ALL_OUTPUTS, "speaker", 0.25);

	MC1408(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

void _119_state::init_119()
{
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, nullptr, nullptr);

	m_extend_sprite_info_ptr = extend_sprite_info_delegate(&_119_state::extend_sprite_info, this);
	m_extend_tile_info_ptr = extend_tile_info_delegate(&_119_state::extend_tile_info, this);
}

ROM_START( 119 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "119_4", 0x0000, 0x2000, CRC(b614229e) SHA1(06d0b17f5ff12222c74ff9325c21268bef25446e) )
	ROM_LOAD( "119_3", 0x2000, 0x2000, CRC(d2a984bf) SHA1(d2d5a83deff894978394461f8779d296b855971f) )
	ROM_LOAD( "119_2", 0x4000, 0x2000, CRC(d96611bc) SHA1(fffb516f00e747931941844b5358fe46d656bfb8) )
	ROM_LOAD( "119_1", 0x6000, 0x2000, CRC(368723e2) SHA1(515724eed41138e2e852f53d63f9a226584126f5) )

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "119_6", 0x0000, 0x2000, CRC(1a7490a4) SHA1(e74141b04ffb63e5cc434fbce89ac0c51e79330f) )
	ROM_LOAD( "119_7", 0x2000, 0x2000, CRC(fcff7f59) SHA1(87a4668ef0c28091c895b0aeae4d4c486396e549) )
	ROM_LOAD( "119_5", 0x4000, 0x2000, CRC(1b08c881) SHA1(b372d614ec41cff49d6ff1c2256170c15069bd55) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "119_8", 0x0000, 0x2000, CRC(6570149c) SHA1(b139edbe7bd2f965804b0c850f87e2ef8e418256) )
	ROM_LOAD( "119_9", 0x2000, 0x2000, BAD_DUMP CRC(b917e2c2) SHA1(8acd598b898204e18a4cfccc40720d149f401b42) ) // FIXED BITS (xxxx1xxx) (but always reads the same?)

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "6331", 0x00, 0x20, CRC(b73e79f3) SHA1(8345d45699c51a90c1d2743623b923531a577993) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "7502", 0x00, 0x20, CRC(52bdbe39) SHA1(e6f126e22944b698bea599760a79bd5cfa8f0d1f) ) // ?? hopefully just video timing, not a bad read
ROM_END

} // anonymous namespace

// all tiles are upside down in ROM, but handled by flipscreen
GAME( 1986, 119, 0, _119, 119, _119_state, init_119, ROT0, "Coreland / Sega", "119 (bootleg?)", MACHINE_SUPPORTS_SAVE )
