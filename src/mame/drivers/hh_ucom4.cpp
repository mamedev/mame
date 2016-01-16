// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  NEC uCOM4 MCU tabletops/handhelds or other simple devices,
  most of them (emulated ones) are VFD electronic games/toys.

  Commonly used VFD(vacuum fluorescent display) are by NEC or Futaba.

  NEC FIP9AM20T (example, Epoch Astro Command)
         grcss

  FIP = fluorescent indicator panel
  g = number of grids
  A = revision of the VFD
  M = custom display
  20 = unique display part number


  known chips:

  serial  device   etc.
----------------------------------------------------------------
 @017     uPD552C  1979, Bambino UFO Master-Blaster Station (ET-02)
 @042     uPD552C  1980, Tomy Cosmic Combat (TN-??)
 @043     uPD552C  1979, Bambino Kick The Goal Soccer (ET-10)
 *044     uPD552C  1979, Bambino Lucky Puck Ice Hockey (ET-08)
 @048     uPD552C  1980, Tomy Tennis (TN-04)
 @049     uPD552C  1981, Bambino Safari (ET-11)
 @054     uPD552C  1980, Epoch Invader From Space

 @031     uPD553C  1979, Bambino Superstar Football (ET-03)
 @049     uPD553C  1979, Mego Mini-Vid Break Free
 @055     uPD553C  1980, Bambino Space Laser Fight (ET-12)
 *073     uPD553C  1980, Sony ST-J75 FM Stereo Tuner
 @080     uPD553C  1980, Epoch Electronic Football
 *102     uPD553C  1981, Bandai Block Out
 @153     uPD553C  1981, Epoch Galaxy II
 @160     uPD553C  1982, Tomy Pac Man (TN-08)
 @170     uPD553C  1982, Bandai Crazy Climber
 @192     uPD553C  1982, Tomy Scramble (TN-10)
 @202     uPD553C  1982, Epoch Astro Command
 @206     uPD553C  1982, Epoch Dracula
 @209     uPD553C  1982, Tomy Caveman (TN-12)
 @258     uPD553C  1984, Tomy Alien Chase (TN-16)

 @512     uPD557LC 1980, Castle Toy Tactix

 *060     uPD650C  1979, Mattel Computer Gin
 *085     uPD650C  1980, Roland TR-808
 *127     uPD650C  198?, Sony OA-S1100 Typecorder (subcpu, have dump)
 *128     uPD650C  1981, Roland TR-606
  133     uPD650C  1982, Roland TB-303 -> tb303.c

  (* denotes not yet emulated by MAME, @ denotes it's in this driver)


TODO:
  - games that rely on the fact that faster/longer strobed elements appear brighter:
    tactix(player 2)

***************************************************************************/

#include "includes/hh_ucom4.h"

// internal artwork
#include "efball.lh"
#include "mvbfree.lh"
#include "tactix.lh" // clickable

#include "hh_ucom4_test.lh" // common test-layout - use external artwork


// machine start/reset

void hh_ucom4_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	memset(m_port, 0, sizeof(m_port));
	m_inp_mux = 0;
	m_grid = 0;
	m_plate = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	/* save_item(NAME(m_display_cache)); */ // don't save!
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_port));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_ucom4_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_ucom4_state::display_update()
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

TIMER_DEVICE_CALLBACK_MEMBER(hh_ucom4_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_ucom4_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_ucom4_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	display_update();
}


UINT8 hh_ucom4_state::read_inputs(int columns)
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

  Bambino UFO Master-Blaster Station (manufactured in Japan)
  * PCB label Emix Corp. ET-02
  * NEC uCOM-44 MCU, labeled EMIX D552C 017
  * cyan VFD display Emix-101, with blue color overlay

  This is Bambino's first game, it is not known if ET-01 exists. Emix Corp.
  wasn't initially a toy company, the first release was through Tomy. Emix
  created the Bambino brand afterwards. It is claimed to be the first
  computerized VFD game (true, unless TI Speak & Spell(1978), or even Invicta
  Electronic Mastermind(1977) are considered games)

  known releases:
  - Japan: "Missile Guerilla Warfare Maneuvers", published by Tomy
  - World: UFO Master-Blaster Station

  NOTE!: MAME external artwork is required

***************************************************************************/

class ufombs_state : public hh_ucom4_state
{
public:
	ufombs_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
};

// handlers

void ufombs_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,3,2,1,0,4,5,6,7,8);
	UINT16 plate = BITSWAP16(m_plate,15,14,13,12,11,7,10,6,9,5,8,4,0,1,2,3);
	display_matrix(10, 9, plate, grid);
}

WRITE8_MEMBER(ufombs_state::grid_w)
{
	// F,G,H0: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTF) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(ufombs_state::plate_w)
{
	// C,D012,I: vfd matrix plate
	int shift = (offset == NEC_UCOM4_PORTI) ? 8 : (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(ufombs_state::speaker_w)
{
	// E01: speaker out
	m_speaker->level_w(data & 3);
}


// config

static INPUT_PORTS_START( ufombs )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, 0x04, IPT_SPECIAL ) PORT_CONDITION("IN.0", 0x0a, EQUALS, 0x00) // pad in the middle, pressed when joystick is centered
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("IN.1") // port B
	PORT_CONFNAME( 0x07, 0x01, "Skill Level" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static const INT16 ufombs_speaker_levels[] = { 0, 32767, -32768, 0 };

static MACHINE_CONFIG_START( ufombs, ufombs_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(ufombs_state, plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(ufombs_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(ufombs_state, speaker_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(ufombs_state, grid_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(ufombs_state, grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(ufombs_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(ufombs_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, ufombs_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bambino Superstar Football (manufactured in Japan)
  * PCB label Emix Corp. ET-03
  * NEC uCOM-43 MCU, labeled D553C 031
  * cyan VFD display Emix-102, with bezel

  The game was rereleased in 1982 as Classic Football, with an improved VFD.

  Press the Kick button to start the game, an automatic sequence follows.
  Then choose a formation(A,B,C) and either pass the ball, and/or start
  running. For more information, refer to the official manual.

  NOTE!: MAME external artwork is required

***************************************************************************/

class ssfball_state : public hh_ucom4_state
{
public:
	ssfball_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_READ8_MEMBER(input_b_r);
};

// handlers

void ssfball_state::prepare_display()
{
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,11,7,3,12,17,13,18,16,14,15,10,9,8,0,1,2,4,5,6);
	display_matrix(16, 9, plate, m_grid);
}

WRITE8_MEMBER(ssfball_state::grid_w)
{
	// C,D(,E3): vfd matrix grid 0-7(,8)
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(ssfball_state::plate_w)
{
	m_port[offset] = data;

	// E,F,G,H,I(not all!): vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// F3,G3: input mux + speaker
	m_inp_mux = (m_port[NEC_UCOM4_PORTF] >> 3 & 1) | (m_port[NEC_UCOM4_PORTG] >> 2 & 2);
	m_speaker->level_w(m_inp_mux);

	// E3: vfd matrix grid 8
	if (offset == NEC_UCOM4_PORTE)
		grid_w(space, offset, data >> 3 & 1);
	else
		prepare_display();
}

READ8_MEMBER(ssfball_state::input_b_r)
{
	// B: input port 2, where B3 is multiplexed
	return m_inp_matrix[2]->read() | read_inputs(2);
}


// config

/* physical button layout and labels is like this:

    [A]    [B]    [C]    [PASS]  [KICK/
       ^FORMATION^                DISPLAY]

                                 [^]
                         [<>]
    (game lvl sw)                [v]
    1<---OFF--->2
*/

static INPUT_PORTS_START( ssfball )
	PORT_START("IN.0") // F3 port B3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Formation A")

	PORT_START("IN.1") // G3 port B3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Game Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.2") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Kick/Display")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Formation C")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Formation B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) // multiplexed, handled in input_b_r

	PORT_START("IN.3") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Ball-carrier Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Ball-carrier Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Ball-carrier Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pass")
INPUT_PORTS_END


static const INT16 ssfball_speaker_levels[] = { 0, 32767, -32768, 0 };

static MACHINE_CONFIG_START( ssfball, ssfball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.3"))
	MCFG_UCOM4_READ_B_CB(READ8(ssfball_state, input_b_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(ssfball_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(ssfball_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(ssfball_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(ssfball_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(ssfball_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(ssfball_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(ssfball_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, ssfball_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bambino Kick The Goal Soccer
  * PCB label Emix Corp. ET-10/08 (PCB is for 2 possible games)
  * NEC uCOM-44 MCU, labeled D552C 043
  * cyan VFD display Emix-105, with bezel overlay

  Press the Display button twice to start the game. Action won't start until
  player 1 presses one of the directional keys. In 2-player mode, player 2
  controls the goalkeeper, defensive players are still controlled by the CPU.

  NOTE!: MAME external artwork is required

***************************************************************************/

class bmsoccer_state : public hh_ucom4_state
{
public:
	bmsoccer_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_READ8_MEMBER(input_a_r);
};

// handlers

void bmsoccer_state::prepare_display()
{
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,11,7,3,12,17,13,18,16,14,15,8,4,0,9,5,1,10,6,2);
	display_matrix(16, 9, plate, m_grid);
}

WRITE8_MEMBER(bmsoccer_state::grid_w)
{
	// C01: input mux
	if (offset == NEC_UCOM4_PORTC)
		m_inp_mux = data & 3;

	// C,D(,E3): vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(bmsoccer_state::plate_w)
{
	// G3: speaker out
	if (offset == NEC_UCOM4_PORTG)
		m_speaker->level_w(data >> 3 & 1);

	// E012,F012,G012,H,I: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// E3: grid 8
	if (offset == NEC_UCOM4_PORTE)
		grid_w(space, offset, data >> 3 & 1);
	else
		prepare_display();
}

READ8_MEMBER(bmsoccer_state::input_a_r)
{
	// port A: multiplexed inputs
	return read_inputs(2);
}


// config

static INPUT_PORTS_START( bmsoccer )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Ball-carrier Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Ball-carrier Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Ball-carrier Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Ball-carrier Up")

	PORT_START("IN.1") // C1 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("Goalkeeper Left") // note: swap buttons if viewed from the same angle as player 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("Goalkeeper Right")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // port B
	PORT_CONFNAME( 0x01, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFNAME( 0x02, 0x00, "Players" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Display/Banana Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Shoot")
INPUT_PORTS_END

static MACHINE_CONFIG_START( bmsoccer, bmsoccer_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(READ8(bmsoccer_state, input_a_r))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.2"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(bmsoccer_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(bmsoccer_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(bmsoccer_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(bmsoccer_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(bmsoccer_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(bmsoccer_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(bmsoccer_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bambino Safari (manufactured in Japan)
  * PCB label Emix Corp. ET-11
  * NEC uCOM-44 MCU, labeled EMIX D552C 049
  * cyan VFD display Emix-108

  NOTE!: MAME external artwork is required

***************************************************************************/

class bmsafari_state : public hh_ucom4_state
{
public:
	bmsafari_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
};

// handlers

void bmsafari_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,0,1,2,3,4,5,6,7,8);
	UINT16 plate = BITSWAP16(m_plate,15,14,13,12,11,7,10,2,9,5,8,4,0,1,6,3);
	display_matrix(10, 9, plate, grid);
}

WRITE8_MEMBER(bmsafari_state::grid_w)
{
	// C,D(,E3): vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(bmsafari_state::plate_w)
{
	// E012,H,I: vfd matrix plate
	int shift = (offset == NEC_UCOM4_PORTE) ? 8 : (offset - NEC_UCOM4_PORTH) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// E3: grid 0
	if (offset == NEC_UCOM4_PORTE)
		grid_w(space, offset, data >> 3 & 1);
	else
		prepare_display();
}

WRITE8_MEMBER(bmsafari_state::speaker_w)
{
	// G0: speaker out
	m_speaker->level_w(data & 1);
}


// config

static INPUT_PORTS_START( bmsafari )
	PORT_START("IN.0") // port A
	PORT_CONFNAME( 0x07, 0x04, "Skill Level" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x01, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
INPUT_PORTS_END

static MACHINE_CONFIG_START( bmsafari, bmsafari_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(bmsafari_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(bmsafari_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(bmsafari_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(bmsafari_state, speaker_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(bmsafari_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(bmsafari_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bambino Space Laser Fight (manufactured in Japan)
  * PCB label Emix Corp. ET-12
  * NEC uCOM-43 MCU, labeled D553C 055
  * cyan VFD display Emix-104, with blue or green color overlay

  This is basically a revamp of their earlier Boxing game (ET-06), case and
  buttons are exactly the same.

  NOTE!: MAME external artwork is required

***************************************************************************/

class splasfgt_state : public hh_ucom4_state
{
public:
	splasfgt_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_READ8_MEMBER(input_b_r);
};

// handlers

void splasfgt_state::prepare_display()
{
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,18,17,13,1,0,8,6,0,10,11,14,15,16,9,5,7,4,2,3);
	display_matrix(16, 9, plate, m_grid);
}

WRITE8_MEMBER(splasfgt_state::grid_w)
{
	// G,H,I0: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTG) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

	// G(grid 0-3): input mux
	m_inp_mux = m_grid & 0xf;

	// I2: vfd matrix plate 6
	if (offset == NEC_UCOM4_PORTI)
		plate_w(space, 4 + NEC_UCOM4_PORTC, data >> 2 & 1);
	else
		prepare_display();
}

WRITE8_MEMBER(splasfgt_state::plate_w)
{
	// F01: speaker out
	if (offset == NEC_UCOM4_PORTF)
		m_speaker->level_w(data & 3);

	// C,D,E,F23(,I2): vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

READ8_MEMBER(splasfgt_state::input_b_r)
{
	// B: multiplexed buttons
	return read_inputs(4);
}


// config

/* physical button layout and labels is like this:

    * left = P1 side *                                         * right = P2 side * (note: in 1P mode, switch sides between turns)

    [  JUMP  ]  [ HIGH ]        (players sw)                   [ HIGH ]  [  JUMP  ]
                                1<--->2         [START/
    [STRAIGHT]  [MEDIUM]                         RESET]        [MEDIUM]  [STRAIGHT]
                                1<---OFF--->2
    [ STOOP  ]  [ LOW  ]        (skill lvl sw)                 [ LOW  ]  [ STOOP  ]
*/

static INPUT_PORTS_START( splasfgt )
	PORT_START("IN.0") // G0 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P1 Position Straight")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("P1 Position Jump")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 Position Stoop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // G1 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("P1 Beam High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("P1 Beam Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 Beam Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // G2 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P2 Position Straight")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P2 Position Jump")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P2 Position Stoop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // G3 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P2 Beam High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P2 Beam Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("P2 Beam Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // port A
	PORT_CONFNAME( 0x01, 0x00, "Players" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFNAME( 0x02, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static const INT16 splasfgt_speaker_levels[] = { 0, 32767, -32768, 0 };

static MACHINE_CONFIG_START( splasfgt, splasfgt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.4"))
	MCFG_UCOM4_READ_B_CB(READ8(splasfgt_state, input_b_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(splasfgt_state, plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(splasfgt_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(splasfgt_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(splasfgt_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(splasfgt_state, grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(splasfgt_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(splasfgt_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, splasfgt_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Crazy Climber (manufactured in Japan)
  * PCB labels SM-020/SM-021
  * NEC uCOM-43 MCU, labeled D553C 170
  * cyan/red/green VFD display NEC FIP6AM2-T no. 1-8 2, with partial color overlay and bezel

  known releases:
  - Japan: FL Crazy Climbing
  - USA: Crazy Climber

  NOTE!: MAME external artwork is required

***************************************************************************/

class bcclimbr_state : public hh_ucom4_state
{
public:
	bcclimbr_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void bcclimbr_state::prepare_display()
{
	UINT8 grid = BITSWAP8(m_grid,7,6,0,1,2,3,4,5);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,16,17,18,19,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
	display_matrix(20, 6, plate, grid);
}

WRITE8_MEMBER(bcclimbr_state::grid_w)
{
	// I2: speaker out
	if (offset == NEC_UCOM4_PORTI)
		m_speaker->level_w(data >> 2 & 1);

	// H,I01: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTH) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(bcclimbr_state::plate_w)
{
	// C,D,E,F: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( bcclimbr )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bcclimbr, bcclimbr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(bcclimbr_state, plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(bcclimbr_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(bcclimbr_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(bcclimbr_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(bcclimbr_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(bcclimbr_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(bcclimbr_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Castle Toy Tactix
  * NEC uCOM-43 MCU, labeled D557LC 512
  * 16 LEDs behind buttons

  Tactix is similar to Merlin, for 1 or 2 players. In 2-player mode, simply
  don't press the Comp Turn button. The four included minigames are:
  1: Capture (reversi)
  2: Jump-Off (peg solitaire)
  3: Triple Play (3 in a row)
  4: Concentration (memory)

  note: MAME external artwork is not needed for this game

***************************************************************************/

class tactix_state : public hh_ucom4_state
{
public:
	tactix_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_WRITE8_MEMBER(input_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(tactix_state::leds_w)
{
	// D,F: 4*4 led matrix
	m_port[offset] = data;
	display_matrix(4, 4, m_port[NEC_UCOM4_PORTD], m_port[NEC_UCOM4_PORTF]);
}

WRITE8_MEMBER(tactix_state::speaker_w)
{
	// G0: speaker out
	m_speaker->level_w(data & 1);
}

WRITE8_MEMBER(tactix_state::input_w)
{
	// C,E0: input mux
	m_port[offset] = data;
	m_inp_mux = (m_port[NEC_UCOM4_PORTE] << 4 & 0x10) | m_port[NEC_UCOM4_PORTC];
}

READ8_MEMBER(tactix_state::input_r)
{
	// A: multiplexed inputs
	return read_inputs(5);
}


// config

static INPUT_PORTS_START( tactix )
	PORT_START("IN.0") // C0 port A
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Button 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 13")

	PORT_START("IN.1") // C1 port A
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Button 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Button 10")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Button 14")

	PORT_START("IN.2") // C2 port A
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Button 7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Button 11")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Button 15")

	PORT_START("IN.3") // C3 port A
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Button 8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Button 12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Button 16")

	PORT_START("IN.4") // E0 port A
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Comp Turn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static MACHINE_CONFIG_START( tactix, tactix_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D557L, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(READ8(tactix_state, input_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tactix_state, input_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tactix_state, leds_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tactix_state, input_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tactix_state, leds_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tactix_state, speaker_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tactix)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Epoch Invader From Space (manufactured in Japan)
  * PCB labels 36010(A/B)
  * NEC uCOM-44 MCU, labeled D552C 054
  * cyan VFD display NEC FIP9AM18T tube no. 0D, with color overlay

  known releases:
  - USA: Invader From Space
  - UK: Invader From Space, published by Grandstand

  NOTE!: MAME external artwork is required

***************************************************************************/

class invspace_state : public hh_ucom4_state
{
public:
	invspace_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void invspace_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,8,9,7,6,5,4,3,2,1,0);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,9,14,13,8,15,11,10,7,11,3,2,6,10,1,5,9,0,4,8);
	display_matrix(19, 9, plate, grid);
}

WRITE8_MEMBER(invspace_state::grid_w)
{
	// I0: speaker out
	if (offset == NEC_UCOM4_PORTI)
		m_speaker->level_w(data & 1);

	// C,D,I1: vfd matrix grid
	int shift = (offset == NEC_UCOM4_PORTI) ? 8 : (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(invspace_state::plate_w)
{
	// E,F,G,H123: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( invspace )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( invspace, invspace_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(invspace_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(invspace_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(invspace_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(invspace_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(invspace_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(invspace_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(invspace_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Epoch Electronic Football (manufactured in Japan)
  * PCB labels 36020(A/B/C)
  * NEC uCOM-43 MCU, labeled D553C 080
  * cyan VFD display NEC FIP10AM15T tube no. 0F, with bezel overlay

  known releases:
  - USA: Electronic Football (aka Pro-Bowl Football)
  - Japan: American Football

  note: MAME external artwork is not needed for this game

***************************************************************************/

class efball_state : public hh_ucom4_state
{
public:
	efball_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void efball_state::prepare_display()
{
	UINT16 plate = BITSWAP16(m_plate,15,14,13,12,11,4,3,0,2,1,6,10,9,5,8,7);
	display_matrix(11, 10, plate, m_grid);
}

WRITE8_MEMBER(efball_state::grid_w)
{
	// H2: speaker out
	if (offset == NEC_UCOM4_PORTH)
		m_speaker->level_w(data >> 2 & 1);

	// F,G,H01: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTF) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(efball_state::plate_w)
{
	// D,E,I: vfd matrix plate
	int shift = (offset == NEC_UCOM4_PORTI) ? 8 : (offset - NEC_UCOM4_PORTD) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( efball )
	PORT_START("IN.0") // port A
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
	PORT_CONFNAME( 0x02, 0x02, "Players" )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P1 Down-Field")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P1 Score-Time")

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass")

	PORT_START("IN.2") // port C
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Kick Return")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Kick")
INPUT_PORTS_END

static MACHINE_CONFIG_START( efball, efball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_READ_C_CB(IOPORT("IN.2"))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(efball_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(efball_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(efball_state, grid_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(efball_state, grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(efball_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(efball_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_efball)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Epoch Galaxy II (manufactured in Japan)
  * PCB labels 19096/96062
  * NEC uCOM-43 MCU, labeled D553C 153
  * cyan/red VFD display NEC FIP10xM20T, with color overlay. x = multiple VFD
    revisions exist, with different graphics: rev B no. 1-8, rev. D no. 2-21.

  known releases:
  - USA: Galaxy II
  - Japan: Astro Wars
  - UK: Astro Wars, published by Grandstand

  NOTE!: MAME external artwork is required

***************************************************************************/

class galaxy2_state : public hh_ucom4_state
{
public:
	galaxy2_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void galaxy2_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	UINT16 plate = BITSWAP16(m_plate,15,3,2,6,1,5,4,0,11,10,7,12,14,13,8,9);
	display_matrix(15, 10, plate, grid);
}

WRITE8_MEMBER(galaxy2_state::grid_w)
{
	// E3: speaker out
	if (offset == NEC_UCOM4_PORTE)
		m_speaker->level_w(data >> 3 & 1);

	// C,D,E01: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(galaxy2_state::plate_w)
{
	// F,G,H,I: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTF) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( galaxy2 )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( galaxy2, galaxy2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(galaxy2_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(galaxy2_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(galaxy2_state, grid_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(galaxy2_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(galaxy2_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(galaxy2_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(galaxy2_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Epoch Astro Command (manufactured in Japan)
  * PCB labels 96111/96112
  * NEC uCOM-43 MCU, labeled D553C 202
  * cyan/red VFD display NEC FIP9AM20T no. 42-42, with color overlay + bezel

  known releases:
  - Japan: Astro Command
  - USA: Astro Command, published by Tandy
  - UK: Scramble, published by Grandstand

  NOTE!: MAME external artwork is required

***************************************************************************/

class astrocmd_state : public hh_ucom4_state
{
public:
	astrocmd_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void astrocmd_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,8,4,5,6,7,0,1,2,3);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,3,2,12,13,14,15,16,17,18,0,1,4,8,5,9,7,11,6,10);
	display_matrix(17, 9, plate, grid);
}

WRITE8_MEMBER(astrocmd_state::grid_w)
{
	// C,D(,E3): vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(astrocmd_state::plate_w)
{
	// E01,F,G,H,I: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	if (offset == NEC_UCOM4_PORTE)
	{
		// E2: speaker out
		m_speaker->level_w(data >> 2 & 1);

		// E3: vfd matrix grid 8
		grid_w(space, offset, data >> 3 & 1);
	}
	else
		prepare_display();
}


// config

static INPUT_PORTS_START( astrocmd )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Missile")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bomb")

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

static MACHINE_CONFIG_START( astrocmd, astrocmd_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(astrocmd_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(astrocmd_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(astrocmd_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(astrocmd_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(astrocmd_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(astrocmd_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(astrocmd_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Epoch Dracula (manufactured in Japan)
  * PCB label 96121
  * NEC uCOM-43 MCU, labeled D553C 206
  * cyan/red/green VFD display NEC FIP8BM20T no. 2-42

  known releases:
  - Japan: Dracula House, yellow case
  - USA: Dracula, red case
  - Other: Dracula, yellow case, published by Hales

  NOTE!: MAME external artwork is required

***************************************************************************/

class edracula_state : public hh_ucom4_state
{
public:
	edracula_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

WRITE8_MEMBER(edracula_state::grid_w)
{
	// C,D: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	display_matrix(18, 8, m_plate, m_grid);
}

WRITE8_MEMBER(edracula_state::plate_w)
{
	// I2: speaker out
	if (offset == NEC_UCOM4_PORTI)
		m_speaker->level_w(data >> 2 & 1);

	// E,F,G,H,I01: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	display_matrix(18, 8, m_plate, m_grid);
}


// config

static INPUT_PORTS_START( edracula )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

static MACHINE_CONFIG_START( edracula, edracula_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(edracula_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(edracula_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(edracula_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mego Mini-Vid Break Free (manufactured in Japan)
  * PCB label Mego 79 rev F
  * NEC uCOM-43 MCU, labeled D553C 049
  * cyan VFD display Futaba DM-4.5 91

  note: MAME external artwork is not needed for this game

***************************************************************************/

class mvbfree_state : public hh_ucom4_state
{
public:
	mvbfree_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
};

// handlers

void mvbfree_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
	UINT16 plate = BITSWAP16(m_plate,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	display_matrix(10, 14, plate, grid);
}

WRITE8_MEMBER(mvbfree_state::grid_w)
{
	// E23,F,G,H: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

	// E01: plate 0,1
	if (offset == NEC_UCOM4_PORTE)
		plate_w(space, 2 + NEC_UCOM4_PORTC, data & 3);
	else
		prepare_display();
}

WRITE8_MEMBER(mvbfree_state::plate_w)
{
	// C,D(,E01): vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(mvbfree_state::speaker_w)
{
	// I0: speaker out
	m_speaker->level_w(data & 1);
}


// config

static INPUT_PORTS_START( mvbfree )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.1") // port B
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED ) // unimplemented p1/p2 buttons
	PORT_CONFNAME( 0x0c, 0x04, "Game Select")
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFSETTING(    0x08, "3" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( mvbfree, mvbfree_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(mvbfree_state, plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(mvbfree_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(mvbfree_state, grid_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(mvbfree_state, grid_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(mvbfree_state, grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(mvbfree_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(mvbfree_state, speaker_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_mvbfree)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Cosmic Combat (manufactured in Japan)
  * PCBs are labeled 2E1019-E01
  * NEC uCOM-44 MCU, labeled D552C 042
  * cyan VFD display NEC FIP32AM18Y tube no. 0E, with color overlay

  known releases:
  - USA: Cosmic Combat
  - Japan: Space Attack

  NOTE!: MAME external artwork is required

***************************************************************************/

class tccombat_state : public hh_ucom4_state
{
public:
	tccombat_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void tccombat_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,8,3,2,1,0,7,6,5,4);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,11,15,3,10,14,2,9,13,1,0,12,8,15,1,5,0,3,7,2,6);
	display_matrix(20, 9, plate, grid);
}

WRITE8_MEMBER(tccombat_state::grid_w)
{
	// I1: speaker out
	if (offset == NEC_UCOM4_PORTI)
		m_speaker->level_w(data >> 1 & 1);

	// C,D,I0: vfd matrix grid
	int shift = (offset == NEC_UCOM4_PORTI) ? 8 : (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(tccombat_state::plate_w)
{
	// E,F123,G,H: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( tccombat )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_CONFNAME( 0x02, 0x02, "Skill Level" )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
INPUT_PORTS_END

static MACHINE_CONFIG_START( tccombat, tccombat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, 400000) // approximation
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tccombat_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tccombat_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tccombat_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tccombat_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tccombat_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tccombat_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tccombat_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Tennis (manufactured in Japan)
  * PCB labeled TOMY TN-04 TENNIS
  * NEC uCOM-44 MCU, labeled D552C 048
  * VFD display NEC FIP11AM15T tube no. 0F, with overlay

  The initial release of this game was in 1979, known as Pro-Tennis,
  it has a D553 instead of D552, with just a little over 50% ROM used.

  Press the Serve button to start, then hit the ball by pressing one of the
  positional buttons when the ball flies over it.

  NOTE!: MAME external artwork is required

***************************************************************************/

class tmtennis_state : public hh_ucom4_state
{
public:
	tmtennis_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(port_e_w);
	DECLARE_READ8_MEMBER(input_r);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(difficulty_switch);

protected:
	virtual void machine_reset() override;
};

// handlers

WRITE8_MEMBER(tmtennis_state::grid_w)
{
	// G,H,I: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTG) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	display_matrix(12, 12, m_plate, m_grid);
}

WRITE8_MEMBER(tmtennis_state::plate_w)
{
	// C,D,F: vfd matrix plate
	int shift = (offset == NEC_UCOM4_PORTF) ? 8 : (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	display_matrix(12, 12, m_plate, m_grid);
}

WRITE8_MEMBER(tmtennis_state::port_e_w)
{
	// E01: input mux
	// E2: speaker out
	// E3: N/C
	m_inp_mux = data & 3;
	m_speaker->level_w(data >> 2 & 1);
}

READ8_MEMBER(tmtennis_state::input_r)
{
	// A,B: multiplexed buttons
	return ~read_inputs(2) >> (offset*4);
}


// config

/* Pro-Tennis physical button layout and labels is like this:

    * left = P2/CPU side *    * right = P1 side *

    [SERVE] [1] [2] [3]       [3] [2] [1] [SERVE]
            [4] [5] [6]       [6] [5] [4]

    PRACTICE<--PRO1-->PRO2    1PLAYER<--OFF-->2PLAYER
*/

static INPUT_PORTS_START( tmtennis )
	PORT_START("IN.0") // E0 port A/B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Serve")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Serve")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("P1 Button 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("P1 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 Button 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P1 Button 5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P1 Button 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 Button 6")

	PORT_START("IN.1") // E1 port A/B
	PORT_CONFNAME( 0x101, 0x100, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, tmtennis_state, difficulty_switch, NULL)
	PORT_CONFSETTING(     0x001, "Practice" )
	PORT_CONFSETTING(     0x100, "Pro 1" ) // -> difficulty_switch
	PORT_CONFSETTING(     0x000, "Pro 2" )
	PORT_CONFNAME( 0x02, 0x00, "Players" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("P2 Button 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("P2 Button 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 Button 5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("P2 Button 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 Button 6")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(tmtennis_state::difficulty_switch)
{
	set_clock();
}


void tmtennis_state::set_clock()
{
	// MCU clock is from an LC circuit oscillating by default at ~360kHz,
	// but on PRO1, the difficulty switch puts a capacitor across the LC circuit
	// to slow it down to ~260kHz.
	m_maincpu->set_unscaled_clock((m_inp_matrix[1]->read() & 0x100) ? 260000 : 360000);
}

void tmtennis_state::machine_reset()
{
	hh_ucom4_state::machine_reset();
	set_clock();
}

static MACHINE_CONFIG_START( tmtennis, tmtennis_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, 360000) // see set_clock
	MCFG_UCOM4_READ_A_CB(READ8(tmtennis_state, input_r))
	MCFG_UCOM4_READ_B_CB(READ8(tmtennis_state, input_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tmtennis_state, plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tmtennis_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tmtennis_state, port_e_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tmtennis_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tmtennis_state, grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tmtennis_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tmtennis_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Pac-Man (manufactured in Japan)
  * PCBs are labeled TN-08 2E108E01
  * NEC uCOM-43 MCU, labeled D553C 160
  * cyan/red/green VFD display NEC FIP8AM18T no. 2-21
  * bright yellow round casing

  known releases:
  - Japan: Puck Man
  - USA: Pac Man
  - UK: Puckman (Tomy), and also published by Grandstand as Munchman
  - Australia: Pac Man-1, published by Futuretronics

  The game will start automatically after turning it on. This Pac Man refuses
  to eat dots with his butt, you can only eat them going right-to-left.

  NOTE!: MAME external artwork is required

***************************************************************************/

class tmpacman_state : public hh_ucom4_state
{
public:
	tmpacman_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void tmpacman_state::prepare_display()
{
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,16,17,18,11,10,9,8,0,2,3,1,4,5,6,7,12,13,14,15) | 0x100;
	display_matrix(19, 8, plate, grid);
}

WRITE8_MEMBER(tmpacman_state::grid_w)
{
	// C,D: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(tmpacman_state::plate_w)
{
	// E1: speaker out
	if (offset == NEC_UCOM4_PORTE)
		m_speaker->level_w(data >> 1 & 1);

	// E023,F,G,H,I: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( tmpacman )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // port B
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tmpacman, tmpacman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_430kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tmpacman_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tmpacman_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tmpacman_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tmpacman_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tmpacman_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tmpacman_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tmpacman_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Scramble (manufactured in Japan)
  * PCBs are labeled TN-10 2E114E01
  * NEC uCOM-43 MCU, labeled D553C 192
  * cyan/red/green VFD display NEC FIP10CM20T no. 2-41

  known releases:
  - World: Scramble
  - USA: Scramble, published by Tandy
  - UK: Astro Blaster, published by Hales (Epoch Astro Command was named Scramble)
  - Germany: Rambler

  NOTE!: MAME external artwork is required

***************************************************************************/

class tmscramb_state : public hh_ucom4_state
{
public:
	tmscramb_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void tmscramb_state::prepare_display()
{
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,18,17,3,15,2,14,1,13,16,0,12,8,4,9,5,10,6,11,7) | 0x400;
	display_matrix(17, 10, plate, m_grid);
}

WRITE8_MEMBER(tmscramb_state::grid_w)
{
	// I2: speaker out
	if (offset == NEC_UCOM4_PORTI)
		m_speaker->level_w(data >> 2 & 1);

	// C,D,I01: vfd matrix grid
	int shift = (offset == NEC_UCOM4_PORTI) ? 8 : (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(tmscramb_state::plate_w)
{
	// E,F,G,H: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( tmscramb )
	PORT_START("IN.0") // port A
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tmscramb, tmscramb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tmscramb_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tmscramb_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tmscramb_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tmscramb_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tmscramb_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tmscramb_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tmscramb_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Caveman (manufactured in Japan)
  * PCBs are labeled TN-12 2E114E03
  * NEC uCOM-43 MCU, labeled D553C 209
  * cyan/red/green VFD display NEC FIP8AM20T no. 2-42

  known releases:
  - World: Caveman
  - USA: Caveman, published by Tandy
  - UK: Cave Man - Jr. Caveman vs Dinosaur, published by Grandstand

  NOTE!: MAME external artwork is required

***************************************************************************/

class tcaveman_state : public hh_ucom4_state
{
public:
	tcaveman_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void tcaveman_state::prepare_display()
{
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,10,11,5,6,7,8,0,9,2,18,17,16,3,15,14,13,12,4,1) | 0x40;
	display_matrix(19, 8, plate, grid);
}

WRITE8_MEMBER(tcaveman_state::grid_w)
{
	// C,D: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE8_MEMBER(tcaveman_state::plate_w)
{
	// E3: speaker out
	if (offset == NEC_UCOM4_PORTE)
		m_speaker->level_w(data >> 3 & 1);

	// E012,F,G,H,I: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}


// config

static INPUT_PORTS_START( tcaveman )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x08, "Professional" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tcaveman, tcaveman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tcaveman_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tcaveman_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tcaveman_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tcaveman_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tcaveman_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tcaveman_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tcaveman_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy Alien Chase (manufactured in Japan)
  * PCBs are labeled TN-16 2E121B01
  * NEC uCOM-43 MCU, labeled D553C 258
  * red/green VFD display NEC FIP9AM24T, with color overlay, 2-sided*

  *Player one views the VFD from the front (grid+filament side) while the
  opposite player views it from the back side (through the conductive traces),
  basically a mirror-image.

  To start the game, simply press [UP]. Hold a joystick direction to move around.

  NOTE!: MAME external artwork is required

***************************************************************************/

class alnchase_state : public hh_ucom4_state
{
public:
	alnchase_state(const machine_config &mconfig, device_type type, std::string tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(output_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(alnchase_state::output_w)
{
	if (offset <= NEC_UCOM4_PORTE)
	{
		// C,D,E0: vfd matrix grid
		int shift = (offset - NEC_UCOM4_PORTC) * 4;
		m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

		// C0(grid 0): input enable PL1
		// D0(grid 4): input enable PL2
		m_inp_mux = (m_grid & 1) | (m_grid >> 3 & 2);

		// E1: speaker out
		if (offset == NEC_UCOM4_PORTE)
			m_speaker->level_w(data >> 1 & 1);
	}

	if (offset >= NEC_UCOM4_PORTE)
	{
		// E23,F,G,H,I: vfd matrix plate
		int shift = (offset - NEC_UCOM4_PORTE) * 4;
		m_plate = ((m_plate << 2 & ~(0xf << shift)) | (data << shift)) >> 2;
	}

	display_matrix(17, 9, m_plate, m_grid);
}

READ8_MEMBER(alnchase_state::input_r)
{
	// A: multiplexed buttons
	return read_inputs(2);
}


// config

/* physical button layout and labels is like this:

    POWER SOUND LEVEL PLAYER
     ON    ON    PRO   TWO        START
      o     o     |     |
      |     |     |     |       [joystick]
      |     |     o     o
     OFF   OFF   AMA   ONE     GAME 0,1,2,3

    1 PLAYER SIDE

    other player side only has a joystick
*/

static INPUT_PORTS_START( alnchase )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.1") // D0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) // on non-mirrored view, swap P2 left/right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)

	PORT_START("IN.2") // port B
	PORT_CONFNAME( 0x01, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x02, "Professional" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( alnchase, alnchase_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(READ8(alnchase_state, input_r))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.2"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(alnchase_state, output_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(alnchase_state, output_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(alnchase_state, output_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(alnchase_state, output_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(alnchase_state, output_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(alnchase_state, output_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(alnchase_state, output_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_ucom4_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ufombs )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-017", 0x0000, 0x0400, CRC(0e208cb3) SHA1(57db6566916c94325e2b67ccb94b4ea3b233487d) )
ROM_END


ROM_START( ssfball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-031", 0x0000, 0x0800, CRC(ff5d91d0) SHA1(9b2c0ae45f1e3535108ee5fef8a9010e00c8d5c3) )
ROM_END


ROM_START( bmsoccer )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-043", 0x0000, 0x0400, CRC(10c2a4ea) SHA1(6ebca7d406e22ff7a8cd529579b55a700da487b4) )
ROM_END


ROM_START( bmsafari )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-049", 0x0000, 0x0400, CRC(82fa3cbe) SHA1(019e7ec784e977eba09997fc46af253054fb222c) )
ROM_END


ROM_START( splasfgt )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-055", 0x0000, 0x0800, CRC(eb471fbd) SHA1(f06cfe567bf6f9ed4dcdc88acdcfad50cd370a02) )
ROM_END


ROM_START( bcclimbr )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-170", 0x0000, 0x0800, CRC(fc2eabdb) SHA1(0f5cc854be7fdf105d9bd2114659d40c65f9d782) )
ROM_END


ROM_START( tactix )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d557lc-512", 0x0000, 0x0800, CRC(1df738cb) SHA1(15a5de28a3c03e6894d29c56b5b424983569ccf2) )
ROM_END


ROM_START( invspace )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-054", 0x0000, 0x0400, CRC(913d9c13) SHA1(f20edb5458e54d2f6d4e45e5d59efd87e05a6f3f) )
ROM_END


ROM_START( efball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-080", 0x0000, 0x0800, CRC(54c1027f) SHA1(6cc98074dae9361fa8c0ed6501b6a57ad325ccbd) )
ROM_END


ROM_START( galaxy2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-153.s01", 0x0000, 0x0800, CRC(70d552b3) SHA1(72d50647701cb4bf85ea947a149a317aaec0f52c) )
ROM_END


ROM_START( astrocmd )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-202.s01", 0x0000, 0x0800, CRC(b4b34883) SHA1(6246d561c2df1f2124575d2ca671ef85b1819edd) )
ROM_END


ROM_START( edracula )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-206.s01", 0x0000, 0x0800, CRC(b524857b) SHA1(c1c89ed5dd4bb1e6e98462dc8fa5af2aa48d8ede) )
ROM_END


ROM_START( mvbfree )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-049", 0x0000, 0x0800, CRC(d64a8399) SHA1(97887e486fa29b1fc4a5a40cacf3c960f67aacbf) )
ROM_END


ROM_START( tccombat )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-042", 0x0000, 0x0400, CRC(d7b5cfeb) SHA1(a267be8e43b7740758eb0881b655b1cc8aec43da) )
ROM_END


ROM_START( tmtennis )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-048", 0x0000, 0x0400, CRC(78702003) SHA1(4d427d4dbeed901770c682338867f58c7b54eee3) )
ROM_END


ROM_START( tmpacman )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-160", 0x0000, 0x0800, CRC(b21a8af7) SHA1(e3122be1873ce76a4067386bf250802776f0c2f9) )
ROM_END


ROM_START( tmscramb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-192", 0x0000, 0x0800, CRC(00fcc501) SHA1(a7771e934bf8268c83f38c7ec0acc668836e0939) )
ROM_END


ROM_START( tcaveman )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-209", 0x0000, 0x0800, CRC(d230d4b7) SHA1(2fb12b60410f5567c5e3afab7b8f5aa855d283be) )
ROM_END


ROM_START( alnchase )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-258", 0x0000, 0x0800, CRC(c5284ff5) SHA1(6a20aaacc9748f0e0335958f3cea482e36153704) )
ROM_END



/*    YEAR  NAME      PARENT COMPAT MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, ufombs,   0,        0, ufombs,   ufombs,   driver_device, 0, "Bambino", "UFO Master-Blaster Station", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1979, ssfball,  0,        0, ssfball,  ssfball,  driver_device, 0, "Bambino", "Superstar Football", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1979, bmsoccer, 0,        0, bmsoccer, bmsoccer, driver_device, 0, "Bambino", "Kick The Goal Soccer", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, bmsafari, 0,        0, bmsafari, bmsafari, driver_device, 0, "Bambino", "Safari (Bambino)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1980, splasfgt, 0,        0, splasfgt, splasfgt, driver_device, 0, "Bambino", "Space Laser Fight", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1982, bcclimbr, 0,        0, bcclimbr, bcclimbr, driver_device, 0, "Bandai", "Crazy Climber (Bandai)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1980, tactix,   0,        0, tactix,   tactix,   driver_device, 0, "Castle Toy", "Tactix", MACHINE_SUPPORTS_SAVE )

CONS( 1980, invspace, 0,        0, invspace, invspace, driver_device, 0, "Epoch", "Invader From Space", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1980, efball,   0,        0, efball,   efball,   driver_device, 0, "Epoch", "Electronic Football (Epoch)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, galaxy2,  0,        0, galaxy2,  galaxy2,  driver_device, 0, "Epoch", "Galaxy II", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, astrocmd, 0,        0, astrocmd, astrocmd, driver_device, 0, "Epoch", "Astro Command", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, edracula, 0,        0, edracula, edracula, driver_device, 0, "Epoch", "Dracula (Epoch)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, mvbfree,  0,        0, mvbfree,  mvbfree,  driver_device, 0, "Mego", "Mini-Vid Break Free", MACHINE_SUPPORTS_SAVE )

CONS( 1980, tccombat, 0,        0, tccombat, tccombat, driver_device, 0, "Tomy", "Cosmic Combat", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1980, tmtennis, 0,        0, tmtennis, tmtennis, driver_device, 0, "Tomy", "Tennis (Tomy)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, tmpacman, 0,        0, tmpacman, tmpacman, driver_device, 0, "Tomy", "Pac Man (Tomy)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, tmscramb, 0,        0, tmscramb, tmscramb, driver_device, 0, "Tomy", "Scramble (Tomy)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, tcaveman, 0,        0, tcaveman, tcaveman, driver_device, 0, "Tomy", "Caveman (Tomy)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, alnchase, 0,        0, alnchase, alnchase, driver_device, 0, "Tomy", "Alien Chase", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
