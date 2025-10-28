// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    skeleton/digijet.cpp

    Skeleton driver for the Volkswagen Digijet series of automotive ECUs

    The Digijet Engine Control Unit (ECU) was used in Volkswagen vehicles
    from the 1980s.

    CPU: MAF 80A39HL
	ROM: ST M2764AF6
	ADC0809CCN
	LM2901
	SN74LS373N
	SN74LS00N
	XTAL 7.372 MHz

	_________________________
	|          7   A 0       |
	|          4   D 8       |
	C          3   C 0    7  |
	O          7     9    4  |
	N  L2      3          0  |
	N  M0    M2764AF6  X  0  |
	|  91          80A39HL   |
	|________________________|

	Connector
	1  Engine RPM
	2  Coolant temperture (ADC IN2)
	3  GND
	4  Throttle switch (T0)
	5  Lambda sensor
	6  GND Air sensor
	7  GND
	8  Check engine (not populated) (P1.3)
	9  GND
	10 GND
	11 Injector (P1.5)
	12 Injector (P1.5)
	13 Ignition (ADC IN1)
	14 Air in temperature (ADC IN3)
	15 Air amount (ADC IN0)
	16 GND
	17 GND
	18 GND
	19 Air sensor power
	20 Fuel pump (P1.4)
	21 Fuel pump (P1.4)
	22 GND
	23 Injector (P1.5)
	24 Injector (P1.5)
	25 GND

**************************************************************************/

/*
    TODO:

	- Map RPM to an input and the interrupt speed
	- Figure out how the ADC is accessed
*/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"
#include "machine/watchdog.h"

#include "vw.lh"

namespace {

class digijet_state : public driver_device
{
public:
	digijet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_watchdog(*this, "watchdog")
		, m_io_adc(*this, "ADC%u", 0U)
	{
	}

	void digijet(machine_config &config);
	void digijet90(machine_config &config);

private:
	required_device<mcs48_cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_ioport_array<4> m_io_adc;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { }
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;

	void p1_w(uint8_t data);
	uint8_t p2_r();
	TIMER_DEVICE_CALLBACK_MEMBER( rpm_int );
	uint8_t read_adc(offs_t offset);
	void start_adc(uint8_t data);

	bool m_rpm = false;
};

void digijet_state::machine_start()
{
	save_item(NAME(m_rpm));
};

void digijet_state::io_map(address_map &map)
{
	map(0x30, 0x3f).r(FUNC(digijet_state::read_adc));
	map(0x30, 0x3f).w(FUNC(digijet_state::start_adc));
};

void digijet_state::prg_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("maincpu", 0);
};

uint8_t digijet_state::read_adc(offs_t offset)
{
	return m_io_adc[offset & 0x03]->read(); //FIXME this is a hack
	// The address lines connected to the ADC are A8-A11 but those can't be accessed externally by this CPU
};

void digijet_state::start_adc(uint8_t data)
{
	;// Connected to the START pin is the CPUs WR and the CPUs RD to the ADC OE and ALE
};

void digijet_state::p1_w(uint8_t data)
{
	bool check = BIT(data,3);
	bool fuel = BIT(data,4);
	bool inject = BIT(data,5);
	bool watchdog = BIT(data,6);

	machine().output().set_value("led_fuel", !fuel);
	machine().output().set_value("led_inject", !inject);
	machine().output().set_value("led_check", !check);
	
	if (watchdog)
		m_watchdog->watchdog_reset();
};

uint8_t digijet_state::p2_r()
{
	// bits 5 and 6 are the lambda sensor
	// 4 and 7 are floating
	uint8_t out = 0x9f;
	//out << LAMBDA1 LAMBDA2
	return out;
};

TIMER_DEVICE_CALLBACK_MEMBER(digijet_state::rpm_int)
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, m_rpm ? CLEAR_LINE : ASSERT_LINE);
	m_rpm = !m_rpm;
}

static INPUT_PORTS_START( digijet )
PORT_START("ADC0")
PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("Air amount")

PORT_START("ADC1")
PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_NAME("Ignition")

PORT_START("ADC2")
PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("Coolant temperature")

PORT_START("ADC3")
PORT_BIT( 0xff, 0x80, IPT_PADDLE_V ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_NAME("Air temperature")

PORT_START("LAMBDA1")
PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )

PORT_START("LAMBDA2")
PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )

PORT_START("THROTTLE")
PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )

INPUT_PORTS_END

void digijet_state::digijet(machine_config &config)
{
	/* basic machine hardware */
	I8039(config, m_maincpu, XTAL(11'000'000));

	m_maincpu->set_addrmap(AS_PROGRAM, &digijet_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &digijet_state::io_map);
	
	m_maincpu->p1_out_cb().set(FUNC(digijet_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(digijet_state::p2_r));
	m_maincpu->t0_in_cb().set_ioport("THROTTLE");

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(68)); // ???

	TIMER(config, "rpm").configure_periodic(FUNC(digijet_state::rpm_int), attotime::from_hz(1000*2/60)); // 1000r/m * 2 (m_rpm toggle) / 60s

	config.set_default_layout(layout_vw);
}

void digijet_state::digijet90(machine_config &config)
{
	/* basic machine hardware */
	I8039(config, m_maincpu, XTAL(7'372'800));

	m_maincpu->set_addrmap(AS_PROGRAM, &digijet_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &digijet_state::io_map);
	
	m_maincpu->p1_out_cb().set(FUNC(digijet_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(digijet_state::p2_r));
	m_maincpu->t0_in_cb().set_ioport("THROTTLE");

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(68)); // 0.68uF * 10kOhm

	TIMER(config, "rpm").configure_periodic(FUNC(digijet_state::rpm_int), attotime::from_hz(1000*2/60)); // 1000rpm / 60s

	config.set_default_layout(layout_vw);
}

ROM_START( digijet )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "vanagon_85_usa_ca.bin", 0x000, 0x800, CRC(2ed7c4c5) SHA1(ae48d8892b44fe76b48bcefd293c15cd47af3fba) ) // Volkswagen Vanagon, 1985, USA, California
ROM_END

ROM_START( digijet90 )
	ROM_REGION( 0x2000, "maincpu", 0 ) // repeats itself 4 times
	ROM_LOAD( "fabb05_03_03.bin", 0x0000, 0x2000, CRC(8c96bcdf) SHA1(73b26914cd15ca3a5e0d7427de9ce4b4e311fb00) ) // Volkswagen 1990, Germany
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT     COMPAT  MACHINE    INPUT    CLASS          INIT        COMPANY       FULLNAME          FLAGS
CONS( 1985, digijet,   digijet90, 0,      digijet,   digijet, digijet_state, empty_init, "Volkswagen", "Digijet",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
CONS( 1990, digijet90, 0,         0,      digijet90, digijet, digijet_state, empty_init, "Volkswagen", "Digijet (1990)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
