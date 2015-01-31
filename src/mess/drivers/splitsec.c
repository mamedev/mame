// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Split Second
  * TMS1400NLL MP7314-N2 (die labeled MP7314)

  This is an electronic handheld reflex gaming device, it's straightforward
  to use. The included mini-games are:
  1, 2, 3: Mad Maze*
  4, 5: Space Attack*
  6: Auto Cross
  7: Stomp
  8: Speedball

  *: higher number indicates higher difficulty


  TODO:
  - MCU clock is unknown

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "splitsec.lh"

// The master clock is a single stage RC oscillator: R=24K, C=100pf,
// according to the TMS 1000 series data manual this is around 375kHz.
// However, this sounds too low-pitched and runs too slow when compared
// to recordings, maybe the RC osc curve is different for TMS1400?

// so for now, the value below is an approximation
#define MASTER_CLOCK (485000)


class splitsec_state : public driver_device
{
public:
	splitsec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<2> m_button_matrix;
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

	virtual void machine_start();
};



/***************************************************************************

  LEDs

***************************************************************************/

// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

// decay time, in steps of 10ms
#define LEDS_DECAY_TIME 4

/* display layout, where number xy is lamp R(x),O(y)

       00    02    04
    10 01 12 03 14 05 16
       11    13    15
    20 21 22 23 24 25 26
       31    33    35
    30 41 32 43 34 45 36
       51    53    55
    40 61 42 63 44 65 46
       71    73    75
    50 60 52 62 54 64 56
       70    72    74
*/

void splitsec_state::leds_update()
{
	UINT16 active_state[0x10];

	for (int i = 0; i < 0x10; i++)
	{
		// update current state
		if (m_r >> i & 1)
			m_leds_state[i] = m_o;

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
			for (int j = 0; j < 8; j++)
				output_set_lamp_value(i*10 + j, active_state[i] >> j & 1);
		}

	memcpy(m_leds_cache, active_state, sizeof(m_leds_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(splitsec_state::leds_decay_tick)
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

READ8_MEMBER(splitsec_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 2; i++)
		if (m_r >> (i+9) & 1)
			k |= m_button_matrix[i]->read();

	return k;
}

WRITE16_MEMBER(splitsec_state::write_r)
{
	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R9,R10: input mux
	// R0-R7: led columns
	m_r = data;
	leds_update();
}

WRITE16_MEMBER(splitsec_state::write_o)
{
	// O0-O6: led rows
	// O7: N/C
	m_o = data;
	leds_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( splitsec )
	PORT_START("IN.0") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 4 separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Select")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void splitsec_state::machine_start()
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


static MACHINE_CONFIG_START( splitsec, splitsec_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(splitsec_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(splitsec_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(splitsec_state, write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", splitsec_state, leds_decay_tick, attotime::from_msec(10))

	MCFG_DEFAULT_LAYOUT(layout_splitsec)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( splitsec )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tms1400nll_mp7314", 0x0000, 0x1000, CRC(e94b2098) SHA1(f0fc1f56a829252185592a2508740354c50bedf8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_splitsec_opla.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END


CONS( 1980, splitsec, 0, 0, splitsec, splitsec, driver_device, 0, "Parker Brothers", "Split Second", GAME_SUPPORTS_SAVE )
