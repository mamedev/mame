// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Chess King Triomphe

The ROM includes (C)1985ICSL, that's Intelligent Chess Software Ltd. For some
reason, the programmer decided to (ab)use the HD6301 undefined opcode TRAP
interrupt for the beeper routine. Very strange.

Hardware notes:
- PCB label: TRIUMPHE CHESS KING
- Hitachi HD6301V1P, 4MHz XTAL
- 8*8 chessboard buttons, 32 LEDs, piezo

It's in the same housing as Chess King Master, they repurposed the green LED
for a power-on LED.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cking_triomphe.lh"


namespace {

class triomphe_state : public driver_device
{
public:
	triomphe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void triomphe(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301v1_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	u16 m_inp_mux = 0;

	// I/O handlers
	u8 input_r();
	void board_w(u8 data);
	void control_w(u8 data);
};

void triomphe_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 triomphe_state::input_r()
{
	// P10-P17: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return ~data;
}

void triomphe_state::board_w(u8 data)
{
	// P30-P37: input mux (chessboard), led data
	m_inp_mux = (m_inp_mux & 0x300) | (data ^ 0xff);
	m_display->write_mx(~data);
}

void triomphe_state::control_w(u8 data)
{
	// P40,P41: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 8 & 0x300);

	// P44,P45: led select
	m_display->write_my(~data >> 4 & 3);

	// P47: speaker out
	m_dac->write(BIT(~data, 7));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( triomphe )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void triomphe_state::triomphe(machine_config &config)
{
	// basic machine hardware
	HD6301V1(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->in_p1_cb().set(FUNC(triomphe_state::input_r));
	m_maincpu->out_p3_cb().set(FUNC(triomphe_state::board_w));
	m_maincpu->in_p4_cb().set_constant(0); // freq sel
	m_maincpu->out_p4_cb().set(FUNC(triomphe_state::control_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_cking_triomphe);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( triomphe )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("chessking_triomphe_m1_hd6301v1e53p", 0x0000, 0x1000, CRC(01f30f08) SHA1(5d8949b8e0a5d15024bb2c13ee6f3eb2ed02f94b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, triomphe, 0,      0,      triomphe, triomphe, triomphe_state, empty_init, "Chess King / Intelligent Chess Software", "Triomphe", MACHINE_SUPPORTS_SAVE )
