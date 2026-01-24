// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

CXG Sphinx Junior (model 237)

NOTE: Before exiting MAME, press the OFF button to turn the power off. Otherwise,
NVRAM won't save properly.

Together with CXG Sphinx Chess Card (and not counting the 1992 rereleases of the
Sphinx Granada family), this is probably the last chess computer with Intelligent
Chess Software's involvement.

TODO:
- verify sound pitch, it's twice higher on a Fidelity MCC, or maybe that one
  was designed for 4MHz? (on CXG Sphinx Chess Card, sound pitch matches MAME)

Hardware notes:

Sphinx Junior:
- PCB label: CXG 237 600-002
- Hitachi HD614140H, 8MHz XTAL
- LCD with 4 7segs and custom segments (same as the one in CXG Pocketchess)
- embedded non-electronic chessboard, piezo

Fidelity Micro Chess Challenger (16 buttons):
- PCB label: CXG 249 600-001
- rest is similar to Sphinx Junior

CXG didn't sell a handheld version of their own, CXG model 249 does not exist.
Fidelity MCC 12-button version has a HD44820 MCU instead (see pchess.cpp).

HD614140HA27 MCU is used in:
- CXG Sphinx Junior
- Fidelity Chess Pal Challenger (Fidelity brand Sphinx Junior)
- Fidelity Micro Chess Challenger (16 buttons)
- Schneider Pocket Chess (same housing as Fidelity MCC)

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs400/hmcs400.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "cxg_junior.lh"


namespace {

class junior_state : public driver_device
{
public:
	junior_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void junior(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(on_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hmcs400_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	// I/O handlers
	void update_lcd();
	template<int N> void lcd_segs_w(u8 data);
	void control_w(u16 data);
	u16 input_r();
};

void junior_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(junior_state::on_button)
{
	if (newval && m_maincpu->stop_mode())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void junior_state::update_lcd()
{
	const u32 lcd_segs = bitswap<30>(m_lcd_segs,4,12,13,5,27,20,17,9,1,24,15,7,29,22,18,10,2,25,14,6,28,21,16,8,0,23,19,11,3,26);
	m_lcd_pwm->write_row(0, m_lcd_com ? ~lcd_segs : lcd_segs);
}

template<int N>
void junior_state::lcd_segs_w(u8 data)
{
	// R0x-R4x: LCD segment data, input mux
	const u8 shift = N * 4;
	m_lcd_segs = (m_lcd_segs & ~(0xf << shift)) | (data << shift);
	update_lcd();
}

void junior_state::control_w(u16 data)
{
	// D0: LCD common
	m_lcd_com = data & 1;

	// D4-D13: LCD segment data
	const u32 mask = 0x3ff << 20;
	m_lcd_segs = (m_lcd_segs & ~mask) | (data << 16 & mask);
	update_lcd();

	// D14: speaker out
	m_dac->write(BIT(data, 14));
}

u16 junior_state::input_r()
{
	u16 data = 0;

	// D1: read buttons from R03-R43
	if ((m_lcd_segs >> 3 & 0x1ffff) & m_inputs->read())
		data |= 2;

	// D2: R30 (freq sel)
	data |= BIT(~m_lcd_segs, 12) << 2;
	return data | 9;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( junior )
	PORT_START("IN.0")
	PORT_BIT(0x00001, IP_ACTIVE_HIGH, IPT_POWER_OFF)
	PORT_BIT(0x00002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D / 4 / Rook")
	PORT_BIT(0x00004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C / 3 / Bishop")
	PORT_BIT(0x00008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Knight")
	PORT_BIT(0x00010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1 / Pawn")
	PORT_BIT(0x00020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H / 8 / Black")
	PORT_BIT(0x00040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G / 7 / White")
	PORT_BIT(0x00080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F / 6 / King")
	PORT_BIT(0x00100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E / 5 / Queen")
	PORT_BIT(0x00200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Setup Position")
	PORT_BIT(0x00400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify Position")
	PORT_BIT(0x00800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear Entry")
	PORT_BIT(0x01000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x02000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound")
	PORT_BIT(0x04000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x08000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x10000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(junior_state::on_button), 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void junior_state::junior(machine_config &config)
{
	// basic machine hardware
	HD614140(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->stop_cb().set(m_maincpu, FUNC(hmcs400_cpu_device::nvram_set_battery));
	m_maincpu->stop_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	m_maincpu->write_r<0x0>().set(FUNC(junior_state::lcd_segs_w<0>));
	m_maincpu->write_r<0x1>().set(FUNC(junior_state::lcd_segs_w<1>));
	m_maincpu->write_r<0x2>().set(FUNC(junior_state::lcd_segs_w<2>));
	m_maincpu->write_r<0x3>().set(FUNC(junior_state::lcd_segs_w<3>));
	m_maincpu->write_r<0x4>().set(FUNC(junior_state::lcd_segs_w<4>));
	m_maincpu->write_d().set(FUNC(junior_state::control_w));
	m_maincpu->read_d().set(FUNC(junior_state::input_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::NOSENSORS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(1, 30);
	m_lcd_pwm->set_bri_levels(0.25);

	config.set_default_layout(layout_cxg_junior);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 914/5);
	screen.set_visarea_full();

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sjunior )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("1988_newcrest_614140ha27", 0x0000, 0x2000, CRC(9eb77d94) SHA1(84306ee39986847f9ae82a1117dc6fb8bd309bab) )

	ROM_REGION( 57412, "screen", 0 )
	ROM_LOAD("pchess.svg", 0, 57412, CRC(7859b1ac) SHA1(518c5cd08fa8562628345e8e28048c01c9e4edd6) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, sjunior, 0,      0,      junior,  junior, junior_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Sphinx Junior", MACHINE_SUPPORTS_SAVE )
