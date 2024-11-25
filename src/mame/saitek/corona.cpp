// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Saitek Corona. Please refer to stratos.cpp for driver notes.

To be brief, Saitek Corona has two "HELIOS" chips, I/O addressing is completely
different compared to Stratos/Turbo King.

*******************************************************************************/

#include "emu.h"
#include "stratos.h"

#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "saitek_corona.lh"


namespace {

// note: sub-class of stratos_base_state (see stratos.h, stratos.cpp)

class corona_state : public stratos_base_state
{
public:
	corona_state(const machine_config &mconfig, device_type type, const char *tag) :
		stratos_base_state(mconfig, type, tag),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void corona(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	memory_view m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<8+1> m_inputs;

	u8 m_control1 = 0;
	u8 m_control2 = 0;
	u8 m_select1 = 0;
	u8 m_select2 = 0;
	u8 m_led_data1 = 0;
	u8 m_led_data2 = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_leds();
	void leds1_w(u8 data);
	void leds2_w(u8 data);
	void select1_w(u8 data);
	void select2_w(u8 data);
	void control1_w(u8 data);
	void control2_w(u8 data);
	u8 control1_r();
	u8 control2_r();
	u8 chessboard_r();
	void lcd_reset_w(u8 data);
};

void corona_state::machine_start()
{
	stratos_base_state::machine_start();

	// register for savestates
	save_item(NAME(m_control1));
	save_item(NAME(m_control2));
	save_item(NAME(m_select1));
	save_item(NAME(m_select2));
	save_item(NAME(m_led_data1));
	save_item(NAME(m_led_data2));
}

void corona_state::machine_reset()
{
	stratos_base_state::machine_reset();

	m_control2 = 0;
	m_rombank.select(0);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// HELIOS

void corona_state::update_leds()
{
	// button leds
	m_display->matrix_partial(0, 2, 1 << (m_control1 >> 5 & 1), ~m_led_data1 & 0xff);
	m_display->write_row(2, ~m_select1 >> 4 & 0xf);

	// chessboard leds
	m_display->matrix_partial(3, 8, 1 << (m_select1 & 0xf), m_led_data2);
}

void corona_state::leds1_w(u8 data)
{
	// d0-d7: button led data
	m_led_data1 = data;
	update_leds();
}

void corona_state::leds2_w(u8 data)
{
	// d0-d7: chessboard led data
	m_led_data2 = data;
	update_leds();
}

void corona_state::select1_w(u8 data)
{
	// d0-d3: chessboard led select
	// d4-d7: black/white leds
	m_select1 = data;
	update_leds();
}

void corona_state::select2_w(u8 data)
{
	// d0-d3: input mux
	// d4-d7: lcd data
	m_select2 = data;
}

void corona_state::control1_w(u8 data)
{
	// d5: button led select
	m_control1 = data;
	update_leds();

	// d6: speaker out
	m_dac->write(data >> 6 & 1);
}

void corona_state::control2_w(u8 data)
{
	// d0,d1: rombank
	m_rombank.select(data & 3);

	// d2 rising edge: write to lcd
	if (~m_control2 & data & 4)
		lcd_data_w(m_select2 >> 4);

	// d6 rising edge: power-off request
	if (~m_control2 & data & 0x40)
		power_off();

	m_control2 = data;
}

u8 corona_state::control1_r()
{
	u8 data = 0;

	// d5: lcd status flag?
	if (m_lcd_ready)
		data |= 0x20;

	// d6: FREQ. SEL related?

	// d7: battery low
	data |= m_inputs[8]->read() << 7;

	return data;
}

u8 corona_state::control2_r()
{
	u8 data = 0;
	u8 sel = m_select2 & 0xf;

	// d5-d7: read button panel
	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;

	return data;
}

u8 corona_state::chessboard_r()
{
	// d0-d7: chessboard sensors
	return ~m_board->read_file(m_select2 & 0xf);
}

void corona_state::lcd_reset_w(u8 data)
{
	// reset lcd?
	m_lcd_ready = true;
	m_lcd_count = 0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void corona_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w(FUNC(corona_state::select1_w));
	map(0x2400, 0x2400).rw(FUNC(corona_state::chessboard_r), FUNC(corona_state::leds1_w));
	map(0x2600, 0x2600).rw(FUNC(corona_state::control1_r), FUNC(corona_state::control1_w));
	map(0x6000, 0x6000).w(FUNC(corona_state::select2_w));
	map(0x6200, 0x6200).w(FUNC(corona_state::lcd_reset_w));
	map(0x6400, 0x6400).w(FUNC(corona_state::leds2_w));
	map(0x6600, 0x6600).rw(FUNC(corona_state::control2_r), FUNC(corona_state::control2_w));

	map(0x8000, 0xffff).view(m_rombank);
	m_rombank[0](0x8000, 0xffff).rom().region("maincpu", 0x0000);
	m_rombank[1](0x8000, 0xffff).rom().region("maincpu", 0x8000);
	m_rombank[2](0x8000, 0xffff).r("extrom", FUNC(generic_slot_device::read_rom));
	m_rombank[3](0x8000, 0xffff).lr8(NAME([]() { return 0xff; }));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( corona )
	PORT_INCLUDE( saitek_stratos )

	PORT_MODIFY("IN.5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("LCD Scroll")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void corona_state::corona(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, 5_MHz_XTAL); // see change_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::main_map);
	m_maincpu->set_periodic_int(FUNC(corona_state::irq0_line_hold), attotime::from_hz(183));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3+8, 8);
	config.set_default_layout(layout_saitek_corona);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// extension rom
	GENERIC_SOCKET(config, "extrom", generic_plain_slot, "saitek_egr");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr").set_filter("egr2");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( corona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_708g_u2.u2", 0x0000, 0x8000, CRC(52568bb4) SHA1(83fe91787e17bbefc2b3ec651ddb11c88990060d) )
	ROM_LOAD("bw2_708a_u3.u3", 0x8000, 0x8000, CRC(32848f73) SHA1(a447543e3eb4757f9afed26fde77b66985eb96a7) )
ROM_END

ROM_START( coronaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_a14c_u2.u2", 0x0000, 0x8000, CRC(be82e199) SHA1(cfcc573774b6907ed137dca01fa7f3fce493a89f) )
	ROM_LOAD("bw2_a14_u3.u3", 0x8000, 0x8000, CRC(abe87285) SHA1(b15f7ddeac78d252cf413ba4085523e44c6d15df) )
ROM_END

ROM_START( coronab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_a14_u2.u2", 0x0000, 0x8000, CRC(92a44b92) SHA1(06e25421c34cf9c30c585b261d04b823c4a39b36) )
	ROM_LOAD("bw2_a14_u3.u3", 0x8000, 0x8000, CRC(abe87285) SHA1(b15f7ddeac78d252cf413ba4085523e44c6d15df) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1990, corona,   0,       0,      corona,  corona, corona_state, empty_init, "Saitek / Heuristic Software", "Kasparov Corona (ver. D+)", MACHINE_SUPPORTS_SAVE ) // aka Corona II
SYST( 1988, coronaa,  corona,  0,      corona,  corona, corona_state, empty_init, "Saitek / Heuristic Software", "Kasparov Corona (ver. C, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, coronab,  corona,  0,      corona,  corona, corona_state, empty_init, "Saitek / Heuristic Software", "Kasparov Corona (ver. C, set 2)", MACHINE_SUPPORTS_SAVE )
