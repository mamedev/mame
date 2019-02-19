// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

    Novag generic MCS-48 based chess computer driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - is presto led handling correct? mux data needs to be auto cleared
      similar to diablo/sexpert

*******************************************************************************

Presto:
- NEC D80C49C MCU(serial 186), OSC from LC circuit measured ~6MHz
- buzzer, 16+4 LEDs, 8*8 chessboard buttons

Octo:
- NEC D80C49HC MCU(serial 111), OSC from LC circuit measured ~12MHz
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


class novagmcs48_state : public novagbase_state
{
public:
	novagmcs48_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void presto(machine_config &config);
	void octo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(octo_cpu_freq);

private:
	required_device<mcs48_cpu_device> m_maincpu;

	// Presto/Octo
	DECLARE_WRITE8_MEMBER(presto_mux_w);
	DECLARE_WRITE8_MEMBER(presto_control_w);
	DECLARE_READ8_MEMBER(presto_input_r);
	DECLARE_MACHINE_RESET(octo);
	void octo_set_cpu_freq();
};



// Devices, I/O

/******************************************************************************
    Presto/Octo
******************************************************************************/

// MCU ports/generic

WRITE8_MEMBER(novagmcs48_state::presto_mux_w)
{
	// D0-D7: input mux low, led data
	m_inp_mux = (m_inp_mux & ~0xff) | (~data & 0xff);
	display_matrix(8, 3, m_inp_mux, m_led_select);
}

WRITE8_MEMBER(novagmcs48_state::presto_control_w)
{
	// P21: input mux high
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 7 & 0x100);

	// P22,P23: speaker lead 1,2
	m_dac->write(BIT(data, 2) & BIT(~data, 3));

	// P24-P26: led select
	m_led_select = ~data >> 4 & 7;
	m_inp_mux &= ~0xff; // ?
}

READ8_MEMBER(novagmcs48_state::presto_input_r)
{
	// P10-P17: multiplexed inputs
	return ~read_inputs(9) & 0xff;
}

void novagmcs48_state::octo_set_cpu_freq()
{
	// Octo was released with either 12MHz or 15MHz CPU
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? (15000000) : (12000000));
}

MACHINE_RESET_MEMBER(novagmcs48_state, octo)
{
	novagbase_state::machine_reset();
	octo_set_cpu_freq();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( presto )
	PORT_INCLUDE( novag_cb_buttons )

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
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, novagmcs48_state, octo_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "12MHz" )
	PORT_CONFSETTING(    0x01, "15MHz" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(novagmcs48_state::octo_cpu_freq)
{
	octo_set_cpu_freq();
}



/******************************************************************************
    Machine Drivers
******************************************************************************/

void novagmcs48_state::presto(machine_config &config)
{
	/* basic machine hardware */
	I8049(config, m_maincpu, 6000000); // LC circuit, measured 6MHz
	m_maincpu->p1_in_cb().set(FUNC(novagmcs48_state::presto_input_r));
	m_maincpu->p2_out_cb().set(FUNC(novagmcs48_state::presto_control_w));
	m_maincpu->bus_out_cb().set(FUNC(novagmcs48_state::presto_mux_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(novagmcs48_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_presto);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void novagmcs48_state::octo(machine_config &config)
{
	presto(config);

	/* basic machine hardware */
	m_maincpu->set_clock(12000000); // LC circuit, measured, see octo_set_cpu_freq

	MCFG_MACHINE_RESET_OVERRIDE(novagmcs48_state, octo)
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



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS             INIT        COMPANY  FULLNAME          FLAGS
CONS( 1984, npresto, 0,       0,      presto,  presto, novagmcs48_state, empty_init, "Novag", "Presto (Novag)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1987, nocto,   npresto, 0,      octo,    octo,   novagmcs48_state, empty_init, "Novag", "Octo (Novag)",   MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
