// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Mitsubishi MELPS 4 MCU tabletops/handhelds or other simple devices,
  most of them are VFD electronic games/toys.

***************************************************************************/

#include "emu.h"
#include "cpu/melps4/m58846.h"
#include "sound/speaker.h"

#include "hh_melps4_test.lh" // common test-layout - use external artwork


class hh_melps4_state : public driver_device
{
public:
	hh_melps4_state(const machine_config &mconfig, device_type type, const char *tag)
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
	optional_ioport_array<4> m_inp_matrix; // max 4
	optional_device<speaker_sound_device> m_speaker;

	// misc common
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

void hh_melps4_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

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

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_melps4_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_melps4_state::display_update()
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

TIMER_DEVICE_CALLBACK_MEMBER(hh_melps4_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_melps4_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_melps4_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	display_update();
}


UINT8 hh_melps4_state::read_inputs(int columns)
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

  Coleco Frogger (manufactured in Japan)
  * PCB label Coleco Frogger Code No. 01-81543, KS-003282 Japan
  * Mitsubishi M58846-701P MCU
  * cyan/red/green VFD display Itron CP5090GLR R1B, with partial color overlay

  NOTE!: MESS external artwork is recommended

***************************************************************************/

class cfrogger_state : public hh_melps4_state
{
public:
	cfrogger_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_melps4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	DECLARE_READ16_MEMBER(input_r);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
};

// handlers

void cfrogger_state::prepare_display()
{
	UINT16 grid = BITSWAP16(m_grid,15,14,13,12,0,1,2,3,4,5,6,7,8,9,10,11);
	UINT16 plate = BITSWAP16(m_plate,12,4,13,5,14,6,15,7,3,11,2,10,1,9,0,8);
	display_matrix(16, 12, plate, grid);
}

WRITE8_MEMBER(cfrogger_state::plate_w)
{
	// Sx,Fx,Gx: vfd matrix plate
	int mask = (offset == MELPS4_PORTS) ? 0xff : 0xf; // port S is 8-bit
	int shift = (offset == MELPS4_PORTS) ? 0 : (offset + 1) * 4;
	m_plate = (m_plate & ~(mask << shift)) | (data << shift);
	prepare_display();

	// F0,F1: input mux
	m_inp_mux = m_plate >> 8 & 3;
}

WRITE16_MEMBER(cfrogger_state::grid_w)
{
	// D0-D11: vfd matrix grid
	m_grid = data;
	prepare_display();
}

WRITE_LINE_MEMBER(cfrogger_state::speaker_w)
{
	// T: speaker out
	m_speaker->level_w(state);
}

READ16_MEMBER(cfrogger_state::input_r)
{
	// K0,K1: multiplexed inputs
	// K2: N/C
	// K3: fixed input
	return (m_inp_matrix[2]->read() & 8) | (read_inputs(2) & 3);
}


// config

static INPUT_PORTS_START( cfrogger )
	PORT_START("IN.0") // F0 port K
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // F1 port K
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN.2") // K3
	PORT_CONFNAME( 0x08, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.3") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, cfrogger_state, reset_button, NULL)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(cfrogger_state::reset_button)
{
	// reset button is directly tied to MCU reset pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}


static MACHINE_CONFIG_START( cfrogger, cfrogger_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M58846, XTAL_600kHz)
	MCFG_MELPS4_READ_K_CB(READ16(cfrogger_state, input_r))
	MCFG_MELPS4_WRITE_S_CB(WRITE8(cfrogger_state, plate_w))
	MCFG_MELPS4_WRITE_F_CB(WRITE8(cfrogger_state, plate_w))
	MCFG_MELPS4_WRITE_G_CB(WRITE8(cfrogger_state, plate_w))
	MCFG_MELPS4_WRITE_D_CB(WRITE16(cfrogger_state, grid_w))
	MCFG_MELPS4_WRITE_T_CB(WRITELINE(cfrogger_state, speaker_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_melps4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_melps4_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cfrogger )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m58846-701p", 0x0000, 0x1000, CRC(ba52a242) SHA1(7fa53b617f4bb54be32eb209e9b88131e11cb518) )
ROM_END



/*    YEAR  NAME      PARENT COMPAT MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1981, cfrogger, 0,        0, cfrogger, cfrogger, driver_device, 0, "Coleco", "Frogger (Coleco)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK )
