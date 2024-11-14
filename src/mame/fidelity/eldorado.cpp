// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:bataais
/*******************************************************************************

Fidelity Eldorado Chess Challenger (model 6119)

Hardware notes:
- PCB label: CXG262-600-001, CXG262-600-101
- TMP80C49AP6-6744 MCU, 2KB internal ROM, 6MHz XTAL
- buzzer, 16 leds, 8*8 chessboard buttons

The chess engine is by Ron Nelson. The hardware was made by CXG for Fidelity,
as seen on the PCB and also confirmed by Ron Nelson.

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_eldorado.lh"


namespace {

class eldorado_state : public driver_device
{
public:
	eldorado_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void eldorado(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	bool m_kp_select = false;
	u8 m_inp_mux = 0;

	// I/O handlers
	void mux_w(u8 data);
	u8 mux_r();
	void control_w(u8 data);
	u8 input_r();
};

void eldorado_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_kp_select));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void eldorado_state::mux_w(u8 data)
{
	// D0-D7: input mux, led data
	m_inp_mux = ~data;
	m_display->write_mx(m_inp_mux);
}

u8 eldorado_state::mux_r()
{
	return ~m_inp_mux;
}

void eldorado_state::control_w(u8 data)
{
	// P24: speaker out
	m_dac->write(BIT(~data, 4));

	// P25,P26: led select
	m_display->write_my(~data >> 5 & 3);

	// P27: input mux highest bit (also goes to T0)
	m_kp_select = !bool(data & 0x80);
}

u8 eldorado_state::input_r()
{
	// P10-P17: multiplexed inputs
	u8 data = 0;

	// read chessboard buttons
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	// read sidepanel keypad
	if (m_kp_select)
		data |= m_inputs->read();

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( eldorado )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Reverse / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Move / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Sound / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Verify / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Problem / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void eldorado_state::eldorado(machine_config &config)
{
	// basic machine hardware
	I8049(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->p1_in_cb().set(FUNC(eldorado_state::input_r));
	m_maincpu->p2_out_cb().set(FUNC(eldorado_state::control_w));
	m_maincpu->bus_in_cb().set(FUNC(eldorado_state::mux_r));
	m_maincpu->bus_out_cb().set(FUNC(eldorado_state::mux_w));
	m_maincpu->t0_in_cb().set(m_maincpu, FUNC(mcs48_cpu_device::p2_r)).bit(7);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_fidel_eldorado);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( feldo )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("tmp80c49ap6-6744_100-1027a01", 0x0000, 0x0800, CRC(3b93b6d2) SHA1(353a741624b4c7fd74a0cf601e2e52f9914b58b8) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1990, feldo, 0,      0,      eldorado, eldorado, eldorado_state, empty_init, "Fidelity Electronics International / CXG Systems", "Eldorado Chess Challenger", MACHINE_SUPPORTS_SAVE )
