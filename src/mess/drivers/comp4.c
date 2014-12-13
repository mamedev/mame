// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Milton Bradley Comp IV
  * TMC0904NL CP0904A (die labeled 4A0970D-04A)
  
  This is small tabletop Mastermind game; a code-breaking game where the player
  needs to find out the correct sequence of colours (numbers in our case).
  It is known as Logic 5 in Europe, and as Pythaligoras in Japan.
  
  Press the R key to start, followed by a set of unique numbers and E.
  Refer to the official manual for more information.


  TODO:
  - MCU clock is unknown

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "comp4.lh"

// master clock is unknown, the value below is an approximation
#define MASTER_CLOCK (250000)


class comp4_state : public driver_device
{
public:
	comp4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<3> m_button_matrix;

	UINT16 m_o;

	UINT16 m_leds_state;
	UINT8 m_leds_decay[0x10];

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
#define LEDS_DECAY_TIME 2

void comp4_state::leds_update()
{
	for (int i = 0; i < 0x10; i++)
	{
		// turn on powered leds
		if (m_leds_state >> i & 1)
			m_leds_decay[i] = LEDS_DECAY_TIME;
		
		// send to output
		output_set_lamp_value(i, (m_leds_decay[i] != 0) ? 1 : 0);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(comp4_state::leds_decay_tick)
{
	// slowly turn off unpowered leds
	for (int i = 0; i < 0x10; i++)
		if (!(m_leds_state >> i & 1) && m_leds_decay[i])
			m_leds_decay[i]--;
	
	leds_update();
}



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(comp4_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 3; i++)
		if (m_o >> (i+1) & 1)
			k |= m_button_matrix[i]->read();
	
	return k;
}

WRITE16_MEMBER(comp4_state::write_r)
{
	// LEDs:
	// R4    R9
	// R10!  R8
	// R2    R7
	// R1    R6
	// R0    R5
	m_leds_state = data;
	leds_update();
}

WRITE16_MEMBER(comp4_state::write_o)
{
	// O0: LEDs common (always writes 1)
	// O1-O3: input mux
	// other bits: N/C
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( comp4 )
	PORT_START("IN.0") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("R")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.1") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.2") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void comp4_state::machine_start()
{
	// zerofill
	m_leds_state = 0;
	memset(m_leds_decay, 0, sizeof(m_leds_decay));

	m_o = 0;
	
	// register for savestates
	save_item(NAME(m_leds_state));
	save_item(NAME(m_leds_decay));

	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( comp4, comp4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(comp4_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(comp4_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(comp4_state, write_r))
	
	MCFG_TIMER_DRIVER_ADD_PERIODIC("leds_decay", comp4_state, leds_decay_tick, attotime::from_msec(10))

	MCFG_DEFAULT_LAYOUT(layout_comp4)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( comp4 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0904nl_cp0904a", 0x0000, 0x0400, CRC(6233ee1b) SHA1(738e109b38c97804b4ec52bed80b00a8634ad453) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_comp4_mpla.pla", 0, 860, CRC(ee9d7d9e) SHA1(25484e18f6a07f7cdb21a07220e2f2a82fadfe7b) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_comp4_opla.pla", 0, 352, CRC(a0f887d1) SHA1(3c666663d484d5bed81e1014f8715aab8a3d489f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_comp4_spla.pla", 0, 157, CRC(e5bddd90) SHA1(4b1c6512c70e5bcd23c2dbf0c88cd8aa2c632a10) )
ROM_END


CONS( 1977, comp4, 0, 0, comp4, comp4, driver_device, 0, "Milton Bradley", "Comp IV", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
