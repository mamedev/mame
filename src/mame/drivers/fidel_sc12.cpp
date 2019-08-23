// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, yoyo_chessboard
/******************************************************************************

Fidelity Sensory 12 Chess Challenger (SC12-B, 6086)
4 versions are known to exist: A,B,C, and X, with increasing CPU speed.
---------------------------------
RE information from netlist by Berger

8*(8+1) buttons, 8+8+2 red LEDs
DIN 41524C printer port
36-pin edge connector
CPU is a R65C02P4, running at 4MHz*

*By default, the CPU frequency is lowered on A13/A14 access, with a factory-set jumper:
/2 on model SC12(1.5MHz), /4 on model 6086(1MHz)

NE556 dual-timer IC:
- timer#1, one-shot at power-on, to CPU _RESET
- timer#2: R1=82K+50K pot at 26K, R2=1K, C=22nF, to CPU _IRQ: ~596Hz, active low=15.25us

Memory map:
-----------
0000-0FFF: 4K RAM (2016 * 2)
2000-5FFF: cartridge
6000-7FFF: control(W)
8000-9FFF: 8K ROM SSS SCM23C65E4
A000-BFFF: keypad(R)
C000-DFFF: 4K ROM TI TMS2732AJL-45
E000-FFFF: 8K ROM Toshiba TMM2764D-2

control: (74LS377)
--------
Q0-Q3: 7442 A0-A3
Q4: enable printer port pin 1 input
Q5: printer port pin 5 output
Q6,Q7: LEDs common anode

7442 0-8: input mux and LEDs cathode
7442 9: buzzer

The keypad is read through a 74HC251, where S0,1,2 is from CPU A0,1,2, Y is connected to CPU D7.
If control Q4 is set, printer data can be read from I0.

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
#include "fidel_sc12.lh" // clickable


namespace {

// note: sub-class of fidel_clockdiv_state (see mame/machine/fidel_clockdiv.*)

class sc12_state : public fidel_clockdiv_state
{
public:
	sc12_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidel_clockdiv_state(mconfig, type, tag),
		m_irq_on(*this, "irq_on"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.0")
	{ }

	// machine drivers
	void sc12(machine_config &config);
	void sc12b(machine_config &config);

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
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input_r);

	u8 m_inp_mux;
};

void sc12_state::machine_start()
{
	fidel_clockdiv_state::machine_start();

	// zerofill/register for savestates
	m_inp_mux = 0;
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(sc12_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


// TTL/generic

WRITE8_MEMBER(sc12_state::control_w)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	m_inp_mux = data & 0xf;
	u16 sel = 1 << m_inp_mux & 0x3ff;

	// 7442 9: speaker out
	m_dac->write(BIT(sel, 9));

	// d6,d7: led select (active low)
	m_display->matrix(~data >> 6 & 3, sel & 0x1ff);

	// d4,d5: printer
	//..
}

READ8_MEMBER(sc12_state::input_r)
{
	u8 data = 0;

	// a0-a2,d7: multiplexed inputs (active low)
	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read button panel
	else if (m_inp_mux == 8)
		data = m_inputs->read();

	return (data >> offset & 1) ? 0 : 0x80;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void sc12_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();
	map(0x2000, 0x5fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(sc12_state::control_w));
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xa007).mirror(0x1ff8).r(FUNC(sc12_state::input_r));
	map(0xc000, 0xcfff).mirror(0x1000).rom();
	map(0xe000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sc12_sidepanel )
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

static INPUT_PORTS_START( sc12 )
	PORT_INCLUDE( fidel_clockdiv_2 )
	PORT_INCLUDE( sc12_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( sc12b )
	PORT_INCLUDE( fidel_clockdiv_4 )
	PORT_INCLUDE( sc12_sidepanel )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void sc12_state::sc12(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 3_MHz_XTAL); // R65C02P3
	m_maincpu->set_addrmap(AS_PROGRAM, &sc12_state::div_trampoline);
	ADDRESS_MAP_BANK(config, m_mainmap).set_map(&sc12_state::main_map).set_options(ENDIANNESS_LITTLE, 8, 16);

	const attotime irq_period = attotime::from_hz(630); // from 556 timer (22nF, 102K, 1K)
	TIMER(config, m_irq_on).configure_periodic(FUNC(sc12_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(15250)); // active for 15.25us
	TIMER(config, "irq_off").configure_periodic(FUNC(sc12_state::irq_off<M6502_IRQ_LINE>), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2, 9);
	config.set_default_layout(layout_fidel_sc12);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "fidel_scc", "bin,dat");
	m_cart->set_device_load(FUNC(sc12_state::cart_load), this);

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void sc12_state::sc12b(machine_config &config)
{
	sc12(config);

	/* basic machine hardware */
	m_maincpu->set_clock(4_MHz_XTAL); // R65C02P4

	// change irq timer frequency
	const attotime irq_period = attotime::from_hz(596); // from 556 timer (22nF, 82K+26K, 1K)
	TIMER(config.replace(), m_irq_on).configure_periodic(FUNC(sc12_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(15250)); // active for 15.25us
	TIMER(config.replace(), "irq_off").configure_periodic(FUNC(sc12_state::irq_off<M6502_IRQ_LINE>), irq_period);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fscc12 ) // model SC12, PCB label 510-1084B01
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("101-1068a01.ic15", 0x8000, 0x2000, CRC(63c76cdd) SHA1(e0771c98d4483a6b1620791cb99a7e46b0db95c4) ) // SSS SCM23C65E4
	ROM_LOAD("orange.ic13",      0xc000, 0x1000, CRC(ed5289b2) SHA1(9b0c7f9ae4102d4a66eb8c91d4e84b9eec2ffb3d) ) // TI TMS2732AJL-45, no label, orange sticker
	ROM_LOAD("red.ic14",         0xe000, 0x2000, CRC(0c4968c4) SHA1(965a66870b0f8ce9549418cbda09d2ff262a1504) ) // TI TMS2764JL-25, no label, red sticker
ROM_END

ROM_START( fscc12b ) // model 6086, PCB label 510-1084B01
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("101-1068a01.ic15", 0x8000, 0x2000, CRC(63c76cdd) SHA1(e0771c98d4483a6b1620791cb99a7e46b0db95c4) ) // SSS SCM23C65E4
	ROM_LOAD("orange.ic13",      0xc000, 0x1000, CRC(45070a71) SHA1(8aeecff828f26fb7081902c757559903be272649) ) // TI TMS2732AJL-45, no label, orange sticker
	ROM_LOAD("red.ic14",         0xe000, 0x2000, CRC(183d3edc) SHA1(3296a4c3bce5209587d4a1694fce153558544e63) ) // Toshiba TMM2764D-2, no label, red sticker
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  CMP MACHINE  INPUT  STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1984, fscc12,  0,       0, sc12,    sc12,  sc12_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger 12", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_TIMING )
CONS( 1984, fscc12b, fscc12,  0, sc12b,   sc12b, sc12_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger 12-B", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_TIMING )
