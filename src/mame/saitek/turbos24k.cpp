// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

SciSys Kasparov Turbo S-24K (model 280)

NOTE: Before exiting MAME, press the STOP button to turn the power off. Otherwise,
NVRAM won't save properly.

Hardware notes:
- PCB label: SciSys ST6-PE-001
- Hitachi HD6301Y0P MCU (mode 2) @ 12MHz (LC osc, no XTAL)
- 8KB RAM (M5M5165P-12L), battery-backed
- 2 LCD clocks, same ones as Turbo 16K
- 16+11 LEDs (23 tri-color, 4 red)
- buttons sensor board, piezo

I/O is similar to Leonardo, and the housing is similar to the newer Turbo King.
It has an unused input (IN.7 0x01) that makes the program work with a magnet
sensor board.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/msm5001n.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_turbos24k.lh"


namespace {

class turbos24k_state : public driver_device
{
public:
	turbos24k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_clock(*this, "clock%u", 0),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_lcd_digits(*this, "ldigit%u.%u", 0U, 0U),
		m_lcd_colon(*this, "lc%u", 0U)
	{ }

	void turbos24k(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device_array<msm5001n_device, 2> m_lcd_clock;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<9> m_inputs;
	output_finder<2, 4> m_lcd_digits;
	output_finder<2> m_lcd_colon;

	u8 m_inp_mux = 0;
	u8 m_led_data[2] = { };
	u16 m_lcd_data[4] = { };

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	template<int N> void lcd_output_w(offs_t offset, u16 data);
	void lcd_enable(u8 enable);

	void standby(int state);
	void update_display();
	void mux_w(u8 data);
	void leds_w(u8 data);

	u8 p2_r();
	void p2_w(u8 data);
	void p5_w(u8 data);
	u8 p6_r();
};

void turbos24k_state::machine_start()
{
	m_lcd_digits.resolve();
	m_lcd_colon.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_lcd_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void turbos24k_state::standby(int state)
{
	if (state)
	{
		m_display->clear();
		lcd_enable(0);
	}
}

INPUT_CHANGED_MEMBER(turbos24k_state::go_button)
{
	if (newval && m_maincpu->standby())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


// LCD

template<int N>
void turbos24k_state::lcd_output_w(offs_t offset, u16 data)
{
	m_lcd_data[N << 1 | (offset & 1)] = data;
	u32 segs = m_lcd_data[N << 1 | 1] << 11 | m_lcd_data[N << 1];

	// unscramble segments
	m_lcd_digits[N][0] = bitswap<7>(segs,12,2,13,17,11,0,1);
	m_lcd_digits[N][1] = bitswap<7>(segs,15,5,16,4,14,3,4);
	m_lcd_digits[N][2] = bitswap<7>(segs,19,9,20,21,18,7,8);
	m_lcd_digits[N][3] = BIT(segs, 10) ? 6 : 0;
	m_lcd_colon[N] = BIT(segs, 6);
}

void turbos24k_state::lcd_enable(u8 enable)
{
	// LCD XTAL can be disabled by software
	for (int i = 0; i < 2; i++)
		m_lcd_clock[i]->set_clock_scale(BIT(enable, i) ? 1.0 : 0.0);
}


// misc

void turbos24k_state::update_display()
{
	m_display->matrix_partial(0, 8, 1 << (m_inp_mux & 0xf), m_led_data[0]);
	m_display->matrix_partial(8, 2, 1 << BIT(m_inp_mux, 5), (m_inp_mux << 2 & 0x300) | m_led_data[1]);
}

void turbos24k_state::mux_w(u8 data)
{
	// d0-d3: input/chessboard led mux
	// d5: button led select
	// d6,d7: button led data
	m_inp_mux = data ^ 0xc0;
	update_display();

	// d4: speaker out
	m_dac->write(BIT(data, 4));
}

void turbos24k_state::leds_w(u8 data)
{
	// d0-d7: button led data
	m_led_data[1] = ~data;
	update_display();
}


// MCU ports

u8 turbos24k_state::p2_r()
{
	u8 data = 0;

	// P20-P22: multiplexed inputs
	if ((m_inp_mux & 0xf) <= 8)
		data = m_inputs[m_inp_mux & 0xf]->read();

	return ~data;
}

void turbos24k_state::p2_w(u8 data)
{
	// P25,P26: chessboard led column data
	m_led_data[0] = (m_led_data[0] & ~3) | (~data >> 5 & 3);
	update_display();
}

void turbos24k_state::p5_w(u8 data)
{
	// P53,P54: LCD clocks enabled
	lcd_enable(~data >> 3 & 3);

	// P55: LCD clocks power (both)
	m_lcd_clock[0]->power_w(BIT(data, 5));
	m_lcd_clock[1]->power_w(BIT(data, 5));

	// P56,P57: chessboard led row data
	m_led_data[0] = (m_led_data[0] & 3) | (~data >> 4 & 0xc);
	update_display();
}

u8 turbos24k_state::p6_r()
{
	// P60-P67: read chessboard
	return ~m_board->read_file(m_inp_mux & 0xf);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void turbos24k_state::main_map(address_map &map)
{
	map(0x4000, 0x5fff).ram().share("nvram");
	map(0x6000, 0x6000).mirror(0x0fff).w(FUNC(turbos24k_state::mux_w));
	map(0x7000, 0x7000).mirror(0x0fff).w(FUNC(turbos24k_state::leds_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( turbos24k )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Play Normal")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Library")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Function")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) // freq sel
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Normal")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Analysis")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Set Up")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel + board config
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Info")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_CONFNAME( 0x04, 0x04, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x04, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, turbos24k_state, go_button, 0) PORT_NAME("Go")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void turbos24k_state::turbos24k(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 12'000'000); // approximation, no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &turbos24k_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(turbos24k_state::standby));
	m_maincpu->in_p2_cb().set(FUNC(turbos24k_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(turbos24k_state::p2_w));
	m_maincpu->out_p5_cb().set(FUNC(turbos24k_state::p5_w));
	m_maincpu->in_p6_cb().set(FUNC(turbos24k_state::p6_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	MSM5001N(config, m_lcd_clock[0], 32.768_kHz_XTAL).write_segs().set(FUNC(turbos24k_state::lcd_output_w<0>));
	MSM5001N(config, m_lcd_clock[1], 32.768_kHz_XTAL).write_segs().set(FUNC(turbos24k_state::lcd_output_w<1>));

	PWM_DISPLAY(config, m_display).set_size(8+2, 8+2);
	config.set_default_layout(layout_saitek_turbos24k);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( turbos24k )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1986_sx6a_scisys_6301y0b40p.u1", 0x0000, 0x4000, CRC(d2a6e194) SHA1(59c61ddb722aa143afb95ce38f5b1b94e4eed604) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, turbos24k, 0,      0,      turbos24k, turbos24k, turbos24k_state, empty_init, "SciSys / Heuristic Software", "Kasparov Turbo S-24K", MACHINE_SUPPORTS_SAVE )
