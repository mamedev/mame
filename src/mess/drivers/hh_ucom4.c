// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  NEC uCOM4 MCU handhelds


***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "tmpacman.lh"


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
	optional_ioport_array<4> m_inp_matrix; // max 4
	optional_device<speaker_sound_device> m_speaker;
	
	// misc common
	UINT16 m_inp_mux;

	virtual void machine_start();
	virtual void machine_reset();

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

	// game-specific handlers
	
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


void hh_ucom4_state::machine_reset()
{
}

/***************************************************************************

  Helper Functions

***************************************************************************/

// LED segments
#if 0
enum
{
	lA = 0x01,
	lB = 0x02,
	lC = 0x04,
	lD = 0x08,
	lE = 0x10,
	lF = 0x20,
	lG = 0x40,
	lDP = 0x80
};
#endif

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



/***************************************************************************

  Minidrivers (I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Tomytronic Pac-Man (manufactured in Japan)
  * boards are labeled TN-08 2E108E01
  * NEC uCOM-43 MCU, labeled D553C 160
  * cyan/red/green VFD display NEC FIP8AM18T
  * bright yellow round casing

  known releases:
  - Japan: Puck Man
  - USA: Pac Man
  - UK: Puckman (Tomy), and also as Munchman, published by Grandstand
  - Australia: Pac Man-1, published by Futuretronics

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

  Game driver(s)

***************************************************************************/

ROM_START( tmpacman )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-160", 0x0000, 0x0800, CRC(b21a8af7) SHA1(e3122be1873ce76a4067386bf250802776f0c2f9) )
ROM_END


CONS( 1981, tmpacman, 0, 0, tmpacman, tmpacman, driver_device, 0, "Tomy", "Pac Man (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
