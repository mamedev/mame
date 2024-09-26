// license:BSD-3-Clause
// copyright-holders:Mike Balfour

/***************************************************************************

    Atari Night Driver hardware

    driver by Mike Balfour

    Games supported:
        * Night Driver

    Known issues:
        * The road boxes in service mode are flipped horizontally and there
          is an extraneous box according to the service manual.

****************************************************************************

    Memory Map:
        0000-01FF   R/W     SCRAM (Scratchpad RAM)
        0200-03FF    W      PFW (Playfield Write)
        0400-05FF    W      HVC (Horiz/Vert/Char for Roadway)
        0600-07FF    R      IN0
        0800-09FF    R      IN1
        0A00-0BFF    W      OUT0
        0C00-0DFF    W      OUT1
        0E00-0FFF    -      OUT2 (Not used)
        8000-83FF    R      PFR (Playfield Read)
        8400-87FF           Steering Reset
        8800-8FFF    -      Spare (Not used)
        9000-97FF    R      Program ROM1
        9800-9FFF    R      Program ROM2
        (F800-FFFF)  R      Program ROM2 - only needed for the 6502 vectors

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"

#include "nitedrvr_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class nitedrvr_state : public driver_device
{
public:
	nitedrvr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_hvc(*this, "hvc"),
		m_steer(*this, "STEER"),
		m_gears(*this, "GEARS"),
		m_in0(*this, "IN0"),
		m_dsw(*this, "DSW%u", 0U),
		m_led(*this, "led"),
		m_track_sel(*this, "track%u", 1U),
		m_gear_sel(*this, "gear%u", 1U)
	{ }

	void nitedrvr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hvc;

	// input
	uint8_t m_gear = 0;
	uint8_t m_track = 0;
	int32_t m_steering_buf = 0;
	int32_t m_steering_val = 0;
	uint8_t m_crash_en = 0;
	uint8_t m_crash_data = 0;
	uint8_t m_crash_data_en = 0;  // IC D8
	uint8_t m_ac_line = 0;
	int32_t m_last_steering_val = 0;
	required_ioport m_steer, m_gears, m_in0;
	required_ioport_array<3> m_dsw;

	// output
	output_finder<> m_led;
	output_finder<3> m_track_sel;
	output_finder<4> m_gear_sel;

	uint8_t steering_reset_r();
	void steering_reset_w(uint8_t data);
	uint8_t in0_r(offs_t offset);
	uint8_t in1_r(offs_t offset);
	void out0_w(uint8_t data);
	void out1_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(crash_toggle_callback);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey);
	void draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int steering();
	void main_map(address_map &map) ATTR_COLD;
};


void nitedrvr_state::draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey)
{
	for (int y = by; y < ey; y++)
	{
		for (int x = bx; x < ex; x++)
			if (cliprect.contains(x, y))
				bitmap.pix(y, x) = 1;
	}
}

void nitedrvr_state::draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int roadway = 0; roadway < 16; roadway++)
	{
		int const bx = m_hvc[roadway];
		int const by = m_hvc[roadway + 16];
		int const ex = bx + ((m_hvc[roadway + 32] & 0xf0) >> 4);
		int const ey = by + (16 - (m_hvc[roadway + 32] & 0x0f));

		draw_box(bitmap, cliprect, bx, by, ex, ey);
	}
}

void nitedrvr_state::draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw tiles manually, note that tile rows are ignored on V&8, V&64, V&128
	for (int offs = 0; offs < 0x80; offs++)
	{
		int const code = m_videoram[offs];
		int const sx = (offs & 0x1f) * 8;
		int const sy = (offs >> 5) * 2 * 8;

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, code, 0, 0, 0, sx, sy);
	}
}

uint32_t nitedrvr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_tiles(bitmap, cliprect);
	draw_roadway(bitmap, cliprect);

	return 0;
}


/***************************************************************************
Steering

When D7 is high, the steering wheel has moved.
If D6 is low, it moved left.  If D6 is high, it moved right.
Be sure to keep returning a direction until steering_reset is called,
because D6 and D7 are apparently checked at different times, and a
change in-between can affect the direction you move.
***************************************************************************/

int nitedrvr_state::steering()
{
	int const this_val = m_steer->read();
	int delta = this_val - m_last_steering_val;

	m_last_steering_val = this_val;

	if (delta > 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;

	// Divide by four to make our steering less sensitive
	m_steering_buf += (delta / 4);

	if (m_steering_buf > 0)
	{
		m_steering_buf--;
		m_steering_val = 0xc0;
	}
	else if (m_steering_buf < 0)
	{
		m_steering_buf++;
		m_steering_val = 0x80;
	}
	else
	{
		m_steering_val = 0x00;
	}

	return m_steering_val;
}

/***************************************************************************
steering_reset
***************************************************************************/

uint8_t nitedrvr_state::steering_reset_r()
{
	m_steering_val = 0;
	return 0;
}

void nitedrvr_state::steering_reset_w(uint8_t data)
{
	m_steering_val = 0;
}


/***************************************************************************
in0_r

Night Driver looks for the following:
    A: $00
        D4 - OPT1
        D5 - OPT2
        D6 - OPT3
        D7 - OPT4
    A: $01
        D4 - TRACK SET
        D5 - BONUS TIME ALLOWED
        D6 - VBLANK
        D7 - !TEST
    A: $02
        D4 - !GEAR 1
        D5 - !GEAR 2
        D6 - !GEAR 3
        D7 - SPARE
    A: $03
        D4 - SPARE
        D5 - DIFFICULT BONUS
        D6 - STEER A
        D7 - STEER B

Fill in the steering and gear bits in a special way.
***************************************************************************/

uint8_t nitedrvr_state::in0_r(offs_t offset)
{
	int const gear = m_gears->read();

	if (gear & 0x10)                m_gear = 1;
	else if (gear & 0x20)           m_gear = 2;
	else if (gear & 0x40)           m_gear = 3;
	else if (gear & 0x80)           m_gear = 4;

	for (uint8_t i = 0; i < 4; i++)
		m_gear_sel[i] = ((m_gear == (i + 1)) ? 1 : 0);

	switch (offset & 0x03)
	{
		case 0x00:                      // No remapping necessary
			return m_dsw[0]->read();
		case 0x01:                      // No remapping necessary
			return m_dsw[1]->read();
		case 0x02:                      // Remap our gear shift
			if (m_gear == 1)
				return 0xe0;
			else if (m_gear == 2)
				return 0xd0;
			else if (m_gear == 3)
				return 0xb0;
			else
				return 0x70;
		case 0x03:                      // Remap our steering
			return (m_dsw[2]->read() | steering());
		default:
			return 0xff;
	}
}

/***************************************************************************
in1_r

Night Driver looks for the following:
    A: $00
        D6 - SPARE
        D7 - COIN 1
    A: $01
        D6 - SPARE
        D7 - COIN 2
    A: $02
        D6 - SPARE
        D7 - !START
    A: $03
        D6 - SPARE
        D7 - !ACC
    A: $04
        D6 - SPARE
        D7 - EXPERT
    A: $05
        D6 - SPARE
        D7 - NOVICE
    A: $06
        D6 - SPARE
        D7 - Special Alternating Signal
    A: $07
        D6 - SPARE
        D7 - Ground

Fill in the track difficulty switch and special signal in a special way.
***************************************************************************/

uint8_t nitedrvr_state::in1_r(offs_t offset)
{
	int const port = m_in0->read();

	m_ac_line = (m_ac_line + 1) % 3;

	if (port & 0x10)                m_track = 0;
	else if (port & 0x20)           m_track = 1;
	else if (port & 0x40)           m_track = 2;

	for (uint8_t i = 0; i < 3; i++)
		m_track_sel[i] = (m_track == i ? 1 : 0);

	switch (offset & 0x07)
	{
		case 0x00:
			return ((port & 0x01) << 7);
		case 0x01:
			return ((port & 0x02) << 6);
		case 0x02:
			return ((port & 0x04) << 5);
		case 0x03:
			return ((port & 0x08) << 4);
		case 0x04:
			if (m_track == 1) return 0x80; else return 0x00;
		case 0x05:
			if (m_track == 0) return 0x80; else return 0x00;
		case 0x06:
			// TODO: fix alternating signal?
			if (m_ac_line == 0) return 0x80; else return 0x00;
		case 0x07:
			return 0x00;
		default:
			return 0xff;
	}
}

/***************************************************************************
out0_w

Sound bits:

D0 = !SPEED1
D1 = !SPEED2
D2 = !SPEED3
D3 = !SPEED4
D4 = SKID1
D5 = SKID2
***************************************************************************/

void nitedrvr_state::out0_w(uint8_t data)
{
	m_discrete->write(NITEDRVR_MOTOR_DATA, data & 0x0f);  // Motor freq data
	m_discrete->write(NITEDRVR_SKID1_EN, data & 0x10);    // Skid1 enable
	m_discrete->write(NITEDRVR_SKID2_EN, data & 0x20);    // Skid2 enable
}

/***************************************************************************
out1_w

D0 = !CRASH - also drives a video invert signal
D1 = ATTRACT
D2 = Spare (Not used)
D3 = Not used?
D4 = LED START
D5 = Spare (Not used)
***************************************************************************/

void nitedrvr_state::out1_w(uint8_t data)
{
	m_led = BIT(data, 4);

	m_crash_en = data & 0x01;

	m_discrete->write(NITEDRVR_CRASH_EN, m_crash_en); // Crash enable
	m_discrete->write(NITEDRVR_ATTRACT_EN, data & 0x02);      // Attract enable (sound disable)

	if (!m_crash_en)
	{
		// Crash reset, set counter high and enable output
		m_crash_data_en = 1;
		m_crash_data = 0x0f;
		// Invert video
		m_palette->set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // BLACK
		m_palette->set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // WHITE
	}
	m_discrete->write(NITEDRVR_BANG_DATA, m_crash_data_en ? m_crash_data : 0);    // Crash Volume
}


TIMER_DEVICE_CALLBACK_MEMBER(nitedrvr_state::crash_toggle_callback)
{
	if (m_crash_en && m_crash_data_en)
	{
		m_crash_data--;
		m_discrete->write(NITEDRVR_BANG_DATA, m_crash_data);  // Crash Volume
		if (!m_crash_data)
			m_crash_data_en = 0;    // Done counting?

		if (m_crash_data & 0x01)
		{
			// Invert video
			m_palette->set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // BLACK
			m_palette->set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // WHITE
		}
		else
		{
			// Normal video
			m_palette->set_pen_color(0, rgb_t(0x00,0x00,0x00)); // BLACK
			m_palette->set_pen_color(1, rgb_t(0xff,0xff,0xff)); // WHITE
		}
	}
}

void nitedrvr_state::machine_start()
{
	m_led.resolve();
	m_track_sel.resolve();
	m_gear_sel.resolve();

	save_item(NAME(m_gear));
	save_item(NAME(m_track));
	save_item(NAME(m_steering_buf));
	save_item(NAME(m_steering_val));
	save_item(NAME(m_crash_en));
	save_item(NAME(m_crash_data));
	save_item(NAME(m_crash_data_en));
	save_item(NAME(m_ac_line));
	save_item(NAME(m_last_steering_val));
}

void nitedrvr_state::machine_reset()
{
	m_gear = 1;
	m_track = 0;
	m_steering_buf = 0;
	m_steering_val = 0;
	m_crash_en = 0;
	m_crash_data = 0x0f;
	m_crash_data_en = 0;
	m_ac_line = 0;
	m_last_steering_val = 0;
}


// Memory Map

void nitedrvr_state::main_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x100); // SCRAM
	map(0x0200, 0x027f).nopr().ram().mirror(0x180).share(m_videoram); // PFW
	map(0x0400, 0x042f).nopr().writeonly().mirror(0x1c0).share(m_hvc); // POSH, POSV, CHAR
	map(0x0430, 0x043f).w("watchdog", FUNC(watchdog_timer_device::reset_w)).mirror(0x1c0);
	map(0x0600, 0x07ff).r(FUNC(nitedrvr_state::in0_r));
	map(0x0800, 0x09ff).r(FUNC(nitedrvr_state::in1_r));
	map(0x0a00, 0x0bff).w(FUNC(nitedrvr_state::out0_w));
	map(0x0c00, 0x0dff).w(FUNC(nitedrvr_state::out1_w));
	map(0x8000, 0x807f).nopw().ram().mirror(0x380).share(m_videoram); // PFR
	map(0x8400, 0x87ff).rw(FUNC(nitedrvr_state::steering_reset_r), FUNC(nitedrvr_state::steering_reset_w));
	map(0x9000, 0x9fff).rom(); // ROM1-ROM2
	map(0xfff0, 0xffff).rom(); // ROM2 for 6502 vectors
}

// Input Ports

static INPUT_PORTS_START( nitedrvr )
	PORT_START("DSW0")  // fake
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	//PORT_DIPSETTING(  0x20, DEF_STR( 1C_1C ) ) // not a typo
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x80, "Playing Time" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x40, "75" )
	PORT_DIPSETTING(    0x80, "100" )
	PORT_DIPSETTING(    0xC0, "125" )

	PORT_START("DSW1")  // fake
	PORT_DIPNAME( 0x10, 0x00, "Track Set" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Reverse ) )
	PORT_DIPNAME( 0x20, 0x20, "Bonus Time" )
	PORT_DIPSETTING(    0x00, DEF_STR ( No ) )
	PORT_DIPSETTING(    0x20, "Score = 350" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("GEARS") // fake
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("1st Gear")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("2nd Gear")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("3rd Gear")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("4th Gear")

	PORT_START("DSW2")  // fake
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // Spare
	PORT_DIPNAME( 0x20, 0x00, "Difficult Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")   // fake
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Novice Track")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Expert Track")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pro Track")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Alternating signal?

	PORT_START("STEER") // fake
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("MOTOR")
	PORT_ADJUSTER( 60, "Motor RPM" )
INPUT_PORTS_END

// Graphics Decode Information

static GFXDECODE_START( gfx_nitedrvr )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x1, 0, 1 )
GFXDECODE_END

// Machine Driver

void nitedrvr_state::nitedrvr(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12.096_MHz_XTAL / 12); // 1 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &nitedrvr_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(nitedrvr_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 3);

	TIMER(config, "crash_timer").configure_periodic(FUNC(nitedrvr_state::crash_toggle_callback), PERIOD_OF_555_ASTABLE(RES_K(180), 330, CAP_U(1)));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12.096_MHz_XTAL / 2, 384, 0, 256, 262, 0, 240);
	// PROM derives VRESET, VBLANK, VSYNC, IRQ from vertical scan count and last VBLANK
	screen.set_screen_update(FUNC(nitedrvr_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nitedrvr);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, nitedrvr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

// ROMs

/*
ROM_START( nitedrvo )       // early revision has the program code stored in 8 chips
    ROM_REGION( 0x10000, "maincpu", 0 )
    ROM_LOAD( "006560-01.h1", 0x9000, 0x0200, NO_DUMP ) // PROM 1
    ROM_LOAD( "006561-01.c1", 0x9200, 0x0200, NO_DUMP ) // PROM 2
    ROM_LOAD( "006562-01.j1", 0x9400, 0x0200, NO_DUMP ) // PROM 3
    ROM_LOAD( "006563-01.d1", 0x9600, 0x0200, NO_DUMP ) // PROM 4
    ROM_LOAD( "006564-01.k1", 0x9800, 0x0200, NO_DUMP ) // PROM 5
    ROM_LOAD( "006565-01.e1", 0x9a00, 0x0200, NO_DUMP ) // PROM 6
    ROM_LOAD( "006566-01.l1", 0x9c00, 0x0200, NO_DUMP ) // PROM 7
    ROM_LOAD( "006567-01.f1", 0x9e00, 0x0200, NO_DUMP ) // PROM 8
ROM_END
*/

ROM_START( nitedrvr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "006569-01.d2", 0x9000, 0x0800, CRC(7afa7542) SHA1(81018e25ebdeae1daf1308676661063b6fd7fd22) ) // mask ROM 1
	ROM_LOAD( "006570-01.f2", 0x9800, 0x0800, CRC(bf5d77b1) SHA1(6f603f8b0973bd89e0e721b66944aac8e9f904d9) ) // mask ROM 2
	ROM_RELOAD(               0xf800, 0x0800 ) // vectors

	ROM_REGION( 0x200, "gfx", 0 )
	ROM_LOAD( "006568-01.p2", 0x0000, 0x0200, CRC(f80d8889) SHA1(ca573543dcce1221459d5693c476cef14bfac4f4) ) // PROM, Alpha-Numeric

	ROM_REGION( 0x100, "sync_prom", 0 )
	ROM_LOAD( "006559-01.h7", 0x0000, 0x0100, CRC(5a8d0e42) SHA1(772220c4c24f18769696ddba26db2bc2e5b0909d) ) // PROM, Sync
ROM_END

} // anonymous namespace


// Game Drivers

GAME( 1976, nitedrvr, 0, nitedrvr, nitedrvr, nitedrvr_state, empty_init, ROT0, "Atari", "Night Driver", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
