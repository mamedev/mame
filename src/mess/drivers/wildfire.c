// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Wildfire, by Bob and Holly Doyle (prototype), and Garry Kitchen
  * AMI S2150, labeled C10641

  This is an electronic handheld pinball game. It has dozens of small leds
  to create the illusion of a moving ball, and even the flippers are leds.
  A drawing of a pinball table is added as overlay.

  NOTE!: MESS external artwork is required to be able to play


  TODO:
  - no sound
  - flipper buttons aren't working correctly
  - some 7segs digits are wrong (mcu on-die decoder is customizable?)
  - MCU clock is unknown

***************************************************************************/

#include "emu.h"
#include "cpu/amis2000/amis2000.h"
#include "sound/speaker.h"

#include "wildfire.lh" // this is a test layout, external artwork is necessary

// master clock is a single stage RC oscillator: R=?K, C=?pf,
// S2150 default frequency is 850kHz
#define MASTER_CLOCK (850000)


class wildfire_state : public driver_device
{
public:
	wildfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	UINT8 m_d;
	UINT16 m_a;

	UINT16 m_leds_state[0x10];
	UINT16 m_leds_cache[0x10];
	UINT8 m_leds_decay[0x100];

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE16_MEMBER(write_a);

	TIMER_DEVICE_CALLBACK_MEMBER(leds_decay_tick);
	void leds_update();
	bool index_is_7segled(int index);

	virtual void machine_start();
};



/***************************************************************************

  LEDs

***************************************************************************/

// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

// decay time, in steps of 10ms
#define LEDS_DECAY_TIME 4

inline bool wildfire_state::index_is_7segled(int index)
{
	// first 3 A are 7segleds
	return (index < 3);
}

void wildfire_state::leds_update()
{
	UINT16 active_state[0x10];

	for (int i = 0; i < 0x10; i++)
	{
		// update current state
		m_leds_state[i] = (~m_a >> i & 1) ? m_d : 0;

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
			if (index_is_7segled(i))
				output_set_digit_value(i, BITSWAP8(active_state[i],7,0,1,2,3,4,5,6) & 0x7f);

			for (int j = 0; j < 8; j++)
				output_set_lamp_value(i*10 + j, active_state[i] >> j & 1);
		}

	memcpy(m_leds_cache, active_state, sizeof(m_leds_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(wildfire_state::leds_decay_tick)
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

READ8_MEMBER(wildfire_state::read_k)
{
	// ?
	return 0xf;
}

WRITE8_MEMBER(wildfire_state::write_d)
{
	m_d = data;
	leds_update();
}

WRITE16_MEMBER(wildfire_state::write_a)
{
	m_a = data;
	leds_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( wildfire )
	PORT_START("IN1") // I
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shooter Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void wildfire_state::machine_start()
{
	// zerofill
	memset(m_leds_state, 0, sizeof(m_leds_state));
	memset(m_leds_cache, 0, sizeof(m_leds_cache));
	memset(m_leds_decay, 0, sizeof(m_leds_decay));

	m_d = 0;
	m_a = 0;

	// register for savestates
	save_item(NAME(m_leds_state));
	save_item(NAME(m_leds_cache));
	save_item(NAME(m_leds_decay));

	save_item(NAME(m_d));
	save_item(NAME(m_a));
}


static MACHINE_CONFIG_START( wildfire, wildfire_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", AMI_S2150, MASTER_CLOCK)
	MCFG_AMI_S2000_READ_I_CB(IOPORT("IN1"))
	MCFG_AMI_S2000_READ_K_CB(READ8(wildfire_state, read_k))
	MCFG_AMI_S2000_WRITE_D_CB(WRITE8(wildfire_state, write_d))
	MCFG_AMI_S2000_WRITE_A_CB(WRITE16(wildfire_state, write_a))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", wildfire_state, leds_decay_tick, attotime::from_msec(10))

	MCFG_DEFAULT_LAYOUT(layout_wildfire)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wildfire )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "us4341385", 0x0000, 0x0400, CRC(84ac0f1f) SHA1(1e00ddd402acfc2cc267c34eed4b89d863e2144f) ) // from patent US4334679, data should be correct (it included checksums)
	ROM_CONTINUE(          0x0600, 0x0200 )
ROM_END


CONS( 1979, wildfire, 0, 0, wildfire, wildfire, driver_device, 0, "Parker Brothers", "Wildfire (prototype)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
