// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Mr. Lars
/*******************************************************************************

Yeno 532 XL

This is the only Yeno chesscomputer that has a common CPU with external EPROM,
others are based on a 1-chip MCU. The 532 XL chess engine is by Ulf Rathsman.

Hardware notes:
- R65C02P4 @ 4 MHz
- 32KB ROM (MBM27C256H-10)
- 8KB battery-backed RAM (HY6264LP-10)
- HD61603, 2 4*7seg LCD screens
- TTL, piezo, 8*8+7 LEDs, button sensors

The software searches for an opening book ROM in 0x4000-0x7fff, it looks like
it's compatible with Conchess L16 / Mephisto HG240. Though the hardware does
not have an edge connector or empty ROM socket for it.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/hd61603.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "yeno_532xl.lh"


namespace {

class y532xl_state : public driver_device
{
public:
	y532xl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd(*this, "lcd"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "lcd%u", 0U)
	{ }

	// machine configs
	void y532xl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<hd61603_device> m_lcd;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<3> m_inputs;
	output_finder<8> m_out_digit;
	output_finder<64> m_out_lcd;

	u8 m_led_data = 0;
	u8 m_cb_mux = 0xff;
	u8 m_control = 0xff;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void lcd_seg_w(u64 data);
	u8 input_r();
	void cb_w(u8 data);
	void led_w(u8 data);
	void control_w(offs_t offset, u8 data);
};

void y532xl_state::machine_start()
{
	m_out_digit.resolve();
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_led_data));
	save_item(NAME(m_cb_mux));
	save_item(NAME(m_control));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void y532xl_state::update_display()
{
	m_display->matrix(~(m_control << 8 | m_cb_mux), m_led_data);
}

void y532xl_state::lcd_seg_w(u64 data)
{
	// output individual segments
	for (int i = 0; i < 64; i++)
		m_out_lcd[i] = BIT(data, i);

	// output digits
	for (int i = 0; i < 8; i++)
	{
		u8 digit_data = bitswap<8>(data,7,3,2,4,5,6,0,1);
		m_out_digit[i] = (i & 3) ? digit_data : digit_data & 0x7f; // no DP for last digit
		data >>= 8;
	}
}

u8 y532xl_state::input_r()
{
	u8 data = 0;

	// read side panel buttons
	for (int i = 0; i < 3; i++)
		if (BIT(~m_control, i))
			data |= m_inputs[i]->read();

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(~m_cb_mux, i))
			data |= m_board->read_file(i);

	return data;
}

void y532xl_state::cb_w(u8 data)
{
	// d0-d3: lcd data (see control_w)
	// d0-d7: chessboard input mux
	m_cb_mux = data;
	update_display();
}

void y532xl_state::led_w(u8 data)
{
	// d0-d7: led data
	m_led_data = data;
	update_display();
}

void y532xl_state::control_w(offs_t offset, u8 data)
{
	u8 prev = m_control;

	// a0-a2,d0: 74259
	u8 mask = 1 << offset;
	m_control = (m_control & ~mask) | ((data & 1) ? mask : 0);

	// Q0-Q2: button input mux
	// Q0: select side leds
	update_display();

	// Q3,Q4: speaker out
	m_dac->write(m_control >> 3 & 3);

	// Q7 rising edge: write to lcd
	if (~prev & m_control & 0x80)
		m_lcd->data_w(m_cb_mux & 0xf);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void y532xl_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w(FUNC(y532xl_state::cb_w));
	map(0x2080, 0x2080).w(FUNC(y532xl_state::led_w));
	map(0x2100, 0x2100).r(FUNC(y532xl_state::input_r));
	map(0x2180, 0x2187).w(FUNC(y532xl_state::control_w));
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( y532xl )
	PORT_START("IN.0")
	PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Show Move")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Clear Board")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multi Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Teach")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Take Back")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / Forward")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C / 3 / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D / 4 / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E / 5 / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F / 6 / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G / 7 / Black")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H / 8 / White")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void y532xl_state::y532xl(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &y532xl_state::main_map);

	const attotime nmi_period = attotime::from_hz(4_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(y532xl_state::nmi_line_pulse), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	HD61603(config, m_lcd, 0);
	m_lcd->write_segs().set(FUNC(y532xl_state::lcd_seg_w));

	PWM_DISPLAY(config, m_display).set_size(9, 8);
	config.set_default_layout(layout_yeno_532xl);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( y532xl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("532xl_00674.u9", 0x8000, 0x8000, CRC(8ded1df6) SHA1(6bcbebb8e1fde472e1ccd0036cb3f00b27f72d89) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, y532xl, 0,      0,      y532xl,  y532xl, y532xl_state, empty_init, "Yeno", "532 XL (Yeno)", MACHINE_SUPPORTS_SAVE )
