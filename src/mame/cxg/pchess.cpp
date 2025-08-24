// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

CXG Pocketchess (model 219)

It says "Pocket Chess" on the front of the handheld itself, "Pocketchess" on the
backside label, manual, box, and advertising.

TODO:
- save switch does not work since MCU emulation doesn't support NVRAM

Hardware notes:
- PCB label: CXG 219 600 001
- Hitachi HD44820 @ ~400kHz (100K resistor on Fidelity MCC)
- LCD with 4 7segs and custom segments, piezo

HD44820B63 MCU is used in:
- CXG Pocketchess
- Fidelity Micro Chess Challenger (12 buttons) (Fidelity brand Pocketchess)

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "cxg_pchess.lh"


namespace {

class pchess_state : public driver_device
{
public:
	pchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void pchess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_lcd_com = 0;
	u32 m_lcd_segs = 0;

	// I/O handlers
	void update_lcd();
	template<int N> void seg_w(u8 data);
	u16 input_r();
	void control_w(u16 data);
};

void pchess_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_lcd_segs));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void pchess_state::update_lcd()
{
	// LCD common is analog (voltage level)
	const u8 com = population_count_32(m_lcd_com & 3);
	const u64 data = (com == 0) ? m_lcd_segs : (com == 2) ? ~m_lcd_segs : 0;
	m_display->write_row(0, data);
}

template<int N>
void pchess_state::seg_w(u8 data)
{
	// R0x-R6x: LCD segments
	const u8 shift = N * 4;
	m_lcd_segs = (m_lcd_segs & ~(0xf << shift)) | (data << shift);
	update_lcd();
}

u16 pchess_state::input_r()
{
	u16 data = 0;

	// D13-D15: read buttons
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 13;

	return ~data;
}

void pchess_state::control_w(u16 data)
{
	// D1: speaker out
	m_dac->write(BIT(data, 1));

	// D3,D4: LCD common
	m_lcd_com = data >> 3 & 3;

	// D7,D8: 2 more LCD segments
	const u32 mask = 3 << 28;
	m_lcd_segs = (m_lcd_segs & ~mask) | (data << 21 & mask);
	update_lcd();

	// D9-D12: input mux
	m_inp_mux = ~data >> 9 & 0xf;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( pchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LV")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MO")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("EN")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void pchess_state::pchess(machine_config &config)
{
	// basic machine hardware
	HD44820(config, m_maincpu, 400'000); // approximation
	m_maincpu->write_r<0>().set(FUNC(pchess_state::seg_w<0>));
	m_maincpu->write_r<1>().set(FUNC(pchess_state::seg_w<1>));
	m_maincpu->write_r<2>().set(FUNC(pchess_state::seg_w<2>));
	m_maincpu->write_r<3>().set(FUNC(pchess_state::seg_w<3>));
	m_maincpu->write_r<4>().set(FUNC(pchess_state::seg_w<4>));
	m_maincpu->write_r<5>().set(FUNC(pchess_state::seg_w<5>));
	m_maincpu->write_r<6>().set(FUNC(pchess_state::seg_w<6>));
	m_maincpu->write_d().set(FUNC(pchess_state::control_w));
	m_maincpu->read_d().set(FUNC(pchess_state::input_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 30);
	m_display->set_bri_levels(0.05);
	config.set_default_layout(layout_cxg_pchess);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/4, 914/4);
	screen.set_visarea_full();

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( pchess )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("1985_white_and_allcock_hd44820b63", 0x0000, 0x2000, CRC(8decfb8f) SHA1(ac216663fe72cc98607ce44c033bc4b13b309ad1) )

	ROM_REGION( 57412, "screen", 0 )
	ROM_LOAD("pchess.svg", 0, 57412, CRC(7859b1ac) SHA1(518c5cd08fa8562628345e8e28048c01c9e4edd6) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, pchess, 0,      0,      pchess,  pchess, pchess_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Pocketchess (CXG)", MACHINE_SUPPORTS_SAVE )
