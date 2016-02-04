// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  ** subclass of hh_ucom4_state (includes/hh_ucom4.h, drivers/hh_ucom4.cpp) **

  Roland TR-606 Drumatix, early 1982
  * NEC uCOM-43 MCU, labeled D650C 128
  * 2*uPD444C 1024x4 Static CMOS SRAM
  * board is packed with discrete components

  TODO:
  - still too much to list here

***************************************************************************/

#include "includes/hh_ucom4.h"

#include "tr606.lh"


class tr606_state : public hh_ucom4_state
{
public:
	tr606_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_ucom4_state(mconfig, type, tag)
	{ }

	TIMER_DEVICE_CALLBACK_MEMBER(tp3_clock) { m_maincpu->set_input_line(0, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(tp3_clear) { m_maincpu->set_input_line(0, CLEAR_LINE); }

	virtual void machine_start() override;
};

// TP2 to MCU CLK: LC circuit(TI S74230), stable sine wave, 2.2us interval
#define TP2_HZ      454545

// MCU interrupt timing is same as in TB303
// TP3 to MCU _INT: square wave, 1.8ms interval, short duty cycle
#define TP3_PERIOD  attotime::from_usec(1800)
#define TP3_LOW     (TP3_PERIOD / 8)


/***************************************************************************

  I/O

***************************************************************************/



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( tr606 )

INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tr606_state::machine_start()
{
	hh_ucom4_state::machine_start();

	// zerofill

	// register for savestates
}

static MACHINE_CONFIG_START( tr606, tr606_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D650, TP2_HZ)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("tp3_clock", tr606_state, tp3_clock, TP3_PERIOD)
	MCFG_TIMER_START_DELAY(TP3_PERIOD - TP3_LOW)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("tp3_clear", tr606_state, tp3_clear, TP3_PERIOD)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_ucom4_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tr606)

	/* sound hardware */
	// discrete...
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tr606 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-128.ic4", 0x0000, 0x0800, CRC(eee88f80) SHA1(ae605ce2b95adc2e0bacde3cd7ed0f39ac88b981) )
ROM_END


CONS( 1982, tr606, 0, 0, tr606, tr606, driver_device, 0, "Roland", "TR-606 Drumatix", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
