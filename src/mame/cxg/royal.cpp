// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

CXG Sphinx Royal family

NOTE: Turn the power switch (or button in supra's case) off before exiting MAME,
otherwise NVRAM won't save properly. And only turn the power off when it's the
user's turn to move, this is warned about in the manual.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch/button to that

Hardware notes:
- HD614042S, 4MHz XTAL
- optional LCD panel(s), see below
- chessboard buttons, 16+4 LEDs, piezo

Royal has 2 LCD panels, Supra has 1 (D12 pin is low), Granada and others have 0.
The LCD panel has 4 7segs (no DP) and 2 unused segments: an x in the middle, and
a white square under the first digit.

The 1992 versions by National Telecommunications System Ltd (Granada CXG-347,
Sierra, Seville) have a lower-speed 3.58MHz XTAL, but since none of them have
LCD panels, users won't notice the slower chess clocks.

Excluding other brands with the same product name, HD614042SJ02 MCU was used in:
- CXG Sphinx Royal (model 240)
- CXG Sphinx Supra (model 048)
- CXG Sphinx Granada (model 247/347)
- CXG Sphinx Seville (model 807)
- CXG Sphinx Sierra (model 647W)
- CXG Sphinx Alicante (model 328, suspected)
- Excalibur Chess Wizard (model 807E), Excalibur brand Sphinx Seville
- Excalibur Stiletto (model 328E/638E), Excalibur brand Sphinx Alicante (suspected)

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs400/hmcs400.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_granada.lh"
#include "cxg_royal.lh"
#include "cxg_supra.lh"


namespace {

class royal_state : public driver_device
{
public:
	royal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_digit(*this, "digit%u", 0U)
	{ }

	void royal(machine_config &config);
	void granada(machine_config &config);
	void supra(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(granada_change_cpu_freq);
	DECLARE_INPUT_CHANGED_MEMBER(supra_on_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hmcs400_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<8> m_out_digit;

	u8 m_inp_mux = 0;
	u8 m_lcd_com = 0;
	u16 m_lcd_segs = 0;
	u8 m_lcd_select = 0;

	// I/O handlers
	void stop_mode(int state);

	void update_lcd();
	template<int N> void lcd_segs_w(u8 data);
	void lcd_com_w(u8 data);

	template<int N> u8 board_r();
	template<int N> void input_w(u8 data);
	void control_w(u16 data);
	u16 input_r();
};

void royal_state::machine_start()
{
	m_out_digit.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_select));
}

INPUT_CHANGED_MEMBER(royal_state::granada_change_cpu_freq)
{
	// 1992 models run at lower speed
	m_maincpu->set_unscaled_clock((newval & 1) ? 4_MHz_XTAL : 3.58_MHz_XTAL);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void royal_state::stop_mode(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

INPUT_CHANGED_MEMBER(royal_state::supra_on_button)
{
	// stop mode check actually comes from D3 high-impedance state
	if (newval && m_maincpu->stop_mode())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


// LCD

void royal_state::update_lcd()
{
	for (int i = 0; i < 2; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u16 data = (com == 0) ? m_lcd_segs : (com == 2) ? ~m_lcd_segs : 0;

		// 2 digits per common
		for (int j = 0; j < 2; j++)
		{
			u8 digit = bitswap<8>(data >> (j * 8), 0,1,2,3,4,5,6,7);
			m_lcd_pwm->write_row(m_lcd_select * 4 + i * 2 + j, digit);
		}
	}
}

template<int N>
void royal_state::lcd_segs_w(u8 data)
{
	// R0x,R6x-R8x: LCD segment data
	m_lcd_segs = (m_lcd_segs & ~(0xf << (N*4))) | (data << (N*4));
	update_lcd();
}

void royal_state::lcd_com_w(u8 data)
{
	// R50-R53: LCD common
	m_lcd_com = data;
	update_lcd();
}


// misc

template<int N>
u8 royal_state::board_r()
{
	// R1x,R2x: read chessboard
	u8 data = 0;

	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	return data >> (N * 4) & 0xf;
}

template<int N>
void royal_state::input_w(u8 data)
{
	// R3x,R4x: input mux, LED data
	const u8 shift = 4 * N;
	m_inp_mux = (m_inp_mux & ~(0xf << shift)) | (data << shift);
	m_led_pwm->write_mx(~m_inp_mux);
}

void royal_state::control_w(u16 data)
{
	// D4,D5: LED select
	// D6-D9: status LEDs (direct)
	m_led_pwm->write_my(data >> 4 & 0x3f);

	// D13: LCD panel select
	m_lcd_select = BIT(data, 13);
	update_lcd();

	// D14: speaker out
	m_dac->write(BIT(data, 14));
}

u16 royal_state::input_r()
{
	u16 data = 0;

	// D0,D1: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	// D10,D11: freq sel
	// D12: 1/2 LCD panels
	return data | 0x140c;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( royal )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set-Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound")
INPUT_PORTS_END

static INPUT_PORTS_START( granada )
	PORT_INCLUDE( royal )

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, royal_state, granada_change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3.58MHz (1992 version)" )
	PORT_CONFSETTING(    0x01, "4MHz (1988 version)" )
INPUT_PORTS_END

static INPUT_PORTS_START( supra )
	PORT_INCLUDE( royal )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, royal_state, supra_on_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void royal_state::royal(machine_config &config)
{
	// basic machine hardware
	HD614042(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->stop_cb().set(m_maincpu, FUNC(hmcs400_cpu_device::nvram_set_battery));
	m_maincpu->stop_cb().append(FUNC(royal_state::stop_mode));
	m_maincpu->write_r<0x0>().set(FUNC(royal_state::lcd_segs_w<0>));
	m_maincpu->read_r<0x1>().set(FUNC(royal_state::board_r<0>));
	m_maincpu->read_r<0x2>().set(FUNC(royal_state::board_r<1>));
	m_maincpu->write_r<0x3>().set(FUNC(royal_state::input_w<0>));
	m_maincpu->write_r<0x4>().set(FUNC(royal_state::input_w<1>));
	m_maincpu->write_r<0x5>().set(FUNC(royal_state::lcd_com_w));
	m_maincpu->write_r<0x6>().set(FUNC(royal_state::lcd_segs_w<3>));
	m_maincpu->write_r<0x7>().set(FUNC(royal_state::lcd_segs_w<2>));
	m_maincpu->write_r<0x8>().set(FUNC(royal_state::lcd_segs_w<1>));
	m_maincpu->write_d().set(FUNC(royal_state::control_w));
	m_maincpu->read_d().set(FUNC(royal_state::input_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(8, 8);
	m_lcd_pwm->set_bri_levels(0.05);
	m_lcd_pwm->set_segmask(0xff, 0x7f);
	m_lcd_pwm->output_digit().set([this](offs_t offset, u64 data) { m_out_digit[offset] = data; });

	PWM_DISPLAY(config, m_led_pwm).set_size(6, 8);
	config.set_default_layout(layout_cxg_royal);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void royal_state::supra(machine_config &config)
{
	royal(config);
	m_maincpu->read_d().set(FUNC(royal_state::input_r)).exor(0x1000);
	config.set_default_layout(layout_cxg_supra);
}

void royal_state::granada(machine_config &config)
{
	supra(config);
	config.set_default_layout(layout_cxg_granada);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sroyal )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("1988_105_newcrest_hd614042sj02", 0x0000, 0x2000, CRC(47334ac9) SHA1(b4dc930e34926f1b33f6d247af45627c891202ff) )
ROM_END

#define rom_granada rom_sroyal
#define rom_supra rom_sroyal

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, sroyal,  0,      0,      royal,   royal,   royal_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Sphinx Royal", MACHINE_SUPPORTS_SAVE )
SYST( 1988, granada, sroyal, 0,      granada, granada, royal_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Sphinx Granada", MACHINE_SUPPORTS_SAVE )
SYST( 1988, supra,   sroyal, 0,      supra,   supra,   royal_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Sphinx Supra", MACHINE_SUPPORTS_SAVE )
