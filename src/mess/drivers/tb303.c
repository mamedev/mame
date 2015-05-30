// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Roland TB-303 Bass Line, 1982, designed by Tadao Kikumoto
  * NEC uCOM-43 MCU, labeled D650C 133
  * 3*uPD444C 1024x4 Static CMOS SRAM
  * board is packed with discrete components

  x

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"

#include "tb303.lh"


class tb303_state : public driver_device
{
public:
	tb303_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_t3_off_timer(*this, "t3_off")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_t3_off_timer;

	TIMER_DEVICE_CALLBACK_MEMBER(t3_clock);
	TIMER_DEVICE_CALLBACK_MEMBER(t3_off);

	virtual void machine_start();
};


// T2 to MCU CLK: LC circuit, stable sine wave, 2.2us interval
#define TB303_T2_CLOCK_HZ   454545 /* in hz */

// T3 to MCU _INT: square wave, 1.8ms interval, short duty cycle
#define TB303_T3_CLOCK      attotime::from_usec(1800)
#define TB303_T3_OFF        (TB303_T3_CLOCK / 8)

TIMER_DEVICE_CALLBACK_MEMBER(tb303_state::t3_off)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(tb303_state::t3_clock)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
	m_t3_off_timer->adjust(TB303_T3_OFF);
}




static INPUT_PORTS_START( tb303 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tb303_state::machine_start()
{
}


static MACHINE_CONFIG_START( tb303, tb303_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D650, TB303_T2_CLOCK_HZ)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("t3_clock", tb303_state, t3_clock, TB303_T3_CLOCK)
	MCFG_TIMER_START_DELAY(TB303_T3_CLOCK)
	MCFG_TIMER_DRIVER_ADD("t3_off", tb303_state, t3_off)

	MCFG_DEFAULT_LAYOUT(layout_tb303)

	/* no video! */

	/* sound hardware */
	// discrete...
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tb303 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-133.ic8", 0x0000, 0x0800, CRC(dd2f26ae) SHA1(7f5e37f38d970219dc9e5d49a20dc5335a5c0b30) )
ROM_END


CONS( 1982, tb303, 0, 0, tb303, tb303, driver_device, 0, "Roland", "TB-303", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
