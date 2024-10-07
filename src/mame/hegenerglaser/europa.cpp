// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Mephisto Europa (aka Schachschule)

Single-chip chess computer, the chess engine is by Frans Morsch.

NOTE: Before exiting MAME, press the STOP button to turn the power off. Otherwise,
NVRAM won't save properly.

Hardware notes:
- PCB label: 957&958-2, europa.H+G
- Hitachi HD63B01Y0F, 8MHz resonator
- 8*8 chessboard buttons, 24 LEDs, piezo

The E62 MCU was used in:
- Mephisto Europa
- Mephisto Europa A
- Mephisto Marco Polo
- Mephisto Manhattan (suspected)

In the early 90s, it was also reproduced in Ukraine as Chess Computer-1 (ШК-1).
It has a typical yellow-brown PCB and Soviet discrete components, and they
sourced the official Mephisto MCU with the same E62 mask ROM serial.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_europa.lh"


namespace {

class europa_state : public driver_device
{
public:
	europa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void europa(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(on_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	u16 m_inp_mux = 0;

	// I/O handlers
	template <int N> void leds_w(u8 data);
	void control_w(u8 data);
	u8 input_r();
	void board_w(u8 data);
};

void europa_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(europa_state::on_button)
{
	if (newval && m_maincpu->standby())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

template <int N>
void europa_state::leds_w(u8 data)
{
	// P10-P17, P30-P37, P40-P47: leds (direct)
	m_display->write_row(N, ~data);
}

void europa_state::control_w(u8 data)
{
	// P20,P21,P24: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | bitswap<3>(~data,4,1,0) << 8;

	// P23: speaker out
	m_dac->write(BIT(data, 3));
}

u8 europa_state::input_r()
{
	// P50-P57: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read() & 0x3f;

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return ~data;
}

void europa_state::board_w(u8 data)
{
	// P60-P67: input mux (chessboard)
	m_inp_mux = (m_inp_mux & 0x700) | (data ^ 0xff);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( europa )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("PLAY")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("POS")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MEM")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White / Black")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LEV")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("RES")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("STOP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("BOOK")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INFO")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("HELP")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_CHANGED_MEMBER(DEVICE_SELF, europa_state, on_button, 0) PORT_NAME("ON")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void europa_state::europa(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p1_cb().set(FUNC(europa_state::leds_w<0>));
	m_maincpu->out_p3_cb().set(FUNC(europa_state::leds_w<1>));
	m_maincpu->out_p4_cb().set(FUNC(europa_state::leds_w<2>));
	m_maincpu->out_p2_cb().set(FUNC(europa_state::control_w));
	m_maincpu->in_p5_cb().set(FUNC(europa_state::input_r));
	m_maincpu->out_p6_cb().set(FUNC(europa_state::board_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_mephisto_europa);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( europa )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("mephisto_hg-morsch_63b01y0e62f", 0x0000, 0x4000, CRC(eaa11f82) SHA1(95fae8dc063e4e4730d08fa894bea61d49ad39d4) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, europa, 0,      0,      europa,  europa, europa_state, empty_init, "Hegener + Glaser", "Mephisto Europa", MACHINE_SUPPORTS_SAVE )
