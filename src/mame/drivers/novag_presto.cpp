// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************
*
* novag_presto.cpp, subdriver of machine/novagbase.cpp, machine/chessbase.cpp

TODO:
- is led handling correct? mux data needs to be auto cleared
  similar to diablo/sexpert

*******************************************************************************

Novag Presto overview:
- NEC D80C49C MCU(serial 186), OSC from LC circuit measured ~6MHz
- buzzer, 16+4 LEDs, 8*8 chessboard buttons

Octo has a NEC D80C49HC MCU(serial 111), OSC from LC circuit measured ~12MHz
The buzzer has a little electronic circuit going on, not sure whatfor.
Otherwise, it's identical to Presto. The MCU internal ROM is same too.

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "novag_presto.lh" // clickable


namespace {

class presto_state : public novagbase_state
{
public:
	presto_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	// machine drivers
	void presto(machine_config &config);
	void octo(machine_config &config);

protected:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;

	// I/O handlers
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input_r);
};

class octo_state : public presto_state
{
public:
	octo_state(const machine_config &mconfig, device_type type, const char *tag) :
		presto_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(octo_cpu_freq) { octo_set_cpu_freq(); }

protected:
	virtual void machine_reset() override;
	void octo_set_cpu_freq();
};

void octo_state::machine_reset()
{
	presto_state::machine_reset();
	octo_set_cpu_freq();
}

void octo_state::octo_set_cpu_freq()
{
	// Octo was released with either 12MHz or 15MHz CPU
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? (15000000) : (12000000));
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// MCU ports/generic

WRITE8_MEMBER(presto_state::mux_w)
{
	// D0-D7: input mux low, led data
	m_inp_mux = (m_inp_mux & ~0xff) | (~data & 0xff);
	display_matrix(8, 3, m_inp_mux, m_led_select);
}

WRITE8_MEMBER(presto_state::control_w)
{
	// P21: input mux high
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 7 & 0x100);

	// P22,P23: speaker lead 1,2
	m_dac->write(BIT(data, 2) & BIT(~data, 3));

	// P24-P26: led select
	m_led_select = ~data >> 4 & 7;
	m_inp_mux &= ~0xff; // ?
}

READ8_MEMBER(presto_state::input_r)
{
	// P10-P17: multiplexed inputs
	return ~read_inputs(9) & 0xff;
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( presto )
	PORT_INCLUDE( generic_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Black/White") // Octo calls it "Change Color"
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Verify / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Set Up / Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Set Level / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Take Back / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Go")
INPUT_PORTS_END

static INPUT_PORTS_START( octo )
	PORT_INCLUDE( presto )

	PORT_START("FAKE")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, octo_state, octo_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "12MHz" )
	PORT_CONFSETTING(    0x01, "15MHz" )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void presto_state::presto(machine_config &config)
{
	/* basic machine hardware */
	I8049(config, m_maincpu, 6000000); // LC circuit, measured 6MHz
	m_maincpu->p1_in_cb().set(FUNC(presto_state::input_r));
	m_maincpu->p2_out_cb().set(FUNC(presto_state::control_w));
	m_maincpu->bus_out_cb().set(FUNC(presto_state::mux_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(presto_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_presto);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void presto_state::octo(machine_config &config)
{
	presto(config);

	/* basic machine hardware */
	m_maincpu->set_clock(12000000); // LC circuit, measured, see octo_set_cpu_freq
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( npresto )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("d80c49c_186", 0x0000, 0x0800, CRC(29a0eb4c) SHA1(e058d6018e53ddcaa3b5ec25b33b8bff091b04db) )
ROM_END

ROM_START( nocto )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("d80c49hc_111", 0x0000, 0x0800, CRC(29a0eb4c) SHA1(e058d6018e53ddcaa3b5ec25b33b8bff091b04db) ) // same program as npresto
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1984, npresto, 0,       0,      presto,  presto, presto_state, empty_init, "Novag", "Presto (Novag)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1987, nocto,   npresto, 0,      octo,    octo,   octo_state,   empty_init, "Novag", "Octo (Novag)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
