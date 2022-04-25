// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard, Berger
/******************************************************************************

Fidelity Elegance Chess Challenger (AS12/6085)

Hardware notes:
- R65C02P4 CPU @ 3/3.57/4MHz
- 3*8KB ROM(TMM2764), 2*2KB RAM(HM6116)
- PCB label 510-1084B01

This is on the SC12B board, with enough modifications to support more leds and
magnetic chess board sensors. See fidel_sc12.cpp for a more technical description.

******************************************************************************/

#include "emu.h"
#include "machine/fidel_clockdiv.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/r65c02.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_as12.lh" // clickable


namespace {

// note: sub-class of fidel_clockdiv_state (see mame/machine/fidel_clockdiv.*)

class as12_state : public fidel_clockdiv_state
{
public:
	as12_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidel_clockdiv_state(mconfig, type, tag),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	// machine configs
	void feleg(machine_config &config);
	void felega(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(switch_cpu_freq) { set_cpu_freq(); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void set_cpu_freq();

private:
	// devices/pointers
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_inputs;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	void update_display();
	void control_w(u8 data);
	void led_w(offs_t offset, u8 data);
	u8 input_r(offs_t offset);

	u16 m_inp_mux = 0;
	u8 m_led_data = 0;
};

void as12_state::machine_start()
{
	fidel_clockdiv_state::machine_start();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}

void as12_state::machine_reset()
{
	set_cpu_freq();
	fidel_clockdiv_state::machine_reset();
}

void as12_state::set_cpu_freq()
{
	// known official CPU speeds: 3MHz, 3.57MHz, 4MHz
	static const XTAL xtal[3] = { 3_MHz_XTAL, 3.579545_MHz_XTAL, 4_MHz_XTAL };
	m_maincpu->set_unscaled_clock(xtal[ioport("FAKE")->read() % 3]);
	div_refresh();
}



/******************************************************************************
    I/O
******************************************************************************/

void as12_state::update_display()
{
	// 8*8(+1) chessboard leds
	m_display->matrix(m_inp_mux, m_led_data);
}

void as12_state::control_w(u8 data)
{
	// d0-d3: 74245 P0-P3
	// 74245 Q0-Q8: input mux, led select
	u16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = bitswap<9>(sel,5,8,7,6,4,3,1,0,2);
	update_display();

	// 74245 Q9: speaker out
	m_dac->write(BIT(sel, 9));

	// d4,d5: printer?
	// d6,d7: N/C?
}

void as12_state::led_w(offs_t offset, u8 data)
{
	// a0-a2,d0: led data via NE591N
	m_led_data = (m_led_data & ~(1 << offset)) | ((data & 1) << offset);
	update_display();
}

u8 as12_state::input_r(offs_t offset)
{
	u8 data = 0;

	// a0-a2,d7: multiplexed inputs (active low)
	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i);

	// read sidepanel buttons
	if (m_inp_mux & 0x100)
		data |= m_inputs->read();

	data = bitswap<8>(data,4,3,2,1,0,5,6,7);
	return (data >> offset & 1) ? 0 : 0x80;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void as12_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();
	map(0x1800, 0x1807).w(FUNC(as12_state::led_w)).nopr();
	map(0x2000, 0x5fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(as12_state::control_w));
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xa007).mirror(0x1ff8).r(FUNC(as12_state::input_r));
	map(0xc000, 0xcfff).mirror(0x1000).rom();
	map(0xe000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( feleg )
	PORT_INCLUDE( fidel_clockdiv_4 )

	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")

	PORT_START("FAKE")
	PORT_CONFNAME( 0x03, 0x02, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, as12_state, switch_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz" )
	PORT_CONFSETTING(    0x01, "3.57MHz" )
	PORT_CONFSETTING(    0x02, "4MHz" )
INPUT_PORTS_END

static INPUT_PORTS_START( felega )
	PORT_INCLUDE( feleg )

	PORT_MODIFY("FAKE") // default to 3.57MHz
	PORT_CONFNAME( 0x03, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, as12_state, switch_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz" )
	PORT_CONFSETTING(    0x01, "3.57MHz" )
	PORT_CONFSETTING(    0x02, "4MHz" )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void as12_state::feleg(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 4_MHz_XTAL); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &as12_state::main_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 600)); // from 556 timer (22nF, 110K, 1K), ideal frequency is 600Hz
	irq_clock.set_pulse_width(attotime::from_usec(17)); // active for 17us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	config.set_default_layout(layout_fidel_as12);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void as12_state::felega(machine_config &config)
{
	feleg(config);

	/* basic machine hardware */
	m_maincpu->set_clock(3.579545_MHz_XTAL);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( feleg ) // model 6085
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("feleg.8000", 0x8000, 0x2000, CRC(e9df31e8) SHA1(31c52bb8f75580c82093eb950959c1bc294189a8) ) // TMM2764, no label
	ROM_LOAD("feleg.c000", 0xc000, 0x1000, CRC(bed9c84b) SHA1(c12f39765b054d2ad81f747e698715ad4246806d) ) // "
	ROM_CONTINUE(          0xc000, 0x1000 ) // 1st half empty
	ROM_LOAD("feleg.e000", 0xe000, 0x2000, CRC(b1fb49aa) SHA1(d8c9687dd564f0fa603e6d684effb1d113ac64b4) ) // "
ROM_END

ROM_START( felega ) // model AS12
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("blue.8000",   0x8000, 0x2000, CRC(2e07e657) SHA1(3238f21bdbf2277851e5a32e18c043e654123f00) ) // M5L2764K-2
	ROM_LOAD("yellow.c000", 0xc000, 0x1000, CRC(fcc48302) SHA1(f60d34229721e8659e9f81c267177daec7723d8f) ) // TMS2732AJL-45
	ROM_LOAD("black.e000",  0xe000, 0x2000, CRC(9142121b) SHA1(264380e7ad36b7b1867658e1af387624d2a72630) ) // TMS2764JL-25
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  CMP  MACHINE  INPUT    STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, feleg,  0,       0,  feleg,   feleg,   as12_state, empty_init, "Fidelity Electronics", "Elegance Chess Challenger (model 6085)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_TIMING )
CONS( 1985, felega, feleg,   0,  felega,  felega,  as12_state, empty_init, "Fidelity Electronics", "Elegance Chess Challenger (model AS12)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_TIMING )
