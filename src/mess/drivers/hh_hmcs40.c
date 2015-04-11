// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Hitachi HMCS40 MCU tabletops/handhelds or other simple devices.

  known chips:

  serial  device    etc.
----------------------------------------------------------------
 *07      HD38750A  1979, Bambino Knock-Em Out Boxing (ET-06B)
 @08      HD38750A  1979, Bambino Basketball (ET-05)
 *45      HD38750A  1981, Vtech Invaders
 *58      HD38750A  1982, Ludotronic(Hanzawa) Grand Prix Turbo
 *62      HD38750A  1982, Actronics(Hanzawa) Pack'n Maze

 *04      HD38800A  1980, Gakken Heiankyo Alien
 @25      HD38800A  1981, Coleco Alien Attack
 @27      HD38800A  1981, Bandai Packri Monster (DM-21Z)
 *51      HD38800A  1981, Actronics(Hanzawa) Twinvader
 @70      HD38800A  1982, Coleco Galaxian
 @73      HD38800A  1982, Mattel Star Hawk
 
 @23      HD38800B  1982, Tomy Kingman (THF-01II)
 *24      HD38800B  1982, Actronics(Hanzawa) Wanted G-Man
 *29      HD38800B  1984, Tomy Portable 6000 Bombman
 *35      HD38800B  1983, Bandai Gundam vs Gelgoog Zaku
 *43      HD38800B  1983, Bandai Dokodemo Dorayaki Doraemon

 @13      HD38820A  1981, Entex Galaxian 2
 @23      HD38820A  1981, Entex Pac Man 2
 @28      HD38820A  1981, Coleco Pac-Man (ver 1)
 @29      HD38820A  1981, Coleco Pac-Man (ver 2)
 *32      HD38820A  198?, Gakken Super Cobra
 *38      HD38820A  1982, Entex Crazy Climber
 *43      HD38820A  1982, Entex Turtles (have dump, +COP411 for audio)
 @45      HD38820A  1982, Coleco Donkey Kong
 @49      HD38820A  1983, Bandai Zackman
 @61      HD38820A  1983, Coleco Ms. Pac-Man
 @70      HD38820A  1983, Parker Brothers Q*Bert
 @88      HD38820A  1984, Tomy Tron (THN-02)

  (* denotes not yet emulated by MESS, @ denotes it's in this driver)

***************************************************************************/





/***************************************************************************

  Mattel Star Hawk (manufactured in Japan)
  * PCBs are labeled Kaken, PT-317B
  * Hitachi HD38800A73 MCU
  * cyan/red VFD display Futaba DM-41ZK, with partial color overlay

  NOTE!: MESS external artwork is recommended

***************************************************************************/






#include "emu.h"
#include "cpu/hmcs40/hmcs40.h"
#include "sound/speaker.h"

#include "hh_hmcs40_test.lh" // test-layout - use external artwork


class hh_hmcs40_state : public driver_device
{
public:
	hh_hmcs40_state(const machine_config &mconfig, device_type type, const char *tag)
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
	virtual void machine_start();
	virtual void machine_reset();
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

  Bambino Basketball - Dribble Away (manufactured in Japan)
  * PCBs are labeled Emix Corp. ET-05
  * Hitachi HD38750A08 MCU
  * green VFD display Emix-106, with bezel overlay

  NOTE!: MESS external artwork is recommended

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
	// R1x-R3x, D0-D3: vfd matrix plate
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

	// D0-D3: plates (update display there)
	plate_w(space, 3 + HMCS40_PORT_R1X, data & 0xf);
}

READ8_MEMBER(bambball_state::input_r)
{
	// R0x: inputs
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_16WAY // "
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
	MCFG_CPU_ADD("maincpu", HD38750, 400000) // approximation - RC osc.
	MCFG_HMCS40_READ_R_CB(0, READ8(bambball_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(bambball_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(bambball_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(bambball_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(bambball_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

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

  NOTE!: MESS external artwork is recommended

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
	// R0x-R3x, D0-D3: vfd matrix plate
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
	return read_inputs(5);
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
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(packmon_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(packmon_state, grid_w))
	MCFG_HMCS40_READ_D_CB(READ16(packmon_state, input_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Zackman
  * Hitachi HD38820A49 MCU
  * cyan/red/yellow VFD display Futaba DM-53Z 3E, with color overlay

  NOTE!: MESS external artwork is recommended

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
	// R0x-R6x,D0,D1: vfd matrix plate
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
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Alien Attack (manufactured in Taiwan)
  * Hitachi HD38800A25 MCU
  * cyan/red VFD display Futaba DM-19Z 1J

  NOTE!: MESS external artwork is recommended

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
	// R0x-R3x, D0-D3: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT32 plate = BITSWAP16(m_plate,11,9,8,10,7,2,0,1,3,4,5,6,12,13,14,15) | (m_plate & 0xf0000);
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
	return read_inputs(7);
}


// config

static INPUT_PORTS_START( alnattck )
	PORT_START("IN.0") // D5 D7
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Move")

	PORT_START("IN.6") // D13 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
INPUT_PORTS_END

static MACHINE_CONFIG_START( alnattck, alnattck_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(alnattck_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(alnattck_state, grid_w))
	MCFG_HMCS40_READ_D_CB(READ16(alnattck_state, input_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Donkey Kong (manufactured in Taiwan)
  * PCB label Coleco Rev C 75790 DK
  * Hitachi HD38820A45 MCU
  * cyan/red VFD display Futaba DM-47ZK 2K, with color overlay

  NOTE!: MESS external artwork is recommended

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
	virtual void machine_start();
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
	UINT32 plate = BITSWAP32(m_plate,31,30,29,24,0,16,8,1,23,17,9,2,18,10,25,27,26,3,15,27,11,11,14,22,6,13,21,5,19,12,20,4);
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
	PORT_START("IN.0") // port D
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x7ff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.1") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)0)
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
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(4, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(5, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(6, WRITE8(cdkong_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(cdkong_state, grid_w))
	MCFG_HMCS40_READ_D_CB(IOPORT("IN.0"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("speaker_decay", cdkong_state, speaker_decay_sim, attotime::from_msec(CDKONG_SPEAKER_DECAY))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

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

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class cgalaxn_state : public hh_hmcs40_state
{
public:
	cgalaxn_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_hmcs40_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_READ8_MEMBER(input_r);
};

// handlers

void cgalaxn_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,1,2,0,11,10,9,8,7,6,5,4,3);
	UINT16 plate = BITSWAP16(m_plate,15,14,5,4,3,2,1,0,7,11,12,9,8,10,6,13);
	display_matrix(14, 12, plate, grid);
}

WRITE8_MEMBER(cgalaxn_state::grid_w)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1: speaker on?

	// D2-D15: vfd matrix plate
	m_plate = data >> 2 & 0x3fff;
	prepare_display();
}

WRITE16_MEMBER(cgalaxn_state::plate_w)
{
	// R10,R11: input mux
	if (offset == HMCS40_PORT_R1X)
		m_inp_mux = data & 3;

	// R1x-R3x: vfd matrix grid
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
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
	PORT_CONFNAME( 0x01, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)0)

	PORT_START("IN.3") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)1)
INPUT_PORTS_END

static MACHINE_CONFIG_START( cgalaxn, cgalaxn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.
	MCFG_HMCS40_READ_R_CB(0, READ8(cgalaxn_state, input_r))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(cgalaxn_state, grid_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(cgalaxn_state, grid_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(cgalaxn_state, grid_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(cgalaxn_state, plate_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Pac-Man (manufactured in Taiwan)
  * PCB label Coleco 75690
  * Hitachi HD38820A28/29 MCU
  * cyan/red VFD display Futaba DM-34Z 2A, with color overlay

  known releases:
  - Japan: Super Pack Monster, by Gakken
  - USA: Pac-Man, published by Coleco (name-license from Midway)

  Select game mode on start:
  - P1 Right: Pac-Man (default game)
  - P1 Left:  Head-to-Head Pac-Man (2-player mode)
  - P1 Up:    Eat & Run
  - P1 Down:  Demo

  BTANB note: 1st version doesn't show the whole maze on power-on

  NOTE!: MESS external artwork is recommended

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
	// R1x-R6x, D1,D2: vfd matrix plate
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
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Ms. Pac-Man (manufactured in Taiwan)
  * PCB label Coleco 911171
  * Hitachi HD38820A61 MCU
  * cyan/red VFD display Futaba DM-60Z 3I, with color overlay

  Select game mode on start:
  - P1 Left:  Ms. Pac-Man (default game)
  - P1 Down:  Head-to-Head Ms. Pac-Man (2-player mode)
  - P1 Up:    Demo

  BTANB note: in demo-mode, she hardly ever walks to the upper two rows

  NOTE!: MESS external artwork is recommended

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
	// R1x-R6x, D0,D1: vfd matrix plate
	int shift = (offset - HMCS40_PORT_R1X) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT16 grid = BITSWAP16(m_grid,15,14,13,11,10,9,8,7,6,5,4,3,2,1,0,1);
	UINT64 plate = BIT(m_plate,15)<<32 | BITSWAP32(m_plate,14,13,12,4,5,6,7,24,23,25,22,21,20,13,24,3,19,14,12,11,24,2,10,8,7,25,0,9,1,18,17,16);
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

	// D0,D1: plate 11+17,6+22 (update display there)
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
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Galaxian 2 (manufactured in Japan)
  * Hitachi HD38820A13 MCU
  * cyan/red/green VFD display Futaba DM-20

  NOTE!: MESS external artwork is recommended

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
	m_grid = data >> 1;
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // separate directional buttons, hence 16way
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_16WAY // "

	PORT_START("IN.1") // D2 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_COCKTAIL PORT_16WAY // separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_COCKTAIL PORT_16WAY // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_COCKTAIL PORT_16WAY // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY // "

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
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Pac Man 2 (manufactured in Japan)
  * Hitachi HD38820A23 MCU
  * cyan/red VFD display Futaba DM-28Z 1G

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class epacman2_state : public egalaxn2_state
{
public:
	epacman2_state(const machine_config &mconfig, device_type type, const char *tag)
		: egalaxn2_state(mconfig, type, tag)
	{ }
};

// handlers are identical to Galaxian 2, so we can use those

// config

static INPUT_PORTS_START( epacman2 )
	PORT_START("IN.0") // D1 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_16WAY // separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_16WAY // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_16WAY // "

	PORT_START("IN.1") // D2 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_COCKTAIL PORT_16WAY // separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_COCKTAIL PORT_16WAY // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_COCKTAIL PORT_16WAY // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY // "

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

static MACHINE_CONFIG_START( epacman2, epacman2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
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

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Q*Bert
  * Hitachi HD38820A70 MCU
  * cyan/red/green/darkgreen VFD display Itron CP5137

  NOTE!: MESS external artwork is recommended

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
	// R0x-R6x,D8: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT32 plate = BITSWAP32(m_plate,31,30,24,25,26,27,28,15,14,29,13,12,11,10,9,8,7,6,5,4,3,2,1,0,16,17,18,19,20,21,22,23);
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
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.
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

	/* no video! */

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

  NOTE!: MESS external artwork is recommended

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
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(kingman_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(kingman_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

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

  NOTE!: MESS external artwork is recommended

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL) PORT_16WAY // separate directional buttons, hence 16way

	PORT_START("IN.1") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL) PORT_16WAY // "

	PORT_START("IN.2") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL) PORT_16WAY // "

	PORT_START("IN.3") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, NULL) PORT_16WAY // "

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, (void *)0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(tmtron_state::input_changed)
{
	update_int1();
}


static MACHINE_CONFIG_START( tmtron, tmtron_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(tmtron_state, plate_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(tmtron_state, grid_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

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


ROM_START( packmon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a27", 0x0000, 0x1000, CRC(86e09e84) SHA1(ac7d3c43667d5720ca513f8ff51d146d9f2af124) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( zackman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a49", 0x0000, 0x1000, CRC(b97f5ef6) SHA1(7fe20e8107361caf9ea657e504be1f8b10b8b03f) )
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



/*    YEAR  NAME       PARENT COMPAT MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, bambball,  0,        0, bambball, bambball, driver_device, 0, "Bambino", "Basketball - Dribble Away", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1981, packmon,   0,        0, packmon,  packmon,  driver_device, 0, "Bandai", "Packri Monster", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1983, zackman,   0,        0, zackman,  zackman,  driver_device, 0, "Bandai", "Zackman", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1981, alnattck,  0,        0, alnattck, alnattck, driver_device, 0, "Coleco", "Alien Attack", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1982, cdkong,    0,        0, cdkong,   cdkong,   driver_device, 0, "Coleco", "Donkey Kong (Coleco)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_IMPERFECT_SOUND )
CONS( 1982, cgalaxn,   0,        0, cgalaxn,  cgalaxn,  driver_device, 0, "Coleco", "Galaxian (Coleco)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1981, cpacman,   0,        0, cpacman,  cpacman,  driver_device, 0, "Gakken (Coleco license)", "Pac-Man (Coleco, Rev. 29)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK ) // original version is Super Puck Monster, by Gakken
CONS( 1981, cpacmanr1, cpacman,  0, cpacman,  cpacman,  driver_device, 0, "Gakken (Coleco license)", "Pac-Man (Coleco, Rev. 28)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK ) // "
CONS( 1983, cmspacmn,  0,        0, cmspacmn, cmspacmn, driver_device, 0, "Coleco", "Ms. Pac-Man (Coleco)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )

CONS( 1981, egalaxn2,  0,        0, egalaxn2, egalaxn2, driver_device, 0, "Entex", "Galaxian 2 (Entex)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1981, epacman2,  0,        0, epacman2, epacman2, driver_device, 0, "Entex", "Pac Man 2 (Entex)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1983, pbqbert,   0,        0, pbqbert,  pbqbert,  driver_device, 0, "Parker Brothers", "Q*Bert (Parker Brothers)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )

CONS( 1982, kingman,   0,        0, kingman,  kingman,  driver_device, 0, "Tomy", "Kingman", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
CONS( 1984, tmtron,    0,        0, tmtron,   tmtron,   driver_device, 0, "Tomy", "Tron (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
