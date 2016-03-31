// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  National Semiconductor COP400 MCU handhelds or other simple devices,
  mostly LED electronic games/toys.

  TODO:
  - non-working games are due to MCU emulation bugs?
  - better not start on visually dumped games before other games are working
    (due to possible dump errors, hard to distinguish between that or MCU bug)

***************************************************************************/

#include "emu.h"
#include "cpu/cop400/cop400.h"
#include "sound/speaker.h"

// internal artwork
#include "ctstein.lh" // clickable
#include "einvaderc.lh" // test-layout(but still playable)
#include "funjacks.lh"
#include "funrlgl.lh"
#include "h2hbaskb.lh"
#include "lightfgt.lh" // clickable

//#include "hh_cop400_test.lh" // common test-layout - use external artwork


class hh_cop400_state : public driver_device
{
public:
	hh_cop400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<5> m_inp_matrix; // max 5
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT8 m_l;                          // MCU port L write data
	UINT8 m_g;                          // MCU port G write data
	UINT8 m_d;                          // MCU port D write data
	int m_so;                           // MCU SO line state
	int m_sk;                           // MCU SK line state
	UINT16 m_inp_mux;                   // multiplexed inputs mask

	UINT8 read_inputs(int columns);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 31 for now)

	UINT32 m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	UINT16 m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	UINT32 m_display_cache[0x20];       // (internal use)
	UINT8 m_display_decay[0x20][0x20];  // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(UINT32 digits, UINT32 mask);
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_cop400_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_l = 0;
	m_g = 0;
	m_d = 0;
	m_so = 0;
	m_sk = 0;
	m_inp_mux = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	/* save_item(NAME(m_display_cache)); */ // don't save!
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_l));
	save_item(NAME(m_g));
	save_item(NAME(m_d));
	save_item(NAME(m_so));
	save_item(NAME(m_sk));
	save_item(NAME(m_inp_mux));
}

void hh_cop400_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_cop400_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			UINT32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
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

TIMER_DEVICE_CALLBACK_MEMBER(hh_cop400_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_cop400_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_cop400_state::set_display_segmask(UINT32 digits, UINT32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void hh_cop400_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

UINT8 hh_cop400_state::read_inputs(int columns)
{
	// active low
	UINT8 ret = 0xff;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret &= m_inp_matrix[i]->read();

	return ret;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Castle Toy Einstein
  * COP421 MCU label ~/927 COP421-NEZ/N
  * 4 lamps, 1-bit sound

  This is a Simon clone, the tones are not harmonic. Two models exist, each
  with a different batteries setup, assume they're same otherwise.

***************************************************************************/

class ctstein_state : public hh_cop400_state
{
public:
	ctstein_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_READ8_MEMBER(read_l);
};

// handlers

WRITE8_MEMBER(ctstein_state::write_g)
{
	// G0-G2: input mux
	m_inp_mux = ~data & 7;
}

WRITE8_MEMBER(ctstein_state::write_l)
{
	// L0-L3: button lamps (strobed)
	display_matrix(4, 1, data & 0xf, 1);
	display_matrix(4, 1, 0, 0);
}

READ8_MEMBER(ctstein_state::read_l)
{
	// L4-L7: multiplexed inputs
	return read_inputs(3) << 4 | 0xf;
}

WRITE_LINE_MEMBER(ctstein_state::write_sk)
{
	// SK: speaker out
	m_speaker->level_w(state);
}


// config

static INPUT_PORTS_START( ctstein )
	PORT_START("IN.0") // G0 port L
	PORT_CONFNAME( 0x0f, 0x01^0x0f, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01^0x0f, "1" )
	PORT_CONFSETTING(    0x02^0x0f, "2" )
	PORT_CONFSETTING(    0x04^0x0f, "3" )
	PORT_CONFSETTING(    0x08^0x0f, "4" )

	PORT_START("IN.1") // G1 port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("Best Score")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2") // G2 port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Green Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Blue Button")
INPUT_PORTS_END

static MACHINE_CONFIG_START( ctstein, ctstein_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 860000) // approximation - RC osc. R=12K to +6V, C=100pf to GND
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_G_CB(WRITE8(ctstein_state, write_g))
	MCFG_COP400_WRITE_L_CB(WRITE8(ctstein_state, write_l))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(ctstein_state, write_sk))
	MCFG_COP400_READ_L_CB(READ8(ctstein_state, read_l))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ctstein)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Head to Head Basketball/Hockey/Soccer
  * COP420 MCU label COP420L-NEZ/N
  * 2-digit 7seg display, 41 other leds, 1-bit sound

  3 Head to Head games were released using this MCU/ROM. They play very much
  the same, only differing on game time.

  An earlier revision of this game runs on TMS1000.

***************************************************************************/

class h2hbaskb_state : public hh_cop400_state
{
public:
	h2hbaskb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_in);
	DECLARE_WRITE_LINE_MEMBER(write_so);
};

// handlers

WRITE8_MEMBER(h2hbaskb_state::write_d)
{
	// D: led select
	m_d = data & 0xf;
}

WRITE8_MEMBER(h2hbaskb_state::write_g)
{
	// G: led select, input mux
	m_inp_mux = ~data;
	m_g = data & 0xf;
}

WRITE8_MEMBER(h2hbaskb_state::write_l)
{
	// D2,D3 double as multiplexer
	UINT16 mask = ((m_d >> 2 & 1) * 0x00ff) | ((m_d >> 3 & 1) * 0xff00);
	UINT16 sel = (m_g | m_d << 4 | m_g << 8 | m_d << 12) & mask;

	// D2+G0,G1 are 7segs
	set_display_segmask(3, 0x7f);

	// L0-L6: digit segments A-G, L0-L4: led data
	// strobe display
	display_matrix(7, 16, data, sel);
	display_matrix(7, 16, 0, 0);
}

READ8_MEMBER(h2hbaskb_state::read_in)
{
	// IN: multiplexed inputs
	return (read_inputs(4) & 7) | (m_inp_matrix[4]->read() & 8);
}

WRITE_LINE_MEMBER(h2hbaskb_state::write_so)
{
	// SO: speaker out
	m_speaker->level_w(state);
}


// config

static INPUT_PORTS_START( h2hbaskb )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Pass CW") // clockwise
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Pass CCW") // counter-clockwise
	PORT_CONFNAME( 0x04, 0x04, "Players" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start/Display")
	PORT_BIT( 0x04, 0x04, IPT_SPECIAL ) PORT_CONDITION("IN.4", 0x04, EQUALS, 0x04)

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Left")
	PORT_CONFNAME( 0x04, 0x04, "Skill Level" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.3") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Right") // only for hockey/soccer
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Left") // "
	PORT_CONFNAME( 0x04, 0x04, "Factory Test" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.4") // G1+IN2, IN3 (factory set)
	PORT_CONFNAME( 0x0c, 0x00, "Game" )
	PORT_CONFSETTING(    0x00, "Basketball" )
	PORT_CONFSETTING(    0x08, "Hockey" )
	PORT_CONFSETTING(    0x0c, "Soccer" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( h2hbaskb, h2hbaskb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP420, 1600000) // approximation - RC osc. R=43K to +9V, C=101pf to GND
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(h2hbaskb_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(h2hbaskb_state, write_g))
	MCFG_COP400_WRITE_L_CB(WRITE8(h2hbaskb_state, write_l))
	MCFG_COP400_READ_IN_CB(READ8(h2hbaskb_state, read_in))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(h2hbaskb_state, write_so))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_h2hbaskb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Space Invader
  * COP444L MCU label /B138 COPL444-HRZ/N INV II (die label HRZ COP 444L/A)
  * 3 7seg LEDs, LED matrix and overlay mask, 1-bit sound

  The first version was on TMS1100 (see hh_tms1k.c), this is the reprogrammed
  second release with a gray case instead of black.

  NOTE!: MAME external artwork is required

***************************************************************************/

class einvaderc_state : public hh_cop400_state
{
public:
	einvaderc_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_WRITE_LINE_MEMBER(write_so);
	DECLARE_WRITE8_MEMBER(write_l);
};

// handlers

void einvaderc_state::prepare_display()
{
	// D0-D2 are 7segs
	for (int y = 0; y < 3; y++)
		m_display_segmask[y] = 0x7f;

	// update display
	UINT8 l = BITSWAP8(m_l,7,6,0,1,2,3,4,5);
	UINT16 grid = (m_d | m_g << 4 | m_sk << 8 | m_so << 9) ^ 0x0ff;
	display_matrix(8, 10, l, grid);
}

WRITE8_MEMBER(einvaderc_state::write_d)
{
	// D: led grid 0-3
	m_d = data;
	prepare_display();
}

WRITE8_MEMBER(einvaderc_state::write_g)
{
	// G: led grid 4-7
	m_g = data;
	prepare_display();
}

WRITE_LINE_MEMBER(einvaderc_state::write_sk)
{
	// SK: speaker out + led grid 8
	m_speaker->level_w(state);
	m_sk = state;
	prepare_display();
}

WRITE_LINE_MEMBER(einvaderc_state::write_so)
{
	// SO: led grid 9
	m_so = state;
	prepare_display();
}

WRITE8_MEMBER(einvaderc_state::write_l)
{
	// L: led state/segment
	m_l = data;
	prepare_display();
}


// config

static INPUT_PORTS_START( einvaderc )
	PORT_START("IN.0") // port IN
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "Amateur" )
	PORT_CONFSETTING(    0x00, "Professional" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( einvaderc, einvaderc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP444, 1000000) // approximation - RC osc. R=47K to +9V, C=100pf to GND(-9V)
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_READ_IN_CB(IOPORT("IN.0"))
	MCFG_COP400_WRITE_D_CB(WRITE8(einvaderc_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(einvaderc_state, write_g))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(einvaderc_state, write_sk))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(einvaderc_state, write_so))
	MCFG_COP400_WRITE_L_CB(WRITE8(einvaderc_state, write_l))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_einvaderc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mattel Funtronics Jacks
  * COP410L MCU bonded directly to PCB (die label COP410L/B NGS)
  * 8 LEDs, 1-bit sound

***************************************************************************/

class funjacks_state : public hh_cop400_state
{
public:
	funjacks_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_READ8_MEMBER(read_l);
	DECLARE_READ8_MEMBER(read_g);
};

// handlers

WRITE8_MEMBER(funjacks_state::write_d)
{
	// D: led grid + input mux
	m_d = m_inp_mux = data ^ 0xf;
	display_matrix(2, 4, m_l, m_d);
}

WRITE8_MEMBER(funjacks_state::write_l)
{
	// L0,L1: led state
	m_l = data & 3;
	display_matrix(2, 4, m_l, m_d);
}

WRITE8_MEMBER(funjacks_state::write_g)
{
	// G1: speaker out
	m_speaker->level_w(data >> 1 & 1);
	m_g = data;
}

READ8_MEMBER(funjacks_state::read_l)
{
	// L4,L5: multiplexed inputs
	return read_inputs(3) & 0x30;
}

READ8_MEMBER(funjacks_state::read_g)
{
	// G1: speaker out state
	// G2,G3: inputs
	return m_inp_matrix[3]->read() | (m_g & 2);
}


// config

static INPUT_PORTS_START( funjacks )
	PORT_START("IN.0") // D0 port G
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN.1") // D1 port G
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("IN.2") // D2 port G
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // positioned at 1 o'clock on panel, increment clockwise
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_START("IN.3") // port G
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) // speaker
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_CONFNAME( 0x08, 0x00, "Players" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( funjacks, funjacks_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP410, 2000000) // approximation - RC osc. R=47K, C=56pf
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(funjacks_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(funjacks_state, write_l))
	MCFG_COP400_WRITE_G_CB(WRITE8(funjacks_state, write_g))
	MCFG_COP400_READ_L_CB(READ8(funjacks_state, read_l))
	MCFG_COP400_READ_G_CB(READ8(funjacks_state, read_g))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_funjacks)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mattel Funtronics Red Light Green Light
  * COP410L MCU bonded directly to PCB (die label COP410L/B NHZ)
  * 14 LEDs, 1-bit sound

  known releases:
  - USA: Funtronics Red Light Green Light
  - USA(rerelease): Funtronics Hot Wheels Drag Race

***************************************************************************/

class funrlgl_state : public hh_cop400_state
{
public:
	funrlgl_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE8_MEMBER(write_g);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
};

// handlers

WRITE8_MEMBER(funrlgl_state::write_d)
{
	// D: led grid
	m_d = data ^ 0xf;
	display_matrix(4, 4, m_l, m_d);
}

WRITE8_MEMBER(funrlgl_state::write_l)
{
	// L0-L3: led state
	// L4-L7: N/C
	m_l = data & 0xf;
	display_matrix(4, 4, m_l, m_d);
}

WRITE8_MEMBER(funrlgl_state::write_g)
{
	// G3: speaker out
	m_speaker->level_w(data >> 3 & 1);
}


// config

static INPUT_PORTS_START( funrlgl )
	PORT_START("IN.0") // port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, funrlgl_state, reset_button, NULL)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(funrlgl_state::reset_button)
{
	// middle button is directly tied to MCU reset pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}


static MACHINE_CONFIG_START( funrlgl, funrlgl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP410, 2000000) // approximation - RC osc. R=51K, C=91pf
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(funrlgl_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(funrlgl_state, write_l))
	MCFG_COP400_WRITE_G_CB(WRITE8(funrlgl_state, write_g))
	MCFG_COP400_READ_G_CB(IOPORT("IN.0"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_funrlgl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Plus One
  * COP410L MCU in 8-pin DIP, label ~/029 MM 57405 (die label COP410L/B NNE)
  * 4 sensors(1 on each die side), 1-bit sound

***************************************************************************/

class plus1_state : public hh_cop400_state
{
public:
	plus1_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
};

// handlers

WRITE8_MEMBER(plus1_state::write_d)
{
	// D?: speaker out
	m_speaker->level_w(data & 1);
}


// config

static INPUT_PORTS_START( plus1 )
	PORT_START("IN.0") // port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( plus1, plus1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP410, 1000000) // approximation - RC osc. R=51K to +5V, C=100pf to GND
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(plus1_state, write_d))
	MCFG_COP400_READ_G_CB(IOPORT("IN.0"))
	MCFG_COP400_READ_L_CB(IOPORT("IN.1"))

	/* no visual feedback! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley (Electronic) Lightfight
  * COP421L MCU label /B119 COP421L-HLA/N
  * LED matrix, 1-bit sound

  Xbox-shaped electronic game for 2 or more players, with long diagonal buttons
  next to each outer LED. The main object of the game is to pinpoint a light
  by pressing 2 buttons. To start, press a skill-level button(P2 button 7/8/9)
  after selecting a game mode(P1 button 6-10).

  The game variations are:
  1: LightFight
  2: NightFight
  3: RiteSite
  4: QuiteBrite
  5: RightLight

***************************************************************************/

class lightfgt_state : public hh_cop400_state
{
public:
	lightfgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE_LINE_MEMBER(write_so);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_READ8_MEMBER(read_g);
};

// handlers

void lightfgt_state::prepare_display()
{
	UINT8 grid = (m_so | m_d << 1) ^ 0x1f;
	display_matrix(5, 5, m_l, grid);
}

WRITE_LINE_MEMBER(lightfgt_state::write_so)
{
	// SO: led grid 0 (and input mux)
	m_so = state;
	prepare_display();
}

WRITE8_MEMBER(lightfgt_state::write_d)
{
	// D: led grid 1-4 (and input mux)
	m_d = data;
	prepare_display();
}

WRITE8_MEMBER(lightfgt_state::write_l)
{
	// L0-L4: led state
	// L5-L7: N/C
	m_l = data & 0x1f;
	prepare_display();
}

WRITE_LINE_MEMBER(lightfgt_state::write_sk)
{
	// SK: speaker out
	m_speaker->level_w(state);
}

READ8_MEMBER(lightfgt_state::read_g)
{
	// G: multiplexed inputs
	m_inp_mux = (m_so | m_d << 1) ^ 0x1f;
	return read_inputs(5);
}


// config

static INPUT_PORTS_START( lightfgt )
	PORT_START("IN.0") // SO port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // note: button 1 is on the left side from player perspective
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_COCKTAIL

	PORT_START("IN.1") // D0 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_COCKTAIL

	PORT_START("IN.2") // D1 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_COCKTAIL

	PORT_START("IN.3") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_COCKTAIL

	PORT_START("IN.4") // D3 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON10 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL
INPUT_PORTS_END

static MACHINE_CONFIG_START( lightfgt, lightfgt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 950000) // approximation - RC osc. R=82K to +6V, C=56pf to GND(-6V)
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_SO_CB(WRITELINE(lightfgt_state, write_so))
	MCFG_COP400_WRITE_D_CB(WRITE8(lightfgt_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(lightfgt_state, write_l))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(lightfgt_state, write_sk))
	MCFG_COP400_READ_G_CB(READ8(lightfgt_state, read_g))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_lightfgt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ctstein )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421-nez_n", 0x0000, 0x0400, CRC(16148e03) SHA1(b2b74891d36813d9a1eefd56a925054997c4b7f7) ) // 2nd half empty
ROM_END


ROM_START( h2hbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l-nmy", 0x0000, 0x0400, CRC(87152509) SHA1(acdb869b65d49b3b9855a557ed671cbbb0f61e2c) )
ROM_END


ROM_START( einvaderc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "copl444-hrz_n_inv_ii", 0x0000, 0x0800, CRC(76400f38) SHA1(0e92ab0517f7b7687293b189d30d57110df20fe0) )
ROM_END


ROM_START( funjacks )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_ngs", 0x0000, 0x0200, CRC(863368ea) SHA1(f116cc27ae721b3a3e178fa13765808bdc275663) )
ROM_END


ROM_START( funrlgl )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_nhz", 0x0000, 0x0200, CRC(4065c3ce) SHA1(f0bc8125d922949e0d7ab1ba89c805a836d20e09) )
ROM_END


ROM_START( plus1 )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_nne", 0x0000, 0x0200, BAD_DUMP CRC(8626fdb8) SHA1(fd241b6dde0e4e86b439cb2c5bb3a82fb257d7e1) ) // still need to verify
ROM_END


ROM_START( lightfgt )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421l-hla_n", 0x0000, 0x0400, CRC(aceb2d65) SHA1(2328cbb195faf93c575f3afa3a1fe0079180edd7) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE   INPUT      INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, ctstein,   0,        0, ctstein,   ctstein,   driver_device, 0, "Castle Toy", "Einstein (Castle Toy)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1980, h2hbaskb,  0,        0, h2hbaskb,  h2hbaskb,  driver_device, 0, "Coleco", "Head to Head Basketball/Hockey/Soccer (COP420L version)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, einvaderc, einvader, 0, einvaderc, einvaderc, driver_device, 0, "Entex", "Space Invader (Entex, COP444L version)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )

CONS( 1979, funjacks,  0,        0, funjacks,  funjacks,  driver_device, 0, "Mattel", "Funtronics Jacks", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1979, funrlgl,   0,        0, funrlgl,   funrlgl,   driver_device, 0, "Mattel", "Funtronics Red Light Green Light", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

CONS( 1980, plus1,     0,        0, plus1,     plus1,     driver_device, 0, "Milton Bradley", "Plus One", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1981, lightfgt,  0,        0, lightfgt,  lightfgt,  driver_device, 0, "Milton Bradley", "Lightfight", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
