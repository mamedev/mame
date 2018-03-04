// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:yoyo_chessboard
/**************************************************************************************************

    Mephisto Monte Carlo
    Mephisto Mega IV
    Mephisto Monte Carlo IV LE
    Mephisto Mondial II
    Mephisto Super Mondial
    Mephisto Super Mondial II

**************************************************************************************************/


#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "machine/timer.h"
#include "screen.h"
#include "speaker.h"
#include "softlist.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "mephisto_montec.lh"
#include "mephisto_megaiv.lh"
#include "mephisto_mondial2.lh"
#include "mephisto_smondial2.lh"


class mephisto_montec_state : public driver_device
{
public:
	mephisto_montec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_beeper(*this, "beeper")
		, m_keys(*this, "KEY.%u", 0)
	{ }


	DECLARE_READ8_MEMBER(montec_input_r);
	DECLARE_READ8_MEMBER(montec_nmi_ack_r);
	DECLARE_WRITE8_MEMBER(montec_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(montec_mux_w);
	DECLARE_WRITE8_MEMBER(montec_led_w);
	DECLARE_WRITE8_MEMBER(montec_beeper_w);
	DECLARE_WRITE8_MEMBER(montec_lcd_data_w);
	DECLARE_WRITE8_MEMBER(montec_ldc_cs0_w);
	DECLARE_WRITE8_MEMBER(montec_ldc_cs1_w);
	DECLARE_WRITE8_MEMBER(montec_lcd_clk_w);

	DECLARE_READ8_MEMBER(megaiv_input_r);
	DECLARE_WRITE8_MEMBER(megaiv_led_w);

	DECLARE_WRITE8_MEMBER(smondial_board_mux_w);
	DECLARE_WRITE8_MEMBER(smondial_led_data_w);

	DECLARE_WRITE8_MEMBER(mondial2_input_mux_w);
	TIMER_DEVICE_CALLBACK_MEMBER(refresh_leds);

	void smondial(machine_config &config);
	void mondial2(machine_config &config);
	void smondial2(machine_config &config);
	void montec(machine_config &config);
	void monteciv(machine_config &config);
	void megaiv(machine_config &config);
	void megaiv_mem(address_map &map);
	void mondial2_mem(address_map &map);
	void montec_mem(address_map &map);
	void smondial2_mem(address_map &map);
	void smondial_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<beep_device> m_beeper;
	optional_ioport_array<2> m_keys;

	uint8_t m_lcd_mux;
	uint8_t m_input_mux;
	uint8_t m_leds_mux;
	uint8_t m_smondial_board_mux;

	struct display_t
	{
		uint8_t pos;
		int8_t  shift;
		uint8_t data;
		uint8_t bit;
	} m_display;
};


void mephisto_montec_state::machine_start()
{
	save_item(NAME(m_lcd_mux));
	save_item(NAME(m_input_mux));
	save_item(NAME(m_leds_mux));
	save_item(NAME(m_smondial_board_mux));
	save_item(NAME(m_display.pos));
	save_item(NAME(m_display.shift));
	save_item(NAME(m_display.data));
	save_item(NAME(m_display.bit));
}

void mephisto_montec_state::machine_reset()
{
	m_lcd_mux = 0x00;
	m_input_mux = 0x00;
	m_leds_mux = 0x00;
	m_smondial_board_mux = 0xff;

	m_display.pos = 0;
	m_display.shift = 0;
	m_display.data = 0;
	m_display.bit = 0;
}

WRITE8_MEMBER(mephisto_montec_state::montec_led_w)
{
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			if (BIT(data, i))
				output().set_led_value(100 + i * 4 + j, BIT(data, 4 + j) ? 0 : 1);
}


WRITE8_MEMBER(mephisto_montec_state::montec_lcd_data_w)
{
	m_display.bit = BIT(data, 7);
}

WRITE8_MEMBER(mephisto_montec_state::montec_ldc_cs0_w)
{
	if (data)
		m_lcd_mux |= 0x01;
	else
		m_lcd_mux &= ~0x01;

	m_display.pos = 0;
	m_display.shift = -1;
	m_display.data = 0;
}

WRITE8_MEMBER(mephisto_montec_state::montec_ldc_cs1_w)
{
	if (data)
		m_lcd_mux |= 0x02;
	else
		m_lcd_mux &= ~0x02;

	m_display.pos = 0;
	m_display.shift = -1;
	m_display.data = 0;
}

WRITE8_MEMBER(mephisto_montec_state::montec_lcd_clk_w)
{
	if (data)
	{
		m_display.data <<= 1;
		m_display.data |= m_display.bit;
		m_display.shift++;
	}

	if (m_display.shift == 8)
	{
		if (m_lcd_mux & 0x01)   output().set_digit_value(0 + m_display.pos, bitswap<8>(m_display.data, 0,3,2,7,6,5,4,1));
		if (m_lcd_mux & 0x02)   output().set_digit_value(4 + m_display.pos, bitswap<8>(m_display.data, 0,3,2,7,6,5,4,1));

		m_display.shift = 0;
		m_display.pos = (m_display.pos + 1) & 3;
		m_display.data = 0;
	}
}


WRITE8_MEMBER(mephisto_montec_state::montec_mux_w)
{
	if (data)
		m_input_mux &= ~(1 << offset);
	else
		m_input_mux |= (1 << offset);
}

READ8_MEMBER(mephisto_montec_state::montec_input_r)
{
	if      (m_input_mux & 0x01)    return m_keys[1]->read();
	else if (m_input_mux & 0x02)    return m_keys[0]->read();

	return m_board->input_r(space, offset) ^ 0xff;
}

READ8_MEMBER(mephisto_montec_state::montec_nmi_ack_r)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return 0;
}

WRITE8_MEMBER(mephisto_montec_state::montec_nmi_ack_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

WRITE8_MEMBER(mephisto_montec_state::montec_beeper_w)
{
	m_beeper->set_state(BIT(data, 7) ? 0 : 1);
}

WRITE8_MEMBER(mephisto_montec_state::megaiv_led_w)
{
	if (m_leds_mux != m_board->mux_r(space, offset))
	{
		m_leds_mux = m_board->mux_r(space, offset);
		for (int i=0; i<8; i++)
		{
			if (!BIT(m_leds_mux, i))
			{
				output().set_led_value(100 + i, BIT(data, 0) | BIT(data, 1));
				output().set_led_value(0 + i, BIT(data, 2) | BIT(data, 3));
				output().set_led_value(8 + i, BIT(data, 4) | BIT(data, 5));
			}
		}
	}

	m_beeper->set_state(BIT(data, 7));
}

READ8_MEMBER(mephisto_montec_state::megaiv_input_r)
{
	if      (m_input_mux & 0x01)    return BIT(m_keys[1]->read(), 0 + offset) << 7;
	else if (m_input_mux & 0x02)    return BIT(m_keys[1]->read(), 4 + offset) << 7;
	else if (m_input_mux & 0x04)    return BIT(m_keys[0]->read(), 0 + offset) << 7;
	else if (m_input_mux & 0x08)    return BIT(m_keys[0]->read(), 4 + offset) << 7;

	return BIT(m_board->input_r(space, offset), offset) << 7;
}


ADDRESS_MAP_START(mephisto_montec_state::montec_mem)
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2400, 0x2400 ) AM_READ(montec_input_r)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_w)
	AM_RANGE( 0x2c00, 0x2c00 ) AM_DEVWRITE("board", mephisto_board_device, led_w)
	AM_RANGE( 0x3400, 0x3400 ) AM_WRITE(montec_led_w)
	AM_RANGE( 0x3000, 0x3001 ) AM_WRITE(montec_mux_w)
	AM_RANGE( 0x3002, 0x3002 ) AM_WRITE(montec_beeper_w)
	AM_RANGE( 0x3004, 0x3004 ) AM_WRITE(montec_lcd_data_w)
	AM_RANGE( 0x3005, 0x3005 ) AM_WRITE(montec_ldc_cs1_w)
	AM_RANGE( 0x3006, 0x3006 ) AM_WRITE(montec_lcd_clk_w)
	AM_RANGE( 0x3007, 0x3007 ) AM_WRITE(montec_ldc_cs0_w)
	AM_RANGE( 0x2000, 0x2000 ) AM_READWRITE(montec_nmi_ack_r, montec_nmi_ack_w)
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

ADDRESS_MAP_START(mephisto_montec_state::megaiv_mem)
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2400, 0x2400 ) AM_WRITE(megaiv_led_w)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_w)
	AM_RANGE( 0x2c00, 0x2c03 ) AM_WRITE(montec_mux_w) AM_READNOP
	AM_RANGE( 0x2c04, 0x2c04 ) AM_WRITE(montec_lcd_data_w)
	AM_RANGE( 0x2c05, 0x2c05 ) AM_WRITE(montec_ldc_cs1_w)
	AM_RANGE( 0x2c06, 0x2c06 ) AM_WRITE(montec_lcd_clk_w)
	AM_RANGE( 0x2c07, 0x2c07 ) AM_WRITE(montec_ldc_cs0_w)
	AM_RANGE( 0x3000, 0x3007 ) AM_READ(megaiv_input_r)
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END


ADDRESS_MAP_START(mephisto_montec_state::smondial2_mem)
	AM_IMPORT_FROM(megaiv_mem)
	AM_RANGE( 0x4000, 0x7fff ) AM_DEVREAD("cartslot", generic_slot_device, read_rom)
ADDRESS_MAP_END


WRITE8_MEMBER(mephisto_montec_state::smondial_board_mux_w)
{
	if (data)
		m_smondial_board_mux &= ~(1 << offset);
	else
		m_smondial_board_mux |= (1 << offset);

	m_board->mux_w(space, offset, m_smondial_board_mux);

	for (int i=0; i<8; i++)
	{
		if (m_leds_mux & 0x03)      output().set_led_value(100 + i, BIT(m_smondial_board_mux, i) ? 0 : 1);
		if (m_leds_mux & 0x0c)      output().set_led_value(  8 + i, BIT(m_smondial_board_mux, i) ? 0 : 1);
		if (m_leds_mux & 0x30)      output().set_led_value(  0 + i, BIT(m_smondial_board_mux, i) ? 0 : 1);
	}
}

WRITE8_MEMBER(mephisto_montec_state::smondial_led_data_w)
{
	if (data & 0x80)
		m_leds_mux &= ~(1 << offset);
	else
		m_leds_mux |= (1 << offset);

	m_beeper->set_state(BIT(m_leds_mux, 7));
}

ADDRESS_MAP_START(mephisto_montec_state::smondial_mem)
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x4000, 0x4007 ) AM_READ(megaiv_input_r)
	AM_RANGE( 0x6400, 0x6407 ) AM_WRITE(smondial_led_data_w)
	AM_RANGE( 0x6800, 0x6807 ) AM_WRITE(smondial_board_mux_w)
	AM_RANGE( 0x6c00, 0x6c03 ) AM_WRITE(montec_mux_w)
	AM_RANGE( 0x6c04, 0x6c04 ) AM_WRITE(montec_lcd_data_w)
	AM_RANGE( 0x6c05, 0x6c05 ) AM_WRITE(montec_ldc_cs1_w)
	AM_RANGE( 0x6c06, 0x6c06 ) AM_WRITE(montec_lcd_clk_w)
	AM_RANGE( 0x6c07, 0x6c07 ) AM_WRITE(montec_ldc_cs0_w)
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

WRITE8_MEMBER(mephisto_montec_state::mondial2_input_mux_w)
{
	uint8_t leds_data = m_board->mux_r(space, offset);
	for (int i=0; i<8; i++)
	{
		if (!BIT(leds_data, i))
		{
			if (data & 0x10)    output().set_led_value(100 + i, 1);
			if (data & 0x20)    output().set_led_value(  8 + i, 1);
			if (data & 0x40)    output().set_led_value(  0 + i, 1);
		}
	}

	m_input_mux = data ^ 0xff;
	m_beeper->set_state(BIT(data, 7));
}


ADDRESS_MAP_START(mephisto_montec_state::mondial2_mem)
	AM_RANGE( 0x0000, 0x07ff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2000, 0x2000 ) AM_WRITE(mondial2_input_mux_w)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_w)
	AM_RANGE( 0x3000, 0x3007 ) AM_READ(megaiv_input_r)
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

TIMER_DEVICE_CALLBACK_MEMBER(mephisto_montec_state::refresh_leds)
{
	for (int i=0; i<8; i++)
	{
		output().set_led_value(0 + i, 0);
		output().set_led_value(8 + i, 0);
		output().set_led_value(100 + i, 0);
	}
}

static INPUT_PORTS_START( montec )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("1 Pawn")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("2 Knight") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("3 Bishop") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("4 Rook")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("5 Queen")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("6 King")   PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("7 Black")  PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("8 White")  PORT_CODE(KEYCODE_8)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("9 Book")   PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("0 Pos")    PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Mem")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Info")     PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Clear")    PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Level")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Enter")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Reset")    PORT_CODE(KEYCODE_DEL)
INPUT_PORTS_END

static INPUT_PORTS_START( megaiv )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("3 Bishop") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("7 Black")  PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Mem")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Enter")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("4 Rook")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("8 White")  PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Info")     PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Reset")    PORT_CODE(KEYCODE_DEL)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("1 Pawn")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("5 Queen")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("9 Book")   PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Clear")    PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("2 Knight") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("6 King")   PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("0 Pos")    PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Level")    PORT_CODE(KEYCODE_L)
INPUT_PORTS_END

static INPUT_PORTS_START( mondial2 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Play")     PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Pos")      PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Mem")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Info")     PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Clear")    PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Level")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Enter")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Reset")    PORT_CODE(KEYCODE_DEL)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("1 Pawn")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("2 Knight") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("3 Bishop") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("4 Rook")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("5 Queen")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("6 King")   PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("7 Black")  PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("8 White")  PORT_CODE(KEYCODE_8)
INPUT_PORTS_END

static INPUT_PORTS_START( smondial2 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("3 Bishop") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("7 Black")  PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Pos")      PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Enter")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("4 Rook")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("8 White")  PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Mem")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Reset")    PORT_CODE(KEYCODE_DEL)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("1 Pawn")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("5 Queen")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("9 Help")   PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Level")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("2 Knight") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("6 King")   PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("0 Info")   PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)        PORT_NAME("Clear")    PORT_CODE(KEYCODE_BACKSPACE)
INPUT_PORTS_END

MACHINE_CONFIG_START(mephisto_montec_state::montec)
	MCFG_CPU_ADD("maincpu", M65C02, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP( montec_mem )
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_montec_state, nmi_line_assert, XTAL(4'000'000) / (1 << 13))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_MEPHISTO_SENSORS_BOARD_ADD("board")

	MCFG_DEFAULT_LAYOUT(layout_mephisto_montec)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mephisto_montec_state::monteciv)
	montec(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL(8'000'000) )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mephisto_montec_state::megaiv)
	montec(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL(4'915'200) )
	MCFG_CPU_PROGRAM_MAP(megaiv_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_montec_state, nmi_line_pulse, XTAL(4'915'200) / (1 << 13))

	MCFG_DEVICE_REMOVE("board")
	MCFG_MEPHISTO_BUTTONS_BOARD_ADD("board")
	MCFG_MEPHISTO_BOARD_DISABLE_LEDS(true)
	MCFG_DEFAULT_LAYOUT(layout_mephisto_megaiv)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mephisto_montec_state::mondial2)
	megaiv(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL(2'000'000) )
	MCFG_CPU_PROGRAM_MAP(mondial2_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_montec_state, nmi_line_pulse, XTAL(2'000'000) / (1 << 12))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("refresh_leds", mephisto_montec_state, refresh_leds, attotime::from_hz(10))
	MCFG_DEFAULT_LAYOUT(layout_mephisto_mondial2)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mephisto_montec_state::smondial)
	megaiv(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL(4'000'000) )
	MCFG_CPU_PROGRAM_MAP(smondial_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_montec_state, nmi_line_pulse, XTAL(4'000'000) / (1 << 13))
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mephisto_montec_state::smondial2)
	smondial(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(smondial2_mem)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "smondial2_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "smondial2")

	MCFG_DEFAULT_LAYOUT(layout_mephisto_smondial2)
MACHINE_CONFIG_END


ROM_START(megaiv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("megaiv.bin", 0x8000, 0x8000, CRC(dee355d2) SHA1(6bc79c0fb169020f017412f5f9696b9ecafbf99f))
ROM_END

ROM_START(monteciv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mciv.bin", 0x8000, 0x8000, CRC(c4887694) SHA1(7f482d2a40fcb3125266e7a5407da315b4f9b49c))
ROM_END

ROM_START(montec)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mc.bin", 0x08000, 0x08000, CRC(05524da9) SHA1(bee2ffe09a27095f733584e0fb1203b95c23e17e))
ROM_END

ROM_START(smondial)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mephisto super mondial I.bin", 0x8000, 0x8000, CRC(c1d7d0a5) SHA1(d7f0da6938458c06925f0936e63915319144d7e0))
ROM_END

ROM_START(smondial2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("supermondial_II.bin", 0x8000, 0x8000, CRC(cd73df4a) SHA1(bad786074be613d7f48bf98b6fdf8178a4a85f5b))
ROM_END

ROM_START(mondial2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mondial II 01 08 87 Morsch.bin", 0x8000, 0x8000, CRC(e5945ce6) SHA1(e75bbf9d54087271d9d46fb1de7634eb957f8db0))
ROM_END


/*    YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT     CLASS                   INIT COMPANY             FULLNAME                      FLAGS */
CONS( 1986, smondial, 0,       0,      smondial,  megaiv,   mephisto_montec_state,  0,   "Hegener & Glaser", "Mephisto Super Mondial",     MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, montec,   0,       0,      montec,    montec,   mephisto_montec_state,  0,   "Hegener & Glaser", "Mephisto Monte Carlo",       MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, mondial2, 0,       0,      mondial2,  mondial2, mephisto_montec_state,  0,   "Hegener & Glaser", "Mephisto Mondial II",        MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, smondial2,0,       0,      smondial2, smondial2,mephisto_montec_state,  0,   "Hegener & Glaser", "Mephisto Super Mondial II",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, megaiv,   0,       0,      megaiv,    megaiv,   mephisto_montec_state,  0,   "Hegener & Glaser", "Mephisto Mega IV",           MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, monteciv, montec,  0,      monteciv,  montec,   mephisto_montec_state,  0,   "Hegener & Glaser", "Mephisto Monte Carlo IV LE", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
