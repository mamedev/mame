// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:yoyo_chessboard
/**************************************************************************************************

    Mephisto Monte Carlo
    Mephisto Mega IV
    Mephisto Monte Carlo IV LE
    Mephisto Super Mondial
    Mephisto Super Mondial II

    smondialb notes:
    - holding CL+INFO+BOOK on boot load the test mode

    TODO:
    - split driver into several files?
    - why are megaiv/smondial2 beeps noisy?
    - add Monte Carlo IV (non-LE)
    - add MM 1000 module

**************************************************************************************************/


#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pcf2100.h"

#include "screen.h"
#include "speaker.h"
#include "softlist.h"

#include "mephisto_montec.lh"
#include "mephisto_megaiv.lh"
#include "mephisto_smondial2.lh"


class mephisto_montec_state : public driver_device
{
public:
	mephisto_montec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_lcd(*this, "lcd%u", 0)
		, m_dac(*this, "dac")
		, m_keys(*this, "KEY.%u", 0)
		, m_digits(*this, "digit%u", 0U)
		, m_low_leds(*this, "led%u", 0U)
		, m_high_leds(*this, "led%u", 100U)
	{ }

	void smondial(machine_config &config);
	void smondial2(machine_config &config);
	void montec(machine_config &config);
	void monteciv(machine_config &config);
	void megaiv(machine_config &config);

private:
	uint8_t montec_input_r();
	uint8_t montec_nmi_ack_r();
	void montec_nmi_ack_w(uint8_t data);
	void montec_mux_w(offs_t offset, uint8_t data);
	void montec_led_w(uint8_t data);
	void montec_beeper_w(uint8_t data);
	void montec_lcd_data_w(uint8_t data);
	void montec_ldc_cs0_w(uint8_t data);
	void montec_ldc_cs1_w(uint8_t data);
	void montec_lcd_clk_w(uint8_t data);
	template<int N> void montec_lcd_s_w(uint32_t data);

	uint8_t megaiv_input_r(offs_t offset);
	void megaiv_led_w(uint8_t data);

	void smondial_board_mux_w(offs_t offset, uint8_t data);
	void smondial_led_data_w(offs_t offset, uint8_t data);

	void megaiv_mem(address_map &map);
	void montec_mem(address_map &map);
	void smondial2_mem(address_map &map);
	void smondial_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device_array<pcf2112_device, 2> m_lcd;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<2> m_keys;
	output_finder<8> m_digits;
	output_finder<16> m_low_leds, m_high_leds;

	uint8_t m_input_mux;
	uint8_t m_leds_mux;
	uint8_t m_smondial_board_mux;
};


void mephisto_montec_state::machine_start()
{
	m_digits.resolve();
	m_low_leds.resolve();
	m_high_leds.resolve();

	save_item(NAME(m_input_mux));
	save_item(NAME(m_leds_mux));
	save_item(NAME(m_smondial_board_mux));
}

void mephisto_montec_state::machine_reset()
{
	m_input_mux = 0x00;
	m_leds_mux = 0x00;
	m_smondial_board_mux = 0xff;
}

void mephisto_montec_state::montec_led_w(uint8_t data)
{
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			if (BIT(data, i))
				m_high_leds[(i << 2) | j] = BIT(~data, 4 + j);
}

template<int N>
void mephisto_montec_state::montec_lcd_s_w(uint32_t data)
{
	for (int i=0; i<4; i++)
		m_digits[i + N*4] = bitswap<8>(data >> (8 * i), 7,4,5,0,1,2,3,6);
}

void mephisto_montec_state::montec_lcd_data_w(uint8_t data)
{
	m_lcd[0]->data_w(BIT(data, 7));
	m_lcd[1]->data_w(BIT(data, 7));
}

void mephisto_montec_state::montec_ldc_cs0_w(uint8_t data)
{
	m_lcd[0]->dlen_w(BIT(data, 7));
}

void mephisto_montec_state::montec_ldc_cs1_w(uint8_t data)
{
	m_lcd[1]->dlen_w(BIT(data, 7));
}

void mephisto_montec_state::montec_lcd_clk_w(uint8_t data)
{
	m_lcd[0]->clb_w(BIT(data, 7));
	m_lcd[1]->clb_w(BIT(data, 7));
}


void mephisto_montec_state::montec_mux_w(offs_t offset, uint8_t data)
{
	if (data)
		m_input_mux &= ~(1 << offset);
	else
		m_input_mux |= (1 << offset);
}

uint8_t mephisto_montec_state::montec_input_r()
{
	if      (m_input_mux & 0x01)    return m_keys[1]->read();
	else if (m_input_mux & 0x02)    return m_keys[0]->read();

	return m_board->input_r() ^ 0xff;
}

uint8_t mephisto_montec_state::montec_nmi_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return 0;
}

void mephisto_montec_state::montec_nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void mephisto_montec_state::montec_beeper_w(uint8_t data)
{
	m_dac->write(BIT(data, 7));
}

void mephisto_montec_state::megaiv_led_w(uint8_t data)
{
	if (m_leds_mux != m_board->mux_r())
	{
		m_leds_mux = m_board->mux_r();
		for (int i=0; i<8; i++)
		{
			if (!BIT(m_leds_mux, i))
			{
				m_high_leds[i] = BIT(data, 0) | BIT(data, 1);
				m_low_leds[0 + i] = BIT(data, 2) | BIT(data, 3);
				m_low_leds[8 + i] = BIT(data, 4) | BIT(data, 5);
			}
		}
	}

	m_dac->write(BIT(data, 7));
}

uint8_t mephisto_montec_state::megaiv_input_r(offs_t offset)
{
	if      (m_input_mux & 0x01)    return BIT(m_keys[1]->read(), 0 + offset) << 7;
	else if (m_input_mux & 0x02)    return BIT(m_keys[1]->read(), 4 + offset) << 7;
	else if (m_input_mux & 0x04)    return BIT(m_keys[0]->read(), 0 + offset) << 7;
	else if (m_input_mux & 0x08)    return BIT(m_keys[0]->read(), 4 + offset) << 7;

	return BIT(m_board->input_r(), offset) << 7;
}


void mephisto_montec_state::montec_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2400, 0x2400).r(FUNC(mephisto_montec_state::montec_input_r));
	map(0x2800, 0x2800).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c00).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0x3400, 0x3400).w(FUNC(mephisto_montec_state::montec_led_w));
	map(0x3000, 0x3001).w(FUNC(mephisto_montec_state::montec_mux_w));
	map(0x3002, 0x3002).w(FUNC(mephisto_montec_state::montec_beeper_w));
	map(0x3004, 0x3004).w(FUNC(mephisto_montec_state::montec_lcd_data_w));
	map(0x3005, 0x3005).w(FUNC(mephisto_montec_state::montec_ldc_cs1_w));
	map(0x3006, 0x3006).w(FUNC(mephisto_montec_state::montec_lcd_clk_w));
	map(0x3007, 0x3007).w(FUNC(mephisto_montec_state::montec_ldc_cs0_w));
	map(0x2000, 0x2000).rw(FUNC(mephisto_montec_state::montec_nmi_ack_r), FUNC(mephisto_montec_state::montec_nmi_ack_w));
	map(0x8000, 0xffff).rom();
}

void mephisto_montec_state::megaiv_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2400, 0x2400).w(FUNC(mephisto_montec_state::megaiv_led_w));
	map(0x2800, 0x2800).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c03).w(FUNC(mephisto_montec_state::montec_mux_w)).nopr();
	map(0x2c04, 0x2c04).w(FUNC(mephisto_montec_state::montec_lcd_data_w));
	map(0x2c05, 0x2c05).w(FUNC(mephisto_montec_state::montec_ldc_cs1_w));
	map(0x2c06, 0x2c06).w(FUNC(mephisto_montec_state::montec_lcd_clk_w));
	map(0x2c07, 0x2c07).w(FUNC(mephisto_montec_state::montec_ldc_cs0_w));
	map(0x3000, 0x3007).r(FUNC(mephisto_montec_state::megaiv_input_r));
	map(0x8000, 0xffff).rom();
}


void mephisto_montec_state::smondial2_mem(address_map &map)
{
	megaiv_mem(map);
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
}


void mephisto_montec_state::smondial_board_mux_w(offs_t offset, uint8_t data)
{
	if (data)
		m_smondial_board_mux &= ~(1 << offset);
	else
		m_smondial_board_mux |= (1 << offset);

	m_board->mux_w(m_smondial_board_mux);

	for (int i=0; i<8; i++)
	{
		if (m_leds_mux & 0x03) m_high_leds[i] = BIT(~m_smondial_board_mux, i);
		if (m_leds_mux & 0x0c) m_low_leds[8 + i] = BIT(~m_smondial_board_mux, i);
		if (m_leds_mux & 0x30) m_low_leds[0 + i] = BIT(~m_smondial_board_mux, i);
	}
}

void mephisto_montec_state::smondial_led_data_w(offs_t offset, uint8_t data)
{
	if (data & 0x80)
		m_leds_mux &= ~(1 << offset);
	else
		m_leds_mux |= (1 << offset);

	m_dac->write(BIT(m_leds_mux, 7));
}

void mephisto_montec_state::smondial_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x4007).r(FUNC(mephisto_montec_state::megaiv_input_r));
	map(0x6400, 0x6407).w(FUNC(mephisto_montec_state::smondial_led_data_w));
	map(0x6800, 0x6807).w(FUNC(mephisto_montec_state::smondial_board_mux_w));
	map(0x6c00, 0x6c03).w(FUNC(mephisto_montec_state::montec_mux_w));
	map(0x6c04, 0x6c04).w(FUNC(mephisto_montec_state::montec_lcd_data_w));
	map(0x6c05, 0x6c05).w(FUNC(mephisto_montec_state::montec_ldc_cs1_w));
	map(0x6c06, 0x6c06).w(FUNC(mephisto_montec_state::montec_lcd_clk_w));
	map(0x6c07, 0x6c07).w(FUNC(mephisto_montec_state::montec_ldc_cs0_w));
	map(0x8000, 0xffff).rom();
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

void mephisto_montec_state::montec(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(8'000'000) / 2); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_montec_state::montec_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_montec_state::nmi_line_assert), attotime::from_hz(XTAL(8'000'000) / (1 << 14)));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PCF2112(config, m_lcd[0], 50); // frequency guessed
	m_lcd[0]->write_segs().set(FUNC(mephisto_montec_state::montec_lcd_s_w<0>));
	PCF2112(config, m_lcd[1], 50); // "
	m_lcd[1]->write_segs().set(FUNC(mephisto_montec_state::montec_lcd_s_w<1>));

	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(300));

	config.set_default_layout(layout_mephisto_montec);
}

void mephisto_montec_state::monteciv(machine_config &config)
{
	montec(config);
	m_maincpu->set_clock(XTAL(8'000'000));
	m_board->set_delay(attotime::from_msec(150));
}

void mephisto_montec_state::megaiv(machine_config &config)
{
	montec(config);
	m_maincpu->set_clock(XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_montec_state::megaiv_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_montec_state::nmi_line_pulse), attotime::from_hz(XTAL(4'915'200) / (1 << 13)));

	MEPHISTO_BUTTONS_BOARD(config.replace(), m_board);
	m_board->set_delay(attotime::from_msec(250));
	m_board->set_disable_leds(true);
	config.set_default_layout(layout_mephisto_megaiv);
}

void mephisto_montec_state::smondial(machine_config &config)
{
	megaiv(config);
	m_maincpu->set_clock(XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_montec_state::smondial_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_montec_state::nmi_line_pulse), attotime::from_hz(XTAL(4'000'000) / (1 << 13)));
}

void mephisto_montec_state::smondial2(machine_config &config)
{
	smondial(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_montec_state::smondial2_mem);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "smondial2_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_smondial2");

	config.set_default_layout(layout_mephisto_smondial2);
}


ROM_START( megaiv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("megaiv.bin", 0x8000, 0x8000, CRC(dee355d2) SHA1(6bc79c0fb169020f017412f5f9696b9ecafbf99f) )
ROM_END

ROM_START( megaiva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mega_iv_17.03.88", 0x8000, 0x8000, CRC(85267e82) SHA1(654c9cf84bf2165fc94f8c4cf9c662786ef3283b) )
ROM_END

ROM_START( monteciv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mciv.bin", 0x8000, 0x8000, CRC(c4887694) SHA1(7f482d2a40fcb3125266e7a5407da315b4f9b49c) )
ROM_END

ROM_START( montec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mc3_12.11.87", 0x8000, 0x8000, CRC(8eb26043) SHA1(26454a37eea29283bbb2762a3a68e95e4be6aa1c) )
ROM_END

ROM_START( monteca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mc2_20.7.87", 0x8000, 0x8000, CRC(05524da9) SHA1(bee2ffe09a27095f733584e0fb1203b95c23e17e) )
ROM_END

ROM_START( smondial )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_a.bin", 0x8000, 0x8000, CRC(c1d7d0a5) SHA1(d7f0da6938458c06925f0936e63915319144d7e0) )
ROM_END

ROM_START( smondialab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_ab.bin", 0x8000, 0x8000, CRC(a8781685) SHA1(fd4c97e13bd398dc4c85e3e1778bf7e59fccd71e) )
ROM_END

ROM_START( smondialb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_b.bin", 0x8000, 0x8000, CRC(6fb89e97) SHA1(b001e657b4fdc097322b28a25c31814f3da7b124) )
ROM_END

ROM_START( smondial2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_ii.bin", 0x8000, 0x8000, CRC(cd73df4a) SHA1(bad786074be613d7f48bf98b6fdf8178a4a85f5b) )
ROM_END


/*    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT      CLASS                  INIT        COMPANY             FULLNAME                     FLAGS */
CONS( 1988, megaiv,     0,        0,      megaiv,    megaiv,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Mega IV (set 1)",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, megaiva,    megaiv,   0,      megaiv,    megaiv,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Mega IV (set 2)",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1986, smondial,   0,        0,      smondial,  megaiv,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial (ver. A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, smondialab, smondial, 0,      smondial,  megaiv,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial (ver. AB)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, smondialb,  smondial, 0,      megaiv,    megaiv,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial (ver. B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, smondial2,  0,        0,      smondial2, smondial2, mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial II", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1987, montec,     0,        0,      montec,    montec,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo (ver. MC3)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, monteca,    montec,   0,      montec,    montec,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo (ver. MC2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, monteciv,   montec,   0,      monteciv,  montec,    mephisto_montec_state, empty_init, "Hegener + Glaser", "Mephisto Monte Carlo IV - Limited Edition", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
