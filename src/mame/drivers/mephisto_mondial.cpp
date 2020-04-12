// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:yoyo_chessboard
/**************************************************************************************************

    Mephisto Mondial
    Mephisto Mondial II

**************************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "mephisto_mondial2.lh"


class mephisto_mondial_state : public driver_device
{
public:
	mephisto_mondial_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_dac(*this, "dac")
		, m_beeper(*this, "beeper")
		, m_keys(*this, "KEY.%u", 0)
		, m_low_leds(*this, "led%u", 0U)
		, m_high_leds(*this, "led%u", 100U)
	{ }

	void mondial(machine_config &config);
	void mondial2(machine_config &config);

private:
	uint8_t mondial2_input_r(offs_t offset);
	void mondial_input_mux_w(uint8_t data);
	uint8_t mondial_input_r(offs_t offset);
	void mondial2_input_mux_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(refresh_leds);

	void mondial_mem(address_map &map);
	void mondial2_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	optional_device<dac_bit_interface> m_dac;
	optional_device<beep_device> m_beeper;
	required_ioport_array<2> m_keys;
	output_finder<16> m_low_leds, m_high_leds;

	uint8_t m_input_mux;
};


void mephisto_mondial_state::machine_start()
{
	m_low_leds.resolve();
	m_high_leds.resolve();

	save_item(NAME(m_input_mux));
}

void mephisto_mondial_state::machine_reset()
{
	m_input_mux = 0x00;
}

uint8_t mephisto_mondial_state::mondial2_input_r(offs_t offset)
{
	if      (m_input_mux & 0x01)    return BIT(m_keys[1]->read(), 0 + offset) << 7;
	else if (m_input_mux & 0x02)    return BIT(m_keys[1]->read(), 4 + offset) << 7;
	else if (m_input_mux & 0x04)    return BIT(m_keys[0]->read(), 0 + offset) << 7;
	else if (m_input_mux & 0x08)    return BIT(m_keys[0]->read(), 4 + offset) << 7;

	return BIT(m_board->input_r(), offset) << 7;
}

void mephisto_mondial_state::mondial2_input_mux_w(uint8_t data)
{
	uint8_t leds_data = m_board->mux_r();
	for (int i=0; i<8; i++)
	{
		if (!BIT(leds_data, i))
		{
			if (data & 0x10) m_high_leds[i] = 1;
			if (data & 0x20) m_low_leds[8 + i] = 1;
			if (data & 0x40) m_low_leds[0 + i] = 1;
		}
	}

	m_input_mux = data ^ 0xff;
	m_dac->write(BIT(data, 7));
	m_maincpu->set_input_line(M65C02_NMI_LINE, CLEAR_LINE);
}


void mephisto_mondial_state::mondial2_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2000, 0x2000).w(FUNC(mephisto_mondial_state::mondial2_input_mux_w));
	map(0x2800, 0x2800).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x3000, 0x3007).r(FUNC(mephisto_mondial_state::mondial2_input_r));
	map(0x8000, 0xffff).rom();
}

uint8_t mephisto_mondial_state::mondial_input_r(offs_t offset)
{
	uint8_t data;
	if (m_input_mux & 0x08)
		data = m_keys[BIT(~m_input_mux, 0)]->read();
	else
		data = m_board->input_r();

	return BIT(data, offset) << 7;
}

void mephisto_mondial_state::mondial_input_mux_w(uint8_t data)
{
	uint8_t leds_data = ~(1 << (data & 0x07));
	m_board->mux_w(leds_data);

	for (int i=0; i<8; i++)
	{
		if (!BIT(leds_data, i))
		{
			if (!(data & 0x10)) m_high_leds[i] = 1;
			if (!(data & 0x20)) m_low_leds[8 + i] = 1;
			if (!(data & 0x40)) m_low_leds[0 + i] = 1;
		}
	}

	m_input_mux = data;
	m_beeper->set_state(BIT(data, 7));
	m_maincpu->set_input_line(M65C02_IRQ_LINE, CLEAR_LINE);
}


void mephisto_mondial_state::mondial_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x1000, 0x1000).w(FUNC(mephisto_mondial_state::mondial_input_mux_w));
	map(0x2000, 0x2000).r(m_board, FUNC(mephisto_board_device::input_r));
	map(0x1800, 0x1807).r(FUNC(mephisto_mondial_state::mondial_input_r));
	map(0xc000, 0xffff).rom();
}

TIMER_DEVICE_CALLBACK_MEMBER(mephisto_mondial_state::refresh_leds)
{
	for (int i=0; i<8; i++)
	{
		m_low_leds[0 + i] = 0;
		m_low_leds[8 + i] = 0;
		m_high_leds[i] = 0;
	}
}


static INPUT_PORTS_START( mondial )
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


void mephisto_mondial_state::mondial2(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_mondial_state::mondial2_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_mondial_state::nmi_line_assert), attotime::from_hz(XTAL(2'000'000) / (1 << 12)));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	MEPHISTO_BUTTONS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(250));
	m_board->set_disable_leds(true);

	TIMER(config, "refresh_leds").configure_periodic(FUNC(mephisto_mondial_state::refresh_leds), attotime::from_hz(2));
	config.set_default_layout(layout_mephisto_mondial2);
}

void mephisto_mondial_state::mondial(machine_config &config)
{
	mondial2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_mondial_state::mondial_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_mondial_state::irq0_line_assert), attotime::from_hz(XTAL(2'000'000) / (1 << 12)));

	config.device_remove("dac");
	config.device_remove("vref");
	BEEP(config, m_beeper, 2048).add_route(ALL_OUTPUTS, "speaker", 0.25); // measured C7(2093Hz)
}


ROM_START( mondial )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mondial_1.bin", 0xc000, 0x4000, CRC(5cde2e26) SHA1(337be35d5120ca12143ca17f8aa0642b313b3851) )
ROM_END

ROM_START( mondial2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mondial_ii_01.08.87", 0x8000, 0x8000, CRC(e5945ce6) SHA1(e75bbf9d54087271d9d46fb1de7634eb957f8db0) )
ROM_END


/*    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT      CLASS                  INIT        COMPANY             FULLNAME                     FLAGS */
CONS( 1985, mondial,    0,        0,      mondial,   mondial,   mephisto_mondial_state, empty_init, "Hegener + Glaser", "Mephisto Mondial",          MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, mondial2,   0,        0,      mondial2,  mondial,   mephisto_mondial_state, empty_init, "Hegener + Glaser", "Mephisto Mondial II",       MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
