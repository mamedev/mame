// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Mephisto Monte Carlo
Mephisto Monte Carlo IV
Mephisto Monte Carlo IV: Limited Edition

The chess engine is by Frans Morsch, but for the IV version it's by Ed Schroeder.
Limited Edition has a twice faster CPU.

Hardware notes:
- R65C02P4 @ 4MHz
- 8KB RAM(battery-backed), 32KB ROM
- expansion slot at underside (not used)
- 2*PCF2112, 2 7seg LCD screens
- 8 tri-color leds (like academy, always 6 red and 2 green?)
- magnetic chessboard with 64 leds, piezo

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"

#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/pcf2100.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_montec.lh"


namespace {

class montec_state : public driver_device
{
public:
	montec_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_latch(*this, "lcd_latch"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd(*this, "lcd%u", 0),
		m_keys(*this, "KEY.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	void montec(machine_config &config);
	void montec4(machine_config &config);
	void montec4le(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<hc259_device> m_lcd_latch;
	required_device<pwm_display_device> m_led_pwm;
	required_device_array<pcf2112_device, 2> m_lcd;
	required_ioport_array<2> m_keys;
	output_finder<8> m_digits;

	void montec_mem(address_map &map) ATTR_COLD;

	template<int N> void lcd_output_w(u32 data);
	void led_w(u8 data);
	void irq_ack_w(u8 data) { irq_ack_r(); }
	u8 irq_ack_r();
	u8 input_r();
};

void montec_state::machine_start()
{
	m_digits.resolve();
}



/*******************************************************************************
    I/O
*******************************************************************************/

template<int N>
void montec_state::lcd_output_w(u32 data)
{
	// lcd segment outputs
	for (int i = 0; i < 4; i++)
		m_digits[i + N*4] = bitswap<8>(data >> (8 * i), 7,4,5,0,1,2,3,6);
}

void montec_state::led_w(u8 data)
{
	// d0-d3: keypad led select
	// d4-d7: keypad led data
	m_led_pwm->matrix(data & 0xf, ~data >> 4 & 0xf);
}

u8 montec_state::irq_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}

u8 montec_state::input_r()
{
	u8 data = 0;

	// 74259 q0/q1 selects keypad
	for (int i = 0; i < 2; i++)
		if (!BIT(m_lcd_latch->output_state(), i))
			data |= m_keys[i]->read();

	return ~m_board->input_r() | data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void montec_state::montec_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).rw(FUNC(montec_state::irq_ack_r), FUNC(montec_state::irq_ack_w));
	map(0x2400, 0x2400).r(FUNC(montec_state::input_r));
	map(0x2800, 0x2800).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c00).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0x3000, 0x3007).w(m_lcd_latch, FUNC(hc259_device::write_d7)).nopr();
	map(0x3400, 0x3400).w(FUNC(montec_state::led_w));
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( montec )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("BOOK / 9")   PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS / 0")    PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM")        PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO")       PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL")         PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV")        PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT")        PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("RES")        PORT_CODE(KEYCODE_F1)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Pawn / 1")   PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Knight / 2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Bishop / 3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Rook / 4")   PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Queen / 5")  PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("King / 6")   PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Black / 7")  PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("White / 8")  PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void montec_state::montec(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 8_MHz_XTAL / 2); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &montec_state::montec_mem);

	const attotime irq_period = attotime::from_hz(8_MHz_XTAL / 2 / 0x2000);
	m_maincpu->set_periodic_int(FUNC(montec_state::irq0_line_assert), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HC259(config, m_lcd_latch);
	m_lcd_latch->q_out_cb<2>().set("dac", FUNC(dac_1bit_device::write)).invert();
	m_lcd_latch->q_out_cb<4>().set(m_lcd[0], FUNC(pcf2112_device::data_w));
	m_lcd_latch->q_out_cb<4>().append(m_lcd[1], FUNC(pcf2112_device::data_w));
	m_lcd_latch->q_out_cb<5>().set(m_lcd[1], FUNC(pcf2112_device::dlen_w));
	m_lcd_latch->q_out_cb<6>().set(m_lcd[0], FUNC(pcf2112_device::clb_w));
	m_lcd_latch->q_out_cb<6>().append(m_lcd[1], FUNC(pcf2112_device::clb_w));
	m_lcd_latch->q_out_cb<7>().set(m_lcd[0], FUNC(pcf2112_device::dlen_w));

	MEPHISTO_SENSORS_BOARD(config, m_board); // internal
	m_board->set_delay(attotime::from_msec(300));

	// video hardware
	PCF2112(config, m_lcd[0], 50); // frequency guessed
	m_lcd[0]->write_segs().set(FUNC(montec_state::lcd_output_w<0>));
	PCF2112(config, m_lcd[1], 50); // "
	m_lcd[1]->write_segs().set(FUNC(montec_state::lcd_output_w<1>));

	PWM_DISPLAY(config, m_led_pwm).set_size(4, 4);
	config.set_default_layout(layout_mephisto_montec);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void montec_state::montec4(machine_config &config)
{
	montec(config);
	m_board->set_delay(attotime::from_msec(150));
}

void montec_state::montec4le(machine_config &config)
{
	montec4(config);

	// basic machine hardware
	M65C02(config.replace(), m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &montec_state::montec_mem);

	const attotime irq_period = attotime::from_hz(8_MHz_XTAL / 0x4000);
	m_maincpu->set_periodic_int(FUNC(montec_state::irq0_line_assert), irq_period);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( montec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mc3_12.11.87", 0x8000, 0x8000, CRC(8eb26043) SHA1(26454a37eea29283bbb2762a3a68e95e4be6aa1c) )
ROM_END

ROM_START( monteca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mc2_20.7.87", 0x8000, 0x8000, CRC(05524da9) SHA1(bee2ffe09a27095f733584e0fb1203b95c23e17e) )
ROM_END


ROM_START( montec4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mc4.bin", 0x8000, 0x8000, CRC(b231be57) SHA1(8bb39db6a3f476090574ef6c681a241a96cfbd5c) )
ROM_END

ROM_START( montec4le )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mc4le.bin", 0x8000, 0x8000, CRC(c4887694) SHA1(7f482d2a40fcb3125266e7a5407da315b4f9b49c) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT      CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, montec,     0,        0,      montec,    montec,    montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo (ver. MC3)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, monteca,    montec,   0,      montec,    montec,    montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo (ver. MC2)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, montec4,    0,        0,      montec4,   montec,    montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo IV", MACHINE_SUPPORTS_SAVE )
SYST( 1990, montec4le,  montec4,  0,      montec4le, montec,    montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo IV: Limited Edition", MACHINE_SUPPORTS_SAVE )
