// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Ideal Electronic Detective
  * TMS0980NLL MP6100A (die labeled 0980B-00)
  hardware (and concept) is very similar to Parker Bros Stop Thief

  This is an electronic board game. It requires game cards with suspect info,
  and good old pen and paper to record game progress. To start the game, enter
  difficulty(1-3), then number of players(1-4), then [ENTER]. Refer to the
  manual for more information.


  TODO:
  - MCU clock is unknown

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "elecdet.lh"

// master clock is unknown, the value below is an approximation
#define MASTER_CLOCK (425000)


class elecdet_state : public driver_device
{
public:
	elecdet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<5> m_button_matrix;
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

void elecdet_state::leds_update()
{
	UINT16 active_state[0x10];

	for (int i = 0; i < 0x10; i++)
	{
		active_state[i] = 0;

		for (int j = 0; j < 0x10; j++)
		{
			int di = j << 4 | i;

			// turn on powered leds
			if (m_power_on && m_leds_state[i] >> j & 1)
				m_leds_decay[di] = LEDS_DECAY_TIME;

			// determine active state
			int ds = (m_leds_decay[di] != 0) ? 1 : 0;
			active_state[i] |= (ds << j);
		}
	}

	// on difference, send to output
	for (int i = 0; i < 0x10; i++)
		if (m_leds_cache[i] != active_state[i])
			output_set_digit_value(i, active_state[i]);

	memcpy(m_leds_cache, active_state, sizeof(m_leds_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(elecdet_state::leds_decay_tick)
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

READ8_MEMBER(elecdet_state::read_k)
{
	// the Vss row is always on
	UINT8 k = m_button_matrix[4]->read();

	// read selected button rows
	for (int i = 0; i < 4; i++)
	{
		const int ki[4] = { 0, 1, 4, 6 };
		if (m_o >> ki[i] & 1)
			k |= m_button_matrix[i]->read();
	}

	return k;
}

WRITE16_MEMBER(elecdet_state::write_r)
{
	// R0-R6: select digit
	UINT8 o = BITSWAP8(m_o,7,5,2,1,4,0,6,3) & 0x7f;
	for (int i = 0; i < 7; i++)
		m_leds_state[i] = (data >> i & 1) ? o : 0;

	leds_update();

	// R7,R8: speaker on
	m_speaker->level_w((data & 0x180 && m_o & 0x80) ? 1 : 0);
}

WRITE16_MEMBER(elecdet_state::write_o)
{
	// O0,O1,O4,O6: input mux
	// O0-O6: led segments A-G
	// O7: speaker out
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(elecdet_state::power_button)
{
	m_power_on = (bool)(FPTR)param;
	m_maincpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}

/* physical button layout and labels is like this:

    [1]   [2]   [3]   [SUSPECT]
    [4]   [5]   [6]   [PRIVATE QUESTION]
    [7]   [8]   [9]   [I ACCUSE]
                [0]   [ENTER]
    [ON]  [OFF]       [END TURN]
*/

static INPUT_PORTS_START( elecdet )
	PORT_START("IN.0") // O0 pin18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Private Question")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.1") // O1 pin17
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // O4 pin14
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("I Accuse")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.3") // O6 pin12
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Suspect")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.4") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, elecdet_state, power_button, (void *)true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("End Turn")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, elecdet_state, power_button, (void *)false)
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

WRITE_LINE_MEMBER(elecdet_state::auto_power_off)
{
	// TMS0980 auto power-off opcode
	if (state)
	{
		m_power_on = false;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}


void elecdet_state::machine_reset()
{
	m_power_on = true;
}

void elecdet_state::machine_start()
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


static MACHINE_CONFIG_START( elecdet, elecdet_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0980, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(elecdet_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(elecdet_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(elecdet_state, write_r))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(elecdet_state, auto_power_off))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", elecdet_state, leds_decay_tick, attotime::from_msec(10))

	MCFG_DEFAULT_LAYOUT(layout_elecdet)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( elecdet )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tms0980nll_mp6100a", 0x0000, 0x1000, CRC(6f396bb8) SHA1(1f104d4ca9bee0d4572be4779b7551dfe20c4f04) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_default_mpla.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_elecdet_opla.pla", 0, 352, CRC(652d19c3) SHA1(75550c2b293453b6b9efed88c8cc77195a53161f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_elecdet_spla.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END


CONS( 1979, elecdet, 0, 0, elecdet, elecdet, driver_device, 0, "Ideal", "Electronic Detective", GAME_SUPPORTS_SAVE )
