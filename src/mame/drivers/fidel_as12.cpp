// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard
/******************************************************************************

Fidelity Elegance Chess Challenger (AS12)

Hardware notes:
- R65C02P4 CPU @ 4MHz
- 3*8KB ROM(TMM2764), 2*2KB RAM(HM6116)
- PCB label 510-1084B01

This is on the SC12B board, with enough modifications to support more leds and
magnetic chess board sensors. See fidel_sc12.cpp for a more technical description.

******************************************************************************/

#include "emu.h"
#include "machine/fidel_clockdiv.h"

#include "cpu/m6502/r65c02.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist.h"
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
		m_irq_on(*this, "irq_on"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.0")
	{ }

	// machine drivers
	void as12(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<timer_device> m_irq_on;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_device<generic_slot_device> m_cart;
	required_ioport m_inputs;

	// address maps
	void main_map(address_map &map);

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	// I/O handlers
	void update_display();
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(input_r);

	u16 m_inp_mux;
	u8 m_led_data;
};

void as12_state::machine_start()
{
	fidel_clockdiv_state::machine_start();

	// zerofill
	m_inp_mux = 0;
	m_led_data = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}



/******************************************************************************
    I/O
******************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(as12_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


// TTL/generic

void as12_state::update_display()
{
	// 8*8(+1) chessboard leds
	m_display->matrix(m_inp_mux, m_led_data);
}

WRITE8_MEMBER(as12_state::control_w)
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

WRITE8_MEMBER(as12_state::led_w)
{
	// a0-a2,d0: led data via NE591N
	m_led_data = (m_led_data & ~(1 << offset)) | ((data & 1) << offset);
	update_display();
}

READ8_MEMBER(as12_state::input_r)
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
	map(0xc000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( as12 )
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
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void as12_state::as12(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 4_MHz_XTAL); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &as12_state::div_trampoline);
	ADDRESS_MAP_BANK(config, m_mainmap).set_map(&as12_state::main_map).set_options(ENDIANNESS_LITTLE, 8, 16);

	const attotime irq_period = attotime::from_hz(585); // from 556 timer (22nF, 110K, 1K)
	TIMER(config, m_irq_on).configure_periodic(FUNC(as12_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(15250)); // active for 15.25us
	TIMER(config, "irq_off").configure_periodic(FUNC(as12_state::irq_off<M6502_IRQ_LINE>), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	config.set_default_layout(layout_fidel_as12);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "fidel_scc", "bin,dat");
	m_cart->set_device_load(FUNC(as12_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( feleg ) // model AS12(or 6085)
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("feleg.1", 0x8000, 0x2000, CRC(e9df31e8) SHA1(31c52bb8f75580c82093eb950959c1bc294189a8) ) // TMM2764, no label
	ROM_LOAD("feleg.2", 0xc000, 0x2000, CRC(bed9c84b) SHA1(c12f39765b054d2ad81f747e698715ad4246806d) ) // "
	ROM_LOAD("feleg.3", 0xe000, 0x2000, CRC(b1fb49aa) SHA1(d8c9687dd564f0fa603e6d684effb1d113ac64b4) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME   PARENT  CMP MACHINE  INPUT  STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1985, feleg, 0,       0, as12,    as12,  as12_state, empty_init, "Fidelity Electronics", "Elegance Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_TIMING )
