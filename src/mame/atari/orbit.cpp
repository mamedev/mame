// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch

/***************************************************************************

Atari Orbit Driver

  game 0 = beginner slow
  game 1 = beginner medium
  game 2 = beginner fast
  game 3 = intermediate slow
  game 4 = intermediate fast
  game 5 = expert fast shells only
  game 6 = expert slow
  game 7 = expert medium
  game 8 = expert fast
  game 9 = super expert

  Flip screen DIP doesn't work because it's not supported by the game.

***************************************************************************/

#include "emu.h"

#include "orbit_a.h"

#include "cpu/m6800/m6800.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class orbit_state : public driver_device
{
public:
	orbit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_discrete(*this, "discrete"),
		m_latch(*this, "latch"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_dsw2(*this, "DSW2"),
		m_bg_tilemap(nullptr),
		m_flip_screen(0)
	{ }

	void orbit(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;

	// devices
	required_device<discrete_sound_device> m_discrete;
	required_device<f9334_device> m_latch;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport m_dsw2;

	// video-related
	tilemap_t *m_bg_tilemap;
	uint8_t m_flip_screen;
	emu_timer *m_irq_off_timer = nullptr;

	void coin_lockout_w(int state);
	void playfield_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_32v);
	void note_w(uint8_t data);
	void note_amp_w(uint8_t data);
	void noise_amp_w(uint8_t data);
	void noise_rst_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_misc_flags(address_space &space, uint8_t val);
};


void orbit_state::playfield_w(offs_t offset, uint8_t data)
{
	m_playfield_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(orbit_state::get_tile_info)
{
	uint8_t const code = m_playfield_ram[tile_index];
	int flags = 0;

	if (BIT(code, 6))
		flags |= TILE_FLIPX;
	if (m_flip_screen)
		flags |= TILE_FLIPY;

	tileinfo.set(3, code & 0x3f, 0, flags);
}


void orbit_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(orbit_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 30);
}


void orbit_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t* p = m_sprite_ram;

	for (int i = 0; i < 16; i++)
	{
		int code = *p++;
		int vpos = *p++;
		int hpos = *p++;
		int const flag = *p++;

		int const layout =
			((flag & 0xc0) == 0x80) ? 1 :
			((flag & 0xc0) == 0xc0) ? 2 : 0;

		int const flip_x = BIT(code, 6);
		int const flip_y = BIT(code, 7);

		int zoom_x = 0x10000;
		int const zoom_y = 0x10000;

		code &= 0x3f;

		if (flag & 1)
			code |= 0x40;
		if (flag & 2)
			zoom_x *= 2;

		vpos = 240 - vpos;

		hpos <<= 1;
		vpos <<= 1;

		m_gfxdecode->gfx(layout)->zoom_transpen(bitmap, cliprect, code, 0, flip_x, flip_y,
			hpos, vpos, zoom_x, zoom_y, 0);
	}
}


uint32_t orbit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_flip_screen = m_dsw2->read() & 8;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Interrupts and timing
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(orbit_state::nmi_32v)
{
	int const scanline = param;
	int const nmistate = (scanline & 32) && m_latch->q2_r();
	m_maincpu->set_input_line(INPUT_LINE_NMI, nmistate ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(orbit_state::irq_off)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(orbit_state::interrupt)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
	m_irq_off_timer->adjust(m_screen->time_until_vblank_end());
}



/*************************************
 *
 *  Bit flags
 *
 *************************************/


void orbit_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
	machine().bookkeeping().coin_lockout_w(1, !state);
}



/*************************************
 *
 *  Write handlers
 *
 *************************************/

void orbit_state::note_w(uint8_t data)
{
	m_discrete->write(ORBIT_NOTE_FREQ, (~data) & 0xff);
}

void orbit_state::note_amp_w(uint8_t data)
{
	m_discrete->write(ORBIT_ANOTE1_AMP, data & 0x0f);
	m_discrete->write(ORBIT_ANOTE2_AMP, data >> 4);
}

void orbit_state::noise_amp_w(uint8_t data)
{
	m_discrete->write(ORBIT_NOISE1_AMP, data & 0x0f);
	m_discrete->write(ORBIT_NOISE2_AMP, data >> 4);
}

void orbit_state::noise_rst_w(uint8_t data)
{
	m_discrete->write(ORBIT_NOISE_EN, 0);
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

void orbit_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).mirror(0x0700).ram();
	map(0x0800, 0x0800).mirror(0x07ff).portr("P1");
	map(0x1000, 0x1000).mirror(0x07ff).portr("P2");
	map(0x1800, 0x1800).mirror(0x07ff).portr("DSW1");
	map(0x2000, 0x2000).mirror(0x07ff).portr("DSW2");
	map(0x2800, 0x2800).mirror(0x07ff).portr("BUTTONS");
	map(0x3000, 0x33bf).mirror(0x0400).ram().w(FUNC(orbit_state::playfield_w)).share(m_playfield_ram);
	map(0x33c0, 0x33ff).mirror(0x0400).ram().share(m_sprite_ram);
	map(0x3800, 0x3800).mirror(0x00ff).w(FUNC(orbit_state::note_w));
	map(0x3900, 0x3900).mirror(0x00ff).w(FUNC(orbit_state::noise_amp_w));
	map(0x3a00, 0x3a00).mirror(0x00ff).w(FUNC(orbit_state::note_amp_w));
	map(0x3c00, 0x3c0f).mirror(0x00f0).w(m_latch, FUNC(f9334_device::write_a0));
	map(0x3e00, 0x3e00).mirror(0x00ff).w(FUNC(orbit_state::noise_rst_w));
	map(0x3f00, 0x3f00).mirror(0x00ff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x6000, 0x7fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( orbit )
	PORT_START("P1")    // 0800
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) // actually buttons
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")    // 1000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) // actually buttons
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")  // 1800
	PORT_DIPNAME( 0x07, 0x00, "Play Time Per Credit" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING( 0x00, "0:30" )
	PORT_DIPSETTING( 0x01, "1:00" )
	PORT_DIPSETTING( 0x02, "1:30" )
	PORT_DIPSETTING( 0x03, "2:00" )
	PORT_DIPSETTING( 0x04, "2:30" )
	PORT_DIPSETTING( 0x05, "3:00" )
	PORT_DIPSETTING( 0x06, "3:30" )
	PORT_DIPSETTING( 0x07, "4:00" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING( 0x00, DEF_STR( English ) )
	PORT_DIPSETTING( 0x08, DEF_STR( Spanish ) )
	PORT_DIPSETTING( 0x10, DEF_STR( French ) )
	PORT_DIPSETTING( 0x18, DEF_STR( German ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play )) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x20, DEF_STR( On ))
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "DSW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "DSW1:8" )

	PORT_START("DSW2")  // 2000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game Reset") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ))
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x08, DEF_STR( On ))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Heat Reset") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_DIPNAME( 0x20, 0x20, "NEXT TEST" ) // should be off
	PORT_DIPSETTING( 0x20, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "DIAG TEST" ) // should be off
	PORT_DIPSETTING( 0x40, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("BUTTONS")   // 2800
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 7 / Strong Gravity") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 6 / Stars") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 5 / Unlimited Supplies") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 4 / Space Stations") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 3 / Black Hole") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 2 / Zero Gravity") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 1 / Negative Gravity") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 0 / Bounce Back") PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout orbit_full_sprite_layout =
{
	8, 32,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	0x100
};


static const gfx_layout orbit_upper_sprite_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	0x100
};


static const gfx_layout orbit_lower_sprite_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0x80,8) },
	0x100
};


static GFXDECODE_START( gfx_orbit )
	GFXDECODE_ENTRY( "sprites", 0, orbit_full_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "sprites", 0, orbit_upper_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "sprites", 0, orbit_lower_sprite_layout, 0, 1 )
	GFXDECODE_SCALE( "tiles",   0, gfx_8x8x1, 0, 1, 2, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void orbit_state::machine_start()
{
	m_irq_off_timer = timer_alloc(FUNC(orbit_state::irq_off), this);

	save_item(NAME(m_flip_screen));
}

void orbit_state::machine_reset()
{
	m_flip_screen = 0;
}


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void orbit_state::orbit(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(12'096'000);

	// basic machine hardware
	M6800(config, m_maincpu, MASTER_CLOCK / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &orbit_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(orbit_state::interrupt));

	TIMER(config, "32v").configure_scanline(FUNC(orbit_state::nmi_32v), "screen", 0, 32);

	F9334(config, m_latch); // M6
	// BIT0 => UNUSED
	// BIT1 => LOCKOUT
	// BIT2 => NMI ENABLE
	// BIT3 => HEAT RST LED
	// BIT4 => PANEL BUS OC
	// BIT5 => PANEL STROBE
	// BIT6 => HYPER LED
	// BIT7 => WARNING SND
	m_latch->q_out_cb<1>().set(FUNC(orbit_state::coin_lockout_w));
	m_latch->q_out_cb<3>().set_output("led0");
	m_latch->q_out_cb<6>().set_output("led1");
	m_latch->q_out_cb<7>().set(m_discrete, FUNC(discrete_device::write_line<ORBIT_WARNING_EN>));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK*2, 384*2, 0, 256*2, 261*2, 0, 240*2);
	m_screen->set_screen_update(FUNC(orbit_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_orbit);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DISCRETE(config, m_discrete, orbit_discrete);
	m_discrete->add_route(0, "lspeaker", 1.0);
	m_discrete->add_route(1, "rspeaker", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( orbit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "033701.h2", 0x6800, 0x400, CRC(6de43b85) SHA1(1643972f45d3a0dd6540158c575cd84cee2b0c9a) )
	ROM_LOAD_NIB_HIGH( "033693.l2", 0x6800, 0x400, CRC(8878409e) SHA1(a14e0161705bbc230f0aec1837ebc41d62178368) )
	ROM_LOAD_NIB_LOW ( "033702.h1", 0x6c00, 0x400, CRC(8166bdcb) SHA1(b7ae6cd46b4aff6e1e1ec9273cf068dec4a8cd46) )
	ROM_LOAD_NIB_HIGH( "033694.l1", 0x6c00, 0x400, CRC(5337a8ee) SHA1(1606bfa652bb5253c387f11c96d77d7a84983344) )
	ROM_LOAD_NIB_LOW ( "033699.f2", 0x7000, 0x400, CRC(b498b36f) SHA1(5d150af193196fccd7c20ba731a020a9ae75e516) )
	ROM_LOAD_NIB_HIGH( "033691.m2", 0x7000, 0x400, CRC(6cbabb21) SHA1(fffb3f7be73c72b4775d8cdfe174c75ae4389cba) )
	ROM_LOAD_NIB_LOW ( "033700.f1", 0x7400, 0x400, CRC(9807c922) SHA1(b6b62530b24d967104f632540ef98f2b4780c3ed) )
	ROM_LOAD_NIB_HIGH( "033692.m1", 0x7400, 0x400, CRC(96167d1b) SHA1(6f272b2f1b30aa94f51ea5710f4114bfdea19f2c) )
	ROM_LOAD_NIB_LOW ( "033697.e2", 0x7800, 0x400, CRC(19ccf0dc) SHA1(7d12c4985bd0a25ef518246faf2849e5a0cf600b) )
	ROM_LOAD_NIB_HIGH( "033689.n2", 0x7800, 0x400, CRC(ea3b70c1) SHA1(5e985fed057f362deaeb5e4049c4e8c1d449d6e1) )
	ROM_LOAD_NIB_LOW ( "033698.e1", 0x7c00, 0x400, CRC(356a7c32) SHA1(a3496c0f9d9f3e2e0b452cdc0e908dc93d179990) )
	ROM_RELOAD(                     0xfc00, 0x400 )
	ROM_LOAD_NIB_HIGH( "033690.n1", 0x7c00, 0x400, CRC(f756ebd4) SHA1(4e473541b712078c6a81901714a6243de348e543) )
	ROM_RELOAD(                     0xfc00, 0x400 )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "033712.b7", 0x0000, 0x800, CRC(cfd43bf2) SHA1(dbca0da6ed355aac921bae5adeef2f384f5fa2c3) )
	ROM_LOAD( "033713.d7", 0x0800, 0x800, CRC(5ac89f4d) SHA1(747889b33cd83510a640e68fb4581a3e881c43a3) )

	ROM_REGION( 0x200, "tiles", 0 )
	ROM_LOAD( "033711.a7", 0x0000, 0x200, CRC(9987174a) SHA1(d2117b6e6d64c29aef8ad8c94256baea493bce5c) )

	ROM_REGION( 0x100, "sync_prom", 0 ) // unused
	ROM_LOAD( "033688.p6", 0x0000, 0x100, CRC(ee66ddba) SHA1(5b9ae4cbf019375c8d54528b69280413c641c4f2) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1978, orbit, 0, orbit, orbit, orbit_state, empty_init, 0, "Atari", "Orbit", MACHINE_SUPPORTS_SAVE )
