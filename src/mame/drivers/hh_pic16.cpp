// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle, Kevin Horton
/***************************************************************************

  GI PIC16xx-driven dedicated handhelds or other simple devices.

  known chips:

  serial  device  etc.
-----------------------------------------------------------
 *024     1655    1979, <unknown> Football (have dump)
 *033     1655A   1979, Toytronic Football (have dump)
 @036     1655A   1979, Ideal Maniac
 *043     1655A   1979, Calfax/Caprice Pro-Action Baseball (have dump)
 *051     1655A   1979, Tiger Half Court Computer Basketball/Sears Electronic Basketball (have dump)
 @053     1655A   1979, Atari Touch Me
 @061     1655A   1980, Lakeside Le Boom
 @094     1655A   1980, GAF Melody Madness
 @110     1650A   1979, Tiger/Tandy Rocket Pinball
 *133     1650A   1980, U.S. Games Programmable Baseball/Tandy 2-Player Baseball (have dump)
 *144     1650A   1980, U.S. Games Football/Tandy 2-Player Football (model 60-2156) (have dump)
 *192     1650    19??, (a phone dialer, have dump)
 *255     1655    19??, (a talking clock, have dump)

  inconsistent:

 *<none>  1655A   1979, U.S. Games Basketball/Tandy Electronic Basketball (have dump)

  (* denotes not yet emulated by MAME, @ denotes it's in this driver)


  TODO:
  - leboom discrete sound for volume decay (simulated for now)

***************************************************************************/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/clock.h"
#include "sound/speaker.h"

#include "leboom.lh" // clickable
#include "maniac.lh" // clickable
#include "melodym.lh" // clickable
#include "rockpin.lh"
#include "touchme.lh" // clickable

//#include "hh_pic16_test.lh" // common test-layout - use external artwork


class hh_pic16_state : public driver_device
{
public:
	hh_pic16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<6> m_inp_matrix; // max 6
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u8 m_a;                         // MCU port A write data
	u8 m_b;                         // " B
	u8 m_c;                         // " C
	u8 m_d;                         // " D
	u16 m_inp_mux;                  // multiplexed inputs mask

	u16 read_inputs(int columns);

	// display common
	int m_display_wait;             // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u32 m_display_cache[0x20];      // (internal use)
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_pic16_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_a = 0;
	m_b = 0;
	m_c = 0;
	m_d = 0;
	m_inp_mux = ~0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	/* save_item(NAME(m_display_cache)); */ // don't save!
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_c));
	save_item(NAME(m_d));
	save_item(NAME(m_inp_mux));
}

void hh_pic16_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_pic16_state::display_update()
{
	u32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			u32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_display_segmask[y] != 0)
				output().set_digit_value(y, active_state[y] & m_display_segmask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x <= m_display_maxx; x++)
			{
				int state = active_state[y] >> x & 1;
				char buf1[0x10]; // lampyx
				char buf2[0x10]; // y.x

				if (x == m_display_maxx)
				{
					// always-on if selected
					sprintf(buf1, "lamp%da", y);
					sprintf(buf2, "%d.a", y);
				}
				else
				{
					sprintf(buf1, "lamp%d", y * mul + x);
					sprintf(buf2, "%d.%d", y, x);
				}
				output().set_value(buf1, state);
				output().set_value(buf2, state);
			}
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_pic16_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_pic16_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_pic16_state::set_display_segmask(u32 digits, u32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void hh_pic16_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

u16 hh_pic16_state::read_inputs(int columns)
{
	// active low
	u16 ret = ~0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (~m_inp_mux >> i & 1)
			ret &= m_inp_matrix[i]->read();

	return ret;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Atari Touch Me
  * PIC1655A-053
  * 2 7seg LEDs + 4 other LEDs, 1-bit sound
  
  This is the handheld version of the 1974 arcade game.
  
  known revisions:
  - Model BH-100 GI C013233 Rev 2 Atari W 1979: PIC1655A-053
  - Model BH-100 C013150 Rev 6 Atari 1979: AMI C10745 (custom ASIC)
  
***************************************************************************/

class touchme_state : public hh_pic16_state
{
public:
	touchme_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_pic16_state(mconfig, type, tag)
	{ }

	void prepare_display();
	void update_speaker();
	DECLARE_READ8_MEMBER(read_a);
	DECLARE_WRITE8_MEMBER(write_b);
	DECLARE_WRITE8_MEMBER(write_c);
};

// handlers

void touchme_state::prepare_display()
{
	set_display_segmask(3, 0x7f);
	display_matrix(7, 7, m_c, ~m_b & 0x7b);
}

void touchme_state::update_speaker()
{
	m_speaker->level_w((m_b >> 7 & 1) | (m_c >> 6 & 2));
}

READ8_MEMBER(touchme_state::read_a)
{
	// A: multiplexed inputs
	return read_inputs(3) & 0xf;
}

WRITE8_MEMBER(touchme_state::write_b)
{
	// B0-B2: input mux
	m_inp_mux = data & 7;
	
	// B0,B1: digit select
	// B3-B6: leds
	m_b = data;
	prepare_display();
	
	// B7: speaker lead 1
	update_speaker();
}

WRITE8_MEMBER(touchme_state::write_c)
{
	// C0-C6: digit segments
	m_c = data;
	prepare_display();

	// C7: speaker lead 2
	update_speaker();
}


// config

static INPUT_PORTS_START( touchme )
	PORT_START("IN.0") // B0 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("High")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Skill")

	PORT_START("IN.1") // B1 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Blue Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Red Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Green Button")

	PORT_START("IN.2") // B2 port A
	PORT_CONFNAME( 0x07, 0x01^0x07, "Game Select")
	PORT_CONFSETTING(    0x01^0x07, "1" )
	PORT_CONFSETTING(    0x02^0x07, "2" )
	PORT_CONFSETTING(    0x04^0x07, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const s16 touchme_speaker_levels[] = { 0, 0x7fff, -0x8000, 0 };

static MACHINE_CONFIG_START( touchme, touchme_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC1655, 300000) // approximation - RC osc. R=100K, C=47pF
	MCFG_PIC16C5x_READ_A_CB(READ8(touchme_state, read_a))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(touchme_state, write_b))
	MCFG_PIC16C5x_READ_C_CB(CONSTANT(0xff))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(touchme_state, write_c))
	
	MCFG_DEVICE_ADD("clock", CLOCK, 300000/4) // PIC CLKOUT, tied to RTCC
	MCFG_CLOCK_SIGNAL_HANDLER(INPUTLINE("maincpu", PIC16C5x_RTCC))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_pic16_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_touchme)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, touchme_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  GAF Melody Madness
  * PIC1655A-094
  * 2 lamps under tube, 1-bit sound
  
  Melody Madness is a tabletop music memory game, shaped like a jukebox.
  It can also be played as a simple electronic piano.

***************************************************************************/

class melodym_state : public hh_pic16_state
{
public:
	melodym_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_pic16_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_b);
	DECLARE_READ8_MEMBER(read_c);
	DECLARE_WRITE8_MEMBER(write_c);
};

// handlers

WRITE8_MEMBER(melodym_state::write_b)
{
	// B2-B6: input mux
	m_inp_mux = data >> 2 & 0x1f;
}

READ8_MEMBER(melodym_state::read_c)
{
	// C0-C4: multiplexed inputs
	return read_inputs(5) | 0xe0;
}

WRITE8_MEMBER(melodym_state::write_c)
{
	// C6: both lamps
	m_display_wait = 2;
	display_matrix(1, 1, ~data >> 6 & 1, 1);
	
	// C7: speaker out
	m_speaker->level_w(~data >> 7 & 1);
}


// config

static INPUT_PORTS_START( melodym )
	PORT_START("IN.0") // B2 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Button 5")

	PORT_START("IN.1") // B3 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Button 7")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Button 8")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Button 9")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Button 10")

	PORT_START("IN.2") // B4 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Button 11")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Button 12")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // there is no button 13
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Button 14")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Button 15")

	PORT_START("IN.3") // B5 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 16")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Button 17")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Button 18")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Button 19")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Button 20")

	PORT_START("IN.4") // B6 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Button 21")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Button 22")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Button 23")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Button 24")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Button 25")

	PORT_START("IN.5") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Novice")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Whiz")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Pro")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Note")
INPUT_PORTS_END

static MACHINE_CONFIG_START( melodym, melodym_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC1655, 1000000) // approximation
	MCFG_PIC16C5x_READ_A_CB(IOPORT("IN.5"))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(melodym_state, write_b))
	MCFG_PIC16C5x_READ_C_CB(READ8(melodym_state, read_c))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(melodym_state, write_c))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_pic16_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_melodym)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Ideal Maniac, by Ralph Baer
  * PIC1655A-036
  * 2 7seg LEDs, 1-bit sound
  
  Maniac is a reflex game for 2-4 players. There are 4 challenges:
  1: Musical Maniac: Press the button as soon as the music stops.
  2: Sounds Abound: Count the number of tones in the song, then press the button
     after the same amount of beeps.
  3: Look Twice: Press the button after the game repeats the first pattern.
  4: Your Time Is Up: Press the button after estimating the duration of the tone.

***************************************************************************/

class maniac_state : public hh_pic16_state
{
public:
	maniac_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_pic16_state(mconfig, type, tag)
	{ }

	void prepare_display();
	void update_speaker();
	DECLARE_WRITE8_MEMBER(write_b);
	DECLARE_WRITE8_MEMBER(write_c);
};

// handlers

void maniac_state::prepare_display()
{
	m_display_state[0] = ~m_b & 0x7f;
	m_display_state[1] = ~m_c & 0x7f;

	set_display_segmask(3, 0x7f);
	set_display_size(7, 2);
	display_update();
}

void maniac_state::update_speaker()
{
	m_speaker->level_w((m_b >> 7 & 1) | (m_c >> 6 & 2));
}

WRITE8_MEMBER(maniac_state::write_b)
{
	// B0-B6: left 7seg
	m_b = data;
	prepare_display();
	
	// B7: speaker lead 1
	update_speaker();
}

WRITE8_MEMBER(maniac_state::write_c)
{
	// C0-C6: right 7seg
	m_c = data;
	prepare_display();

	// C7: speaker lead 2
	update_speaker();
}


// config

static INPUT_PORTS_START( maniac )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // top button, increment clockwise
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END

static const s16 maniac_speaker_levels[] = { 0, 0x7fff, -0x8000, 0 };

static MACHINE_CONFIG_START( maniac, maniac_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC1655, 1000000) // approximation - RC osc. R=~13.4K, C=470pF
	MCFG_PIC16C5x_READ_A_CB(IOPORT("IN.0"))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(maniac_state, write_b))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(maniac_state, write_c))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_pic16_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_maniac)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, maniac_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Lakeside Le Boom
  * PIC1655A-061
  * 1 led, 1-bit sound with RC circuit for volume decay
  
  This is a tabletop timebomb defusion game. It's shaped like an aerial bomb,
  and starts 'ticking' when the player opens the keypad door. To begin, select
  the game mode, rows(keypad size), and fuse duration.
  
  Game modes as described on the box:
  1: Eliminate the buttons one by one in the order set out by the computer. Press
     one twice and you'll be sorry!
  2: For 2 or more players. Take turns pressing the buttons, remember which ones.
     Press a button a second time and watch out, it's all over.
  3: The computer picks one secret button that stops the fuse. You must press it
     on your 5th turn. Listen to the clues and you'll do fine.
  4: The computer picks a secret combination. Find it first by listening to the
     clues. Find the right order and you'll get it to fizzle out.
  
***************************************************************************/

class leboom_state : public hh_pic16_state
{
public:
	leboom_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_pic16_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER(read_a);
	DECLARE_WRITE8_MEMBER(write_b);
	DECLARE_WRITE8_MEMBER(write_c);

	void speaker_decay_reset();
	TIMER_DEVICE_CALLBACK_MEMBER(speaker_decay_sim);
	double m_speaker_volume;

protected:
	virtual void machine_start() override;
};

// handlers

void leboom_state::speaker_decay_reset()
{
	if (~m_c & 0x80)
		m_speaker_volume = 1.0;

	m_speaker->set_output_gain(0, m_speaker_volume);
}

TIMER_DEVICE_CALLBACK_MEMBER(leboom_state::speaker_decay_sim)
{
	// volume decays when speaker is off (divisor and timer period determine duration)
	speaker_decay_reset();
	m_speaker_volume /= 1.015;
}

READ8_MEMBER(leboom_state::read_a)
{
	// A: multiplexed inputs
	return read_inputs(6) & 0xf;
}

WRITE8_MEMBER(leboom_state::write_b)
{
	// B0-B5: input mux
	m_inp_mux = data & 0x3f;
}

WRITE8_MEMBER(leboom_state::write_c)
{
	// C4: single led
	display_matrix(1, 1, data >> 4 & 1, 1);

	// C7: speaker on
	m_c = data;
	speaker_decay_reset();

	// C6: speaker out
	m_speaker->level_w(data >> 6 & 1);
}


// config

static INPUT_PORTS_START( leboom )
	PORT_START("IN.0") // B0 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Red Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Red Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Red Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Red Button 4")

	PORT_START("IN.1") // B1 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Red-Red Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Red-Green Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Red-Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Red-Blue Button")

	PORT_START("IN.2") // B2 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Shortest")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Short")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Long")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Longest")

	PORT_START("IN.3") // B3 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Yellow Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Yellow Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Yellow Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Yellow Button 4")

	PORT_START("IN.4") // B4 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Blue Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Blue Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Blue Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Blue Button 4")

	PORT_START("IN.5") // B5 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Blue Button 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Blue Button 6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Blue Button 7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Blue Button 8")
INPUT_PORTS_END

void leboom_state::machine_start()
{
	hh_pic16_state::machine_start();

	// zerofill/init
	m_speaker_volume = 0;
	save_item(NAME(m_speaker_volume));
}

static MACHINE_CONFIG_START( leboom, leboom_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC1655, 1000000) // approximation
	MCFG_PIC16C5x_READ_A_CB(READ8(leboom_state, read_a))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(leboom_state, write_b))
	MCFG_PIC16C5x_READ_C_CB(CONSTANT(0xff))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(leboom_state, write_c))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_pic16_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_leboom)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("speaker_decay", leboom_state, speaker_decay_sim, attotime::from_msec(25))
MACHINE_CONFIG_END





/***************************************************************************

  Tiger Electronics Rocket Pinball (model 7-460)
  * PIC1655A-110, 69-11397
  * 3 7seg LEDs + 44 other LEDs, 1-bit sound
  
  known releases:
  - Hong Kong: Rocket Pinball
  - USA(1): Rocket Pinball (model 60-2140), distributed by Tandy
  - USA(2): Cosmic Pinball (model 49-65456), distributed by Sears
  
***************************************************************************/

class rockpin_state : public hh_pic16_state
{
public:
	rockpin_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_pic16_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_a);
	DECLARE_WRITE8_MEMBER(write_b);
	DECLARE_WRITE8_MEMBER(write_c);
	DECLARE_WRITE8_MEMBER(write_d);
};

// handlers

void rockpin_state::prepare_display()
{
	// 3 7seg leds from ports A and B
	set_display_segmask(7, 0x7f);
	display_matrix(7, 3, m_b, m_a, false);
	
	// 44 leds from ports C and D
	for (int y = 0; y < 6; y++)
		m_display_state[y+3] = (m_d >> y & 1) ? m_c : 0;

	set_display_size(8, 3+6);
	display_update();
}

WRITE8_MEMBER(rockpin_state::write_a)
{
	// A3,A4: speaker out
	m_speaker->level_w(data >> 3 & 3);
	
	// A0-A2: select digit
	m_a = ~data & 7;
	prepare_display();
}

WRITE8_MEMBER(rockpin_state::write_b)
{
	// B0-B6: digit segments
	m_b = data & 0x7f;
	prepare_display();
}

WRITE8_MEMBER(rockpin_state::write_c)
{
	// C0-C7: led data
	m_c = ~data;
	prepare_display();
}

WRITE8_MEMBER(rockpin_state::write_d)
{
	// D0-D5: led select
	m_d = ~data;
	prepare_display();
}


// config

static INPUT_PORTS_START( rockpin )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Ball")
INPUT_PORTS_END

static const s16 rockpin_speaker_levels[] = { 0, 0x7fff, -0x8000, 0 };

static MACHINE_CONFIG_START( rockpin, rockpin_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC1650, 500000) // approximation - RC osc. R=47K, C=47pF
	MCFG_PIC16C5x_READ_A_CB(IOPORT("IN.0"))
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(rockpin_state, write_a))
	MCFG_PIC16C5x_READ_B_CB(CONSTANT(0xff))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(rockpin_state, write_b))
	MCFG_PIC16C5x_READ_C_CB(CONSTANT(0xff))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(rockpin_state, write_c))
	MCFG_PIC16C5x_READ_D_CB(CONSTANT(0xff))
	MCFG_PIC16C5x_WRITE_D_CB(WRITE8(rockpin_state, write_d))
	
	MCFG_DEVICE_ADD("clock", CLOCK, 500000/4) // PIC CLKOUT, tied to RTCC
	MCFG_CLOCK_SIGNAL_HANDLER(INPUTLINE("maincpu", PIC16C5x_RTCC))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_pic16_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_rockpin)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, rockpin_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( touchme )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic1655a-053", 0x0000, 0x0400, CRC(f0858f0a) SHA1(53ffe111d43db1c110847590350ef62f02ed5e0e) )
ROM_END


ROM_START( melodym )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic1655a-094", 0x0000, 0x0400, CRC(6d35bd7b) SHA1(20e326085878f69a9d4ef1651ef4443f27188567) )
ROM_END


ROM_START( maniac )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic1655a-036", 0x0000, 0x0400, CRC(a96f7011) SHA1(e97ae44d3c1e74c7e1024bb0bdab03eecdc9f827) )
ROM_END


ROM_START( leboom )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic1655a-061", 0x0000, 0x0400, CRC(5880eea1) SHA1(e3795b347fd5df9de084da36e33f6b70fbc0b0ae) )
ROM_END


ROM_START( rockpin )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic1650a-110_69-11397", 0x0000, 0x0400, CRC(d5396e77) SHA1(952feaff70fde53a9eda84c54704520d50749e78) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE INPUT    INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, touchme,   0,        0, touchme, touchme, driver_device, 0, "Atari", "Touch Me (handheld, Rev 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1980, melodym,   0,        0, melodym, melodym, driver_device, 0, "GAF", "Melody Madness", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, maniac,    0,        0, maniac,  maniac,  driver_device, 0, "Ideal", "Maniac", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1980, leboom,    0,        0, leboom,  leboom,  driver_device, 0, "Lakeside", "Le Boom", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, rockpin,   0,        0, rockpin, rockpin, driver_device, 0, "Tiger Electronics", "Rocket Pinball", MACHINE_SUPPORTS_SAVE )
