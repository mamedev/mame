// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Hitachi HMCS40 MCU tabletops/handhelds or other simple devices,
  most of them are VFD electronic games/toys.

  known chips:

  serial  device    etc.
----------------------------------------------------------------
 @07      HD38750A  1979, Bambino Knock-Em Out Boxing (ET-06B)
 @08      HD38750A  1979, Bambino Dribble Away Basketball (ET-05)
 @45      HD38750A  1981, VTech Invaders
 *56      HD38750A  1981, Actronics(Hanzawa) Twinvader (small brown version)
 *58      HD38750A  1981, Actronics(Hanzawa) Challenge Racer/Ludotronic(Hanzawa) Grand Prix Turbo
 *62      HD38750A  1982, Actronics(Hanzawa) Pack'n Maze

 @04      HD38800A  1980, Gakken Heiankyo Alien
 @25      HD38800A  1981, Coleco Alien Attack
 @27      HD38800A  1981, Bandai Packri Monster
 *31      HD38800A  1981, Entex Select-a-Game cartridge: Space Invader 2 (have dump)
 *37      HD38800A  1981, Entex Select-a-Game cartridge: Baseball 4 (have dump)
 *38      HD38800A  1981, Entex Select-a-Game cartridge: Pinball (have dump)
 *41      HD38800A  1982, Gakken Puck Monster
 *51      HD38800A  1981, Actronics(Hanzawa) Twinvader (larger white version)
 @70      HD38800A  1982, Coleco Galaxian
 @73      HD38800A  1982, Bandai(Mattel) Star Hawk (PT-317B)
 @77      HD38800A  1982, Bandai Frisky Tom (PT-327A)
 @88      HD38800A  1984, Tomy Tron (THN-02)

 @01      HD38800B  1982, Gakken Crazy Kong
 @19      HD38800B  1982, Bandai Zaxxon
 @23      HD38800B  1982, Tomy Kingman (THF-01II)
 *24      HD38800B  1982, Actronics(Hanzawa) Wanted G-Man
 *29      HD38800B  1984, Tomy Portable 6000 Bombman
 *35      HD38800B  1983, Bandai Gundam vs Gelgoog Zaku
 @43      HD38800B  1983, Bandai Dokodemo Dorayaki Doraemon (PT-412)
 @52      HD38800B  1983, Bandai Ultra Man (PT-424)

 @09      HD38820A  1980, Mattel World Championship Baseball
 @13      HD38820A  1981, Entex Galaxian 2
 @23      HD38820A  1981, Entex Pac Man 2
 @28      HD38820A  1981, Coleco Pac-Man (ver 1)
 @29      HD38820A  1981, Coleco Pac-Man (ver 2)
 *32      HD38820A  198?, Gakken Super Cobra
 *38      HD38820A  1982, Entex Crazy Climber
 @42      HD38820A  1982, Entex Stargate
 @43      HD38820A  1982, Entex Turtles
 @45      HD38820A  1982, Coleco Donkey Kong
 @49      HD38820A  1983, Bandai Zackman
 @61      HD38820A  1983, Coleco Ms. Pac-Man
 @63      HD38820A  1983, Bandai Pengo
 @65      HD38820A  1983, Bandai Burger Time (PT-389)
 @69      HD38820A  1983, Gakken Dig Dug
 @70      HD38820A  1983, Parker Brothers Q*Bert
 @85      HD38820A  1984, Bandai Machine Man (PT-438)
 @88      HD38820A  1984, Bandai Pair Match (PT-460) (1/2)
 @89      HD38820A  1984, Bandai Pair Match (PT-460) (2/2)

 *75      HD44801A  1982, Alpha 8201 protection MCU (have dump)

 *35      HD44801B  1983, Alpha 8302 protection MCU (have dump)
 *42      HD44801B  1984, Alpha 8303 protection MCU (have dump)

  (* denotes not yet emulated by MAME, @ denotes it's in this driver)


  TODO:
  - cdkong discrete sound (simple volume decay, simulated for now)
  - cgalaxn discrete sound (alien attacking sound effect)
  - gckong random lockups (tap the jump button repeatedly): mcu stack overflow,
    works ok if stack levels is increased, 38800 B rev. has more stack levels?
    Or it could be a race condition: irq happening too late/early.
  - epacman booting the game in demo mode, pacman should go straight to the
    upper-left power pill: mcu cycle/interrupt timing related
  - Though very uncommon when compared to games with LED/lamp display,
    some games may manipulate VFD plate brightness by strobing it longer,
    eg. cgalaxn when the player ship explodes.

***************************************************************************/

#include "emu.h"
#include "cpu/hmcs40/hmcs40.h"
#include "cpu/cop400/cop400.h"
#include "sound/speaker.h"

// internal artwork
#include "pairmtch.lh"

#include "hh_hmcs40_test.lh" // common test-layout - use external artwork


class hh_hmcs40_state : public driver_device
{
public:
	hh_hmcs40_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT8 m_r[8];                       // MCU R ports write data (optional)
	UINT16 m_d;                         // MCU D port write data (optional)
	UINT8 m_int[2];                     // MCU INT0/1 pins state
	UINT16 m_inp_mux;                   // multiplexed inputs mask

	UINT16 read_inputs(int columns);
	void refresh_interrupts(void);
	void set_interrupt(int line, int state);
	DECLARE_INPUT_CHANGED_MEMBER(single_interrupt_line);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 47 for now)

	UINT32 m_grid;                      // VFD current row data
	UINT64 m_plate;                     // VFD current column data

	UINT64 m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	UINT16 m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	UINT64 m_display_cache[0x20];       // (internal use)
	UINT8 m_display_decay[0x20][0x40];  // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, UINT64 setx, UINT32 sety);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_hmcs40_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	memset(m_r, 0, sizeof(m_r));
	memset(m_int, 0, sizeof(m_int));
	m_d = 0;
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

	save_item(NAME(m_r));
	save_item(NAME(m_int));
	save_item(NAME(m_d));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_hmcs40_state::machine_reset()
{
	refresh_interrupts();
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_hmcs40_state::display_update()
{
	UINT64 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			UINT64 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
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

TIMER_DEVICE_CALLBACK_MEMBER(hh_hmcs40_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_hmcs40_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_hmcs40_state::display_matrix(int maxx, int maxy, UINT64 setx, UINT32 sety)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT64 mask = (U64(1) << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (U64(1) << maxx)) : 0;

	display_update();
}


UINT16 hh_hmcs40_state::read_inputs(int columns)
{
	UINT16 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}


// interrupt handling

void hh_hmcs40_state::refresh_interrupts()
{
	for (int i = 0; i < 2; i++)
		m_maincpu->set_input_line(i, m_int[i] ? ASSERT_LINE : CLEAR_LINE);
}

void hh_hmcs40_state::set_interrupt(int line, int state)
{
	line = line ? 1 : 0;
	state = state ? 1 : 0;

	if (state != m_int[line])
	{
		if (machine().phase() >= MACHINE_PHASE_RESET)
			m_maincpu->set_input_line(line, state ? ASSERT_LINE : CLEAR_LINE);
		m_int[line] = state;
	}
}

INPUT_CHANGED_MEMBER(hh_hmcs40_state::single_interrupt_line)
{
	set_interrupt((int)(FPTR)param, newval);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Bambino Dribble Away Basketball (manufactured in Japan)
  * PCBs are labeled Emix Corp. ET-05
  * Hitachi HD38750A08 MCU
  * green VFD display Emix-106, with bezel overlay

  NOTE!: MAME external artwork is required

***************************************************************************/

class bambball_state : public hh_hmcs40_state
{
public:
	bambball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(bambball_state::plate_w)
{
	// R1x-R3x(,D0-D3): vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 plate = BITSWAP16(m_plate,13,8,4,12,9,10,14,1,7,0,15,11,6,3,5,2);
	display_matrix(16, 9, plate, m_grid);
}

WRITE16_MEMBER(bambball_state::grid_w)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D7-D10: input mux
	m_inp_mux = data >> 7 & 0xf;

	// D7-D15: vfd matrix grid
	m_grid = data >> 7 & 0x1ff;

	// D0-D3: more plates (update display there)
	plate_w(space, 3 + HMCS40_PORT_R1X, data & 0xf);
}

READ8_MEMBER(bambball_state::input_r)
{
	// R0x: multiplexed inputs
	return read_inputs(4);
}


// config

static INPUT_PORTS_START( bambball )
	PORT_START("IN.0") // D7 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Dribble Low")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Dribble Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Dribble High")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Shoot")

	PORT_START("IN.1") // D8 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // D9 port R0x
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Display")

	PORT_START("IN.3") // D10 port R0x
	PORT_CONFNAME( 0x07, 0x01, "Skill Level")
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFNAME( 0x08, 0x08, "Players" )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bambball, bambball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38750, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, READ8(bambball_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bambball_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bambball_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bambball_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bambball_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bambino Knock-Em Out Boxing
  * PCBs are labeled Emix Corp. ET-06B
  * Hitachi HD38750A07 MCU
  * cyan VFD display Emix-103, with blue or green color overlay

  NOTE!: MAME external artwork is required

***************************************************************************/

class bmboxing_state : public hh_hmcs40_state
{
public:
	bmboxing_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

void bmboxing_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,0,1,2,3,4,5,6,7,8);
	UINT32 plate = BITSWAP16(m_plate,15,14,13,12,1,2,0,3,11,4,10,7,5,6,9,8);
	display_matrix(12, 9, plate, grid);
}

WRITE8_MEMBER(bmboxing_state::plate_w)
{
	// R1x-R3x: vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(bmboxing_state::grid_w)
{
	// D13: speaker out
	m_speaker->level_w(data >> 13 & 1);

	// D9-D12: input mux
	m_inp_mux = data >> 9 & 0xf;

	// D4-D12: vfd matrix grid
	m_grid = data >> 4 & 0x1ff;
	prepare_display();
}

READ8_MEMBER(bmboxing_state::input_r)
{
	// R0x: multiplexed inputs
	return read_inputs(4);
}


// config

/* physical button layout and labels is like this:

    * left = P2 side *                                       * right = P1 side *

    [ BACK ]  [ HIGH ]        (players sw)                   [ HIGH ]  [ BACK ]
                              1<--->2         [START/
    [NORMAL]  [MEDIUM]                         RESET]        [MEDIUM]  [NORMAL]
                              1<---OFF--->2
    [ DUCK ]  [ LOW  ]        (skill lvl sw)                 [ LOW  ]  [ DUCK ]
*/

static INPUT_PORTS_START( bmboxing )
	PORT_START("IN.0") // D9 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P1 Punch High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 Punch Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("P1 Punch Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // D10 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P1 Position Normal")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 Position Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P1 Position Ducking")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // D11 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("P2 Punch High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 Punch Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("P2 Punch Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // D12 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 Position Normal")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("P2 Position Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("P2 Position Ducking")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // port D
	PORT_CONFNAME( 0x0001, 0x0000, "Players" )
	PORT_CONFSETTING(      0x0000, "1" )
	PORT_CONFSETTING(      0x0001, "2" )
	PORT_CONFNAME( 0x0002, 0x0000, "Skill Level" )
	PORT_CONFSETTING(      0x0000, "1" )
	PORT_CONFSETTING(      0x0002, "2" )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bmboxing, bmboxing_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38750, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, READ8(bmboxing_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bmboxing_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bmboxing_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bmboxing_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bmboxing_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.4"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Frisky Tom (manufactured in Japan)
  * PCBs are labeled Kaken Corp., PT-327A
  * Hitachi HD38800A77 MCU
  * cyan/red/green VFD display Futaba DM-43ZK 2E

  NOTE!: MAME external artwork is required

***************************************************************************/

class bfriskyt_state : public hh_hmcs40_state
{
public:
	bfriskyt_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int1();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

void bfriskyt_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,8,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP24(m_plate,23,22,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21);
	display_matrix(22, 8, plate, grid);
}

WRITE8_MEMBER(bfriskyt_state::plate_w)
{
	// R0x-R3x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(bfriskyt_state::grid_w)
{
	// D6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// D11-D15: input mux
	UINT8 inp_mux = data >> 11 & 0x1f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D8-D15: vfd matrix grid
	m_grid = data >> 8 & 0xff;

	// D0-D5: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x3f0000);
	prepare_display();
}

void bfriskyt_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(5));
}


// config

static INPUT_PORTS_START( bfriskyt )
	PORT_START("IN.0") // D11 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, NULL)

	PORT_START("IN.1") // D12 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, NULL)

	PORT_START("IN.2") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, NULL)

	PORT_START("IN.3") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, NULL)

	PORT_START("IN.4") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, NULL)

	PORT_START("IN.5") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(bfriskyt_state::input_changed)
{
	update_int1();
}


static MACHINE_CONFIG_START( bfriskyt, bfriskyt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(bfriskyt_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bfriskyt_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bfriskyt_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bfriskyt_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bfriskyt_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Packri Monster (manufactured in Japan)
  * PCB label DM-21ZA2
  * Hitachi HD38800A27 MCU
  * cyan/red/green VFD display Futaba DM-21ZK 2B, with bezel overlay

  known releases:
  - Japan: FL Packri Monster
  - USA(World?): Packri Monster
  - USA/Canada: Hungry Monster, published by Tandy
  - other: Gobble Man/Ogre Monster, published by Tandy

  NOTE!: MAME external artwork is required

***************************************************************************/

class packmon_state : public hh_hmcs40_state
{
public:
	packmon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ16_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(packmon_state::plate_w)
{
	// R0x-R3x(,D0-D3): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,0,1,2,3,4,5,6,19,18,17,16,15,14,13,12,11,10,9,8,7);
	display_matrix(20, 10, plate, grid);
}

WRITE16_MEMBER(packmon_state::grid_w)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D11-D15: input mux
	m_inp_mux = data >> 11 & 0x1f;

	// D6-D15: vfd matrix grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D3: plate 9-12 (update display there)
	plate_w(space, 4, data & 0xf);
}

READ16_MEMBER(packmon_state::input_r)
{
	// D5: multiplexed inputs
	return read_inputs(5) & 0x20;
}


// config

static INPUT_PORTS_START( packmon )
	PORT_START("IN.0") // D11 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START )

	PORT_START("IN.1") // D12 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D13 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.3") // D14 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.4") // D15 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
INPUT_PORTS_END

static MACHINE_CONFIG_START( packmon, packmon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(packmon_state, grid_w))
	MCFG_HMCS40_READ_D_CB(READ16(packmon_state, input_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai/Mattel Star Hawk (manufactured in Japan)
  * PCBs are labeled Kaken, PT-317B
  * Hitachi HD38800A73 MCU
  * cyan/red VFD display Futaba DM-41ZK, with partial color overlay + bezel

  Kaken was a subsidiary of Bandai. The original Japanese release is unknown,
  was it canceled and only released in the USA?

  known releases:
  - Japan: ?
  - USA: Star Hawk, published by Mattel

  NOTE!: MAME external artwork is required

***************************************************************************/

class msthawk_state : public hh_hmcs40_state
{
public:
	msthawk_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int0();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

void msthawk_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,19,20,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
	display_matrix(21, 10, plate, grid);
}

WRITE8_MEMBER(msthawk_state::plate_w)
{
	// R0x-R3x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(msthawk_state::grid_w)
{
	// D5: speaker out
	m_speaker->level_w(data >> 5 & 1);

	// D10-D15: input mux
	UINT8 inp_mux = data >> 10 & 0x3f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D6-D15: vfd matrix grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D4: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x1f0000);
	prepare_display();
}

void msthawk_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(6));
}


// config

static INPUT_PORTS_START( msthawk )
	PORT_START("IN.0") // D10 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, NULL) PORT_NAME("Score")

	PORT_START("IN.1") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, NULL) PORT_NAME("Land")

	PORT_START("IN.2") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, NULL)

	PORT_START("IN.3") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, NULL)

	PORT_START("IN.4") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, NULL)

	PORT_START("IN.5") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, NULL)

	PORT_START("IN.6") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1) PORT_NAME("Fire")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(msthawk_state::input_changed)
{
	update_int0();
}


static MACHINE_CONFIG_START( msthawk, msthawk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(msthawk_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(msthawk_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(msthawk_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(msthawk_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(msthawk_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Zaxxon (manufactured in Japan, licensed from Sega)
  * PCB label FL Zaxxon
  * Hitachi HD38800B19 MCU
  * cyan/red/blue VFD display NEC FIP11BM24T no. 4-8, half of it reflected
    with a one-way mirror to give the illusion of a 3D display

  NOTE!: MAME external artwork is required

***************************************************************************/

class bzaxxon_state : public hh_hmcs40_state
{
public:
	bzaxxon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int1();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

WRITE8_MEMBER(bzaxxon_state::plate_w)
{
	// R0x-R3x(,D0-D2): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,6,7,8,9,10,5,4,3,2,1,0);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,5,7,0,1,2,3,4,6,19,16,17,18,15,14,13,12,10,8,9,11) | 0x800;
	display_matrix(20, 11, plate, grid);
}

WRITE16_MEMBER(bzaxxon_state::grid_w)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D7-D10: input mux
	UINT8 inp_mux = data >> 7 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D5-D15: vfd matrix grid
	m_grid = data >> 5 & 0x7ff;

	// D0-D2: plate 7-9 (update display there)
	plate_w(space, 4, data & 7);
}

void bzaxxon_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(4));
}


// config

static INPUT_PORTS_START( bzaxxon )
	PORT_START("IN.0") // D7 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, NULL)

	PORT_START("IN.1") // D8 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, NULL)

	PORT_START("IN.2") // D9 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, NULL)

	PORT_START("IN.3") // D10 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, NULL)

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.5") // port D
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0xfff7, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(bzaxxon_state::input_changed)
{
	update_int1();
}


static MACHINE_CONFIG_START( bzaxxon, bzaxxon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(bzaxxon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bzaxxon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bzaxxon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bzaxxon_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bzaxxon_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.5"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Zackman "The Pit, FL Exploration of Space" (manufactured in Japan)
  * Hitachi QFP HD38820A49 MCU
  * cyan/red/yellow VFD display Futaba DM-53Z 3E, with color overlay

  NOTE!: MAME external artwork is required

***************************************************************************/

class zackman_state : public hh_hmcs40_state
{
public:
	zackman_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int0();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

WRITE8_MEMBER(zackman_state::plate_w)
{
	// R0x-R6x(,D0,D1): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP32(m_plate,31,30,27,0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,29,28,23,22,21,20,19,18,17,16,15,14,13,12);
	display_matrix(29, 8, plate, grid);
}

WRITE16_MEMBER(zackman_state::grid_w)
{
	// D2: speaker out
	m_speaker->level_w(data >> 2 & 1);

	// D11-D14: input mux
	UINT8 inp_mux = data >> 11 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D8-D15: vfd matrix grid
	m_grid = data >> 8 & 0xff;

	// D0,D1: plate 12,13 (update display there)
	plate_w(space, 7, data & 3);
}

void zackman_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(4));
}


// config

static INPUT_PORTS_START( zackman )
	PORT_START("IN.0") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, NULL)

	PORT_START("IN.1") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, NULL)

	PORT_START("IN.2") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, NULL)

	PORT_START("IN.3") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, NULL)

	PORT_START("IN.4") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(zackman_state::input_changed)
{
	update_int0();
}


static MACHINE_CONFIG_START( zackman, zackman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(zackman_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(zackman_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Pengo (manufactured in Japan, licensed from Sega)
  * PCB label FL Pengo(in katakana)
  * Hitachi QFP HD38820A63 MCU
  * cyan/red/blue VFD display Futaba DM-68ZK 3D DM-63

  NOTE!: MAME external artwork is required

***************************************************************************/

class bpengo_state : public hh_hmcs40_state
{
public:
	bpengo_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int0();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

void bpengo_state::prepare_display()
{
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP32(m_plate,31,30,29,28,23,22,21,16,17,18,19,20,27,26,25,24,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
	display_matrix(25, 8, plate, grid);
}

WRITE8_MEMBER(bpengo_state::plate_w)
{
	// R0x-R6x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(bpengo_state::grid_w)
{
	// D10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// D12-D15: input mux
	UINT8 inp_mux = data >> 12 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D0-D7: vfd matrix grid
	m_grid = data & 0xff;
	prepare_display();
}

void bpengo_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(4));
}


// config

static INPUT_PORTS_START( bpengo )
	PORT_START("IN.0") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, NULL)

	PORT_START("IN.1") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, NULL)

	PORT_START("IN.2") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, NULL)

	PORT_START("IN.3") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, NULL)

	PORT_START("IN.4") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)

	PORT_START("IN.5") // port D
	PORT_CONFNAME( 0x0800, 0x0000, "Factory Test" )
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0800, DEF_STR( On ) )
	PORT_BIT( 0xf7ff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(bpengo_state::input_changed)
{
	update_int0();
}


static MACHINE_CONFIG_START( bpengo, bpengo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(bpengo_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bpengo_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.5"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Burger Time (manufactured in Japan, licensed from Data East)
  * PCB label Kaken Corp. PT-389 Burger Time
  * Hitachi QFP HD38820A65 MCU
  * cyan/red/green VFD display NEC FIP6AM25T no. 21-21

  NOTE!: MAME external artwork is required

***************************************************************************/

class bbtime_state : public hh_hmcs40_state
{
public:
	bbtime_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int0();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

void bbtime_state::prepare_display()
{
	UINT8 grid = BITSWAP8(m_grid,7,6,0,1,2,3,4,5);
	UINT32 plate = BITSWAP32(m_plate,31,30,29,28,25,24,26,27,22,23,15,14,12,11,10,8,7,6,4,1,5,9,13,3,2,16,17,18,19,20,0,21) | 0x1;
	display_matrix(28, 6, plate, grid);
}

WRITE8_MEMBER(bbtime_state::plate_w)
{
	// R0x-R6x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(bbtime_state::grid_w)
{
	// D3: speaker out
	m_speaker->level_w(data >> 3 & 1);

	// D10-D14: input mux
	UINT8 inp_mux = data >> 10 & 0x1f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D4-D9: vfd matrix grid
	m_grid = data >> 4 & 0x3f;
	prepare_display();
}

void bbtime_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(5));
}


// config

static INPUT_PORTS_START( bbtime )
	PORT_START("IN.0") // D10 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, NULL)

	PORT_START("IN.1") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, NULL)

	PORT_START("IN.2") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, NULL)

	PORT_START("IN.3") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, NULL)

	PORT_START("IN.4") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, NULL)

	PORT_START("IN.5") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(bbtime_state::input_changed)
{
	update_int0();
}


static MACHINE_CONFIG_START( bbtime, bbtime_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(bbtime_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bbtime_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Dokodemo Dorayaki Doraemon (FL LSI Game Push Up) (manufactured in Japan)
  * PCB label Kaken Corp PT-412 FL-Doreamon(in katakana)
  * Hitachi HD38800B43 MCU
  * cyan/red/blue VFD display Futaba DM-71

  NOTE!: MAME external artwork is required

***************************************************************************/

class bdoramon_state : public hh_hmcs40_state
{
public:
	bdoramon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
};

// handlers

WRITE8_MEMBER(bdoramon_state::plate_w)
{
	// R0x-R3x(,D0-D3): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,7,6);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,11,19,18,17,16,15,14,13,12,10,9,8,7,6,5,4,3,2,1,0);
	display_matrix(19, 8, plate, grid);
}

WRITE16_MEMBER(bdoramon_state::grid_w)
{
	// D7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// D8-D15: vfd matrix grid
	m_grid = data >> 8 & 0xff;

	// D0-D3: plate 15-18 (update display there)
	plate_w(space, 4, data & 0xf);
}


// config

static INPUT_PORTS_START( bdoramon )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.1") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)

	PORT_START("IN.2") // port D
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xff8f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // port R2x
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bdoramon, bdoramon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(bdoramon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bdoramon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bdoramon_state, plate_w))
	MCFG_HMCS40_READ_R_CB(2, IOPORT("IN.3"))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bdoramon_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bdoramon_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.2"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Ultra Man (FL LSI Game Push Up) (manufactured in Japan)
  * PCB label Kaken Corp. PT-424 FL Ultra Man
  * Hitachi HD38800B52 MCU
  * cyan/red/blue VFD display NEC FIP8BM25T no. 21-8 2

  NOTE!: MAME external artwork is required

***************************************************************************/

class bultrman_state : public hh_hmcs40_state
{
public:
	bultrman_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
};

// handlers

WRITE8_MEMBER(bultrman_state::plate_w)
{
	// R0x-R3x(,D0-D2): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,18,17,16,15,14,13,12,11,2,10,9,8,7,6,5,4,3,0,1);
	display_matrix(18, 8, plate, grid);
}

WRITE16_MEMBER(bultrman_state::grid_w)
{
	// D7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// D8-D15: vfd matrix grid
	m_grid = data >> 8 & 0xff;

	// D0-D2: plate 15-17 (update display there)
	plate_w(space, 4, data & 7);
}


// config

static INPUT_PORTS_START( bultrman )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.1") // port D
	PORT_CONFNAME( 0x0010, 0x0000, "Factory Test" )
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0010, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xff8f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bultrman, bultrman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(bultrman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bultrman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bultrman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bultrman_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bultrman_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.1"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Machine Man (FL Flat Type) (manufactured in Japan)
  * PCB label Kaken PT-438
  * Hitachi QFP HD38820A85 MCU
  * cyan/red/green VFD display NEC FIP5CM33T no. 4 21

  NOTE!: MAME external artwork is required

***************************************************************************/

class machiman_state : public hh_hmcs40_state
{
public:
	machiman_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
};

// handlers

void machiman_state::prepare_display()
{
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18);
	display_matrix(19, 5, plate, m_grid);
}

WRITE8_MEMBER(machiman_state::plate_w)
{
	// R0x-R3x,R6012: vfd matrix plate
	int shift = (offset == HMCS40_PORT_R6X) ? 16 : offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(machiman_state::grid_w)
{
	// D13: speaker out
	m_speaker->level_w(data >> 13 & 1);

	// D0-D4: vfd matrix grid
	m_grid = data & 0x1f;
	prepare_display();
}


// config

static INPUT_PORTS_START( machiman )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.1") // port D
	PORT_BIT( 0x3fff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
INPUT_PORTS_END

static MACHINE_CONFIG_START( machiman, machiman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(machiman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(machiman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(machiman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(machiman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(machiman_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(machiman_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.1"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Pair Match (manufactured in Japan)
  * PCB label Kaken Corp. PT-460
  * Hitachi QFP HD38820A88 MCU(main), HD38820A89(audio)
  * cyan/red VFD display

  This is a memory game, the difference is instead of pictures, the player
  needs to match sound effects. It has an extra MCU for sound. The case is
  shaped like a glossy black pyramid. Star Trek fans will recognize it as
  a prop used in TNG Ten Forward.

  note: MAME external artwork is not needed for this game

***************************************************************************/

class pairmtch_state : public hh_hmcs40_state
{
public:
	pairmtch_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);

	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(sound2_w);
	DECLARE_WRITE16_MEMBER(speaker_w);
};

// handlers: maincpu side

WRITE8_MEMBER(pairmtch_state::plate_w)
{
	// R2x,R3x,R6x: vfd matrix plate
	int shift = (offset == HMCS40_PORT_R6X) ? 8 : (offset-2) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	display_matrix(12, 6, m_plate, m_grid);
}

WRITE16_MEMBER(pairmtch_state::grid_w)
{
	// D7: sound reset (to audiocpu reset line)
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	// D9: sound start (to audiocpu INT0)
	m_audiocpu->set_input_line(0, (data & 0x200) ? ASSERT_LINE : CLEAR_LINE);

	// D10,D15: input mux
	m_inp_mux = (data >> 10 & 1) | (data >> 14 & 2);

	// D0-D5: vfd matrix grid
	m_grid = data & 0x3f;
	display_matrix(12, 6, m_plate, m_grid);
}

READ8_MEMBER(pairmtch_state::input_r)
{
	// R4x: multiplexed inputs
	return read_inputs(2);
}

WRITE8_MEMBER(pairmtch_state::sound_w)
{
	// R5x: soundlatch (to audiocpu R2x)
	soundlatch_byte_w(space, 0, BITSWAP8(data,7,6,5,4,0,1,2,3));
}


// handlers: audiocpu side

WRITE8_MEMBER(pairmtch_state::sound2_w)
{
	// R2x: soundlatch (to maincpu R5x)
	soundlatch2_byte_w(space, 0, BITSWAP8(data,7,6,5,4,0,1,2,3));
}

WRITE16_MEMBER(pairmtch_state::speaker_w)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1: sound ack (to maincpu INT0)
	m_maincpu->set_input_line(0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
}


// config

static INPUT_PORTS_START( pairmtch )
	PORT_START("IN.0") // D10 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.1") // D15 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.2") // port D
	PORT_CONFNAME( 0x0040, 0x0000, "Factory Test" )
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0040, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_CONFNAME( 0x0800, 0x0800, "Players" )
	PORT_CONFSETTING(      0x0800, "1" )
	PORT_CONFSETTING(      0x0000, "2" )
	PORT_CONFNAME( 0x3000, 0x2000, "Skill Level" )
	PORT_CONFSETTING(      0x2000, "1" )
	PORT_CONFSETTING(      0x1000, "2" )
	PORT_CONFSETTING(      0x0000, "3" )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x86bf, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( pairmtch, pairmtch_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(pairmtch_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(pairmtch_state, plate_w))
	MCFG_HMCS40_READ_R_CB(4, READ8(pairmtch_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(pairmtch_state, sound_w))
	MCFG_HMCS40_READ_R_CB(5, READ8(driver_device, soundlatch2_byte_r))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(pairmtch_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(pairmtch_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.2"))

	MCFG_CPU_ADD("audiocpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(pairmtch_state, sound2_w))
	MCFG_HMCS40_READ_R_CB(2, READ8(driver_device, soundlatch_byte_r))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(pairmtch_state, speaker_w))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_pairmtch)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Alien Attack (manufactured in Taiwan)
  * Hitachi HD38800A25 MCU
  * cyan/red VFD display Futaba DM-19Z 1J

  It looks like Coleco took Gakken's Heiankyo Alien and turned it into a more
  action-oriented game.

  NOTE!: MAME external artwork is required

***************************************************************************/

class alnattck_state : public hh_hmcs40_state
{
public:
	alnattck_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ16_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(alnattck_state::plate_w)
{
	// R0x-R3x(,D0-D3): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,19,18,17,16,11,9,8,10,7,2,0,1,3,4,5,6,12,13,14,15);
	display_matrix(20, 10, plate, m_grid);
}

WRITE16_MEMBER(alnattck_state::grid_w)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D7-D13: input mux
	m_inp_mux = data >> 7 & 0x7f;

	// D6-D15: vfd matrix grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D3: plate 16-19 (update display there)
	plate_w(space, 4, data & 0xf);
}

READ16_MEMBER(alnattck_state::input_r)
{
	// D5: multiplexed inputs
	return read_inputs(7) & 0x20;
}


// config

static INPUT_PORTS_START( alnattck )
	PORT_START("IN.0") // D7 line D5
	PORT_CONFNAME( 0x20, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x20, "2" )

	PORT_START("IN.1") // D8 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D9 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.3") // D10 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.4") // D11 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN.5") // D12 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Move")

	PORT_START("IN.6") // D13 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Fire")
INPUT_PORTS_END

static MACHINE_CONFIG_START( alnattck, alnattck_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(alnattck_state, grid_w))
	MCFG_HMCS40_READ_D_CB(READ16(alnattck_state, input_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Donkey Kong (manufactured in Taiwan, licensed from Nintendo)
  * PCB label Coleco Rev C 75790 DK
  * Hitachi QFP HD38820A45 MCU
  * cyan/red VFD display Futaba DM-47ZK 2K, with color overlay

  NOTE!: MAME external artwork is required

***************************************************************************/

class cdkong_state : public hh_hmcs40_state
{
public:
	cdkong_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	int m_speaker_volume;
	TIMER_DEVICE_CALLBACK_MEMBER(speaker_decay_sim);

protected:
	virtual void machine_start() override;
};

// handlers

// Sound is controlled by two pins: D3 for pitch, and R13 for on/off. When turned
// off, it does not mute instantly, but volume slowly decays. Until we emulate it
// with discrete audio, this crude simulation will do.

#define CDKONG_SPEAKER_MAX 0x10000
#define CDKONG_SPEAKER_DECAY 50

TIMER_DEVICE_CALLBACK_MEMBER(cdkong_state::speaker_decay_sim)
{
	m_speaker->set_output_gain(0, m_speaker_volume / (double)CDKONG_SPEAKER_MAX);
	m_speaker_volume /= 2;
}


void cdkong_state::prepare_display()
{
	UINT32 plate = BITSWAP32(m_plate,31,30,29,24,0,16,8,1,23,17,9,2,18,10,25,27,26,3,15,27,11,11,14,22,6,13,21,5,19,12,20,4) | 0x800800;
	display_matrix(29, 11, plate, m_grid);
}

WRITE8_MEMBER(cdkong_state::plate_w)
{
	// R13: speaker on
	if (offset == HMCS40_PORT_R1X && data & 8)
		m_speaker_volume = CDKONG_SPEAKER_MAX;

	// R0x-R6x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(cdkong_state::grid_w)
{
	// D3: speaker out
	m_speaker->level_w(data >> 3 & 1);

	// D4-D14: vfd matrix grid
	m_grid = data >> 4 & 0x7ff;
	prepare_display();
}


// config

static INPUT_PORTS_START( cdkong )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.1") // port D
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x7ff8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void cdkong_state::machine_start()
{
	hh_hmcs40_state::machine_start();

	// zerofill/init
	m_speaker_volume = 0;
	save_item(NAME(m_speaker_volume));
}

static MACHINE_CONFIG_START( cdkong, cdkong_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(cdkong_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.1"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("speaker_decay", cdkong_state, speaker_decay_sim, attotime::from_msec(CDKONG_SPEAKER_DECAY))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Galaxian (manufactured in Taiwan)
  * PCB label Coleco Rev A 75718
  * Hitachi HD38800A70 MCU
  * cyan/red VFD display Futaba DM-36Z 2H, with color overlay

  Select game mode on start:
  - P1 Left:  Galaxian (default game)
  - P1 Right: Midway's Attackers
  - P2 Left:  Head-to-Head Galaxian (2-player mode, short)
  - P2 Right: Head-to-Head Galaxian (2-player mode, long)

  NOTE!: MAME external artwork is required

***************************************************************************/

class cgalaxn_state : public hh_hmcs40_state
{
public:
	cgalaxn_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE16_MEMBER(plate_w);
	DECLARE_READ8_MEMBER(input_r);

	DECLARE_INPUT_CHANGED_MEMBER(player_switch);
};

// handlers

void cgalaxn_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,1,2,0,11,10,9,8,7,6,5,4,3);
	UINT16 plate = BITSWAP16(m_plate,15,14,6,5,4,3,2,1,7,8,9,10,11,0,12,13);
	display_matrix(15, 12, plate, grid);
}

WRITE8_MEMBER(cgalaxn_state::grid_w)
{
	// R10,R11: input mux
	if (offset == HMCS40_PORT_R1X)
		m_inp_mux = data & 3;

	// R1x-R3x: vfd matrix grid
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(cgalaxn_state::plate_w)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1: start alien attack whine sound effect (edge triggered)

	// D2-D15: vfd matrix plate
	m_plate = (m_plate & 0x4000) | (data >> 2 & 0x3fff);
	prepare_display();
}

READ8_MEMBER(cgalaxn_state::input_r)
{
	// R0x: multiplexed inputs
	return read_inputs(2);
}


// config

static INPUT_PORTS_START( cgalaxn )
	PORT_START("IN.0") // R10 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // R11 port R0x
	PORT_CONFNAME( 0x01, 0x01, "Players" ) PORT_CHANGED_MEMBER(DEVICE_SELF, cgalaxn_state, player_switch, NULL)
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.3") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(cgalaxn_state::player_switch)
{
	// 2-player switch directly enables plate 14
	m_plate = (m_plate & 0x3fff) | (newval ? 0 : 0x4000);
	prepare_display();
}


static MACHINE_CONFIG_START( cgalaxn, cgalaxn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, READ8(cgalaxn_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(cgalaxn_state, grid_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(cgalaxn_state, grid_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(cgalaxn_state, grid_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(cgalaxn_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Pac-Man (manufactured in Taiwan, licensed from Midway)
  * PCB label Coleco 75690
  * Hitachi QFP HD38820A28/29 MCU
  * cyan/red VFD display Futaba DM-34Z 2A, with color overlay

  known releases:
  - USA: Pac-Man, by Coleco (name-license from Midway)
  - Japan: Super Puck Monster, published by Gakken

  Select game mode on start:
  - P1 Right: Pac-Man (default game)
  - P1 Left:  Head-to-Head Pac-Man (2-player mode)
  - P1 Up:    Eat & Run
  - P1 Down:  Demo

  BTANB note: 1st version doesn't show the whole maze on power-on

  NOTE!: MAME external artwork is required

***************************************************************************/

class cpacman_state : public hh_hmcs40_state
{
public:
	cpacman_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(cpacman_state::plate_w)
{
	// R1x-R6x(,D1,D2): vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,0,1,2,3,4,5,6,7,8,9,10);
	UINT32 plate = BITSWAP32(m_plate,31,30,29,28,27,0,1,2,3,8,9,10,11,16,17,18,19,25,26,23,22,21,20,24,15,14,13,12,4,5,6,7);
	display_matrix(27, 11, plate, grid);
}

WRITE16_MEMBER(cpacman_state::grid_w)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D13-D15: input mux
	m_inp_mux = data >> 13 & 7;

	// D5-D15: vfd matrix grid
	m_grid = data >> 5 & 0x7ff;

	// D1,D2: plate 8,14 (update display there)
	plate_w(space, 6 + HMCS40_PORT_R1X, data >> 1 & 3);
}

READ8_MEMBER(cpacman_state::input_r)
{
	// R0x: multiplexed inputs
	return read_inputs(3);
}


// config

static INPUT_PORTS_START( cpacman )
	PORT_START("IN.0") // D13 port R0x
	PORT_CONFNAME( 0x01, 0x01, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // D14 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D15 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
INPUT_PORTS_END

static MACHINE_CONFIG_START( cpacman, cpacman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, READ8(cpacman_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(cpacman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(cpacman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(cpacman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(cpacman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(cpacman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(cpacman_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(cpacman_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Ms. Pac-Man (manufactured in Taiwan, licensed from Midway)
  * PCB label Coleco 911171
  * Hitachi QFP HD38820A61 MCU
  * cyan/red VFD display Futaba DM-60Z 3I, with color overlay

  Select game mode on start:
  - P1 Left:  Ms. Pac-Man (default game)
  - P1 Down:  Head-to-Head Ms. Pac-Man (2-player mode)
  - P1 Up:    Demo

  BTANB note: in demo-mode, she hardly ever walks to the upper two rows

  NOTE!: MAME external artwork is required

***************************************************************************/

class cmspacmn_state : public hh_hmcs40_state
{
public:
	cmspacmn_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(cmspacmn_state::plate_w)
{
	// R1x-R6x(,D0,D1): vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,11,10,9,8,7,6,5,4,3,2,1,0,1);
	UINT64 plate = BIT(m_plate,15)<<32 | BITSWAP32(m_plate,14,13,12,4,5,6,7,24,23,25,22,21,20,13,24,3,19,14,12,11,24,2,10,8,7,25,0,9,1,18,17,16) | 0x1004080;
	display_matrix(33, 12, plate, grid);
}

WRITE16_MEMBER(cmspacmn_state::grid_w)
{
	// D2: speaker out
	m_speaker->level_w(data >> 2 & 1);

	// D13-D15: input mux
	m_inp_mux = data >> 13 & 7;

	// D5-D15: vfd matrix grid
	m_grid = data >> 5 & 0x7ff;

	// D0,D1: more plates (update display there)
	plate_w(space, 6 + HMCS40_PORT_R1X, data & 3);
}

READ8_MEMBER(cmspacmn_state::input_r)
{
	// R0x: multiplexed inputs
	return read_inputs(3);
}


// config

static INPUT_PORTS_START( cmspacmn )
	PORT_START("IN.0") // D13 port R0x
	PORT_CONFNAME( 0x01, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // D14 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("IN.2") // D15 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
INPUT_PORTS_END

static MACHINE_CONFIG_START( cmspacmn, cmspacmn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, READ8(cmspacmn_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(cmspacmn_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(cmspacmn_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(cmspacmn_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(cmspacmn_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(cmspacmn_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(cmspacmn_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(cmspacmn_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Galaxian 2 (manufactured in Japan)
  * PCB labels ENTEX GALAXIAN PB-118/116/097 80-210137/135/114
  * Hitachi QFP HD38820A13 MCU
  * cyan/red/green VFD display Futaba DM-20

  NOTE!: MAME external artwork is required

***************************************************************************/

class egalaxn2_state : public hh_hmcs40_state
{
public:
	egalaxn2_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

void egalaxn2_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,15,14,13,12,7,6,5,4,3,2,1,0,19,18,17,16,11,10,9,8);
	display_matrix(24, 15, plate, grid);
}

WRITE16_MEMBER(egalaxn2_state::grid_w)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1-D4: input mux
	m_inp_mux = data >> 1 & 0xf;

	// D1-D15: vfd matrix grid
	m_grid = data >> 1 & 0x7fff;
	prepare_display();
}

WRITE8_MEMBER(egalaxn2_state::plate_w)
{
	// R1x-R6x: vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

READ8_MEMBER(egalaxn2_state::input_r)
{
	// R0x: multiplexed inputs
	return read_inputs(4);
}


// config

static INPUT_PORTS_START( egalaxn2 )
	PORT_START("IN.0") // D1 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.1") // D2 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.2") // D3 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // D4 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x02, "Skill Level" )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x0c, 0x00, "Players" )
	PORT_CONFSETTING(    0x08, "0 (Demo)" ) // for Demo mode: need to hold down Fire button at power-on
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )
INPUT_PORTS_END


static MACHINE_CONFIG_START( egalaxn2, egalaxn2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, READ8(egalaxn2_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(egalaxn2_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(egalaxn2_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(egalaxn2_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(egalaxn2_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(egalaxn2_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(egalaxn2_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(egalaxn2_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Pac Man 2 (manufactured in Japan)
  * PCB labels ENTEX PAC-MAN PB-093/094 80-210149/50/51
  * Hitachi QFP HD38820A23 MCU
  * cyan/red VFD display Futaba DM-28Z 1G(cyan Pac-Man) or DM-28 1K(orange Pac-Man)

  2 VFD revisions are known, the difference is Pac-Man's color: cyan or red.

  NOTE!: MAME external artwork is required

***************************************************************************/
#if 0
class epacman2_state : public egalaxn2_state
{
public:
	epacman2_state(const machine_config &mconfig, device_type type, const char *tag)
		: egalaxn2_state(mconfig, type, tag)
	{ }
};
#endif
// handlers are identical to Galaxian 2, so we can use those

// config

static INPUT_PORTS_START( epacman2 )
	PORT_START("IN.0") // D1 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.1") // D2 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.2") // D3 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P1 Skill Control")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Demo Light Test")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // D4 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x02, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFNAME( 0x0c, 0x04, "Players" )
	PORT_CONFSETTING(    0x08, "0 (Demo)" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END





/***************************************************************************

  Entex Turtles (manufactured in Japan)
  * PCB label 560359
  * Hitachi QFP HD38820A43 MCU
  * COP411L sub MCU, labeled COP411L-KED/N
  * cyan/red/green VFD display NEC FIP15BM32T

  NOTE!: MAME external artwork is required

***************************************************************************/

class eturtles_state : public hh_hmcs40_state
{
public:
	eturtles_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag),
		m_cop_irq(0)
	{ }

	virtual void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	UINT8 m_cop_irq;
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	DECLARE_WRITE8_MEMBER(cop_irq_w);
	DECLARE_READ8_MEMBER(cop_latch_r);
	DECLARE_READ8_MEMBER(cop_ack_r);

	void update_int();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	virtual void machine_start() override;
};

// handlers: maincpu side

void eturtles_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,1,14,13,12,11,10,9,8,7,6,5,4,3,2,0);
	UINT32 plate = BITSWAP32(m_plate,31,30,11,12,18,19,16,17,22,15,20,21,27,26,23,25,24,2,3,1,0,6,4,5,10,9,2,8,7,14,1,13);
	display_matrix(30, 15, plate | (grid >> 5 & 8), grid); // grid 8 also forces plate 3 high
}

WRITE8_MEMBER(eturtles_state::plate_w)
{
	m_r[offset] = data;

	// R0x-R6x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(eturtles_state::grid_w)
{
	m_d = data;

	// D1-D6: input mux
	UINT8 inp_mux = data >> 1 & 0x3f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int();
	}

	// D1-D15: vfd matrix grid
	m_grid = data >> 1 & 0x7fff;
	prepare_display();
}

void eturtles_state::update_int()
{
	// INT0/1 on multiplexed inputs, and from COP D0
	UINT8 inp = read_inputs(6);
	set_interrupt(0, (inp & 1) | m_cop_irq);
	set_interrupt(1, inp & 2);
}


// handlers: COP side

WRITE_LINE_MEMBER(eturtles_state::speaker_w)
{
	// SK: speaker out
	m_speaker->level_w(!state);
}

WRITE8_MEMBER(eturtles_state::cop_irq_w)
{
	// D0: maincpu INT0 (active low)
	m_cop_irq = ~data & 1;
	update_int();
}

READ8_MEMBER(eturtles_state::cop_latch_r)
{
	// L0-L3: soundlatch from maincpu R0x
	return m_r[0];
}

READ8_MEMBER(eturtles_state::cop_ack_r)
{
	// G0: ack from maincpu D0
	return m_d & 1;
}


// config

static INPUT_PORTS_START( eturtles )
	PORT_START("IN.0") // D1 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)

	PORT_START("IN.1") // D2 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)

	PORT_START("IN.2") // D3 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)

	PORT_START("IN.3") // D4 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)

	PORT_START("IN.4") // D5 INT0/1
	PORT_CONFNAME( 0x01, 0x01, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)

	PORT_START("IN.5") // D6 INT0/1
	PORT_CONFNAME( 0x03, 0x00, "Players" ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_CONFSETTING(    0x02, "0 (Demo)" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(eturtles_state::input_changed)
{
	update_int();
}


void eturtles_state::machine_start()
{
	hh_hmcs40_state::machine_start();

	// register for savestates
	save_item(NAME(m_cop_irq));
}

static MACHINE_CONFIG_START( eturtles, eturtles_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(eturtles_state, grid_w))

	MCFG_CPU_ADD("audiocpu", COP411, 215000) // approximation
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_DISABLED) // guessed
	MCFG_COP400_WRITE_SK_CB(WRITELINE(eturtles_state, speaker_w))
	MCFG_COP400_WRITE_D_CB(WRITE8(eturtles_state, cop_irq_w))
	MCFG_COP400_READ_L_CB(READ8(eturtles_state, cop_latch_r))
	MCFG_COP400_READ_G_CB(READ8(eturtles_state, cop_ack_r))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Stargate (manufactured in Japan)
  * PCB label 5603521/31
  * Hitachi QFP HD38820A42 MCU
  * COP411L sub MCU, labeled ~/B8236 COP411L-KEC/N
  * cyan/red/green VFD display NEC FIP15AM32T (EL628-003) no. 2-421, with partial color overlay

  NOTE!: MAME external artwork is required

***************************************************************************/

class estargte_state : public eturtles_state
{
public:
	estargte_state(const machine_config &mconfig, device_type type, const char *tag)
		: eturtles_state(mconfig, type, tag)
	{ }

	virtual void prepare_display() override;
	DECLARE_READ8_MEMBER(cop_data_r);
};

// handlers (most of it is in eturtles_state above)

void estargte_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,0,14,13,12,11,10,9,8,7,6,5,4,3,2,1);
	UINT32 plate = BITSWAP32(m_plate,31,30,29,15,17,19,21,23,25,27,26,24,3,22,20,18,16,14,12,10,8,6,4,2,0,1,3,5,7,9,11,13);
	display_matrix(29, 14, plate, grid);
}

READ8_MEMBER(estargte_state::cop_data_r)
{
	// L0-L3: soundlatch from maincpu R0x
	// L7: ack from maincpu D0
	return m_r[0] | (m_d << 7 & 0x80);
}


// config

static INPUT_PORTS_START( estargte )
	PORT_START("IN.0") // D1 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL) PORT_NAME("Inviso")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL) PORT_NAME("Smart Bomb")

	PORT_START("IN.1") // D2 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL) PORT_NAME("Change Direction")

	PORT_START("IN.2") // D3 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)

	PORT_START("IN.3") // D4 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL) PORT_NAME("Thrust")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // D5 INT0/1
	PORT_CONFNAME( 0x01, 0x00, "Players" ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_CONFSETTING(    0x00, "0 (Demo)" ) // yes, same value as 1-player, hold the Inviso button at boot to enter demo mode
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFNAME( 0x02, 0x02, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, NULL)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.5") // D6 INT0/1
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( estargte, estargte_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(eturtles_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(eturtles_state, grid_w))

	MCFG_CPU_ADD("audiocpu", COP411, 190000) // approximation
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_DISABLED) // guessed
	MCFG_COP400_WRITE_SK_CB(WRITELINE(eturtles_state, speaker_w))
	MCFG_COP400_WRITE_D_CB(WRITE8(eturtles_state, cop_irq_w))
	MCFG_COP400_READ_L_CB(READ8(estargte_state, cop_data_r))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Gakken Heiankyo Alien (manufactured in Japan)
  * Hitachi HD38800A04 MCU
  * cyan/red VFD display Futaba DM-11Z 1H

  known releases:
  - Japan: Heiankyo Alien
  - USA: Earth Invaders, published by CGL

  NOTE!: MAME external artwork is required

***************************************************************************/

class ghalien_state : public hh_hmcs40_state
{
public:
	ghalien_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_READ16_MEMBER(input_r);
};

// handlers

WRITE8_MEMBER(ghalien_state::plate_w)
{
	// R0x-R3x(,D10-D13): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	UINT32 plate = BITSWAP24(m_plate,23,22,21,20,14,12,10,8,9,13,15,2,0,1,3,11,7,5,4,6,19,17,16,18);
	display_matrix(20, 10, plate, grid);
}

WRITE16_MEMBER(ghalien_state::grid_w)
{
	// D14: speaker out
	m_speaker->level_w(data >> 14 & 1);

	// D0-D6: input mux
	m_inp_mux = data & 0x7f;

	// D0-D9: vfd matrix grid
	m_grid = data & 0x3ff;

	// D10-D13: more plates (update display there)
	plate_w(space, 4, data >> 10 & 0xf);
}

READ16_MEMBER(ghalien_state::input_r)
{
	// D15: multiplexed inputs
	return read_inputs(7) & 0x8000;
}


// config

static INPUT_PORTS_START( ghalien )
	PORT_START("IN.0") // D0 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // D1 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.2") // D2 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.3") // D3 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.4") // D4 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Dig")

	PORT_START("IN.5") // D5 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bury")

	PORT_START("IN.6") // D6 line D15
	PORT_CONFNAME( 0x8000, 0x0000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x0000, "Amateur" )
	PORT_CONFSETTING(      0x8000, "Professional" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ghalien, ghalien_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(ghalien_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(ghalien_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(ghalien_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(ghalien_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(ghalien_state, grid_w))
	MCFG_HMCS40_READ_D_CB(READ16(ghalien_state, input_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Gakken Crazy Kong (manufactured in Japan)
  * PCB label ZENY 5603601
  * Hitachi HD38800B01 MCU
  * cyan/red/blue VFD display Futaba DM-54Z 2H, with bezel overlay

  known releases:
  - Japan: Crazy Kong
  - USA: Super Kong, published by CGL

  NOTE!: MAME external artwork is required

***************************************************************************/

class gckong_state : public hh_hmcs40_state
{
public:
	gckong_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int1();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

WRITE8_MEMBER(gckong_state::plate_w)
{
	// R0x-R3x(,D0,D1): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,0,1,2,3,4,5,6,7,8,9,10);
	UINT32 plate = BITSWAP32(m_plate,31,30,29,28,27,26,25,6,7,8,12,13,14,15,16,17,18,17,16,12,11,10,9,8,7,6,5,4,3,2,1,0) | 0x8000;
	display_matrix(32, 11, plate, grid);
}

WRITE16_MEMBER(gckong_state::grid_w)
{
	// D2: speaker out
	m_speaker->level_w(data >> 2 & 1);

	// D5-D8: input mux
	UINT8 inp_mux = data >> 5 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D5-D15: vfd matrix grid
	m_grid = data >> 5 & 0x7ff;

	// D0,D1: more plates (update display there)
	plate_w(space, 4, data & 3);
}

void gckong_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(4));
}


// config

static INPUT_PORTS_START( gckong )
	PORT_START("IN.0") // D5 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, NULL)

	PORT_START("IN.1") // D6 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, NULL)

	PORT_START("IN.2") // D7 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, NULL)

	PORT_START("IN.3") // D8 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, NULL)

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)

	PORT_START("IN.5") // port D
	PORT_CONFNAME( 0x0010, 0x0000, "Skill Level" )
	PORT_CONFSETTING(      0x0000, "A" )
	PORT_CONFSETTING(      0x0010, "B" )
	PORT_BIT( 0xffef, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(gckong_state::input_changed)
{
	update_int1();
}


static MACHINE_CONFIG_START( gckong, gckong_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(gckong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(gckong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(gckong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(gckong_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(gckong_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.5"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Gakken Dig Dug (manufactured in Japan)
  * PCB label Gakken DIG-DAG KS-004283(A/B)
  * Hitachi QFP HD38820A69 MCU
  * cyan/red/green VFD display Futaba DM-69Z 3F, with color overlay

  NOTE!: MAME external artwork is required

***************************************************************************/

class gdigdug_state : public hh_hmcs40_state
{
public:
	gdigdug_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int1();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

WRITE8_MEMBER(gdigdug_state::plate_w)
{
	// R0x-R6x(,D0-D3): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT32 plate = BITSWAP32(m_plate,30,31,0,1,2,3,4,5,6,7,20,21,22,27,26,25,28,29,24,23,15,14,13,12,8,9,10,11,19,18,17,16);
	display_matrix(32, 9, plate, m_grid);
}

WRITE16_MEMBER(gdigdug_state::grid_w)
{
	// D6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// D11-D15: input mux
	UINT8 inp_mux = data >> 11 & 0x1f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D7-D15: vfd matrix grid
	m_grid = data >> 7 & 0x1ff;

	// D0-D3: more plates (update display there)
	plate_w(space, 7, data & 0xf);
}

void gdigdug_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(5));
}


// config

static INPUT_PORTS_START( gdigdug )
	PORT_START("IN.0") // D11 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, NULL)

	PORT_START("IN.1") // D12 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, NULL)

	PORT_START("IN.2") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, NULL)

	PORT_START("IN.3") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, NULL)

	PORT_START("IN.4") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, NULL)

	PORT_START("IN.5") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(gdigdug_state::input_changed)
{
	update_int1();
}


static MACHINE_CONFIG_START( gdigdug, gdigdug_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(gdigdug_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(gdigdug_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mattel World Championship Baseball
  * PCB label MEL-001 Baseball Rev. B
  * Hitachi QFP HD38820A09 MCU
  * cyan/red/green VFD display Futaba DM-24ZK 1G, with etched overlay

  To start the game in 2-player mode, simply turn the game on. For 1-player,
  turn the game on while holding the 1-key and use the visitor's side keypad
  to play offsense.

  NOTE!: MAME external artwork is required

***************************************************************************/

class mwcbaseb_state : public hh_hmcs40_state
{
public:
	mwcbaseb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

void mwcbaseb_state::prepare_display()
{
	UINT8 grid = BITSWAP8(m_grid,0,1,2,3,4,5,6,7);
	display_matrix(16, 8, m_plate, grid);
}

WRITE8_MEMBER(mwcbaseb_state::plate_w)
{
	// R1x-R3x,R6x: vfd matrix plate
	int shift = (offset == HMCS40_PORT_R6X) ? 12 : (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(mwcbaseb_state::grid_w)
{
	// D9-D15: input mux
	m_inp_mux = data >> 9 & 0x7f;

	// D0-D7: vfd matrix grid
	m_grid = data & 0xff;
	prepare_display();
}

WRITE8_MEMBER(mwcbaseb_state::speaker_w)
{
	// R50,R51+R52(tied together): speaker out
	m_speaker->level_w(data & 7);
}

READ8_MEMBER(mwcbaseb_state::input_r)
{
	// R4x: multiplexed inputs
	return read_inputs(7);
}


// config

/* physical button layout and labels is like this:

        (visitor team side)                                       (home team side)
    COMP PITCH                     [SCORE]       [INNING]
    [1]      [2]      [3]                                     [1]      [2]      [3]
    NEW PITCHER       PINCH HITTER                            NEW PITCHER       PINCH HITTER

    [4]      [5]      [6]                                     [4]      [5]      [6]
    BACKWARD (pitch)  FORWARD                                 BACKWARD (pitch)  FORWARD

    [7]      [8]      [9]                                     [7]      [8]      [9]

    BUNT     NORMAL   HR SWING                                BUNT     NORMAL   HR SWING
    [CLEAR]  [0]      [ENTER]                                 [CLEAR]  [0]      [ENTER]
    SLOW     CURVE    FAST                                    SLOW     CURVE    FAST
*/

static INPUT_PORTS_START( mwcbaseb )
	PORT_START("IN.0") // D9 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("P2 4") // note: P1 = left/visitor, P2 = right/home
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("P2 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("P2 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("P2 1")

	PORT_START("IN.1") // D10 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P2 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P2 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P2 5")

	PORT_START("IN.2") // D11 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P2 Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("P2 Clear")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("P2 0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P2 9")

	PORT_START("IN.3") // D12 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Inning")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Score")

	PORT_START("IN.4") // D13 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P1 Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("P1 Clear")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("P1 0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("P1 9")

	PORT_START("IN.5") // D14 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("P1 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("P1 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("P1 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("P1 5")

	PORT_START("IN.6") // D15 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("P1 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("P1 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("P1 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("P1 1")
INPUT_PORTS_END


static const INT16 mwcbaseb_speaker_levels[] = { 0, 16384, -16384, 0, -16384, 0, -32768, -16384 };

static MACHINE_CONFIG_START( mwcbaseb, mwcbaseb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(mwcbaseb_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(mwcbaseb_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(mwcbaseb_state, plate_w))
	MCFG_HMCS40_READ_R_CB(4, READ8(mwcbaseb_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(mwcbaseb_state, speaker_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(mwcbaseb_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(mwcbaseb_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(8, mwcbaseb_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Q*Bert
  * PCB label 13662 REV-4
  * Hitachi QFP HD38820A70 MCU
  * cyan/red/green/darkgreen VFD display Itron CP5137

  NOTE!: MAME external artwork is required

***************************************************************************/

class pbqbert_state : public hh_hmcs40_state
{
public:
	pbqbert_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
};

// handlers

WRITE8_MEMBER(pbqbert_state::plate_w)
{
	// R0x-R6x(,D8): vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT32 plate = BITSWAP32(m_plate,31,30,24,25,26,27,28,15,14,29,13,12,11,10,9,8,7,6,5,4,3,2,1,0,16,17,18,19,20,21,22,23) | 0x400000;
	display_matrix(30, 8, plate, m_grid);
}

WRITE16_MEMBER(pbqbert_state::grid_w)
{
	// D14: speaker out
	m_speaker->level_w(data >> 14 & 1);

	// D0-D7: vfd matrix grid
	m_grid = data & 0xff;

	// D8: plate 25 (update display there)
	plate_w(space, 7, data >> 8 & 1);
}


// config

static INPUT_PORTS_START( pbqbert )
	PORT_START("IN.0") // port D
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) // up-left
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) // up-right
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) // down-right
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) // down-left
	PORT_BIT( 0xe1ff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( pbqbert, pbqbert_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(pbqbert_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(pbqbert_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.0"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy Kingman (manufactured in Japan)
  * PCBs are labeled THF-01II 2E138E01/2E128E02
  * Hitachi HD38800B23 MCU
  * cyan/red/blue VFD display Futaba DM-65ZK 3A

  NOTE!: MAME external artwork is required

***************************************************************************/

class kingman_state : public hh_hmcs40_state
{
public:
	kingman_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int0();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

void kingman_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,9,0,1,2,3,4,5,6,7,8);
	UINT32 plate = BITSWAP24(m_plate,23,6,7,5,4,3,2,1,0,13,12,20,19,18,17,16,10,11,9,8,14,15,13,12);
	display_matrix(23, 9, plate, grid);
}

WRITE8_MEMBER(kingman_state::plate_w)
{
	// R0x-R3x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(kingman_state::grid_w)
{
	// D6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// D12-D15: input mux
	UINT8 inp_mux = data >> 12 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D7-D15: vfd matrix grid
	m_grid = data >> 7 & 0x1ff;

	// D0-D4: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x1f0000);
	prepare_display();
}

void kingman_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(4));
}


// config

static INPUT_PORTS_START( kingman )
	PORT_START("IN.0") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, NULL)

	PORT_START("IN.1") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, NULL)

	PORT_START("IN.2") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, NULL)

	PORT_START("IN.3") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, NULL)

	PORT_START("IN.4") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(kingman_state::input_changed)
{
	update_int0();
}


static MACHINE_CONFIG_START( kingman, kingman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(kingman_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Tron (manufactured in Japan)
  * PCBs are labeled THN-02 2E114E07
  * Hitachi HD38800A88 MCU
  * cyan/red/green VFD display NEC FIP10AM24T no. 2-8 1

  NOTE!: MAME external artwork is required

***************************************************************************/

class tmtron_state : public hh_hmcs40_state
{
public:
	tmtron_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);

	void update_int1();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};

// handlers

void tmtron_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,11,10,1,2,3,4,5,6,7,8,9,0);
	UINT32 plate = BITSWAP24(m_plate,23,5,2,21,1,6,7,9,10,11,21,0,19,3,4,8,3,18,17,16,12,13,14,15);
	display_matrix(23, 10, plate, grid);
}

WRITE8_MEMBER(tmtron_state::plate_w)
{
	// R0x-R3x: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(tmtron_state::grid_w)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D12-D15: input mux
	UINT8 inp_mux = data >> 12 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D6-D15: vfd matrix grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D3,D5: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x2f0000);
	prepare_display();
}

void tmtron_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(4));
}


// config

static INPUT_PORTS_START( tmtron )
	PORT_START("IN.0") // D12 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL)

	PORT_START("IN.1") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL)

	PORT_START("IN.2") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL)

	PORT_START("IN.3") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL)

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(tmtron_state::input_changed)
{
	update_int1();
}


static MACHINE_CONFIG_START( tmtron, tmtron_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(tmtron_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  VTech Invaders (manufactured in Taiwan)
  * Hitachi HD38750A45 MCU
  * cyan/red VFD display Futaba DM-26Z 1G, with bezel

  known releases:
  - USA: Invaders/Sonic Invader
  - UK: Cosmic Invader, published by Grandstand
  - UK: Galactic Invaders, published by Prinztronic

  NOTE!: MAME external artwork is required

***************************************************************************/

class vinvader_state : public hh_hmcs40_state
{
public:
	vinvader_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
};

// handlers

WRITE8_MEMBER(vinvader_state::plate_w)
{
	// R1x-R3x(,D4-D6): vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 plate = BITSWAP16(m_plate,15,11,7,3,10,6,14,2,9,5,13,1,8,4,12,0);
	display_matrix(12, 9, plate, m_grid);
}

WRITE16_MEMBER(vinvader_state::grid_w)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D7-D15: vfd matrix grid
	m_grid = data >> 7 & 0x1ff;

	// D4-D6: more plates (update display there)
	plate_w(space, 3 + HMCS40_PORT_R1X, data >> 4 & 7);
}


// config

static INPUT_PORTS_START( vinvader )
	PORT_START("IN.0") // port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port D
	PORT_CONFNAME( 0x0002, 0x0000, "Skill Level")
	PORT_CONFSETTING(      0x0000, "1" )
	PORT_CONFSETTING(      0x0002, "2" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xfff5, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( vinvader, vinvader_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38750, 400000) // approximation
	MCFG_HMCS40_READ_R_CB(0, IOPORT("IN.0"))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(vinvader_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(vinvader_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(vinvader_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(vinvader_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.1"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bambball )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a08", 0x0000, 0x0800, CRC(907fef18) SHA1(73fe7ca7c6332268a3a9abc5ac88ada2991012fb) )
	ROM_CONTINUE(           0x0f00, 0x0080 )
ROM_END


ROM_START( bmboxing )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a07", 0x0000, 0x0800, CRC(7f33e259) SHA1(c5fcdd6bf060c96666354f09f0570c754f6ed4e0) )
	ROM_CONTINUE(           0x0f00, 0x0080 )
ROM_END


ROM_START( bfriskyt )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a77", 0x0000, 0x1000, CRC(a2445c4f) SHA1(0aaccfec90b66d27dae194d4462d88e654c41578) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( packmon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a27", 0x0000, 0x1000, CRC(86e09e84) SHA1(ac7d3c43667d5720ca513f8ff51d146d9f2af124) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( msthawk )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a73", 0x0000, 0x1000, CRC(a4f9a523) SHA1(465f06b02e2e7d2277218fd447830725790a816c) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( bzaxxon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b19", 0x0000, 0x1000, CRC(4fecb80d) SHA1(7adf079480ffd3825ad5ae1eaa4d892eecbcc42d) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( zackman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a49", 0x0000, 0x1000, CRC(b97f5ef6) SHA1(7fe20e8107361caf9ea657e504be1f8b10b8b03f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( bpengo )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a63", 0x0000, 0x1000, CRC(ebd6bc64) SHA1(0a322c47b9553a2739a85908ce64b9650cf93d49) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( bbtime )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a65", 0x0000, 0x1000, CRC(33611faf) SHA1(29b6a30ed543688d31ec2aa18f7938fa4eef30b0) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( bdoramon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b43", 0x0000, 0x1000, CRC(9387ca42) SHA1(8937e208934b34bd9f49700aa50287dfc8bda76c) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( bultrman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b52", 0x0000, 0x1000, CRC(88d372dc) SHA1(f2ac3b89be8afe6fb65914ccebe1a56316b9472a) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( machiman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a85", 0x0000, 0x1000, CRC(894b4954) SHA1(cab49638a326b031aa548301beb16f818759ef62) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( pairmtch )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a88", 0x0000, 0x1000, CRC(ffa35730) SHA1(5a80b9025aaad2ac0ab0b1436a1355ae8cd3f868) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 0x2000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a89", 0x0000, 0x1000, CRC(3533ec56) SHA1(556d69e78a0ee1bf766fce16ed58992d7272d57f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( alnattck )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a25", 0x0000, 0x1000, CRC(18b50869) SHA1(11e9d5f7b4ae818b077b0ee14a3b43190e20bff3) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cdkong )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a45", 0x0000, 0x1000, CRC(196b8070) SHA1(da85d1eb4b048b77f3168630662ab94ec9baa262) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cgalaxn )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a70", 0x0000, 0x1000, CRC(a4c5ed1d) SHA1(0f647cb78437d7e62411febf7c9ce3c5b6753a80) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cpacman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a29", 0x0000, 0x1000, CRC(1082d577) SHA1(0ef73132bd41f6ca1e4c001ae19f7f7c97eaa8d1) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END

ROM_START( cpacmanr1 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a28", 0x0000, 0x1000, CRC(d2ed57e5) SHA1(f56f1341485ac28ea9e6cc4d162fab18d8a4c977) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cmspacmn )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a61", 0x0000, 0x1000, CRC(76276318) SHA1(9d6ff3f49b4cdaee5c9e238c1ed638bfb9b99aa7) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( egalaxn2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a13", 0x0000, 0x1000, CRC(112b721b) SHA1(4a185bc57ea03fe64f61f7db4da37b16eeb0cb54) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( epacman2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a23", 0x0000, 0x1000, CRC(6eab640f) SHA1(509bdd02be915089e13769f22a08e03509f03af4) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( estargte )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a42", 0x0000, 0x1000, CRC(5f6d55a6) SHA1(0da32149790fa5f16097338fc80536b462169e0c) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 0x0200, "audiocpu", 0 )
	ROM_LOAD( "cop411l-kec_n", 0x0000, 0x0200, CRC(fbd3c2d3) SHA1(65b8b24d38678c3fa970bfd639e9449a75a28927) )
ROM_END


ROM_START( eturtles )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a43", 0x0000, 0x1000, CRC(446aa4e2) SHA1(d1c0fb14ea7081def53b1174964b39eed1e5d5e6) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 0x0200, "audiocpu", 0 )
	ROM_LOAD( "cop411l-ked_n", 0x0000, 0x0200, CRC(503d26e9) SHA1(a53d24d62195bfbceff2e4a43199846e0950aef6) )
ROM_END


ROM_START( ghalien )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a04", 0x0000, 0x1000, CRC(019c3328) SHA1(9f1029c5c479f78350952c4f18747341ba5ea7a0) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( gckong )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b01", 0x0000, 0x1000, CRC(d5a2cca3) SHA1(37bb5784383daab672ed1e0e2362c7a40d8d9b3f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( gdigdug )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a69", 0x0000, 0x1000, CRC(501165a9) SHA1(8a15d00c4aa66e870cadde33148426463560d2e6) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( mwcbaseb )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a09", 0x0000, 0x1000, CRC(25ba7dc0) SHA1(69e0a867fdcf07b454b1faf835e576ae782432c0) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( pbqbert )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a70", 0x0000, 0x1000, CRC(be7c80b4) SHA1(0617a80ef7fe188ea221de32e760d45fd4318c67) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( kingman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b23", 0x0000, 0x1000, CRC(f8dfe14f) SHA1(660610d92ae7e5f92bddf5a3bcc2296b2ec3946b) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( tmtron )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a88", 0x0000, 0x1000, CRC(33db9670) SHA1(d6f747a59356526698784047bcfdbb59e79b9a23) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( vinvader )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a45", 0x0000, 0x0800, CRC(32de6056) SHA1(70238c6c40c3d513f8eced1cb81bdd4dbe12f16c) )
	ROM_CONTINUE(           0x0f00, 0x0080 )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, bambball,  0,        0, bambball, bambball, driver_device, 0, "Bambino", "Dribble Away Basketball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1979, bmboxing,  0,        0, bmboxing, bmboxing, driver_device, 0, "Bambino", "Knock-Em Out Boxing", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1982, bfriskyt,  0,        0, bfriskyt, bfriskyt, driver_device, 0, "Bandai", "Frisky Tom (Bandai)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, packmon,   0,        0, packmon,  packmon,  driver_device, 0, "Bandai", "Packri Monster", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, msthawk,   0,        0, msthawk,  msthawk,  driver_device, 0, "Bandai (Mattel license)", "Star Hawk (Mattel)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, bzaxxon,   0,        0, bzaxxon,  bzaxxon,  driver_device, 0, "Bandai", "Zaxxon (Bandai)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1983, zackman,   0,        0, zackman,  zackman,  driver_device, 0, "Bandai", "Zackman", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, bpengo,    0,        0, bpengo,   bpengo,   driver_device, 0, "Bandai", "Pengo (Bandai)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1983, bbtime,    0,        0, bbtime,   bbtime,   driver_device, 0, "Bandai", "Burger Time (Bandai)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1983, bdoramon,  0,        0, bdoramon, bdoramon, driver_device, 0, "Bandai", "Dokodemo Dorayaki Doraemon", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, bultrman,  0,        0, bultrman, bultrman, driver_device, 0, "Bandai", "Ultra Man (Bandai)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1984, machiman,  0,        0, machiman, machiman, driver_device, 0, "Bandai", "Machine Man", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, pairmtch,  0,        0, pairmtch, pairmtch, driver_device, 0, "Bandai", "Pair Match", MACHINE_SUPPORTS_SAVE )

CONS( 1981, alnattck,  0,        0, alnattck, alnattck, driver_device, 0, "Coleco", "Alien Attack", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, cdkong,    0,        0, cdkong,   cdkong,   driver_device, 0, "Coleco", "Donkey Kong (Coleco)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_IMPERFECT_SOUND )
CONS( 1982, cgalaxn,   0,        0, cgalaxn,  cgalaxn,  driver_device, 0, "Coleco", "Galaxian (Coleco)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_IMPERFECT_SOUND )
CONS( 1981, cpacman,   0,        0, cpacman,  cpacman,  driver_device, 0, "Coleco", "Pac-Man (Coleco, Rev. 29)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, cpacmanr1, cpacman,  0, cpacman,  cpacman,  driver_device, 0, "Coleco", "Pac-Man (Coleco, Rev. 28)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, cmspacmn,  0,        0, cmspacmn, cmspacmn, driver_device, 0, "Coleco", "Ms. Pac-Man (Coleco)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1981, egalaxn2,  0,        0, egalaxn2, egalaxn2, driver_device, 0, "Entex", "Galaxian 2 (Entex)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, epacman2,  0,        0, egalaxn2, epacman2, driver_device, 0, "Entex", "Pac Man 2 (Entex)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, estargte,  0,        0, estargte, estargte, driver_device, 0, "Entex", "Stargate (Entex)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, eturtles,  0,        0, eturtles, eturtles, driver_device, 0, "Entex", "Turtles (Entex)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1980, ghalien,   0,        0, ghalien,  ghalien,  driver_device, 0, "Gakken", "Heiankyo Alien (Gakken)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gckong,    0,        0, gckong,   gckong,   driver_device, 0, "Gakken", "Crazy Kong (Gakken)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1983, gdigdug,   0,        0, gdigdug,  gdigdug,  driver_device, 0, "Gakken", "Dig Dug (Gakken)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1980, mwcbaseb,  0,        0, mwcbaseb, mwcbaseb, driver_device, 0, "Mattel", "World Championship Baseball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1983, pbqbert,   0,        0, pbqbert,  pbqbert,  driver_device, 0, "Parker Brothers", "Q*Bert (Parker Brothers)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1982, kingman,   0,        0, kingman,  kingman,  driver_device, 0, "Tomy", "Kingman", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, tmtron,    0,        0, tmtron,   tmtron,   driver_device, 0, "Tomy", "Tron (Tomy)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1981, vinvader,  0,        0, vinvader, vinvader, driver_device, 0, "VTech", "Invaders (VTech)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
