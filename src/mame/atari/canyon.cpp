// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Canyon Bomber hardware

    driver by Mike Balfour

    Games supported:
        * Canyon Bomber

    Known issues:
        * none at this time

****************************************************************************

    Memory Map:
        0000-01FF       WRAM
        0400-04FF       W A0=0:MOTOR1, A0=1:MOTOR2
        0500-05FF       W A0=0:EXPLODE, A0=1:TIMER RESET
        0600-067F       W A0=0:WHISTLE1, A0=1:WHISTLE2
        0680-06FF       W A0=0:LED1, A0=1:LED2
        0700-077F       W A0=0:ATTRACT1, A0=1:ATTRACT2
        0800-0FFF       DISPLAY / RAM
        1000-17FF       SWITCHES
        1800-1FFF       OPTIONS
        2000-27FF       ROM1
        2800-2FFF       ROM2
        3000-37FF       ROM3 (Language ROM)
        3800-3FFF       ROM4 (Program ROM)

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)


    2008-07
    Dip locations verified with manual.

***************************************************************************/

#include "emu.h"

#include "canyon_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class canyon_state : public driver_device
{
public:
	canyon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_discrete(*this, "discrete"),
		m_in(*this, "IN%u", 1U),
		m_dsw(*this, "DSW")
	{ }

	void canyon(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<f9334_device> m_outlatch;
	required_device<discrete_sound_device> m_discrete;

	required_ioport_array<2> m_in;
	required_ioport m_dsw;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t switches_r(offs_t offset);
	uint8_t options_r(offs_t offset);
	void output_latch_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void motor_w(offs_t offset, uint8_t data);
	void explode_w(uint8_t data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bombs(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Write handlers
 *
 *************************************/

void canyon_state::motor_w(offs_t offset, uint8_t data)
{
	m_discrete->write(NODE_RELATIVE(CANYON_MOTOR1_DATA, (offset & 0x01)), data & 0x0f);
}


void canyon_state::explode_w(uint8_t data)
{
	m_discrete->write(CANYON_EXPLODE_DATA, data >> 4);
}


void canyon_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(canyon_state::get_bg_tile_info)
{
	uint8_t const code = m_videoram[tile_index];

	tileinfo.set(0, code & 0x3f, code >> 7, 0);
}


void canyon_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(canyon_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


void canyon_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 2; i++)
	{
		int const x = m_videoram[0x3d0 + 2 * i + 0x1];
		int const y = m_videoram[0x3d0 + 2 * i + 0x8];
		int const c = m_videoram[0x3d0 + 2 * i + 0x9];


			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			c >> 3,
			i,
			!(c & 0x80), 0,
			224 - x,
			240 - y, 0);
	}
}


void canyon_state::draw_bombs(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 2; i++)
	{
		int const sx = 254 - m_videoram[0x3d0 + 2 * i + 0x5];
		int const sy = 246 - m_videoram[0x3d0 + 2 * i + 0xc];

		rectangle rect(sx, sx + 1, sy, sy + 1);
		rect &= cliprect;

		bitmap.fill(1 + 2 * i, rect);
	}
}


uint32_t canyon_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	draw_bombs(bitmap, cliprect);

	// watchdog is disabled during service mode
	m_watchdog->watchdog_enable(!(m_in[1]->read() & 0x10));

	return 0;
}


/*************************************
 *
 *  Palette generation
 *
 *************************************/

void canyon_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x80, 0x80, 0x80)); // GREY
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // BLACK
	palette.set_pen_color(2, rgb_t(0x80, 0x80, 0x80)); // GREY
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0xff)); // WHITE
}


/*************************************
 *
 *  Read handlers
 *
 *************************************/

uint8_t canyon_state::switches_r(offs_t offset)
{
	uint8_t val = 0;

	if ((m_in[1]->read() >> (offset & 7)) & 1)
		val |= 0x80;

	if ((m_in[0]->read() >> (offset & 3)) & 1)
		val |= 0x01;

	return val;
}


uint8_t canyon_state::options_r(offs_t offset)
{
	return (m_dsw->read() >> (2 * (~offset & 3))) & 3;
}




/*************************************
 *
 *  Write handlers
 *
 *************************************/

void canyon_state::output_latch_w(offs_t offset, uint8_t data)
{
	// ADR1 = D, ADR8 = A2, ADR7 = A1, ADR0 = A0
	m_outlatch->write_bit((offset & 0x180) >> 6 | BIT(offset, 0), BIT(offset, 1));
}




/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void canyon_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x00ff).mirror(0x100).ram();
	map(0x0400, 0x0401).w(FUNC(canyon_state::motor_w));
	map(0x0500, 0x0500).w(FUNC(canyon_state::explode_w));
	map(0x0501, 0x0501).w(m_watchdog, FUNC(watchdog_timer_device::reset_w)); // watchdog, disabled in service mode
	map(0x0600, 0x0603).select(0x0180).w(FUNC(canyon_state::output_latch_w));
	map(0x0800, 0x0bff).ram().w(FUNC(canyon_state::videoram_w)).share(m_videoram);
	map(0x1000, 0x17ff).r(FUNC(canyon_state::switches_r)).nopw();  // sloppy code writes here
	map(0x1800, 0x1fff).r(FUNC(canyon_state::options_r));
	map(0x2000, 0x3fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( canyon )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x03, DEF_STR( German ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW:3" )    // Manual says these are unused
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW:4" )    // Manual says these are unused
	PORT_DIPNAME( 0x30, 0x00, "Misses Per Play" ) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Hiscore Reset") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) /* SLAM */

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 20, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 30, "Motor 2 RPM" )

	PORT_START("WHISTLE1")
	PORT_ADJUSTER( 70, "Whistle 1 Freq" )

	PORT_START("WHISTLE2")
	PORT_ADJUSTER( 80, "Whistle 2 Freq" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0x4, 0x5, 0x6, 0x7, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static const gfx_layout sprite_layout =
{
	32, 16,
	4,
	1,
	{ 0 },
	{
		0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000,
		0x00F, 0x00E, 0x00D, 0x00C, 0x00B, 0x00A, 0x009, 0x008,
		0x107, 0x106, 0x105, 0x104, 0x103, 0x102, 0x101, 0x100,
		0x10F, 0x10E, 0x10D, 0x10C, 0x10B, 0x10A, 0x109, 0x108
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
	},
	0x200
};


static GFXDECODE_START( gfx_canyon )
	GFXDECODE_ENTRY( "tiles",   0, tile_layout,   0, 2 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout, 0, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void canyon_state::canyon(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12.096_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &canyon_state::main_map);

	F9334(config, m_outlatch); // C7
	m_outlatch->q_out_cb<0>().set("discrete", FUNC(discrete_device::write_line<CANYON_WHISTLE1_EN>));
	m_outlatch->q_out_cb<1>().set("discrete", FUNC(discrete_device::write_line<CANYON_WHISTLE2_EN>));
	m_outlatch->q_out_cb<2>().set_output("led0"); // 1 PLAYER LAMP
	m_outlatch->q_out_cb<3>().set_output("led1"); // 2 PLAYER LAMP
	m_outlatch->q_out_cb<4>().set("discrete", FUNC(discrete_device::write_line<CANYON_ATTRACT1_EN>));
	m_outlatch->q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<CANYON_ATTRACT2_EN>));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 8);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12.096_MHz_XTAL / 2, 384, 0, 256, 262, 0, 240); // HSYNC = 15,750 Hz
	screen.set_screen_update(FUNC(canyon_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, m6502_device::NMI_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_canyon);
	PALETTE(config, m_palette, FUNC(canyon_state::palette), 4);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	DISCRETE(config, m_discrete, canyon_discrete);
	m_discrete->add_route(0, "speaker", 1.0, 0);
	m_discrete->add_route(1, "speaker", 1.0, 1);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( canyon )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "9499-01.j1", 0x3000, 0x0400, CRC(31800767) SHA1(d4aebe12d3c45a2a8a361dc6f63e1a6230a78c17) )
	ROM_LOAD_NIB_HIGH( "9503-01.p1", 0x3000, 0x0400, CRC(1eddbe28) SHA1(7d30280bf9edff743c16386d7cdec78094477996) )
	ROM_LOAD         ( "9496-01.d1", 0x3800, 0x0800, CRC(8be15080) SHA1(095c15e9ac91623b2d514858dca2e4c261d36fd0) )

	ROM_REGION( 0x0400, "tiles", 0 )
	ROM_LOAD( "9492-01.n8", 0x0000, 0x0400, CRC(7449f754) SHA1(a8ffc39e1a86c94487551f5026eedbbd066b12c9) )

	ROM_REGION( 0x0100, "sprites", 0 )
	ROM_LOAD_NIB_LOW ( "9506-01.m5", 0x0000, 0x0100, CRC(0d63396a) SHA1(147fae3b02a86310c8d022a7e7cfbf71ea511616) )
	ROM_LOAD_NIB_HIGH( "9505-01.n5", 0x0000, 0x0100, CRC(60507c07) SHA1(fcb76890cbaa37e02392bf8b97f7be9a6fe6a721) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "9491-01.j6", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // sync (not used)
ROM_END


ROM_START( canyonp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "cbp3000l.j1", 0x3000, 0x0800, CRC(49cf29a0) SHA1(b58f024f45f85e5c2a48a95c60e80fd1be60eaac) )
	ROM_LOAD_NIB_HIGH( "cbp3000m.p1", 0x3000, 0x0800, CRC(b4385c23) SHA1(b550dfe9182f2b29aedba160a0917ca78b82f0e7) )
	ROM_LOAD_NIB_LOW ( "cbp3800l.h1", 0x3800, 0x0800, CRC(c7ee4431) SHA1(7a0f4454a981c4e9ee27e273e9a8379458e660e5) )
	ROM_LOAD_NIB_HIGH( "cbp3800m.r1", 0x3800, 0x0800, CRC(94246a9a) SHA1(5ff8b69fb744a5f62d4cf291e8f25e3620b479e7) )

	ROM_REGION( 0x0400, "tiles", 0 )
	ROM_LOAD( "9492-01.n8", 0x0000, 0x0400, CRC(7449f754) SHA1(a8ffc39e1a86c94487551f5026eedbbd066b12c9) )

	ROM_REGION( 0x0100, "sprites", 0 )
	ROM_LOAD_NIB_LOW ( "9506-01.m5", 0x0000, 0x0100, CRC(0d63396a) SHA1(147fae3b02a86310c8d022a7e7cfbf71ea511616) )
	ROM_LOAD_NIB_HIGH( "9505-01.n5", 0x0000, 0x0100, CRC(60507c07) SHA1(fcb76890cbaa37e02392bf8b97f7be9a6fe6a721) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "9491-01.j6", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )  // sync (not used)
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1977, canyon,  0,      canyon, canyon, canyon_state, empty_init, ROT0, "Atari", "Canyon Bomber",             MACHINE_SUPPORTS_SAVE )
GAME( 1977, canyonp, canyon, canyon, canyon, canyon_state, empty_init, ROT0, "Atari", "Canyon Bomber (prototype)", MACHINE_SUPPORTS_SAVE )
