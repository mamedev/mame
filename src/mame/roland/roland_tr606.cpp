// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

Roland TR-606 Drumatix, early 1982

Hardware notes:
- NEC uCOM-43 MCU, labeled D650C 128
- 2*uPD444C 1024x4 Static CMOS SRAM
- board is packed with discrete components

TODO:
- everything

***************************************************************************/

#include "emu.h"

#include "cpu/ucom4/ucom4.h"
#include "machine/clock.h"


namespace {

class tr606_state : public driver_device
{
public:
	tr606_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void tr606(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<ucom4_cpu_device> m_maincpu;
};

void tr606_state::machine_start()
{
}



/***************************************************************************
    Inputs
***************************************************************************/

static INPUT_PORTS_START( tr606 )
INPUT_PORTS_END



/***************************************************************************
    Machine Configs
***************************************************************************/

void tr606_state::tr606(machine_config &config)
{
	// basic machine hardware
	NEC_D650(config, m_maincpu, 454545); // LC circuit(TI S74230), 2.2us

	auto &irq_clock(CLOCK(config, "irq_clock"));
	irq_clock.set_period(attotime::from_usec(1800)); // clock rate 1.8ms (same as tb303)
	irq_clock.set_duty_cycle(0.1); // short duty cycle
	irq_clock.signal_handler().set_inputline(m_maincpu, 0);

	// sound hardware
	// discrete...
}



/***************************************************************************
    ROM Definitions
***************************************************************************/

ROM_START( tr606 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-128.ic4", 0x0000, 0x0800, CRC(eee88f80) SHA1(ae605ce2b95adc2e0bacde3cd7ed0f39ac88b981) )
ROM_END

} // anonymous namespace



/***************************************************************************
    Drivers
***************************************************************************/

SYST( 1982, tr606, 0, 0, tr606, tr606, tr606_state, empty_init, "Roland", "TR-606 Drumatix", MACHINE_IS_SKELETON )
