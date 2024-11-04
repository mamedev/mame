// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:Berger
/*******************************************************************************

Mephisto Mondial 68000XL

It was made for the US market. The chess engine is actually the one from
Mephisto Dallas.

Hardware notes:
- TS68000CP12 @ 12MHz
- 64KB ROM
- 16KB RAM
- PCF2112T LCD driver

TODO:
- does it have waitstates, like glasgow/amsterdam?

*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pcf2100.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_mondial68k.lh"


namespace {

class mondial68k_state : public driver_device
{
public:
	mondial68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_latch(*this, "lcd_latch"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd(*this, "lcd"),
		m_inputs(*this, "IN.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	void mondial68k(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<hc259_device> m_lcd_latch;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pcf2112_device> m_lcd;
	required_ioport_array<4> m_inputs;
	output_finder<4> m_digits;

	u8 m_input_mux = 0xff;
	u8 m_board_mux = 0xff;

	void mondial68k_mem(address_map &map) ATTR_COLD;

	void update_leds();
	void lcd_output_w(u32 data);
	void input_mux_w(u8 data);
	void board_mux_w(u8 data);
	u8 inputs_r();
};

void mondial68k_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_input_mux));
	save_item(NAME(m_board_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void mondial68k_state::update_leds()
{
	m_led_pwm->matrix(m_input_mux >> 6, ~m_board_mux);
}

void mondial68k_state::lcd_output_w(u32 data)
{
	// output LCD digits (note: last digit DP segment is unused)
	for (int i = 0; i < 4; i++)
		m_digits[i] = bitswap<8>((data & 0x7fffffff) >> (8 * i), 7,4,5,0,1,2,3,6);
}

void mondial68k_state::board_mux_w(u8 data)
{
	// d0-d7: chessboard mux, led data
	m_board_mux = data;
	update_leds();
}

void mondial68k_state::input_mux_w(u8 data)
{
	// d0-d3: button mux
	// d6,d7: led select
	m_input_mux = data;
	update_leds();
}

u8 mondial68k_state::inputs_r()
{
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 4; i++)
		if (!BIT(m_input_mux, i))
			data |= m_inputs[i]->read();

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (!BIT(m_board_mux, i))
			data |= m_board->read_rank(i);

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mondial68k_state::mondial68k_mem(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x800000, 0x800000).r(FUNC(mondial68k_state::inputs_r));
	map(0x820000, 0x82000f).nopr().w(m_lcd_latch, FUNC(hc259_device::write_d0)).umask16(0xff00);
	map(0x840000, 0x840000).w(FUNC(mondial68k_state::input_mux_w));
	map(0x860000, 0x860000).w(FUNC(mondial68k_state::board_mux_w));
	map(0xc00000, 0xc03fff).ram();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mondial68k )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // combine ENT/CL for NEW GAME

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) // combine ENT/CL for NEW GAME
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mondial68k_state::mondial68k(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mondial68k_state::mondial68k_mem);

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL / 256); // 128Hz
	m_maincpu->set_periodic_int(FUNC(mondial68k_state::irq6_line_hold), irq_period);

	HC259(config, m_lcd_latch);
	m_lcd_latch->q_out_cb<0>().set(m_lcd, FUNC(pcf2112_device::clb_w));
	m_lcd_latch->q_out_cb<1>().set(m_lcd, FUNC(pcf2112_device::data_w));
	m_lcd_latch->q_out_cb<2>().set(m_lcd, FUNC(pcf2112_device::dlen_w));
	m_lcd_latch->parallel_out_cb().set("dac", FUNC(dac_2bit_ones_complement_device::write)).rshift(6).mask(0x3);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	// video hardware
	PCF2112(config, m_lcd, 50); // frequency guessed
	m_lcd->write_segs().set(FUNC(mondial68k_state::lcd_output_w));

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_mephisto_mondial68k);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mondl68k )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("68000xl_u_06.11.87", 0x0000, 0x8000, CRC(aebe482a) SHA1(900c91ec836cd65e4cd38e50555976ab8064be41) )
	ROM_LOAD16_BYTE("68000xl_l_06.11.87", 0x0001, 0x8000, CRC(564e32c5) SHA1(8c9df46bc5ced114e72fb663f1055d775b8e2e0b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE      INPUT       CLASS             INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, mondl68k,  0,        0,      mondial68k,  mondial68k, mondial68k_state, empty_init, "Hegener + Glaser", "Mephisto Mondial 68000XL", MACHINE_SUPPORTS_SAVE )
