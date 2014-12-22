// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Stop Thief
  * TMS0980NLL MP6101B (die labeled 0980B-01A)
  
  Stop Thief is actually a board game, the electronic device emulated here
  (called Electronic Crime Scanner) is an accessory. To start a game, press
  the ON button. Otherwise, it is in test-mode where you can hear all sounds.


  TODO:
  - MCU clock is unknown
  - stopthiep: unable to start a game (may be intentional?)

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "stopthie.lh"

// master clock is unknown, the value below is an approximation
#define MASTER_CLOCK (425000)


class stopthief_state : public driver_device
{
public:
	stopthief_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<3> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_o;
	bool m_power_on;

	UINT16 m_leds_state[0x10];
	UINT16 m_leds_cache[0x10];
	UINT8 m_leds_decay[0x100];

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	DECLARE_INPUT_CHANGED_MEMBER(power_button);
	DECLARE_WRITE_LINE_MEMBER(auto_power_off);

	TIMER_DEVICE_CALLBACK_MEMBER(leds_decay_tick);
	void leds_update();

	virtual void machine_reset();
	virtual void machine_start();
};



/***************************************************************************

  LEDs

***************************************************************************/

// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

// decay time, in steps of 10ms
#define LEDS_DECAY_TIME 4

void stopthief_state::leds_update()
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
			int ds = (m_power_on && m_leds_decay[di] != 0) ? 1 : 0;
			active_state[i] |= (ds << j);
		}
	}
	
	// on difference, send to output
	for (int i = 0; i < 0x10; i++)
		if (m_leds_cache[i] != active_state[i])
			output_set_digit_value(i, active_state[i]);
	
	memcpy(m_leds_cache, active_state, sizeof(m_leds_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(stopthief_state::leds_decay_tick)
{
	// slowly turn off unpowered leds
	for (int i = 0; i < 0x100; i++)
		if (!(m_leds_state[i & 0xf] >> (i>>4) & 1) && m_leds_decay[i])
			m_leds_decay[i]--;
	
	leds_update();
}



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(stopthief_state::read_k)
{
	// the Vss row is always on
	UINT8 k = m_button_matrix[2]->read();

	// read selected button rows
	for (int i = 0; i < 2; i++)
	{
		const int ki[2] = { 0, 6 };
		if (m_o >> ki[i] & 1)
			k |= m_button_matrix[i]->read();
	}

	return k;
}

WRITE16_MEMBER(stopthief_state::write_r)
{
	// R0-R2: select digit
	UINT8 o = BITSWAP8(m_o,3,5,2,1,4,0,6,7) & 0x7f;
	for (int i = 0; i < 3; i++)
		m_leds_state[i] = (data >> i & 1) ? o : 0;
	
	leds_update();
	
	// R3-R8: speaker on
	m_speaker->level_w((data & 0x1f8 && m_o & 8) ? 1 : 0);
}

WRITE16_MEMBER(stopthief_state::write_o)
{
	// O0,O6: input mux
	// O3: speaker out
	// O0-O2,O4-O7: led segments A-G
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(stopthief_state::power_button)
{
	m_power_on = (bool)(FPTR)param;
	m_maincpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}

/* physical button layout and labels is like this:

    [1] [2] [OFF]
    [3] [4] [ON]
    [5] [6] [T, TIP]
    [7] [8] [A, ARREST]
    [9] [0] [C, CLUE]
*/

static INPUT_PORTS_START( stopthief )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.1") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.2") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, stopthief_state, power_button, (void *)true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Tip")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_NAME("Arrest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("Clue")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, stopthief_state, power_button, (void *)false)
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

WRITE_LINE_MEMBER(stopthief_state::auto_power_off)
{
	// TMS0980 auto power-off opcode
	if (state)
	{
		m_power_on = false;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}


void stopthief_state::machine_reset()
{
	m_power_on = true;
}

void stopthief_state::machine_start()
{
	// zerofill
	memset(m_leds_state, 0, sizeof(m_leds_state));
	memset(m_leds_cache, 0, sizeof(m_leds_cache));
	memset(m_leds_decay, 0, sizeof(m_leds_decay));

	m_o = 0;
	m_power_on = false;

	// register for savestates
	save_item(NAME(m_leds_state));
	save_item(NAME(m_leds_cache));
	save_item(NAME(m_leds_decay));

	save_item(NAME(m_o));
	save_item(NAME(m_power_on));
}


static MACHINE_CONFIG_START( stopthief, stopthief_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0980, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(stopthief_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(stopthief_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(stopthief_state, write_r))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(stopthief_state, auto_power_off))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", stopthief_state, leds_decay_tick, attotime::from_msec(10))
	
	MCFG_DEFAULT_LAYOUT(layout_stopthie)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( stopthie )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tms0980nll_mp6101b", 0x0000, 0x1000, CRC(8bde5bb4) SHA1(8c318fcce67acc24c7ae361f575f28ec6f94665a) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_default_mpla.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthie_opla.pla", 0, 352, CRC(50337a48) SHA1(4a9ea62ed797a9ac5190eec3bb6ebebb7814628c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_stopthie_spla.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END

ROM_START( stopthiep )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "us4341385", 0x0000, 0x1000, CRC(07aec38a) SHA1(0a3d0956495c0d6d9ea771feae6c14a473a800dc) ) // from patent US4341385, data should be correct (it included checksums)

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_default_mpla.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthie_opla.pla", 0, 352, CRC(50337a48) SHA1(4a9ea62ed797a9ac5190eec3bb6ebebb7814628c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_stopthie_spla.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END


CONS( 1979, stopthie,  0,        0, stopthief, stopthief, driver_device, 0, "Parker Brothers", "Stop Thief (Electronic Crime Scanner)", GAME_SUPPORTS_SAVE )
CONS( 1979, stopthiep, stopthie, 0, stopthief, stopthief, driver_device, 0, "Parker Brothers", "Stop Thief (Electronic Crime Scanner) (prototype)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
