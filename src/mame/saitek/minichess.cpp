// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

SciSys Mini Chess, pocket calculator style chesscomputer

It's the first chess program on HMCS40. The engine was written by Mark Taylor
with assistance from David Levy.

Hardware notes:
- Hitachi 44801A34 MCU @ ~400kHz
- 4-digit LCD screen

Excluding resellers with same title, this MCU was used in:
- SciSys Mini Chess - 1st use
- SciSys Junior Chess
- SciSys Graduate Chess
- SciSys Chess Partner 3000
- SciSys Chess Partner 4000

MCU clock is via a resistor, this less accurate than with an XTAL, so the speed
may vary. Graduate Chess appears to have a 62K resistor between the OSC pins,
which would make it around 500kHz?

On CP3000/4000 they added a level slider. This will oscillate the level switch
input pin, so the highest level setting is the same as level 2 on Mini Chess.
It works on the old A34 MCU because the game keeps reading D0 while computing.

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_minichess.lh"


namespace {

class mini_state : public driver_device
{
public:
	mini_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_comp_timer(*this, "comp_timer"),
		m_computing(*this, "computing"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void smchess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<timer_device> m_comp_timer;
	output_finder<> m_computing;
	required_ioport_array<5> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_lcd_select = 0;
	u8 m_lcd_data = 0;

	TIMER_DEVICE_CALLBACK_MEMBER(computing) { m_computing = 1; }

	void update_lcd();
	template<int N> void seg_w(u8 data);
	void mux_w(u16 data);
	u16 input_r();
};

void mini_state::machine_start()
{
	m_computing.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_select));
	save_item(NAME(m_lcd_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void mini_state::update_lcd()
{
	u8 data = (m_lcd_select & 1) ? (m_lcd_data ^ 0xff) : m_lcd_data;
	data = bitswap<8>(data,2,4,6,7,5,1,0,3);
	m_display->matrix(m_lcd_select >> 2, data);
}

template<int N>
void mini_state::seg_w(u8 data)
{
	// R2x,R3x: LCD segment data
	m_lcd_data = (m_lcd_data & ~(0xf << (N*4))) | (data << (N*4));
	update_lcd();
}

void mini_state::mux_w(u16 data)
{
	// D9-D12: input mux
	m_inp_mux = ~data >> 9 & 0xf;

	// D3,D5-D8: CD4066 to LCD
	u8 sel = data >> 3 & 0x3f;

	// "computing" segment goes on when LCD isn't driven
	if (~m_lcd_select & sel & 1)
	{
		m_computing = 0;
		m_comp_timer->adjust(attotime::from_msec(100));
	}

	m_lcd_select = sel;
	update_lcd();
}

u16 mini_state::input_r()
{
	// D0,D2: switches
	u16 data = m_inputs[4]->read() & 5;

	// D13-D15: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 13;

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( smchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3 / Bishop")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6 / King")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7 / White")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8 / Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("FP") // find position

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_M) PORT_NAME("MM") // multi move
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mini_state::smchess(machine_config &config)
{
	// basic machine hardware
	HD44801(config, m_maincpu, 400'000); // approximation
	m_maincpu->write_r<2>().set(FUNC(mini_state::seg_w<0>));
	m_maincpu->write_r<3>().set(FUNC(mini_state::seg_w<1>));
	m_maincpu->write_d().set(FUNC(mini_state::mux_w));
	m_maincpu->read_d().set(FUNC(mini_state::input_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	m_display->set_refresh(attotime::from_hz(30));
	config.set_default_layout(layout_saitek_minichess);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2.5, 567/2.5);
	screen.set_visarea_full();

	TIMER(config, m_comp_timer).configure_generic(FUNC(mini_state::computing));
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( smchess )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("44801a34_scisys-w_ltd_proj_t", 0x0000, 0x2000, CRC(be71f1c0) SHA1(6b4d5c8f8491c82bdec1938bd83c14e826ff3e30) )

	ROM_REGION( 48645, "screen", 0 )
	ROM_LOAD("smchess.svg", 0, 48645, CRC(19beaa99) SHA1(2d738bd6953dfd7a2c8c37814badd0aac2960c8c) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, smchess, 0,      0,      smchess, smchess, mini_state, empty_init, "SciSys / Philidor Software", "Mini Chess", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
