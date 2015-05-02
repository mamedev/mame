// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  National Semiconductor COP400 MCU handhelds or other simple devices,
  mostly LED electronic games/toys.

***************************************************************************/

#include "emu.h"
#include "cpu/cop400/cop400.h"
#include "sound/speaker.h"

// internal artwork
#include "einvaderc.lh" // test-layout(but still playable)
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
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety);

protected:
	virtual void machine_start();
	virtual void machine_reset();
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
				output_set_digit_value(y, active_state[y] & m_display_segmask[y]);

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
				output_set_value(buf1, state);
				output_set_value(buf2, state);
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

void hh_cop400_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	display_update();
}


UINT8 hh_cop400_state::read_inputs(int columns)
{
	UINT8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Entex Space Invader
  * COP444L MCU labeled /B138 COPL444-HRZ/N INV II (die labeled HRZ COP 444L/A)
  * 3 7seg LEDs, LED matrix and overlay mask, 1bit sound

  The first version was on TMS1100 (see hh_tms1k.c), this is the reprogrammed
  second release with a gray case instead of black.

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class einvaderc_state : public hh_cop400_state
{
public:
	einvaderc_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE_LINE_MEMBER(write_so);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_d);
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
	UINT16 grid = (m_d | m_g << 4 | m_sk << 8 | m_so << 9) ^ 0x0f0;
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
	// SK: speaker out
	m_speaker->level_w(state);

	// SK: led grid 8
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
	PORT_START("IN.0")
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
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_DISABLED) // guessed
	MCFG_COP400_READ_IN_CB(IOPORT("IN.0"))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(einvaderc_state, write_so))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(einvaderc_state, write_sk))
	MCFG_COP400_WRITE_G_CB(WRITE8(einvaderc_state, write_g))
	MCFG_COP400_WRITE_D_CB(WRITE8(einvaderc_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(einvaderc_state, write_l))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_einvaderc)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley (Electronic) Lightfight
  * COP421L MCU labeled /B119 COP421L-HLA/N
  * LED matrix, 1bit sound
  
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
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_READ8_MEMBER(read_g);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
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
	return read_inputs(5) ^ 0xf;
}


// config

static INPUT_PORTS_START( lightfgt )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // note: button 1 is on the left side from player perspective
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_COCKTAIL

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_COCKTAIL

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_COCKTAIL

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_COCKTAIL

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_COCKTAIL
INPUT_PORTS_END

static MACHINE_CONFIG_START( lightfgt, lightfgt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 950000) // approximation - RC osc. R=82K to +6V, C=56pf to GND(-6V)
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_DISABLED) // guessed
	MCFG_COP400_WRITE_SO_CB(WRITELINE(lightfgt_state, write_so))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(lightfgt_state, write_sk))
	MCFG_COP400_READ_G_CB(READ8(lightfgt_state, read_g))
	MCFG_COP400_WRITE_D_CB(WRITE8(lightfgt_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(lightfgt_state, write_l))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_lightfgt)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( einvaderc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "entexsi.bin", 0x0000, 0x0800, CRC(76400f38) SHA1(0e92ab0517f7b7687293b189d30d57110df20fe0) )
ROM_END


ROM_START( lightfgt )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "lightfight.bin", 0x0000, 0x0400, CRC(aceb2d65) SHA1(2328cbb195faf93c575f3afa3a1fe0079180edd7) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE   INPUT      INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1981, einvaderc, einvader, 0, einvaderc, einvaderc, driver_device, 0, "Entex", "Space Invader (Entex, COP444)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )

CONS( 1981, lightfgt,  0,        0, lightfgt,  lightfgt,  driver_device, 0, "Milton Bradley", "Lightfight", GAME_SUPPORTS_SAVE )
