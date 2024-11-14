// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch, Ivan Vangelista, Ryan Holtz
/***************************************************************************

Atari Flyball Driver

Etched in copper on top of board:
    FLYBALL
    ATARI (c)1976
    A005629
    MADE IN USA
    PAT NO 3793483

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "machine/netlist.h"

#include "netlist/nl_setup.h"
#include "nl_flyball.h"


namespace {

static constexpr XTAL MASTER_CLOCK  = 12.096_MHz_XTAL;
static constexpr XTAL PIXEL_CLOCK   = MASTER_CLOCK / 2;


class flyball_state : public driver_device
{
public:
	flyball_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_bat_sound(*this, "sound_nl:bat_sound"),
		m_footstep(*this, "sound_nl:footstep"),
		m_crowd_sl(*this, "sound_nl:crowd_sl"),
		m_crowd_on(*this, "sound_nl:crowd_on"),
		m_crowd_vl(*this, "sound_nl:crowd_vl"),
		m_playfield_ram(*this, "playfield_ram"),
		m_lamp(*this, "lamp0")
	{ }

	void flyball(machine_config &config);

private:
	uint8_t input_r();
	uint8_t scanline_r();
	uint8_t potsense_r();
	void potmask_w(uint8_t data);
	void pitcher_pic_w(uint8_t data);
	void ball_vert_w(uint8_t data);
	void ball_horz_w(uint8_t data);
	void pitcher_vert_w(uint8_t data);
	void pitcher_horz_w(uint8_t data);
	void misc_w(offs_t offset, uint8_t data);
	void lamp_w(int state);

	TILEMAP_MAPPER_MEMBER(get_memory_offset);
	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void flyball_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(joystick_callback);
	TIMER_CALLBACK_MEMBER(pot_clear_callback);
	TIMER_CALLBACK_MEMBER(quarter_callback);

	void flyball_map(address_map &map) ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<f9334_device> m_outlatch;
	required_device<netlist_mame_logic_input_device> m_bat_sound;
	required_device<netlist_mame_logic_input_device> m_footstep;
	required_device<netlist_mame_logic_input_device> m_crowd_sl;
	required_device<netlist_mame_logic_input_device> m_crowd_on;
	required_device<netlist_mame_logic_input_device> m_crowd_vl;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_playfield_ram;

	output_finder<> m_lamp;

	/* video-related */
	tilemap_t  *m_tmap = nullptr;
	uint8_t    m_pitcher_vert = 0;
	uint8_t    m_pitcher_horz = 0;
	uint8_t    m_pitcher_pic = 0;
	uint8_t    m_ball_vert = 0;
	uint8_t    m_ball_horz = 0;

	/* misc */
	uint8_t    m_potmask = 0;
	uint8_t    m_potsense = 0;

	emu_timer *m_pot_assert_timer[64]{};
	emu_timer *m_pot_clear_timer = nullptr;
	emu_timer *m_quarter_timer = nullptr;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

TILEMAP_MAPPER_MEMBER(flyball_state::get_memory_offset)
{
	if (col == 0)
		col = num_cols;

	return num_cols * (num_rows - row) - col;
}


TILE_GET_INFO_MEMBER(flyball_state::get_tile_info)
{
	uint8_t data = m_playfield_ram[tile_index];
	int flags = ((data & 0x40) ? TILE_FLIPX : 0) | ((data & 0x80) ? TILE_FLIPY : 0);
	int code = data & 63;

	if ((flags & TILE_FLIPX) && (flags & TILE_FLIPY))
		code += 64;

	tileinfo.set(0, code, 0, flags);
}


void flyball_state::video_start()
{
	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(flyball_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(flyball_state::get_memory_offset)), 8, 16, 32, 16);
}


uint32_t flyball_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pitcherx = m_pitcher_horz;
	int pitchery = m_pitcher_vert - 31;

	int ballx = m_ball_horz - 1;
	int bally = m_ball_vert - 17;

	m_tmap->mark_all_dirty();

	/* draw playfield */
	m_tmap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw pitcher */
	m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, m_pitcher_pic ^ 0xf, 0, 1, 0, pitcherx, pitchery, 1);

	/* draw ball */
	for (int y = bally; y < bally + 2; y++)
		for (int x = ballx; x < ballx + 2; x++)
			if (cliprect.contains(x, y))
				bitmap.pix(y, x) = 1;

	return 0;
}


TIMER_CALLBACK_MEMBER(flyball_state::pot_clear_callback)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(flyball_state::joystick_callback)
{
	int potsense = param;

	if (potsense & ~m_potmask)
	{
		// pot irq is active at hsync
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_pot_clear_timer->adjust(attotime::from_ticks(32, PIXEL_CLOCK), 0);
	}

	m_potsense |= potsense;
}


TIMER_CALLBACK_MEMBER(flyball_state::quarter_callback)
{
	int scanline = param;
	int potsense[64], i;

	memset(potsense, 0, sizeof potsense);

	potsense[ioport("STICK1_Y")->read()] |= 1;
	potsense[ioport("STICK1_X")->read()] |= 2;
	potsense[ioport("STICK0_Y")->read()] |= 4;
	potsense[ioport("STICK0_X")->read()] |= 8;

	for (i = 0; i < 64; i++)
		if (potsense[i] != 0)
			m_pot_assert_timer[potsense[i]]->adjust(m_screen->time_until_pos(scanline + i), potsense[i]);

	scanline += 0x40;
	scanline &= 0xff;

	m_quarter_timer->adjust(m_screen->time_until_pos(scanline), scanline);

	m_potsense = 0;
	m_potmask = 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/* two physical buttons (start game and stop runner) share the same port bit */
uint8_t flyball_state::input_r()
{
	return ioport("IN0")->read() & ioport("IN1")->read();
}

uint8_t flyball_state::scanline_r()
{
	return m_screen->vpos() & 0x3f;
}

uint8_t flyball_state::potsense_r()
{
	return m_potsense & ~m_potmask;
}

void flyball_state::potmask_w(uint8_t data)
{
	m_potmask |= data & 0xf;
}

void flyball_state::pitcher_pic_w(uint8_t data)
{
	m_pitcher_pic = data & 0xf;
}

void flyball_state::ball_vert_w(uint8_t data)
{
	m_ball_vert = data;
}

void flyball_state::ball_horz_w(uint8_t data)
{
	m_ball_horz = data;
}

void flyball_state::pitcher_vert_w(uint8_t data)
{
	m_pitcher_vert = data;
}

void flyball_state::pitcher_horz_w(uint8_t data)
{
	m_pitcher_horz = data;
}

void flyball_state::misc_w(offs_t offset, uint8_t data)
{
	// address and data lines passed through inverting buffers
	m_outlatch->write_d0(~offset, ~data);
}

void flyball_state::lamp_w(int state)
{
	m_lamp = state ? 1 : 0;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void flyball_state::flyball_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x00ff).mirror(0x100).ram();
	map(0x0800, 0x0800).noprw();
	map(0x0801, 0x0801).w(FUNC(flyball_state::pitcher_pic_w));
	map(0x0802, 0x0802).r(FUNC(flyball_state::scanline_r));
	map(0x0803, 0x0803).r(FUNC(flyball_state::potsense_r));
	map(0x0804, 0x0804).w(FUNC(flyball_state::ball_vert_w));
	map(0x0805, 0x0805).w(FUNC(flyball_state::ball_horz_w));
	map(0x0806, 0x0806).w(FUNC(flyball_state::pitcher_vert_w));
	map(0x0807, 0x0807).w(FUNC(flyball_state::pitcher_horz_w));
	map(0x0900, 0x0900).w(FUNC(flyball_state::potmask_w));
	map(0x0a00, 0x0a07).w(FUNC(flyball_state::misc_w));
	map(0x0b00, 0x0b00).r(FUNC(flyball_state::input_r));
	map(0x0d00, 0x0eff).writeonly().share("playfield_ram").nopr();
	map(0x1000, 0x1fff).rom().region("maincpu", 0);
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( flyball )
	PORT_START("IN0") /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING( 0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, "Innings Per Game" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING( 0x00, "1" )
	PORT_DIPSETTING( 0x40, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "DSW1:1" )

	PORT_START("STICK1_Y") /* IN1 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("STICK1_X") /* IN2 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("STICK0_Y") /* IN3 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("STICK0_X") /* IN4 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("IN1") /* IN5 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xFE, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BAT_VOL")
	PORT_ADJUSTER( 50, "Bat Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "bat_volume")

	PORT_START("MAIN_VOL")
	PORT_ADJUSTER( 50, "Main Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "main_volume")
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout flyball_tiles_layout =
{
	8, 16,    /* width, height */
	128,      /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80      /* increment */
};

static const gfx_layout flyball_sprites_layout =
{
	16, 16,   /* width, height */
	16,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
		0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
	},
	0x100     /* increment */
};

static GFXDECODE_START( gfx_flyball )
	GFXDECODE_ENTRY( "gfx1", 0, flyball_tiles_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, flyball_sprites_layout, 2, 2 )
GFXDECODE_END


void flyball_state::flyball_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x3f, 0x3f, 0x3f));  // tiles, ball
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(2, rgb_t(0xff ,0xff, 0xff));  // sprites
	palette.set_pen_color(3, rgb_t(0x00, 0x00, 0x00));
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void flyball_state::machine_start()
{
	/* address bits 0 through 8 are inverted */
	uint8_t *ROM = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();
	std::vector<uint8_t> buf(len);
	for (int i = 0; i < len; i++)
		buf[i ^ 0x1ff] = ROM[i];
	memcpy(ROM, &buf[0], len);

	for (int i = 0; i < 64; i++)
		m_pot_assert_timer[i] = timer_alloc(FUNC(flyball_state::joystick_callback), this);
	m_pot_clear_timer = timer_alloc(FUNC(flyball_state::pot_clear_callback), this);
	m_quarter_timer = timer_alloc(FUNC(flyball_state::quarter_callback), this);
	m_lamp.resolve();

	save_item(NAME(m_pitcher_vert));
	save_item(NAME(m_pitcher_horz));
	save_item(NAME(m_pitcher_pic));
	save_item(NAME(m_ball_vert));
	save_item(NAME(m_ball_horz));
	save_item(NAME(m_potmask));
	save_item(NAME(m_potsense));
}

void flyball_state::machine_reset()
{
	m_quarter_timer->adjust(m_screen->time_until_pos(0));

	m_pitcher_vert = 0;
	m_pitcher_horz = 0;
	m_pitcher_pic = 0;
	m_ball_vert = 0;
	m_ball_horz = 0;
	m_potmask = 0;
	m_potsense = 0;
}


void flyball_state::flyball(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &flyball_state::flyball_map);

	F9334(config, m_outlatch); // F7
	m_outlatch->q_out_cb<2>().set(m_bat_sound, FUNC(netlist_mame_logic_input_device::write));
	m_outlatch->q_out_cb<3>().set(m_crowd_sl, FUNC(netlist_mame_logic_input_device::write));
	m_outlatch->q_out_cb<4>().set(m_crowd_on, FUNC(netlist_mame_logic_input_device::write));
	m_outlatch->q_out_cb<5>().set(m_footstep, FUNC(netlist_mame_logic_input_device::write));
	m_outlatch->q_out_cb<6>().set(m_crowd_vl, FUNC(netlist_mame_logic_input_device::write));
	m_outlatch->q_out_cb<7>().set(FUNC(flyball_state::lamp_w)); // 1 player lamp

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 262, 0, 240);
	m_screen->set_screen_update(FUNC(flyball_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_flyball);
	PALETTE(config, m_palette, FUNC(flyball_state::flyball_palette), 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(flyball))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:bat_sound", "BAT_SOUND.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:footstep", "FOOTSTEP.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:crowd_sl", "CROWD_SL.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:crowd_on", "CROWD_ON.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:crowd_vl", "CROWD_VL.IN", 0);
	NETLIST_ANALOG_INPUT(config, "sound_nl:bat_volume", "R75.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:main_volume", "R95.DIAL");

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(1.0 / 0.13, -(1.0 / 0.13) * 0.78);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( flyball )
	ROM_REGION( 0x1000, "maincpu", 0 )  /* program */
	ROM_LOAD( "6129-02.d5", 0x0000, 0x0200, CRC(105ffe40) SHA1(20225571ccf76df5d96a42168d9223cccdff90a8) )
	ROM_LOAD( "6130-02.f5", 0x0200, 0x0200, CRC(188210e1) SHA1(6d837dd9ea44d16f0d54ea9e14260de5f7c05b6b) )
	ROM_LOAD( "6131-01.h5", 0x0400, 0x0200, CRC(a9c7e858) SHA1(aee4a359d6a5729dc1be5b8ce8fbe54d032d12b0) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6132-01.j5", 0x0600, 0x0200, CRC(31fefd8a) SHA1(97e3ef278ce2175cd33c0f3147bdf7974752c836) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6133-01.k5", 0x0800, 0x0200, CRC(6fdb09b1) SHA1(04ad412b437bb24739b3e31fa5a413e63d5897f8) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6134-01.m5", 0x0A00, 0x0200, CRC(7b526c73) SHA1(e47c8f33b7edc143ab1713556c59b93571933daa) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6135-01.n5", 0x0C00, 0x0200, CRC(b352cb51) SHA1(39b9062fb51d0a78a47dcd470ceae47fcdbd7891) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6136-02.r5", 0x0E00, 0x0200, CRC(ae06a0f5) SHA1(6034176b255eeaa2980e8fef1b17ef6f0a743941) )

	ROM_REGION( 0x0C00, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "6142.l2", 0x0000, 0x0200, CRC(65650cfa) SHA1(7d17455146fc9def22c7bd06f7fde32df0a0c2bc) )
	ROM_LOAD( "6139.j2", 0x0200, 0x0200, CRC(a5d1358e) SHA1(33cecbe40ae299549a3395e3dffbe7b6021803ba) )
	ROM_LOAD( "6141.m2", 0x0400, 0x0200, CRC(98b5f803) SHA1(c4e323ced2393fa4a9720ff0086c559fb9b3a9f8) )
	ROM_LOAD( "6140.k2", 0x0600, 0x0200, CRC(66aeec61) SHA1(f577bad015fe9e3708fd95d5d2bc438997d14d2c) )

	ROM_REGION( 0x0400, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "6137.e2", 0x0000, 0x0200, CRC(68961fda) SHA1(a06c7b453cce04716f49bd65ecfe1ba67cb8681e) )
	ROM_LOAD16_BYTE( "6138.f2", 0x0001, 0x0200, CRC(aab314f6) SHA1(6625c719fdc000d6af94bc9474de8f7e977cee97) )
ROM_END

ROM_START( flyball1 )
	ROM_REGION( 0x1000, "maincpu", 0 )  /* program */
	ROM_LOAD( "6129.d5", 0x0000, 0x0200, CRC(17eda069) SHA1(e4ef0bf4546cf00668d759a188e0989a4f003825) )
	ROM_LOAD( "6130.f5", 0x0200, 0x0200, CRC(a756955b) SHA1(220b7f1789bba4481d595b36b4bae25f98d3ad8d) )
	ROM_LOAD( "6131.h5", 0x0400, 0x0200, CRC(a9c7e858) SHA1(aee4a359d6a5729dc1be5b8ce8fbe54d032d12b0) )
	ROM_LOAD( "6132.j5", 0x0600, 0x0200, CRC(31fefd8a) SHA1(97e3ef278ce2175cd33c0f3147bdf7974752c836) )
	ROM_LOAD( "6133.k5", 0x0800, 0x0200, CRC(6fdb09b1) SHA1(04ad412b437bb24739b3e31fa5a413e63d5897f8) )
	ROM_LOAD( "6134.m5", 0x0A00, 0x0200, CRC(7b526c73) SHA1(e47c8f33b7edc143ab1713556c59b93571933daa) )
	ROM_LOAD( "6135.n5", 0x0C00, 0x0200, CRC(b352cb51) SHA1(39b9062fb51d0a78a47dcd470ceae47fcdbd7891) )
	ROM_LOAD( "6136.r5", 0x0E00, 0x0200, CRC(1622d890) SHA1(9ad342aefdc02e022eb79d84d1c856bed538bebe) )

	ROM_REGION( 0x0C00, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "6142.l2", 0x0000, 0x0200, CRC(65650cfa) SHA1(7d17455146fc9def22c7bd06f7fde32df0a0c2bc) )
	ROM_LOAD( "6139.j2", 0x0200, 0x0200, CRC(a5d1358e) SHA1(33cecbe40ae299549a3395e3dffbe7b6021803ba) )
	ROM_LOAD( "6141.m2", 0x0400, 0x0200, CRC(98b5f803) SHA1(c4e323ced2393fa4a9720ff0086c559fb9b3a9f8) )
	ROM_LOAD( "6140.k2", 0x0600, 0x0200, CRC(66aeec61) SHA1(f577bad015fe9e3708fd95d5d2bc438997d14d2c) )

	ROM_REGION( 0x0400, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "6137.e2", 0x0000, 0x0200, CRC(68961fda) SHA1(a06c7b453cce04716f49bd65ecfe1ba67cb8681e) )
	ROM_LOAD16_BYTE( "6138.f2", 0x0001, 0x0200, CRC(aab314f6) SHA1(6625c719fdc000d6af94bc9474de8f7e977cee97) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1976, flyball,  0,       flyball, flyball, flyball_state, empty_init, 0, "Atari", "Flyball (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1976, flyball1, flyball, flyball, flyball, flyball_state, empty_init, 0, "Atari", "Flyball (rev 1)", MACHINE_SUPPORTS_SAVE )
