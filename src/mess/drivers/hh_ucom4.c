// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  NEC uCOM4 MCU handhelds



  serial  device  etc
------------------------------------------
 @048     uPD552  1980, Tomy Tennis
 *085     uPD650  1980, Roland TR-808
  102     uPD553  1981, Bandai Block Out
 *128     uPD650  1982, Roland TR-606
  133     uPD650  1982, Roland TB-303
 @160     uPD553  1982, Tomy Pac Man
 @206     uPD553  1982, Epoch Dracula
 @258     uPD553  1984, Tomy Alien Chase

  (* denotes not yet emulated by MESS, @ denotes it's in this driver)

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

// test-layouts - use external artwork
#include "alnchase.lh"
#include "edracula.lh"
#include "tmpacman.lh"
#include "tmtennis.lh"


class hh_ucom4_state : public driver_device
{
public:
	hh_ucom4_state(const machine_config &mconfig, device_type type, const char *tag)
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
	optional_ioport_array<3> m_inp_matrix; // max 3
	optional_device<speaker_sound_device> m_speaker;
	
	// misc common
	UINT16 m_inp_mux;

	UINT8 read_inputs(int columns);

	virtual void machine_start();

	// display common
	int m_display_wait;
	int m_display_maxy;
	int m_display_maxx;
	
	UINT32 m_grid;
	UINT32 m_plate;
	
	UINT32 m_display_state[0x20];
	UINT32 m_display_cache[0x20];
	UINT8 m_display_decay[0x20][0x20];
	UINT16 m_7seg_mask[0x20];

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety);

	// game-specific handlers
	DECLARE_WRITE8_MEMBER(edracula_grid_w);
	DECLARE_WRITE8_MEMBER(edracula_plate_w);
	DECLARE_WRITE8_MEMBER(edracula_port_i_w);
	
	DECLARE_READ8_MEMBER(tmtennis_input_r);
	DECLARE_WRITE8_MEMBER(tmtennis_grid_w);
	DECLARE_WRITE8_MEMBER(tmtennis_plate_w);
	DECLARE_WRITE8_MEMBER(tmtennis_port_e_w);
	void tmtennis_set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(tmtennis_difficulty_switch);
	DECLARE_MACHINE_RESET(tmtennis);
	
	DECLARE_READ8_MEMBER(alnchase_input_r);
	DECLARE_WRITE8_MEMBER(alnchase_display_w);
	DECLARE_WRITE8_MEMBER(alnchase_port_e_w);
};


void hh_ucom4_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, 0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_7seg_mask, 0, sizeof(m_7seg_mask));
	
	m_inp_mux = 0;
	m_grid = 0;
	m_plate = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_cache));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_7seg_mask));

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}



/***************************************************************************

  Helper Functions

***************************************************************************/



// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.


void hh_ucom4_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x < m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			int ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_7seg_mask[y] != 0)
				output_set_digit_value(y, active_state[y] & m_7seg_mask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x < m_display_maxx; x++)
				output_set_lamp_value(y * mul + x, active_state[y] >> x & 1);
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_ucom4_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x < m_display_maxx; x++)
			if (!(m_display_state[y] >> x & 1) && m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;
	
	display_update();
}

void hh_ucom4_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;

	// update current state
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? setx : 0;
	
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

  Minidrivers (I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Epoch Dracula (manufactured in Japan)
  * PCB label 96121
  * NEC uCOM-43 MCU, labeled D553C 206
  * cyan/red/green VFD display NEC FIP8BM20T (FIP=fluorescent indicator panel)

  known releases:
  - Japan: Dracula House, yellow case
  - USA: Dracula, red case
  - Other: Dracula, yellow case, published by Hales

  NOTE!: MESS external artwork is recommended


***************************************************************************/

WRITE8_MEMBER(hh_ucom4_state::edracula_grid_w)
{
	// port C/D: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

	display_matrix(18, 8, m_plate, m_grid);
}

WRITE8_MEMBER(hh_ucom4_state::edracula_plate_w)
{
	// port E/F/G/H/I01: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	display_matrix(18, 8, m_plate, m_grid);
}

WRITE8_MEMBER(hh_ucom4_state::edracula_port_i_w)
{
	edracula_plate_w(space, offset, data & 3);

	// I2: speaker out
	m_speaker->level_w(data >> 2 & 1);
}


static INPUT_PORTS_START( edracula )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END


static MACHINE_CONFIG_START( edracula, hh_ucom4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN.0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(hh_ucom4_state, edracula_grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(hh_ucom4_state, edracula_grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(hh_ucom4_state, edracula_plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(hh_ucom4_state, edracula_plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(hh_ucom4_state, edracula_plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(hh_ucom4_state, edracula_plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(hh_ucom4_state, edracula_port_i_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_edracula)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END




/***************************************************************************

  Tomy(tronic) Tennis (manufactured in Japan)
  * board labeled TOMY TN-04 TENNIS
  * NEC uCOM-44 MCU, labeled D552C 048
  * VFD display NEC FIP11AM15T

  The initial release of this game was in 1979, known as Pro-Tennis,
  it has a D553 instead of D552, with just a little over 50% ROM used.

  This is an early VFD simple electronic tennis game. Player 1 is on the right
  side, player 2 or CPU on the left. Each player has six possible positions
  where to hit the ball. A backdrop behind the VFD shows a tennis court.

  NOTE!: MESS external artwork is recommended

***************************************************************************/


READ8_MEMBER(hh_ucom4_state::tmtennis_input_r)
{
	// port A/B: buttons
	return ~read_inputs(2) >> (offset*4);
}

WRITE8_MEMBER(hh_ucom4_state::tmtennis_port_e_w)
{
	// E0/E1: input mux
	// E2: speaker out
	// E3: N/C
	m_inp_mux = data & 3;
	m_speaker->level_w(data >> 2 & 1);
}

WRITE8_MEMBER(hh_ucom4_state::tmtennis_plate_w)
{
	// port C/D/F: vfd matrix plate
	if (offset == NEC_UCOM4_PORTF) offset--;
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	display_matrix(12, 12, m_plate, m_grid);
}

WRITE8_MEMBER(hh_ucom4_state::tmtennis_grid_w)
{
	// port G/H/I: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTG) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

	display_matrix(12, 12, m_plate, m_grid);
}



/* Pro-Tennis physical button layout and labels is like this:

    [SERVE] [1] [2] [3]       [3] [2] [1] [SERVE]
            [4] [5] [6]       [6] [5] [4]

    PRACTICE<--PRO1-->PRO2    1PLAYER<--OFF-->2PLAYER
*/

static INPUT_PORTS_START( tmtennis )
	PORT_START("IN.0") // E0 port A/B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Serve")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Serve")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 )

	PORT_START("IN.1") // E1 port A/B
	PORT_CONFNAME( 0x101, 0x100, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_ucom4_state, tmtennis_difficulty_switch, NULL)
	PORT_CONFSETTING(     0x001, "Practice" )
	PORT_CONFSETTING(     0x100, "Pro 1" ) // -> tmtennis_difficulty_switch
	PORT_CONFSETTING(     0x000, "Pro 2" )
	PORT_CONFNAME( 0x02, 0x00, "Players" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
INPUT_PORTS_END

void hh_ucom4_state::tmtennis_set_clock()
{
	// MCU clock is from an LC circuit oscillating by default at ~360kHz,
	// but on PRO1, the difficulty switch puts a capacitor across the LC circuit
	// to slow it down to ~260kHz.
	m_maincpu->set_unscaled_clock(m_inp_matrix[1]->read() & 0x100 ? 260000 : 360000);
}

INPUT_CHANGED_MEMBER(hh_ucom4_state::tmtennis_difficulty_switch)
{
	tmtennis_set_clock();
}

MACHINE_RESET_MEMBER(hh_ucom4_state, tmtennis)
{
	tmtennis_set_clock();
}


static MACHINE_CONFIG_START( tmtennis, hh_ucom4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, 360000) // see tmtennis_set_clock
	MCFG_UCOM4_READ_A_CB(READ8(hh_ucom4_state, tmtennis_input_r))
	MCFG_UCOM4_READ_B_CB(READ8(hh_ucom4_state, tmtennis_input_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(hh_ucom4_state, tmtennis_plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(hh_ucom4_state, tmtennis_plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(hh_ucom4_state, tmtennis_port_e_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(hh_ucom4_state, tmtennis_plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(hh_ucom4_state, tmtennis_grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(hh_ucom4_state, tmtennis_grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(hh_ucom4_state, tmtennis_grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tmtennis)

	MCFG_MACHINE_RESET_OVERRIDE(hh_ucom4_state, tmtennis)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/***************************************************************************

  Tomy(tronic) Pac-Man (manufactured in Japan)
  * boards are labeled TN-08 2E108E01
  * NEC uCOM-43 MCU, labeled D553C 160
  * cyan/red/green VFD display NEC FIP8AM18T
  * bright yellow round casing

  known releases:
  - Japan: Puck Man
  - USA: Pac Man
  - UK: Puckman (Tomy), and also as Munchman, published by Grandstand
  - Australia: Pac Man-1, published by Futuretronics

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( tmpacman )
INPUT_PORTS_END


static MACHINE_CONFIG_START( tmpacman, hh_ucom4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_430kHz)

	MCFG_DEFAULT_LAYOUT(layout_tmpacman)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Tomy Alien Chase (manufactured in Japan)
  * boards are labeled TN-16 2E121B01
  * NEC uCOM-43 MCU, labeled D553C 258
  * red/green VFD display NEC FIP9AM24T with color overlay, 2-sided*

  *Player one views the VFD from the front (grid+filament side) while the
  opposite player views it from the back side (through the conductive traces),
  basically a mirror-image.

  This is a space-themed tabletop VFD electronic game. To start, simply
  press [UP]. Hold a joystick direction to move around.

  NOTE!: MESS external artwork is recommended

***************************************************************************/



READ8_MEMBER(hh_ucom4_state::alnchase_input_r)
{
	// port A: buttons
	return read_inputs(2);
}

WRITE8_MEMBER(hh_ucom4_state::alnchase_display_w)
{
	if (offset <= NEC_UCOM4_PORTE)
	{
		// C/D/E0: vfd matrix grid
		int shift = (offset - NEC_UCOM4_PORTC) * 4;
		m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

		// C0(grid 0): input enable PL1
		// D0(grid 4): input enable PL2
		m_inp_mux = (m_grid & 1) | (m_grid >> 3 & 2);
	}

	if (offset >= NEC_UCOM4_PORTE)
	{
		// E23/F/G/H/I: vfd matrix plate
		int shift = (offset - NEC_UCOM4_PORTE) * 4;
		m_plate = ((m_plate << 2 & ~(0xf << shift)) | (data << shift)) >> 2;
	}

	display_matrix(17, 9, m_plate, m_grid);
}

WRITE8_MEMBER(hh_ucom4_state::alnchase_port_e_w)
{
	alnchase_display_w(space, offset, data);

	// E1: speaker out
	m_speaker->level_w(data >> 1 & 1);
}

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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("IN.2") // port B
	PORT_CONFNAME( 0x01, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x02, "Professional" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( alnchase, hh_ucom4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(READ8(hh_ucom4_state, alnchase_input_r))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN.2"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(hh_ucom4_state, alnchase_display_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(hh_ucom4_state, alnchase_display_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(hh_ucom4_state, alnchase_port_e_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(hh_ucom4_state, alnchase_display_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(hh_ucom4_state, alnchase_display_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(hh_ucom4_state, alnchase_display_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(hh_ucom4_state, alnchase_display_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_alnchase)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( edracula )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-206", 0x0000, 0x0800, CRC(b524857b) SHA1(c1c89ed5dd4bb1e6e98462dc8fa5af2aa48d8ede) )
ROM_END


ROM_START( tmtennis )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-048", 0x0000, 0x0400, CRC(78702003) SHA1(4d427d4dbeed901770c682338867f58c7b54eee3) )
ROM_END



ROM_START( tmpacman )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-160", 0x0000, 0x0800, CRC(b21a8af7) SHA1(e3122be1873ce76a4067386bf250802776f0c2f9) )
ROM_END


ROM_START( alnchase )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-258", 0x0000, 0x0800, CRC(c5284ff5) SHA1(6a20aaacc9748f0e0335958f3cea482e36153704) )
ROM_END



CONS( 1982, edracula, 0, 0, edracula, edracula, driver_device, 0, "Epoch", "Dracula (Epoch)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1980, tmtennis, 0, 0, tmtennis, tmtennis, driver_device, 0, "Tomy", "Tennis (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1982, tmpacman, 0, 0, tmpacman, tmpacman, driver_device, 0, "Tomy", "Pac Man (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1984, alnchase, 0, 0, alnchase, alnchase, driver_device, 0, "Tomy", "Alien Chase", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
