// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  NEC uCOM4 MCU tabletops/handhelds or other simple devices.


  known chips:

  serial  device   etc.
----------------------------------------------------------------
 @031     uPD553C  1979, Bambino Superstar Football (ET-03)
 *042     uPD552C  1979, Tomy Space Attack
 @048     uPD552C  1980, Tomy Tennis (TN-04)
 @049     uPD553C  1979, Mego Mini-Vid Break Free
 @055     uPD553C  1980, Bambino Laser Fight (ET-12)
 *085     uPD650C  1980, Roland TR-808
 *102     uPD553C  1981, Bandai Block Out
 *127     uPD650C  198?  Sony OA-S1100 Typecorder (subcpu, have dump)
 *128     uPD650C  1982, Roland TR-606
  133     uPD650C  1982, Roland TB-303 -> tb303.c
 @160     uPD553C  1982, Tomy Pac Man (TN-08)
 @192     uPD553C  1982, Tomy Scramble (TN-10)
 @202     uPD553C  1982, Epoch Astro Command
 @206     uPD553C  1982, Epoch Dracula
 @209     uPD553C  1982, Tomy Caveman (TN-12)
 @258     uPD553C  1984, Tomy Alien Chase (TN-16)

  (* denotes not yet emulated by MESS, @ denotes it's in this driver)

***************************************************************************/




/***************************************************************************

  Mego Mini-Vid Break Free (manufactured in Japan)
  * PCB label Mego 79 rev F
  * NEC uCOM-43 MCU, labeled D553C 031
  * cyan VFD display Futaba DM-4.5 91

  NOTE!: MESS external artwork is recommended

***************************************************************************/


/***************************************************************************

  Tomy(tronic) Caveman (manufactured in Japan)
  * PCBs are labeled TN-12 2E114E03
  * NEC uCOM-43 MCU, labeled D553C 209
  * cyan/red/green VFD display NEC FIP8AM20T no. 2-42

  NOTE!: MESS external artwork is recommended

***************************************************************************/


/***************************************************************************

  Tomy(tronic) Scramble (manufactured in Japan)
  * PCBs are labeled TN-10 2E114E01
  * NEC uCOM-43 MCU, labeled D553C 192
  * cyan/red/green VFD display NEC FIP10CM20T no. 2-41

  NOTE!: MESS external artwork is recommended

***************************************************************************/


#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "hh_ucom4_test.lh" // test-layout - use external artwork


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
	optional_ioport_array<5> m_inp_matrix; // max 5
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT8 m_port[9];                    // MCU port A-I write data (optional)
	UINT16 m_inp_mux;                   // multiplexed inputs mask

	UINT8 read_inputs(int columns);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 31 for now)

	UINT32 m_grid;                      // VFD current row data
	UINT32 m_plate;                     // VFD current column data

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

  Bambino Superstar Football (manufactured in Japan)
  * PCB label Emix Corp. ET-03
  * NEC uCOM-43 MCU, labeled D553C 031
  * green VFD display Emix-102

  Press the Kick button to start the game, an automatic sequence follows.
  Then choose a formation(A,B,C) and either pass the ball, and/or start
  running. For more information, refer to the official manual.

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class ssfball_state : public hh_ucom4_state
{
public:
	ssfball_state(const machine_config &mconfig, device_type type, const char *tag)
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

static INPUT_PORTS_START( ssfball )
	PORT_START("IN.0") // F3 port B3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Formation A")

	PORT_START("IN.1") // G3 port B3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.2") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Kick/Display")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Formation C")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Formation B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) // multiplexed, handled in ssfball_input_b_r

	PORT_START("IN.3") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pass")
INPUT_PORTS_END


static const INT16 ssfball_speaker_levels[] = { 0, 32767, -32768, 0 };

static MACHINE_CONFIG_START( ssfball, ssfball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, ssfball_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bambino Space Laser Fight (manufactured in Japan)
  * PCB label Emix Corp. ET-12
  * NEC uCOM-43 MCU, labeled D553C 055
  * cyan VFD display Emix-104, with color overlay (blue or green overlay, depending on region)

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class splasfgt_state : public hh_ucom4_state
{
public:
	splasfgt_state(const machine_config &mconfig, device_type type, const char *tag)
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
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, splasfgt_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Epoch Astro Command (manufactured in Japan)
  * PCB label 96111
  * NEC uCOM-43 MCU, labeled D553C 202
  * cyan/red VFD display NEC FIP9AM20T no. 42-42, with color overlay (FIP=fluorescent indicator panel)

  known releases:
  - Japan: Astro Command
  - USA: Astro Command, published by Tandy
  - UK: Scramble, published by Grandstand

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class astrocmd_state : public hh_ucom4_state
{
public:
	astrocmd_state(const machine_config &mconfig, device_type type, const char *tag)
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

	/* no video! */

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

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class edracula_state : public hh_ucom4_state
{
public:
	edracula_state(const machine_config &mconfig, device_type type, const char *tag)
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Tennis (manufactured in Japan)
  * PCB labeled TOMY TN-04 TENNIS
  * NEC uCOM-44 MCU, labeled D552C 048
  * VFD display NEC FIP11AM15T tube no. 0F

  The initial release of this game was in 1979, known as Pro-Tennis,
  it has a D553 instead of D552, with just a little over 50% ROM used.

  This is an early VFD simple electronic tennis game. Each player has six possible
  positions where to hit the ball. A backdrop behind the VFD shows a tennis court.

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class tmtennis_state : public hh_ucom4_state
{
public:
	tmtennis_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(port_e_w);
	DECLARE_READ8_MEMBER(input_r);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(difficulty_switch);

protected:
	virtual void machine_reset();
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
	if (offset == NEC_UCOM4_PORTF) offset--;
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
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

	/* no video! */

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

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class tmpacman_state : public hh_ucom4_state
{
public:
	tmpacman_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
};

// handlers

void tmpacman_state::prepare_display()
{
	UINT16 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,16,17,18,11,10,9,8,0,2,3,1,4,5,6,7,12,13,14,15);
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_16WAY // separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_16WAY // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_16WAY // "

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

	/* no video! */

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

  This is a space-themed tabletop VFD electronic game. To start, simply
  press [UP]. Hold a joystick direction to move around.

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class alnchase_state : public hh_ucom4_state
{
public:
	alnchase_state(const machine_config &mconfig, device_type type, const char *tag)
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
	// A: buttons
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ssfball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-031", 0x0000, 0x0800, CRC(ff5d91d0) SHA1(9b2c0ae45f1e3535108ee5fef8a9010e00c8d5c3) )
ROM_END


ROM_START( splasfgt )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-055", 0x0000, 0x0800, CRC(eb471fbd) SHA1(f06cfe567bf6f9ed4dcdc88acdcfad50cd370a02) )
ROM_END


ROM_START( astrocmd )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-202.s01", 0x0000, 0x0800, CRC(b4b34883) SHA1(6246d561c2df1f2124575d2ca671ef85b1819edd) )
ROM_END


ROM_START( edracula )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-206.s01", 0x0000, 0x0800, CRC(b524857b) SHA1(c1c89ed5dd4bb1e6e98462dc8fa5af2aa48d8ede) )
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



/*    YEAR  NAME       PARENT COMPAT MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, ssfball,   0,        0, ssfball,  ssfball,  driver_device, 0, "Bambino", "Superstar Football", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1980, splasfgt,  0,        0, splasfgt, splasfgt, driver_device, 0, "Bambino", "Space Laser Fight", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1982, astrocmd,  0,        0, astrocmd, astrocmd, driver_device, 0, "Epoch", "Astro Command", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1982, edracula,  0,        0, edracula, edracula, driver_device, 0, "Epoch", "Dracula (Epoch)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1980, tmtennis,  0,        0, tmtennis, tmtennis, driver_device, 0, "Tomy", "Tennis (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1982, tmpacman,  0,        0, tmpacman, tmpacman, driver_device, 0, "Tomy", "Pac Man (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1984, alnchase,  0,        0, alnchase, alnchase, driver_device, 0, "Tomy", "Alien Chase", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
