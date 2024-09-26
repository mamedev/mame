// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

Roland TR-808 Rhythm Composer, drum machine from 1980

Hardware notes:
- NEC uCOM-43 MCU, labeled D650C 085
- 4*uPD444C 1024x4 Static CMOS SRAM
- board is packed with discrete components

TODO:
- everything

***************************************************************************/

#include "emu.h"

#include "cpu/ucom4/ucom4.h"
#include "machine/clock.h"


namespace {

class tr808_state : public driver_device
{
public:
	tr808_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void tr808(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<ucom4_cpu_device> m_maincpu;
};

void tr808_state::machine_start()
{
}



/***************************************************************************
    Inputs
***************************************************************************/

static INPUT_PORTS_START( tr808 )
INPUT_PORTS_END



/***************************************************************************
    Machine Configs
***************************************************************************/

void tr808_state::tr808(machine_config &config)
{
	// basic machine hardware
	NEC_D650(config, m_maincpu, 500000); // 2us according to schematics

	auto &irq_clock(CLOCK(config, "irq_clock"));
	irq_clock.set_period(attotime::from_usec(1900)); // clock rate 1.9ms
	irq_clock.set_duty_cycle(0.1); // short duty cycle
	irq_clock.signal_handler().set_inputline(m_maincpu, 0);

	// sound hardware
	// discrete...
}



/***************************************************************************
    ROM Definitions
***************************************************************************/

ROM_START( tr808 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-085.ic4", 0x0000, 0x0800, CRC(06f0c405) SHA1(eddac7af6396c3f220914d9cad2c9e398872b034) )
ROM_END

} // anonymous namespace



/***************************************************************************
    Drivers
***************************************************************************/

SYST( 1980, tr808, 0, 0, tr808, tr808, tr808_state, empty_init, "Roland", "TR-808 Rhythm Composer", MACHINE_IS_SKELETON )
