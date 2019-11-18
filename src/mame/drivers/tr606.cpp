// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Roland TR-606 Drumatix, early 1982
  * NEC uCOM-43 MCU, labeled D650C 128
  * 2*uPD444C 1024x4 Static CMOS SRAM
  * board is packed with discrete components

  TODO:
  - still too much to list here

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "machine/timer.h"

#include "tr606.lh"


class tr606_state : public driver_device
{
public:
	tr606_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void tr606(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<ucom4_cpu_device> m_maincpu;

	TIMER_DEVICE_CALLBACK_MEMBER(tp3_clock) { m_maincpu->set_input_line(0, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(tp3_clear) { m_maincpu->set_input_line(0, CLEAR_LINE); }
};

void tr606_state::machine_start()
{
	// zerofill

	// register for savestates
}

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

void tr606_state::tr606(machine_config &config)
{
	/* basic machine hardware */
	NEC_D650(config, m_maincpu, TP2_HZ);

	timer_device &tp3_clock(TIMER(config, "tp3_clock"));
	tp3_clock.configure_periodic(FUNC(tr606_state::tp3_clock), TP3_PERIOD);
	tp3_clock.set_start_delay(TP3_PERIOD - TP3_LOW);
	TIMER(config, "tp3_clear").configure_periodic(FUNC(tr606_state::tp3_clear), TP3_PERIOD);

	/* video hardware */
	config.set_default_layout(layout_tr606);

	/* sound hardware */
	// discrete...
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tr606 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-128.ic4", 0x0000, 0x0800, CRC(eee88f80) SHA1(ae605ce2b95adc2e0bacde3cd7ed0f39ac88b981) )
ROM_END


CONS( 1982, tr606, 0, 0, tr606, tr606, tr606_state, empty_init, "Roland", "TR-606 Drumatix", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
