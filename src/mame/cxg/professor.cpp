// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

CXG Sphinx Chess Professor (CXG-243)

NOTE: Before exiting MAME, press the OFF button to turn the power off. Otherwise,
NVRAM won't save properly.

The chess engine is by Frans Morsch, similar to the one in Mephisto Europa.
For some reason, they've put the row LEDs on the right instead of on the left.

Hardware notes:
- PCB label: 243 600 001
- Hitachi HD63B01Y0P, 8MHz XTAL
- Sanyo LC7580, LCD with 5 7segs and custom segments
- 8*8 chessboard buttons, 16 LEDs, piezo

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/lc7580.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "cxg_professor.lh"


namespace {

class professor_state : public driver_device
{
public:
	professor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd(*this, "lcd"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void professor(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(on_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<lc7580_device> m_lcd;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;
	output_finder<2, 52> m_out_lcd;

	u16 m_inp_mux = 0;

	// I/O handlers
	void lcd_output_w(offs_t offset, u64 data);
	template <int N> void leds_w(u8 data);
	void control_w(u8 data);
	u8 input_r();
	void board_w(u8 data);
};

void professor_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(professor_state::on_button)
{
	// standby check actually comes from P27 high-impedance state
	if (newval && m_maincpu->standby())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void professor_state::lcd_output_w(offs_t offset, u64 data)
{
	for (int i = 0; i < 52; i++)
		m_out_lcd[offset][i] = BIT(data, i);
}

template <int N>
void professor_state::leds_w(u8 data)
{
	// P10-P17, P40-P47: leds (direct)
	m_display->write_row(N, ~data);
}

void professor_state::control_w(u8 data)
{
	// P20-P22: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 8 & 0x700);

	// P23: speaker out
	m_dac->write(BIT(data, 3));

	// P24: LC7580 DATA
	// P25: LC7580 CLK
	// P26: LC7580 CE
	m_lcd->data_w(BIT(data, 4));
	m_lcd->clk_w(BIT(data, 5));
	m_lcd->ce_w(BIT(data, 6));
}

u8 professor_state::input_r()
{
	// P50-P57: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return ~data;
}

void professor_state::board_w(u8 data)
{
	// P60-P67: input mux (chessboard)
	m_inp_mux = (m_inp_mux & 0x700) | (data ^ 0xff);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( professor )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("Reset")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Position")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Test")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Monitor")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, professor_state, on_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void professor_state::professor(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->standby_cb().append(m_lcd, FUNC(lc7580_device::inh_w));
	m_maincpu->out_p1_cb().set(FUNC(professor_state::leds_w<0>));
	m_maincpu->out_p4_cb().set(FUNC(professor_state::leds_w<1>));
	m_maincpu->out_p2_cb().set(FUNC(professor_state::control_w));
	m_maincpu->in_p5_cb().set(FUNC(professor_state::input_r));
	m_maincpu->out_p6_cb().set(FUNC(professor_state::board_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	LC7580(config, m_lcd, 0);
	m_lcd->write_segs().set(FUNC(professor_state::lcd_output_w));
	m_lcd->nvram_enable_backup(true);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 921/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_cxg_professor);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( scprof )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1988_107_newcrest_hd6301y0j76p", 0x0000, 0x4000, CRC(681456c7) SHA1(99f8ab7369dbc2c93335affc38838295a8a2c5f3) )

	ROM_REGION( 61580, "screen", 0 )
	ROM_LOAD("scprof.svg", 0, 61580, CRC(6ebdf57c) SHA1(b0c01f1f251a569a2503a1053d7e676b1e6d9b0a) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, scprof, 0,      0,      professor, professor, professor_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx Chess Professor", MACHINE_SUPPORTS_SAVE )
