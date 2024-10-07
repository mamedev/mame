// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

CXG "Adversary" Sphinx 40 / 50

This is a modular chesscomputer, similar to Mephisto's 3-drawers one.
Chesscomputer on the right, LCD in the middle, and future expansion on the left.
The only difference between 40 and 50 is the board size (40cm vs 50cm).

The chess engine is Cyrus 68K, by Mark Taylor, with advice from David Levy.
It's not related to the Z80 version of Cyrus, only by name.

This chessboard was also used on the Sphinx 40 / 50 Plus, which is another
incarnation of Frans Morsch's Dominator program.

TODO:
- unmapped read from 0x200000, looks like expansion ROM
- verify XTAL and irq source/frequency

Hardware notes:

Distribution board:
- PCB label: C 1987 CXG SYSTEMS S.A 68K 600 203
- Hitachi HD46821P (6821 PIA)
- piezo, connector to chessboard (magnet sensors, 8*8 leds)

Program/CPU module:
- PCB label: (C) 1987 NEWCREST TECHNOLOGY LTD, CXG-68K-600-001
- daughterboard for buttons, label CXG SYSTEMS, 68K-600-101
- Signetics SCN68000C8N64 @ 8MHz
- 64KB ROM (2*GI 27256-20)
- 128KB RAM (4*KM41464AP-12 64kx4 DRAM)
- 2KB battery-backed RAM (NEC D449C)

LCD module
- PCB label: CXG-68K-600-302
- Hitachi HD61603 LCD Driver
- 2 displays (4 digits each)

*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"
#include "video/hd61603.h"

#include "speaker.h"

// internal artwork
#include "cxg_sphinx40.lh"


namespace {

class sphinx40_state : public driver_device
{
public:
	sphinx40_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram", 0x800, ENDIANNESS_BIG),
		m_pia(*this, "pia"),
		m_lcd(*this, "lcd"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_inputs(*this, "IN.%u", 0),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "lcd%u", 0U)
	{ }

	// machine configs
	void sphinx40(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	memory_share_creator<u8> m_nvram;
	required_device<pia6821_device> m_pia;
	required_device<hd61603_device> m_lcd;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_ioport_array<4> m_inputs;
	output_finder<8> m_out_digit;
	output_finder<64> m_out_lcd;

	u8 m_cb_mux = 0;
	u8 m_led_data = 0;
	u8 m_inp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void lcd_seg_w(u64 data);

	void update_display();
	void cb_mux_w(u8 data);
	void cb_leds_w(u8 data);
	u8 cb_r();

	u8 input_r();
	void input_w(u8 data);
	void lcd_w(u8 data);

	u8 nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, u8 data) { m_nvram[offset] = data; }
};

void sphinx40_state::machine_start()
{
	m_out_digit.resolve();
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_cb_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// HD61603 LCD

void sphinx40_state::lcd_seg_w(u64 data)
{
	// output individual segments
	for (int i = 0; i < 64; i++)
		m_out_lcd[i] = BIT(data, i);

	// output digits
	for (int i = 0; i < 8; i++)
	{
		m_out_digit[i] = data & 0x7f;
		data >>= 8;
	}
}


// 6821 PIA

void sphinx40_state::update_display()
{
	m_display->matrix(m_cb_mux, m_led_data);
}

void sphinx40_state::cb_mux_w(u8 data)
{
	// PA0-PA7: chessboard input/led mux
	m_cb_mux = ~data;
	update_display();
}

void sphinx40_state::cb_leds_w(u8 data)
{
	// PB0-PB7: chessboard leds
	m_led_data = ~data;
	update_display();
}


// TTL

u8 sphinx40_state::cb_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard)
	for (int i = 0; i < 8; i++)
		if (BIT(m_cb_mux, i))
			data |= m_board->read_rank(i, true);

	return data;
}

void sphinx40_state::input_w(u8 data)
{
	// d0-d3: input mux (buttons)
	m_inp_mux = ~data & 0xf;
}

u8 sphinx40_state::input_r()
{
	u8 data = 0;

	// d0-d4: multiplexed inputs (buttons)
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data & 0x1f;
}

void sphinx40_state::lcd_w(u8 data)
{
	// d0-d3: HD61603 data
	m_lcd->data_w(data & 0xf);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sphinx40_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x11ffff).ram();
	map(0x400000, 0x400fff).rw(FUNC(sphinx40_state::nvram_r), FUNC(sphinx40_state::nvram_w)).umask16(0x00ff);
	map(0x70fcf1, 0x70fcf1).r(FUNC(sphinx40_state::cb_r));
	map(0x70fd71, 0x70fd71).r(FUNC(sphinx40_state::input_r));
	map(0x70fdb1, 0x70fdb1).w(FUNC(sphinx40_state::input_w));
	map(0x70fde0, 0x70fde0).w(FUNC(sphinx40_state::lcd_w));
	map(0x70fff0, 0x70fff7).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0xff00);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sphinx40 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Clock")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Function")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4) PORT_NAME("Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Forwards")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("What If?")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3) PORT_NAME("Bishop")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2) PORT_NAME("Knight")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Backwards")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Analysis")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1) PORT_NAME("Pawn")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("White")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set-Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5) PORT_NAME("Queen")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sphinx40_state::sphinx40(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sphinx40_state::main_map);

	const attotime irq_period = attotime::from_hz(8'000'000 / 0x1000);
	m_maincpu->set_periodic_int(FUNC(sphinx40_state::irq4_line_hold), irq_period);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(sphinx40_state::cb_mux_w));
	m_pia->writepb_handler().set(FUNC(sphinx40_state::cb_leds_w));
	m_pia->cb2_handler().set("dac", FUNC(dac_1bit_device::write));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	HD61603(config, m_lcd, 0);
	m_lcd->write_segs().set(FUNC(sphinx40_state::lcd_seg_w));

	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_cxg_sphinx40);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sphinx40 )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("gold.u3",   0x0000, 0x8000, CRC(e7cccd12) SHA1(4542f62963ab78796626c0c938e39e715d1c19f8) )
	ROM_LOAD16_BYTE("orange.u2", 0x0001, 0x8000, CRC(9e0bbd15) SHA1(5867f35489d15c1e395f6b2aa91a76d74ad6f2f4) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, sphinx40, 0,      0,      sphinx40, sphinx40, sphinx40_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Sphinx 40", MACHINE_SUPPORTS_SAVE )
