// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Kasparov Prisma (model 281)

NOTE: Before exiting MAME, press the STOP button to turn the power off. Otherwise,
NVRAM won't save properly. To force a cold boot, hold the PLAY button and trigger
a power on/reset (F3).

It's the 'sequel' to Simultano, and the first chess computer with a H8 CPU. Even
though H8 is much faster than 6502, it plays weaker, probably due to less RAM.
And/or it could also be due to the programmer(s) being unfamiliar with H8.

Hardware notes:
- PCB label: ST9A-PE-001
- Hitachi H8/325 MCU, 20MHz XTAL
- Epson SED1502F, LCD screen (same as Saitek Simultano)
- piezo, 16+3 LEDs, button sensors chessboard

In 1992, it was also sold by Tandy as Chess Champion 2150L, still manufactured
by Saitek. Overall, the hardware is the same, but with a slower CPU (16MHz XTAL).

TODO:
- are older versions of Prisma on PROM H8/325 just like with Blitz?

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"
#include "video/sed1500.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_prisma.lh"


namespace {

class prisma_state : public driver_device
{
public:
	prisma_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_lcd(*this, "lcd"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void prisma(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<sed1502_device> m_lcd;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;
	output_finder<16, 34> m_out_lcd;

	u8 m_lcd_data = 0;
	u8 m_lcd_address = 0;
	u8 m_lcd_control = 0;
	u8 m_inp_mux = 0;
	u8 m_led_select = 0;
	u8 m_led_direct = 0;

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void lcd_output_w(offs_t offset, u64 data);

	void standby(int state);
	void update_leds();

	void p1_w(u8 data);
	void p2_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
	void p6_w(u8 data);
	u8 p7_r();
};

void prisma_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_address));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_direct));
}

INPUT_CHANGED_MEMBER(prisma_state::change_cpu_freq)
{
	// 12MHz and 24MHz versions don't exist, but the software supports it
	static const XTAL freq[4] = { 16_MHz_XTAL, 20_MHz_XTAL, 24_MHz_XTAL, 12_MHz_XTAL };
	m_maincpu->set_unscaled_clock(freq[bitswap<2>(newval,7,0)]);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void prisma_state::standby(int state)
{
	if (state)
	{
		// clear display
		for (int i = 0; i < 0x80; i++)
			m_lcd->write(i, 0);

		m_led_pwm->clear();
		m_lcd_pwm->clear();
	}
}

INPUT_CHANGED_MEMBER(prisma_state::go_button)
{
	if (newval && m_maincpu->standby())
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


// LCD

void prisma_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void prisma_state::lcd_output_w(offs_t offset, u64 data)
{
	if (!m_maincpu->standby())
		m_lcd_pwm->write_row(offset, data);
}


// misc

void prisma_state::update_leds()
{
	m_led_pwm->matrix_partial(0, 2, m_led_select, m_inp_mux);
	m_led_pwm->matrix_partial(2, 1, 1, m_led_direct);
}

void prisma_state::p1_w(u8 data)
{
	// P10-P13: direct leds
	m_led_direct = (data & 0xf) ^ 3;
	update_leds();

	// P14: speaker out
	m_dac->write(BIT(data, 4));

	// P16: ext power (no need to emulate it)
}

void prisma_state::p2_w(u8 data)
{
	// P20-P27: input mux, led data
	m_inp_mux = bitswap<8>(~data,7,6,5,4,0,3,1,2);
	update_leds();
}

u8 prisma_state::p3_r()
{
	// P30-P37: LCD data (never reads here)
	logerror("read from LCD\n");
	return 0xff;
}

void prisma_state::p3_w(u8 data)
{
	// P30-P37: LCD data
	m_lcd_data = bitswap<8>(data,3,4,5,6,7,0,1,2);
}

void prisma_state::p4_w(u8 data)
{
	// P40: LCD CS
	// P41: LCD RD
	// P42: LCD WR
	if (~data & m_lcd_control & 4 && ~data & 1)
		m_lcd->write(m_lcd_address, m_lcd_data);
	m_lcd_control = data;
}

u8 prisma_state::p5_r()
{
	u8 data = 0;

	// P50-P52: read buttons
	for (int i = 0; i < 3; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	// P53: battery status
	data |= m_inputs[3]->read() << 3;

	return ~data | 0xf0;
}

void prisma_state::p5_w(u8 data)
{
	// P54,P55: led select
	m_led_select = ~data >> 4 & 3;
	update_leds();
}

void prisma_state::p6_w(u8 data)
{
	// P60-P66: LCD address
	m_lcd_address = data & 0x7f;
}

u8 prisma_state::p7_r()
{
	u8 data = 0;

	// P70-P77: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	return bitswap<8>(~data,6,5,3,2,0,1,7,4);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( prisma )
	PORT_START("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Swap Side")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("New Game")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_CONFNAME( 0x81, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, prisma_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x81, "12MHz (unofficial)" )
	PORT_CONFSETTING(    0x00, "16MHz (Chess Champion 2150L)" )
	PORT_CONFSETTING(    0x01, "20MHz (Prisma)" )
	PORT_CONFSETTING(    0x80, "24MHz (unofficial)" )

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Normal")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Stop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Analysis")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Info")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Function")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Sound")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Set Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.3")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, prisma_state, go_button, 0) PORT_NAME("Go")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void prisma_state::prisma(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->nvram_set_default_value(~0);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(prisma_state::standby));
	m_maincpu->write_port1().set(FUNC(prisma_state::p1_w));
	m_maincpu->write_port2().set(FUNC(prisma_state::p2_w));
	m_maincpu->read_port3().set(FUNC(prisma_state::p3_r));
	m_maincpu->write_port3().set(FUNC(prisma_state::p3_w));
	m_maincpu->read_port4().set_constant(0xff);
	m_maincpu->write_port4().set(FUNC(prisma_state::p4_w));
	m_maincpu->read_port5().set(FUNC(prisma_state::p5_r));
	m_maincpu->write_port5().set(FUNC(prisma_state::p5_w));
	m_maincpu->write_port6().set(FUNC(prisma_state::p6_w));
	m_maincpu->read_port7().set(FUNC(prisma_state::p7_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	SED1502(config, m_lcd, 32768).write_segs().set(FUNC(prisma_state::lcd_output_w));
	PWM_DISPLAY(config, m_lcd_pwm).set_size(16, 34);
	m_lcd_pwm->set_refresh(attotime::from_hz(30));
	m_lcd_pwm->output_x().set(FUNC(prisma_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(873/2, 1080/2);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2+1, 8);
	config.set_default_layout(layout_saitek_prisma);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( prisma )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("90_saitek_86051150st9_3258l02p.u1", 0x0000, 0x8000, CRC(b6f8384f) SHA1(a4e8a4a45009c15bda1778512a87dea756aae6d8) )

	ROM_REGION( 795951, "screen", 0 )
	ROM_LOAD("simultano.svg", 0, 795951, CRC(ac9942bb) SHA1(f9252e5bf7b8af698a403c3f8f5ea9e475e0bf0b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1990, prisma, 0,      0,      prisma,  prisma, prisma_state, empty_init, "Saitek / Heuristic Software", "Kasparov Prisma", MACHINE_SUPPORTS_SAVE )
