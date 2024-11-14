// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

AVR-Max Chess Computer, aka "SHAH"
In Germany it was named AVR-Max-Schachzwerg

The chess engine is H.G. Muller's micro-Max 4.8, ATMega88 port and prototype
hardware design by Andre Adrian. PCB finalization by Elektor, published in
Elektor magazine in 2009.

The LCD version uses Elektor's ATM18 (aka CC2) system, and a 2-wire LCD module.
It was not manufactured by Elektor, only described as a homebrew project.

FN 1 = new game, FN 2 = set level, FN 3 = principle variation.
Moves are confirmed by pressing GO twice.

The chess program is the same for all versions, only the display differs.

Hardware notes:

LED version:
- PCB label 081101-1 (c)Elektor v1.4
- Atmel ATMega88P-20PU @ ~8MHz, 8KB internal ROM
- 4-digit 7seg display, button panel, no sound

LCD version:
- Elektor ATM18 Testboard (has ATmega88 TQFP32 @ 16MHz)
- Elektor 2-wire LCD board (CD4094, HD44780U, 4*20 display)
- custom keypad board

Elektor magazine references (English):
- 04/2008: ATM18 AVR Board
- 05/2008: Two-wire LCD
- 09/2009: ATM18 Mini Chess Computer
- 11/2009: AVR-Max Chess Computer

For the German magazine, AVR-Max-Schachzwerg and CC2-Schachzwerg are in 06/2009.

TODO:
- AVR8 SLEEP opcode is not working, it's used for power-saving here and was
  harmless to hack out, but needs to be put back when it's emulated

*******************************************************************************/

#include "emu.h"

#include "cpu/avr8/avr8.h"
#include "video/hd44780.h"
#include "video/pwm.h"

#include "screen.h"

// internal artwork
#include "avrmax.lh"
#include "atm18mcc.lh"


namespace {

class avrmax_state : public driver_device
{
public:
	avrmax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_digit_pwm(*this, "digit_pwm"),
		m_lcd(*this, "lcd"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void base(machine_config &config);
	void avrmax(machine_config &config);
	void atm18mcc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<atmega88_device> m_maincpu;
	optional_device<pwm_display_device> m_digit_pwm;
	optional_device<hd44780_device> m_lcd;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_shift_reg = 0;
	int m_shift_clk = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void input_w(u8 data);
	u8 input_r();
	void digit_w(u8 data);
	void segment_w(u8 data);
	void lcd_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void avrmax_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_shift_reg));
	save_item(NAME(m_shift_clk));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common

void avrmax_state::input_w(u8 data)
{
	// PC0-PC3: input mux
	m_inp_mux = ~data & 0xf;
}

u8 avrmax_state::input_r()
{
	u8 data = 0;

	// PB0-PB2: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data & 7;
}


// LED handlers

void avrmax_state::digit_w(u8 data)
{
	// PC0-PC3: also digit select
	input_w(data);
	m_digit_pwm->write_my(m_inp_mux);
}

void avrmax_state::segment_w(u8 data)
{
	// PD0-PD7: digit 7seg data
	m_digit_pwm->write_mx(~data);
}


// LCD handlers

u32 avrmax_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xffffff, cliprect);
	const u8 *render = m_lcd->render();

	// draw lcd characters
	for (int i = 0; i < 80; i++)
	{
		const u8 *src = render + 16 * i;
		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 5; x++)
			{
				int row = bitswap<2>(i / 20, 0, 1);
				int col = i % 20;
				u32 color = (BIT(src[y], 4 - x)) ? 0 : 0xe8e8e8;
				bitmap.pix(row * 9 + y + 1, col * 6 + x + 1) = color;
			}
		}
	}

	return 0;
}

void avrmax_state::lcd_w(u8 data)
{
	// PD2: CD4094 data
	int shift_data = BIT(data, 2);

	// PD3: CD4094 clock
	int shift_clk = BIT(data, 3);
	if (shift_clk && !m_shift_clk)
		m_shift_reg = (m_shift_reg << 1) | shift_data;
	m_shift_clk = shift_clk;

	// CD4094 Q2-Q5: LCD data
	// CD4094 Q6: LCD RS
	// CD4094 Q7: LCD E (ANDed with CD4094 data)
	m_lcd->db_w(m_shift_reg << 3 & 0xf0);
	m_lcd->rs_w(BIT(m_shift_reg, 5));
	m_lcd->e_w(BIT(m_shift_reg, 6) & shift_data);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void avrmax_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void avrmax_state::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( avrmax )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("FN") PORT_CODE(KEYCODE_N)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("GO") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void avrmax_state::base(machine_config &config)
{
	// basic machine hardware
	ATMEGA88(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &avrmax_state::main_map);
	m_maincpu->set_addrmap(AS_DATA, &avrmax_state::data_map);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->gpio_in<atmega88_device::GPIOB>().set(FUNC(avrmax_state::input_r));
	m_maincpu->gpio_out<atmega88_device::GPIOC>().set(FUNC(avrmax_state::input_w));
}

void avrmax_state::avrmax(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_clock(8'000'000); // internal R/C clock
	m_maincpu->gpio_out<atmega88_device::GPIOC>().set(FUNC(avrmax_state::digit_w));
	m_maincpu->gpio_out<atmega88_device::GPIOD>().set(FUNC(avrmax_state::segment_w));

	// video hardware
	PWM_DISPLAY(config, m_digit_pwm).set_size(4, 8);
	m_digit_pwm->set_segmask(0xf, 0xff);
	config.set_default_layout(layout_avrmax);
}

void avrmax_state::atm18mcc(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->gpio_out<atmega88_device::GPIOD>().set(FUNC(avrmax_state::lcd_w));

	// video hardware
	auto &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(20 * 6 + 1, 4 * 9 + 1);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(avrmax_state::screen_update));

	HD44780U(config, m_lcd, 270'000); // TODO: clock not measured, datasheet typical clock used
	// HD44780UA02 is required for certain international characters in cc2schach,
	// the English version can optionally use a more standard HD44780[U]A00 display
	m_lcd->set_default_bios_tag("a02");

	config.set_default_layout(layout_atm18mcc);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( avrmax )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "elektor_081101-41.ic1", 0x0000, 0x2000, CRC(86d2a654) SHA1(3c235b8f6f735eaf408f54cbf44872166e7161d5) ) // avrmax_en-v1.0.hex

	// HACK: changed SLEEP to NOP
	ROM_FILL( 0x025c, 2, 0x00 )
	ROM_FILL( 0x0f8e, 2, 0x00 )
	ROM_FILL( 0x0fde, 2, 0x00 )
	ROM_FILL( 0x1020, 2, 0x00 )
	ROM_FILL( 0x1060, 2, 0x00 )
	ROM_FILL( 0x129a, 2, 0x00 )

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )
ROM_END

ROM_START( avrmaxg )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "elektor_081101-41_d.ic1", 0x0000, 0x2000, CRC(18ec7a56) SHA1(a018421aa0ad8cce3d852f7519dec3691f3c55a0) ) // avrmax_de-v1.0.hex

	// HACK: changed SLEEP to NOP
	ROM_FILL( 0x025c, 2, 0x00 )
	ROM_FILL( 0x0f8e, 2, 0x00 )
	ROM_FILL( 0x0fde, 2, 0x00 )
	ROM_FILL( 0x1020, 2, 0x00 )
	ROM_FILL( 0x1060, 2, 0x00 )
	ROM_FILL( 0x129a, 2, 0x00 )

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )
ROM_END

ROM_START( atm18mcc )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "avrmax_cc2_en-v1.0.bin", 0x0000, 0x2000, CRC(715f4642) SHA1(d20739c6caa49a01e002b3dfbf0f39abf7992540) )

	// HACK: changed SLEEP to NOP
	ROM_FILL( 0x05da, 2, 0x00 )
	ROM_FILL( 0x1466, 2, 0x00 )
	ROM_FILL( 0x14a2, 2, 0x00 )
	ROM_FILL( 0x14de, 2, 0x00 )
	ROM_FILL( 0x1528, 2, 0x00 )
	ROM_FILL( 0x16fe, 2, 0x00 )

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )
ROM_END

ROM_START( cc2schach )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "avrmax_cc2_de-v1.0.bin", 0x0000, 0x2000, CRC(64cfc646) SHA1(4c371ea9f48c8745cf5f5bcf10973838e239e564) )

	// HACK: changed SLEEP to NOP
	ROM_FILL( 0x05da, 2, 0x00 )
	ROM_FILL( 0x1466, 2, 0x00 )
	ROM_FILL( 0x14a2, 2, 0x00 )
	ROM_FILL( 0x14de, 2, 0x00 )
	ROM_FILL( 0x1528, 2, 0x00 )
	ROM_FILL( 0x16fe, 2, 0x00 )

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 2009, avrmax,    0,        0,      avrmax,   avrmax, avrmax_state, empty_init, "Elektor", "AVR-Max Chess Computer (English)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 2009, avrmaxg,   avrmax,   0,      avrmax,   avrmax, avrmax_state, empty_init, "Elektor", "AVR-Max-Schachzwerg (German)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW ) // German 'text'

SYST( 2009, atm18mcc,  0,        0,      atm18mcc, avrmax, avrmax_state, empty_init, "Elektor", "ATM18 Mini Chess Computer (English)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 2009, cc2schach, atm18mcc, 0,      atm18mcc, avrmax, avrmax_state, empty_init, "Elektor", "CC2-Schachzwerg (German)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
