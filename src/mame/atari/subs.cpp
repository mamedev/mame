// license:BSD-3-Clause
// copyright-holders:Mike Balfour

/***************************************************************************

    Atari Subs hardware

    driver by Mike Balfour

    Games supported:
        * Subs

    Known issues:
        * none at this time

****************************************************************************

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"

#include "subs_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "layout/generic.h"


namespace {

class subs_state : public driver_device
{
public:
	subs_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_dial(*this, "DIAL%u", 1U),
		m_in(*this, "IN%u", 0U),
		m_dsw(*this, "DSW")
	{ }

	void subs(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<discrete_device> m_discrete;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	required_ioport_array<2> m_dial;
	required_ioport_array<2> m_in;
	required_ioport m_dsw;

	int32_t m_steering_buf[2]{};
	uint8_t m_steering_val[2]{};
	int32_t m_last_val[2]{};

	void steer_reset_w(uint8_t data);
	uint8_t control_r(offs_t offset);
	uint8_t coin_r(offs_t offset);
	uint8_t options_r(offs_t offset);
	template <uint8_t Pen> void invert_w(int state);
	void noise_reset_w(uint8_t data);

	void palette(palette_device &palette) const;

	template <uint8_t Which> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);

	template <uint8_t Which> int steering();

	void main_map(address_map &map) ATTR_COLD;
};


template <uint8_t Pen>
void subs_state::invert_w(int state)
{
	if (state)
	{
		m_palette->set_pen_color(Pen, rgb_t(0x00, 0x00, 0x00));
		m_palette->set_pen_color(Pen + 1, rgb_t(0xff, 0xff, 0xff));
	}
	else
	{
		m_palette->set_pen_color(Pen + 1, rgb_t(0x00, 0x00, 0x00));
		m_palette->set_pen_color(Pen, rgb_t(0xff, 0xff, 0xff));
	}
}


template <uint8_t Which> // left 0, right 1
uint32_t subs_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// for every character in the Video RAM, check if it has been modified
	// since last time and update it accordingly.
	for (int offs = 0x400 - 1; offs >= 0; offs--)
	{
		int sonar_window[2]{};

		int charcode = m_videoram[offs];

		// Which monitor is this for?
		int const enable = Which ? charcode & 0x40 : charcode & 0x80;

		int const sx = 8 * (offs % 32);
		int const sy = 8 * (offs / 32);

		// Special hardware logic for sonar windows */
		if ((sy >= (128 + 64)) && (sx < 32))
			sonar_window[0] = 1;
		else if ((sy >= (128 + 64)) && (sx >= (128 + 64 + 32)))
			sonar_window[1] = 1;
		else
			charcode = charcode & 0x3f;

		// draw the screen
		if ((enable || sonar_window[Which]) && (!sonar_window[!Which]))
			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
					charcode, 1,
					0, 0, sx, sy);
		else
			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
					0, 1,
					0, 0, sx, sy);
	}

	// draw the motion objects
	for (int offs = 0; offs < 4; offs++)
	{
		int const sx = m_spriteram[0x00 + (offs * 2)] - 16;
		int const sy = m_spriteram[0x08 + (offs * 2)] - 16;
		int charcode = m_spriteram[0x09 + (offs * 2)];
		int sub_enable;
		if (offs < 2)
			sub_enable = m_spriteram[0x01 + (offs * 2)] & 0x80;
		else
			sub_enable = 1;

		int const prom_set = charcode & 0x01;
		charcode = (charcode >> 3) & 0x1f;

		// special check for drawing other screen's sub
		if ((offs != Which) || (sub_enable))
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					charcode + 32 * prom_set,
					0,
					0, 0, sx, sy, 0);
	}

	if (!Which)
	{
		// Update sound
		m_discrete->write(SUBS_LAUNCH_DATA, m_spriteram[5] & 0x0f);   // Launch data
		m_discrete->write(SUBS_CRASH_DATA, m_spriteram[5] >> 4);      // Crash/explode data
	}

	return 0;
}


/*************************************
 *
 *  Palette generation
 *
 *************************************/

void subs_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); // BLACK - modified on video invert
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff)); // WHITE - modified on video invert
	palette.set_pen_color(2, rgb_t(0x00, 0x00, 0x00)); // BLACK - modified on video invert
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0xff)); // WHITE - modified on video invert
}


/***************************************************************************
machine initialization
***************************************************************************/

void subs_state::machine_start()
{
	save_item(NAME(m_steering_buf));
	save_item(NAME(m_steering_val));
	save_item(NAME(m_last_val));
}

void subs_state::machine_reset()
{
	m_steering_buf[0] = 0;
	m_steering_buf[1] = 0;
	m_steering_val[0] = 0x00;
	m_steering_val[1] = 0x00;
}

/***************************************************************************
interrupt
***************************************************************************/
INTERRUPT_GEN_MEMBER(subs_state::interrupt)
{
	// only do NMI interrupt if not in TEST mode
	if ((m_in[1]->read() & 0x40) == 0x40)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/***************************************************************************
Steering

When D7 is high, the steering wheel has moved.
If D6 is high, it moved left.  If D6 is low, it moved right.
Be sure to keep returning a direction until steer_reset is called.
***************************************************************************/
template <uint8_t Which>
int subs_state::steering()
{
	int const this_val = m_dial[!Which]->read();

	int delta = this_val - m_last_val[Which];
	m_last_val[Which] = this_val;
	if (delta > 128) delta -= 256;
	else if (delta < -128) delta += 256;
	// Divide by four to make our steering less sensitive
	m_steering_buf[Which] += (delta / 4);

	if (m_steering_buf[Which] > 0)
	{
			m_steering_buf[Which]--;
			m_steering_val[Which] = 0xc0;
	}
	else if (m_steering_buf[Which] < 0)
	{
			m_steering_buf[Which]++;
			m_steering_val[Which] = 0x80;
	}

	return m_steering_val[Which];
}

/***************************************************************************
steer_reset
***************************************************************************/
void subs_state::steer_reset_w(uint8_t data)
{
	m_steering_val[0] = 0x00;
	m_steering_val[1] = 0x00;
}

/***************************************************************************
control_r
***************************************************************************/
uint8_t subs_state::control_r(offs_t offset)
{
	int const inport = m_in[0]->read();

	switch (offset & 0x07)
	{
		case 0x00:      return ((inport & 0x01) << 7);  // diag step
		case 0x01:      return ((inport & 0x02) << 6);  // diag hold
		case 0x02:      return ((inport & 0x04) << 5);  // slam
		case 0x03:      return ((inport & 0x08) << 4);  // spare
		case 0x04:      return ((steering<0>() & 0x40) << 1);  // steer dir 1
		case 0x05:      return ((steering<0>() & 0x80) << 0);  // steer flag 1
		case 0x06:      return ((steering<1>() & 0x40) << 1);  // steer dir 2
		case 0x07:      return ((steering<1>() & 0x80) << 0);  // steer flag 2
	}

	return 0;
}

/***************************************************************************
coin_r
***************************************************************************/
uint8_t subs_state::coin_r(offs_t offset)
{
	int const inport = m_in[1]->read();

	switch (offset & 0x07)
	{
		case 0x00:      return ((inport & 0x01) << 7);  // coin 1
		case 0x01:      return ((inport & 0x02) << 6);  // start 1
		case 0x02:      return ((inport & 0x04) << 5);  // coin 2
		case 0x03:      return ((inport & 0x08) << 4);  // start 2
		case 0x04:      return ((inport & 0x10) << 3);  // VBLANK
		case 0x05:      return ((inport & 0x20) << 2);  // fire 1
		case 0x06:      return ((inport & 0x40) << 1);  // test
		case 0x07:      return ((inport & 0x80) << 0);  // fire 2
	}

	return 0;
}

/***************************************************************************
options_r
***************************************************************************/
uint8_t subs_state::options_r(offs_t offset)
{
	int const opts = m_dsw->read();

	switch (offset & 0x03)
	{
		case 0x00:      return ((opts & 0xc0) >> 6);        // language
		case 0x01:      return ((opts & 0x30) >> 4);        // credits
		case 0x02:      return ((opts & 0x0c) >> 2);        // game time
		case 0x03:      return ((opts & 0x03) >> 0);        // extended time
	}

	return 0;
}


/***************************************************************************
sub sound functions
***************************************************************************/

void subs_state::noise_reset_w(uint8_t data)
{
	// Pulse noise reset
	m_discrete->write(SUBS_NOISE_RESET, 0);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void subs_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x008f).ram();
	map(0x0000, 0x0000).w(FUNC(subs_state::noise_reset_w));
	map(0x0000, 0x0007).r(FUNC(subs_state::control_r));
	map(0x0020, 0x0020).w(FUNC(subs_state::steer_reset_w));
	map(0x0020, 0x0027).r(FUNC(subs_state::coin_r));
//  map(0x0040, 0x0040).w(FUNC(subs_state::timer_reset_w));
	map(0x0060, 0x0063).r(FUNC(subs_state::options_r));
	map(0x0060, 0x006f).w("latch", FUNC(ls259_device::write_a0));
	map(0x0090, 0x009f).ram().share(m_spriteram);
	map(0x00a0, 0x01ff).ram();
	map(0x0800, 0x0bff).ram().share(m_videoram);
	map(0x2000, 0x3fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( subs )
	PORT_START("DSW") // OPTIONS
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Credit/Time" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Each Coin Buys Time" )
	PORT_DIPSETTING(    0x02, "Fixed Time" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Language ) )  PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x04, DEF_STR( French ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Spanish) )
	PORT_DIPSETTING(    0x0c, DEF_STR( German) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x40, "Game Length" )  PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, "0:30 Minutes" )
	PORT_DIPSETTING(    0x20, "1:00 Minutes" )
	PORT_DIPSETTING(    0x40, "1:30 Minutes" )
	PORT_DIPSETTING(    0x60, "2:00 Minutes" )
	PORT_DIPSETTING(    0x80, "2:30 Minutes" )
	PORT_DIPSETTING(    0xa0, "3:00 Minutes" )
	PORT_DIPSETTING(    0xc0, "3:30 Minutes" )
	PORT_DIPSETTING(    0xe0, "4:00 Minutes" )

	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Diag Step
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Diag Hold
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_TILT )    // Slam
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  // Spare
	PORT_BIT ( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED ) // Filled in with steering information

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("lscreen", FUNC(screen_device::vblank))
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout playfield_layout =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout motion_layout =
{
	16,16,
	64,
	1,
	{ 0 },
	{ 3 + 0x400*8, 2 + 0x400*8, 1 + 0x400*8, 0 + 0x400*8,
		7 + 0x400*8, 6 + 0x400*8, 5 + 0x400*8, 4 + 0x400*8,
		3, 2, 1, 0, 7, 6, 5, 4 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};


static GFXDECODE_START( gfx_subs )
	GFXDECODE_ENTRY( "playfield", 0, playfield_layout, 0, 2 )
	GFXDECODE_ENTRY( "motion",    0, motion_layout,    0, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void subs_state::subs(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12'096'000 / 16);      // clock input is the "4H" signal
	m_maincpu->set_addrmap(AS_PROGRAM, &subs_state::main_map);
	m_maincpu->set_periodic_int(FUNC(subs_state::interrupt), attotime::from_hz(4*57));


	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_subs);

	PALETTE(config, m_palette, FUNC(subs_state::palette), 4);

	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(57);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); //not accurate
	lscreen.set_size(32*8, 32*8);
	lscreen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	lscreen.set_screen_update(FUNC(subs_state::screen_update<0>));
	lscreen.set_palette(m_palette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(57);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); //not accurate
	rscreen.set_size(32*8, 32*8);
	rscreen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	rscreen.set_screen_update(FUNC(subs_state::screen_update<1>));
	rscreen.set_palette(m_palette);


	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	DISCRETE(config, m_discrete, subs_discrete).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1);

	ls259_device &latch(LS259(config, "latch")); // C9
	latch.q_out_cb<0>().set_output("led0").invert(); // START LAMP 1
	latch.q_out_cb<1>().set_output("led1").invert(); // START LAMP 2
	latch.q_out_cb<2>().set(m_discrete, FUNC(discrete_device::write_line<SUBS_SONAR2_EN>));
	latch.q_out_cb<3>().set(m_discrete, FUNC(discrete_device::write_line<SUBS_SONAR1_EN>));
	// Schematics show crash and explode reversed.  But this is proper.
	latch.q_out_cb<4>().set(m_discrete, FUNC(discrete_device::write_line<SUBS_EXPLODE_EN>));
	latch.q_out_cb<5>().set(m_discrete, FUNC(discrete_device::write_line<SUBS_CRASH_EN>));
	latch.q_out_cb<6>().set(FUNC(subs_state::invert_w<0>));
	latch.q_out_cb<7>().set(FUNC(subs_state::invert_w<2>));
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( subs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "34196.e2",     0x2000, 0x0100, CRC(7c7a04c3) SHA1(269d9f7573cc5da4412f53d647127c4884435353) )   // ROM 0 D4-D7
	ROM_LOAD_NIB_LOW ( "34194.e1",     0x2000, 0x0100, CRC(6b1c4acc) SHA1(3a743b721d9e7e9bdc4533aeeab294eb0ea27500) )   // ROM 0 D0-D3
	ROM_LOAD( "34190.p1",              0x2800, 0x0800, CRC(a88aef21) SHA1(3811c137041ca43a6e49fbaf7d9d8ef37ba190a2) )
	ROM_LOAD( "34191.p2",              0x3000, 0x0800, CRC(2c652e72) SHA1(097b665e803cbc57b5a828403a8d9a258c19e97f) )
	ROM_LOAD( "34192.n2",              0x3800, 0x0800, CRC(3ce63d33) SHA1(a413cb3e0d03dc40a50f5b03b76a4edbe7906f3e) )

	ROM_REGION( 0x0800, "playfield", 0 )
	ROM_LOAD( "34211.m4",     0x0000, 0x0800, CRC(fa8d4409) SHA1(a83b7a835212d31fe421d537fa0d78f234c26f5b) )

	ROM_REGION( 0x0800, "motion", 0 )
	ROM_LOAD( "34216.d7",     0x0000, 0x0200, CRC(941d28b4) SHA1(89388ec06546dc567aa5dbc6a7898974f2871ecc) )
	ROM_LOAD( "34218.e7",     0x0200, 0x0200, CRC(f4f4d874) SHA1(d99ad9a74611f9851f6bfa6000ebd70e1a364f5d) )
	ROM_LOAD( "34217.d8",     0x0400, 0x0200, CRC(a7a60da3) SHA1(34fc21cc1ca69d58d3907094dc0a3faaf6f461b3) )
	ROM_LOAD( "34219.e8",     0x0600, 0x0200, CRC(99a5a49b) SHA1(2cb429f8de73c7d78dc83e47f1448ea4340c333d) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1977, subs, 0, subs, subs, subs_state, empty_init, ROT0, "Atari", "Subs", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
