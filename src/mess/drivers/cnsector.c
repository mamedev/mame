// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Code Name: Sector
  * MP0905BNL ZA0379 (die labeled 0970F-05B)
  
  This is a tabletop submarine pursuit game. A grid board and small toy
  boats are used to remember your locations (a Paint app should be ok too).
  Refer to the official manual for more information, it is not a simple game.


  TODO:
  - MCU clock is unknown

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "cnsector.lh"

// master clock is unknown, the value below is an approximation
#define MASTER_CLOCK (250000)


class cnsector_state : public driver_device
{
public:
	cnsector_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<5> m_button_matrix;

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

void cnsector_state::leds_update()
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
			output_set_digit_value(i, active_state[i]);
	
	memcpy(m_leds_cache, active_state, sizeof(m_leds_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(cnsector_state::leds_decay_tick)
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

READ8_MEMBER(cnsector_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 5; i++)
		if (m_o >> i & 1)
			k |= m_button_matrix[i]->read();
	
	return k;
}

WRITE16_MEMBER(cnsector_state::write_r)
{
	// R0-R5: select digit (right-to-left)
	for (int i = 0; i < 6; i++)
		m_leds_state[i] = (data >> i & 1) ? m_o : 0;
	leds_update();

	// R6-R9: direction leds
	for (int i = 6; i < 10; i++)
		output_set_lamp_value(i - 6, data >> i & 1);
}

WRITE16_MEMBER(cnsector_state::write_o)
{
	// O0-O4: input mux
	// O0-O7: digit segments
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cnsector )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Next Ship")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_NAME("Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Range")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("Aim")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME("Evasive Sub") // expert button
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("Recall")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("Sub Finder") // expert button
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_NAME("Slower")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_NAME("Teach Mode")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("Faster")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Move Ship")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void cnsector_state::machine_start()
{
	// zerofill
	memset(m_leds_state, 0, sizeof(m_leds_state));
	memset(m_leds_cache, 0, sizeof(m_leds_cache));
	memset(m_leds_decay, 0, sizeof(m_leds_decay));

	m_o = 0;
	
	// register for savestates
	save_item(NAME(m_leds_state));
	save_item(NAME(m_leds_cache));
	save_item(NAME(m_leds_decay));

	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( cnsector, cnsector_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(cnsector_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(cnsector_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(cnsector_state, write_r))
	
	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", cnsector_state, leds_decay_tick, attotime::from_msec(10))

	MCFG_DEFAULT_LAYOUT(layout_cnsector)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cnsector )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0905bnl_za0379", 0x0000, 0x0400, CRC(201036e9) SHA1(b37fef86bb2bceaf0ac8bb3745b4702d17366914) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_cnsector_mpla.pla", 0, 860, CRC(059f5bb4) SHA1(2653766f9fd74d41d44013bb6f54c0973a6080c9) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_cnsector_opla.pla", 0, 352, CRC(7c0bdcd6) SHA1(dade774097e8095dca5deac7b2367d0c701aca51) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_cnsector_spla.pla", 0, 157, CRC(56c37a4f) SHA1(18ecc20d2666e89673739056483aed5a261ae927) )
ROM_END


CONS( 1977, cnsector, 0, 0, cnsector, cnsector, driver_device, 0, "Parker Brothers", "Code Name: Sector", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
