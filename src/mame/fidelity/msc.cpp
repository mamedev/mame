// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Fidelity Mini Sensory Chess Challenger (model MSC, 1981 version)

Two versions exist, both of them are model MSC. The 1981 version has a Z8 MCU,
the 1982 version has an I8049. They can also be distinguished from the button
panel design, the 2nd version has rectangular buttons. See sc6.cpp for the
2nd version.

Hardware notes:
- Zilog Z8 MCU (probably Z8601, custom label: SR0016 1001011A01 or SR0022
  1001011B01), 8MHz XTAL
- buzzer, 18 leds, 8*8 chessboard buttons, module slot

released modules, * denotes not dumped yet:
- CAC: Challenger Advanced Chess
- CBO: Challenger Book Openings
- *CGG: Challenger Greatest Games

As noted in the hash file: The modules have 2 programs in them, one for Z8
and one for MCS48. A12 is forced high or low to select the bank.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/z8/z8.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_msc_v1.lh"


namespace {

class msc_state : public driver_device
{
public:
	msc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void msc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<z8_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u8 m_led_select = 0;
	u16 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void mux_w(u8 data);
	void control_w(u8 data);
	u8 input_r();
};

void msc_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_led_select));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void msc_state::update_display()
{
	m_display->matrix(m_led_select, m_inp_mux);
}

void msc_state::mux_w(u8 data)
{
	// P20-P27: input mux, led data
	m_inp_mux = (m_inp_mux & 0x100) | data;
	update_display();
}

void msc_state::control_w(u8 data)
{
	// P34: speaker out
	m_dac->write(BIT(~data, 4));

	// P35,P36: led select
	m_led_select = ~data >> 5 & 3;

	// P37: input mux highest bit, led data
	m_inp_mux = (m_inp_mux & 0xff) | (data << 1 & 0x100);
	update_display();
}

u8 msc_state::input_r()
{
	// P30-P33,P04-P07: multiplexed inputs
	u8 data = 0;

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	// read button panel
	if (m_inp_mux & 0x100)
		data |= m_inputs->read();

	return bitswap<8>(~data,0,1,2,3,4,5,6,7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void msc_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0xf000).r("cartslot", FUNC(generic_slot_device::read_rom));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( msc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Speaker / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("RE")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void msc_state::msc(machine_config &config)
{
	// basic machine hardware
	Z8601(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &msc_state::main_map);
	m_maincpu->p0_in_cb().set(FUNC(msc_state::input_r)).mask(0xf0);
	m_maincpu->p0_in_cb().append_constant(0x0f).mask(0x0f);
	m_maincpu->p2_out_cb().set(FUNC(msc_state::mux_w));
	m_maincpu->p3_in_cb().set(FUNC(msc_state::input_r)).mask(0x0f);
	m_maincpu->p3_in_cb().append_constant(0xf0).mask(0xf0);
	m_maincpu->p3_out_cb().set(FUNC(msc_state::control_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 9);
	config.set_default_layout(layout_fidel_msc_v1);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_msc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_msc");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( miniscco )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("sr0016_1001011a01", 0x0000, 0x0800, CRC(c8cd9bf1) SHA1(4ba165555b8419b03b2ef355da0ed9675315e18b) ) // internal ROM
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, miniscco, miniscc, 0,      msc,     msc,   msc_state, empty_init, "Fidelity Electronics", "Mini Sensory Chess Challenger (Z8 version)", MACHINE_SUPPORTS_SAVE )
