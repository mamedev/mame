// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Chess King Pocket Micro De-Luxe

Hardware notes:
- Hitachi HD44868 @ ~800kHz (33K resistor)
- LCD with 4 7segs and custom segments, piezo

HD44868A07 MCU is used in:
- Chess King Pocket Micro De-Luxe
- Chess King Mighty Midget De-Luxe
- Mephisto Teufelchen (H+G brand Pocket Micro De-Luxe)

TODO:
- dump/add the first version (Pocket Micro), does it fit in this driver?

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "cking_pmicrodx.lh"


namespace {

class pmicrodx_state : public driver_device
{
public:
	pmicrodx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void pmicrodx(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { refresh_irq(1 << param); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD { refresh_irq(); }

private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<5> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_lcd_com = 0;
	u64 m_lcd_segs = 0;

	void update_lcd();
	template<int N> void seg_w(u8 data);
	u8 read_buttons();
	void refresh_irq(u8 mask = ~0);
	u16 input_r();
	void control_w(u16 data);
};

void pmicrodx_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_lcd_segs));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void pmicrodx_state::update_lcd()
{
	// LCD common is analog (voltage level)
	const u8 com = population_count_32(m_lcd_com & 3);
	const u64 data = (com == 0) ? m_lcd_segs : (com == 2) ? ~m_lcd_segs : 0;
	m_display->write_row(0, data);
}

template<int N>
void pmicrodx_state::seg_w(u8 data)
{
	// R0x-R6x: LCD segments
	const u8 shift = N * 4;
	m_lcd_segs = (m_lcd_segs & ~(u64(0xf << shift))) | (u64(data) << shift);
	update_lcd();
}

u8 pmicrodx_state::read_buttons()
{
	u8 data = 0;

	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return data;
}

void pmicrodx_state::refresh_irq(u8 mask)
{
	// half of the buttons are tied to MCU interrupt pins
	mask &= read_buttons();
	for (int i = 0; i < 2; i++)
		m_maincpu->set_input_line(i, BIT(mask, i) ? CLEAR_LINE : ASSERT_LINE);
}

u16 pmicrodx_state::input_r()
{
	// D0: battery status
	u16 data = m_inputs[4]->read() & 1;

	// D2,D3: read buttons
	data |= read_buttons() & 0xc;
	return ~data;
}

void pmicrodx_state::control_w(u16 data)
{
	// D1: speaker out
	m_dac->write(BIT(data, 1));

	// D4-D6: input mux
	m_inp_mux = ~data >> 4 & 7;
	refresh_irq();

	// D7,D8: LCD common
	m_lcd_com = data >> 7 & 3;

	// D9-D15: more LCD segments
	m_lcd_segs = (m_lcd_segs & 0x0fff'ffffULL) | (u64(data & 0xfe00) << 19);
	update_lcd();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

#define INPUT_CHANGED(x) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, pmicrodx_state, input_changed, x)

static INPUT_PORTS_START( pmicrodx )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) INPUT_CHANGED(0) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) INPUT_CHANGED(0) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) INPUT_CHANGED(0) PORT_CODE(KEYCODE_L) PORT_NAME("LV")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) INPUT_CHANGED(1) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) INPUT_CHANGED(1) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) INPUT_CHANGED(1) PORT_CODE(KEYCODE_T) PORT_NAME("TB")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("EN")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")

	PORT_START("IN.4")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void pmicrodx_state::pmicrodx(machine_config &config)
{
	// basic machine hardware
	HD44868(config, m_maincpu, 800'000); // approximation
	m_maincpu->write_r<0>().set(FUNC(pmicrodx_state::seg_w<0>));
	m_maincpu->write_r<1>().set(FUNC(pmicrodx_state::seg_w<1>));
	m_maincpu->write_r<2>().set(FUNC(pmicrodx_state::seg_w<2>));
	m_maincpu->write_r<3>().set(FUNC(pmicrodx_state::seg_w<3>));
	m_maincpu->write_r<4>().set(FUNC(pmicrodx_state::seg_w<4>));
	m_maincpu->write_r<5>().set(FUNC(pmicrodx_state::seg_w<5>));
	m_maincpu->write_r<6>().set(FUNC(pmicrodx_state::seg_w<6>));
	m_maincpu->write_d().set(FUNC(pmicrodx_state::control_w));
	m_maincpu->read_d().set(FUNC(pmicrodx_state::input_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 35);
	m_display->set_bri_levels(0.05);
	config.set_default_layout(layout_cking_pmicrodx);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/3, 851/3);
	screen.set_visarea_full();

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( pmicrodx )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("chessking_mark_2_hd44868a07", 0x0000, 0x2000, CRC(aef47e60) SHA1(97cb7b51ce354c54c6f0faa903d5bd70d5a108ba) )
	ROM_IGNORE( 0x2000 ) // ignore factory test banks

	ROM_REGION( 74477, "screen", 0 )
	ROM_LOAD("pmicrodx.svg", 0, 74477, CRC(34563496) SHA1(dcca2223cc35d54955caead8ff14e6f96b4155ce) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1984, pmicrodx, 0,      0,      pmicrodx, pmicrodx, pmicrodx_state, empty_init, "Chess King / Intelligent Software", "Pocket Micro De-Luxe", MACHINE_SUPPORTS_SAVE )
