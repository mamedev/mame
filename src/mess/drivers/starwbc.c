// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Kenner Star Wars - Electronic Battle Command
  * TMS1100 MCU, labeled MP3438A
  
  This is a small tabletop space-dogfighting game. To start the game,
  press BASIC/INTER/ADV and enter P#(number of players), then
  START TURN. Refer to the official manual for more information.

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "starwbc.lh"

// master clock is a single stage RC oscillator: R=51K, C=47pf,
// according to the TMS 1000 series data manual this is around 350kHz
#define MASTER_CLOCK (350000)


class starwbc_state : public driver_device
{
public:
	starwbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<5> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_r;
	UINT16 m_o;

	UINT16 m_leds_state[0x10];
	UINT16 m_leds_cache[0x10];
	UINT8 m_leds_decay[0x100];

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);
	
	TIMER_DEVICE_CALLBACK_MEMBER(leds_decay_tick);
	void leds_update();
	void prepare_and_update();

	virtual void machine_start();
};



/***************************************************************************

  LEDs

***************************************************************************/

// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

// decay time, in steps of 10ms
#define LEDS_DECAY_TIME 4

void starwbc_state::leds_update()
{
	UINT16 active_state[0x10];
	
	for (int i = 0; i < 0x10; i++)
	{
		active_state[i] = 0;
		
		for (int j = 0; j < 0x10; j++)
		{
			int di = j << 4 | i;
			
			// turn on powered leds
			if (m_leds_state[i] >> j & 1)
				m_leds_decay[di] = LEDS_DECAY_TIME;
			
			// determine active state
			int ds = (m_leds_decay[di] != 0) ? 1 : 0;
			active_state[i] |= (ds << j);
		}
	}
	
	// on difference, send to output
	for (int i = 0; i < 0x10; i++)
		if (m_leds_cache[i] != active_state[i])
		{
			output_set_digit_value(i, active_state[i]);
			
			for (int j = 0; j < 8; j++)
				output_set_lamp_value(i*10 + j, active_state[i] >> j & 1);
		}
	
	memcpy(m_leds_cache, active_state, sizeof(m_leds_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(starwbc_state::leds_decay_tick)
{
	// slowly turn off unpowered leds
	for (int i = 0; i < 0x100; i++)
		if (!(m_leds_state[i & 0xf] >> (i>>4) & 1) && m_leds_decay[i])
			m_leds_decay[i]--;
	
	leds_update();
}

void starwbc_state::prepare_and_update()
{
	UINT8 o = (m_o << 4 & 0xf0) | (m_o >> 4 & 0x0f);
	const UINT8 mask[5] = { 0x30, 0xff, 0xff, 0x7f, 0x7f };
	
	// R0,R2,R4,R6,R8
	for (int i = 0; i < 5; i++)
		m_leds_state[i*2] = (m_r >> (i*2) & 1) ? (o & mask[i]) : 0;

	leds_update();
}



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(starwbc_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 5; i++)
	{
		const int ki[5] = { 0, 1, 3, 5, 7 };
		if (m_r >> ki[i] & 1)
			k |= m_button_matrix[i]->read();
	}

	return k;
}

WRITE16_MEMBER(starwbc_state::write_r)
{
	// R0,R2,R4: select lamp row
	// R6,R8: select digit
	// R0,R1,R3,R5,R7: input mux
	// R9: piezo speaker
	m_speaker->level_w(data >> 9 & 1);
	
	m_r = data;
	prepare_and_update();
}

WRITE16_MEMBER(starwbc_state::write_o)
{
	// O0-O7: leds state
	m_o = data;
	prepare_and_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

/* physical button layout and labels is like this:

    (reconnnaissance=yellow)        (tactical reaction=green)
    [MAGNA] [ENEMY]                 [EM]       [BS]   [SCR]

    [BASIC] [INTER]    [START TURN] [END TURN] [MOVE] [FIRE]
    [ADV]   [P#]       [<]          [^]        [>]    [v]
    (game=blue)        (maneuvers=red)                       */

static INPUT_PORTS_START( starwbc )
	PORT_START("IN.0") // R0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("Basic Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("Intermediate Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("Advanced Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_P) PORT_NAME("Player Number")

	PORT_START("IN.1") // R1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Turn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("End Turn")

	PORT_START("IN.2") // R3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Magna Scan") // only used in adv. game
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("Enemy Scan") // only used in adv. game
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("Screen Up")

	PORT_START("IN.3") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("Evasive Maneuvers")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_NAME("Fire")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("Battle Stations")

	PORT_START("IN.4") // R7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void starwbc_state::machine_start()
{
	// zerofill
	memset(m_leds_state, 0, sizeof(m_leds_state));
	memset(m_leds_cache, 0, sizeof(m_leds_cache));
	memset(m_leds_decay, 0, sizeof(m_leds_decay));

	m_r = 0;
	m_o = 0;
	
	// register for savestates
	save_item(NAME(m_leds_state));
	save_item(NAME(m_leds_cache));
	save_item(NAME(m_leds_decay));

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( starwbc, starwbc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(starwbc_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(starwbc_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(starwbc_state, write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", starwbc_state, leds_decay_tick, attotime::from_msec(10))
	
	MCFG_DEFAULT_LAYOUT(layout_starwbc)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( starwbc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3438a", 0x0000, 0x0800, CRC(c12b7069) SHA1(d1f39c69a543c128023ba11cc6228bacdfab04de) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_starwbc_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_opla.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END

ROM_START( starwbcp )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "us4270755", 0x0000, 0x0800, BAD_DUMP CRC(fb3332f2) SHA1(a79ac81e239983cd699b7cfcc55f89b203b2c9ec) ) // from patent US4270755, may have errors

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_starwbc_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_opla.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END


CONS( 1979, starwbc,  0,       0, starwbc, starwbc, driver_device, 0, "Kenner", "Star Wars - Electronic Battle Command", GAME_SUPPORTS_SAVE )
CONS( 1979, starwbcp, starwbc, 0, starwbc, starwbc, driver_device, 0, "Kenner", "Star Wars - Electronic Battle Command (prototype)", GAME_SUPPORTS_SAVE )
