// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/***************************************************************************

Saitek Corona. Please refer to saitek_stratos.cpp for driver notes.

To be brief, Saitek Corona has two "HELIOS" chips, I/O addressing is completely
different compared to Stratos/Turbo King.

***************************************************************************/

#include "emu.h"
#include "includes/saitek_stratos.h"

#include "cpu/m6502/m65c02.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "softlist.h"
#include "speaker.h"

// internal artwork
#include "saitek_corona.lh" // clickable


namespace {

// note: sub-class of saitek_stratos_state (see mame/includes/saitek_stratos.h, mame/drivers/saitek_stratos.cpp)

class corona_state : public saitek_stratos_state
{
public:
	corona_state(const machine_config &mconfig, device_type type, const char *tag) :
		saitek_stratos_state(mconfig, type, tag),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void corona(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<address_map_bank_device> m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8+1> m_inputs;

	void main_map(address_map &map);
	void rombank_map(address_map &map);

	// I/O handlers
	void update_leds();
	DECLARE_WRITE8_MEMBER(leds1_w);
	DECLARE_WRITE8_MEMBER(leds2_w);
	DECLARE_WRITE8_MEMBER(select1_w);
	DECLARE_WRITE8_MEMBER(select2_w);
	DECLARE_WRITE8_MEMBER(control1_w);
	DECLARE_WRITE8_MEMBER(control2_w);
	DECLARE_READ8_MEMBER(control1_r);
	DECLARE_READ8_MEMBER(control2_r);
	DECLARE_READ8_MEMBER(chessboard_r);
	DECLARE_WRITE8_MEMBER(lcd_reset_w);

	u8 m_control1;
	u8 m_control2;
	u8 m_select1;
	u8 m_select2;
	u8 m_led_data1;
	u8 m_led_data2;
};

void corona_state::machine_start()
{
	saitek_stratos_state::machine_start();

	// zerofill
	m_control1 = 0;
	m_control2 = 0;
	m_select1 = 0;
	m_select2 = 0;
	m_led_data1 = 0;
	m_led_data2 = 0;

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
	saitek_stratos_state::machine_reset();

	m_rombank->set_bank(0);
}



/******************************************************************************
    I/O
******************************************************************************/

// HELIOS

void corona_state::update_leds()
{
	// button leds
	m_display->matrix_partial(0, 2, 1 << (m_control1 >> 5 & 1), (~m_led_data1 & 0xff), false);
	m_display->write_row(2, ~m_select1 >> 4 & 0xf);

	// chessboard leds
	m_display->matrix_partial(3, 8, 1 << (m_select1 & 0xf), m_led_data2);
}

WRITE8_MEMBER(corona_state::leds1_w)
{
	// d0-d7: button led data
	m_led_data1 = data;
	update_leds();
}

WRITE8_MEMBER(corona_state::leds2_w)
{
	// d0-d7: chessboard led data
	m_led_data2 = data;
	update_leds();
}

WRITE8_MEMBER(corona_state::select1_w)
{
	// d0-d3: chessboard led select
	// d4-d7: black/white leds
	m_select1 = data;
	update_leds();
}

WRITE8_MEMBER(corona_state::select2_w)
{
	// d0-d3: input mux
	// d4-d7: lcd data
	m_select2 = data;
}

WRITE8_MEMBER(corona_state::control1_w)
{
	// d5: button led select
	m_control1 = data;
	update_leds();

	// d6: speaker out
	m_dac->write(data >> 6 & 1);
}

WRITE8_MEMBER(corona_state::control2_w)
{
	// d0,d1: rombank
	m_rombank->set_bank(data & 3);

	// d2 rising edge: write to lcd
	if (~m_control2 & data & 4)
		lcd_data_w(m_select2 >> 4);

	// d6 rising edge: power-off request
	if (~m_control2 & data & 0x40)
		power_off();

	m_control2 = data;
}

READ8_MEMBER(corona_state::control1_r)
{
	u8 data = 0;

	// d5: lcd status flag?
	if (m_lcd_ready)
		data |= 0x20;

	// d6: FREQ. SEL related?

	// d7: battery low
	data |= m_inputs[8]->read();

	return data;
}

READ8_MEMBER(corona_state::control2_r)
{
	u8 data = 0;
	u8 sel = m_select2 & 0xf;

	// d5-d7: read button panel
	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;

	return data;
}

READ8_MEMBER(corona_state::chessboard_r)
{
	// d0-d7: chessboard sensors
	return ~m_board->read_file(m_select2 & 0xf);
}

WRITE8_MEMBER(corona_state::lcd_reset_w)
{
	// reset lcd?
	m_lcd_ready = true;
	m_lcd_count = 0;
}



/******************************************************************************
    Address Maps
******************************************************************************/

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
	map(0x8000, 0xffff).m(m_rombank, FUNC(address_map_bank_device::amap8));
}

void corona_state::rombank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x0ffff).rom().region("maincpu", 0);
	map(0x10000, 0x17fff).r(m_extrom, FUNC(generic_slot_device::read_rom));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( corona )
	PORT_INCLUDE( saitek_stratos )

	PORT_MODIFY("IN.5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CODE(KEYCODE_G) PORT_NAME("LCD Scroll")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void corona_state::corona(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 5_MHz_XTAL); // see set_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::main_map);
	m_maincpu->set_periodic_int(FUNC(corona_state::irq0_line_hold), attotime::from_hz(183));

	ADDRESS_MAP_BANK(config, "rombank").set_map(&corona_state::rombank_map).set_options(ENDIANNESS_LITTLE, 8, 17, 0x8000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(3+8, 8);
	config.set_default_layout(layout_saitek_corona);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* extension rom */
	GENERIC_CARTSLOT(config, m_extrom, generic_plain_slot, "saitek_egr", "bin");
	m_extrom->set_device_load(FUNC(corona_state::extrom_load), this);

	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

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

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  CMP MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1990, corona,   0,       0, corona,  corona, corona_state, empty_init, "Saitek", "Kasparov Corona (ver. D+)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK ) // aka Corona II
CONS( 1988, coronaa,  corona,  0, corona,  corona, corona_state, empty_init, "Saitek", "Kasparov Corona (ver. C)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )
