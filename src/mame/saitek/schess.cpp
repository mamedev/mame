// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:bataais
/*******************************************************************************

SciSys Sensor Chess (model 221)
(not to be confused with Saitek Kasparov Sensor Chess)

Two versions were released. The 1st version is fake-wood brown plastic, and 2nd
version is grey plastic. The brown version only has 256 bytes RAM. The grey version
has 1KB but does not use the extra available RAM. The ROM(software) is assumed to
be the same for each version.

Buttons are unresponsive, this also applies to the chessboard buttons. The program
is not interrupt-driven and only checks the buttons around twice per second. The user
needs to hold a button for up to half a second to get a response. In MAME, a delay is
applied to the sensorboard interface to make sure that all clicks work there. But that
also means it's not possible to play blitz style (which wouldn't be possible on the
real device anyway, since it reacts so slowly).

Hardware notes:
- Synertek 6502A @ ~2MHz
- 4KB ROM(AMI 2332)
- 1KB RAM(2*2114), or 256 bytes RAM(GTE 3539)
- buzzer, 64+12 leds, button chessboard
- expansion slot at top-right (dummy empty cartridge by default)

Expansion modules: (* denotes not dumped)
- Strong Play Module
- Classical Style Super Strong
- *Hyper Modern Super Strong

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m6502.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "saitek_schess.lh"


namespace {

class schess_state : public driver_device
{
public:
	schess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void schess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_data = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void leds1_w(u8 data);
	void leds2_w(offs_t offset, u8 data);
	void control_w(u8 data);
	u8 input_r();
};

void schess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void schess_state::update_display()
{
	u8 led_data = bitswap<8>(m_led_data,4,5,6,7,3,2,1,0);
	m_display->matrix_partial(0, 8, 1 << m_inp_mux, led_data);
}

void schess_state::leds1_w(u8 data)
{
	// chessboard leds (muxed)
	m_led_data = ~data;
	update_display();
}

void schess_state::leds2_w(offs_t offset, u8 data)
{
	// button panel leds (direct)
	m_display->write_row(8 + (offset ? 1 : 0), ~data);
}

void schess_state::control_w(u8 data)
{
	// d0-d3: input mux, led select
	m_inp_mux = data & 0xf;
	update_display();

	// d5: speaker out
	m_dac->write(BIT(data, 5));

	// other: ?
}

u8 schess_state::input_r()
{
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read other buttons
	else if (m_inp_mux < 10)
		data = m_inputs[m_inp_mux & 1]->read();
	else
		data = m_inputs[2]->read();

	return bitswap<8>(data,4,5,6,7,3,2,1,0);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void schess_state::main_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0x4000).w(FUNC(schess_state::leds1_w));
	map(0x4800, 0x4800).w(FUNC(schess_state::control_w));
	map(0x5000, 0x5000).select(0x0800).w(FUNC(schess_state::leds2_w));
	map(0x6000, 0x6000).r(FUNC(schess_state::input_r));
	map(0x8000, 0x8fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xf000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( schess )
	PORT_START("IN.0")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Player V Computer")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Change Sides")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Clear Board")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Legal")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")

	PORT_START("IN.2")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Player V Player")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Interrupt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Bishop")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void schess_state::schess(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 2'000'000); // approximation, no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &schess_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_ticks(1'115'000, 2'000'000)); // see driver notes

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "schess_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_schess");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8+2, 8);
	config.set_default_layout(layout_saitek_schess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( schess )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("y02-rom", 0xf000, 0x1000, CRC(25181e03) SHA1(4d3a606b196b9019c00795b2cd65ce10fbef932c) ) // AMI 2332
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, schess, 0,      0,      schess,  schess, schess_state, empty_init, "SciSys / Heuristic Software", "Sensor Chess", MACHINE_SUPPORTS_SAVE )
