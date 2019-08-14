// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:Berger
/***************************************************************************

Mephisto Mondial 68000XL

Hardware:
- TS68000CP12 @ 12MHz
- 64KB ROM
- 16KB RAM
- PCF2112T LCD driver

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "mephisto_mondial68k.lh"


class mondial68k_state : public driver_device
{
public:
	mondial68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_board(*this, "board")
		, m_display(*this, "display")
		, m_inputs(*this, "IN.%u", 0)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void mondial68k(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mondial68k_mem(address_map &map);

	DECLARE_WRITE8_MEMBER(lcd_dlen_w);
	DECLARE_WRITE8_MEMBER(lcd_clb_w);
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_WRITE8_MEMBER(input_mux_w);
	DECLARE_WRITE8_MEMBER(board_mux_w);
	DECLARE_READ8_MEMBER(inputs_r);
	TIMER_DEVICE_CALLBACK_MEMBER(refresh_leds);

	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;
	output_finder<8> m_digits;
	output_finder<16> m_leds;

	uint8_t m_input_mux;
	uint8_t m_board_mux;
	uint8_t m_lcd_shift;
	uint8_t m_dac_data;
};


void mondial68k_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_input_mux));
	save_item(NAME(m_board_mux));
	save_item(NAME(m_lcd_shift));
	save_item(NAME(m_dac_data));
}

void mondial68k_state::machine_reset()
{
	m_input_mux = 0;
	m_board_mux = 0;
	m_lcd_shift = 0;
	m_dac_data = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(mondial68k_state::refresh_leds)
{
	for (int i=0; i<16; i++)
		m_leds[0 + i] = 0;
}

WRITE8_MEMBER( mondial68k_state::lcd_clb_w )
{
	if (BIT(data, 0))
		m_lcd_shift++;
}

WRITE8_MEMBER( mondial68k_state::lcd_dlen_w )
{
	m_lcd_shift = 0;
}

WRITE8_MEMBER( mondial68k_state::lcd_data_w )
{
	if (m_lcd_shift > 0 && m_lcd_shift < 0x21)
		m_display->write_element((m_lcd_shift - 1) / 8, (m_lcd_shift - 1) % 8, BIT(data, 0));
}

WRITE8_MEMBER( mondial68k_state::speaker_w )
{
	m_dac_data ^= 1;
	m_dac->write(m_dac_data);
}

WRITE8_MEMBER(mondial68k_state::board_mux_w)
{
	m_board_mux = data;
}

WRITE8_MEMBER(mondial68k_state::input_mux_w)
{
	m_input_mux = data;
	for (int i=0; i<8; i++)
	{
		if (!BIT(m_board_mux, i))
		{
			if (BIT(m_input_mux, 7))   m_leds[0 + i] = 1;
			if (BIT(m_input_mux, 6))   m_leds[8 + i] = 1;
		}
	}
}

READ8_MEMBER(mondial68k_state::inputs_r)
{
	if      (!(m_input_mux & 0x01))    return m_inputs[0]->read();
	else if (!(m_input_mux & 0x02))    return m_inputs[1]->read();
	else if (!(m_input_mux & 0x04))    return m_inputs[2]->read();
	else if (!(m_input_mux & 0x08))    return m_inputs[3]->read();

	uint8_t data = 0x00;
	for (int i=0; i<8; i++)
		if (!BIT(m_board_mux, i))
			data |= m_board->read_rank(i);

	return data;
}

void mondial68k_state::mondial68k_mem(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x800000, 0x800001).r(FUNC(mondial68k_state::inputs_r));
	map(0x820000, 0x820001).w(FUNC(mondial68k_state::lcd_clb_w));
	map(0x820002, 0x820003).w(FUNC(mondial68k_state::lcd_data_w));
	map(0x820004, 0x820005).w(FUNC(mondial68k_state::lcd_dlen_w));
	map(0x82000c, 0x82000d).nopw();
	map(0x82000e, 0x82000f).w(FUNC(mondial68k_state::speaker_w));
	map(0x840000, 0x840001).w(FUNC(mondial68k_state::input_mux_w));
	map(0x860000, 0x860001).w(FUNC(mondial68k_state::board_mux_w));
	map(0xc00000, 0xc03fff).ram();
}


static INPUT_PORTS_START( mondial68k )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
INPUT_PORTS_END


void mondial68k_state::mondial68k(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mondial68k_state::mondial68k_mem);
	m_maincpu->set_periodic_int(FUNC(mondial68k_state::irq5_line_hold), attotime::from_hz(128));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	m_display->output_digit().set([this] (offs_t offset, u8 data, u8 mem_mask) { m_digits[offset] = bitswap<8>(data, 7,4,5,0,1,2,3,6); });
	config.set_default_layout(layout_mephisto_mondial68k);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	TIMER(config, "refresh_leds").configure_periodic(FUNC(mondial68k_state::refresh_leds), attotime::from_hz(10));
}


/***************************************************************************
  ROM definitions
***************************************************************************/

ROM_START( mondl68k )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("68000xl-06-11-87-u.bin", 0x0000, 0x8000, CRC(aebe482a) SHA1(900c91ec836cd65e4cd38e50555976ab8064be41))
	ROM_LOAD16_BYTE("68000xl-06-11-87-l.bin", 0x0001, 0x8000, CRC(564e32c5) SHA1(8c9df46bc5ced114e72fb663f1055d775b8e2e0b))
ROM_END


/***************************************************************************
  Game drivers
***************************************************************************/

/*    YEAR, NAME,      PARENT    COMPAT  MACHINE      INPUT       CLASS             INIT        COMPANY             FULLNAME                     FLAGS */
CONS( 1988, mondl68k,  0,        0,      mondial68k,  mondial68k, mondial68k_state, empty_init, "Hegener + Glaser", "Mephisto Mondial 68000XL",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
