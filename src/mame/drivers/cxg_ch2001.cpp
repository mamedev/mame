// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************
*
* cxg_ch2001.cpp, subdriver of machine/chessbase.cpp

*******************************************************************************

CXG Chess 2001 overview:
- Zilog Z8400APS @ 4 MHz (8MHz XTAL)
- 2KB RAM HM6116, 16KB ROM D27128D
- TTL, piezo, 8*8+9 LEDs, magnetic sensors

******************************************************************************/

#include "emu.h"
#include "includes/chessbase.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "cxg_ch2001.lh" // clickable


namespace {

class ch2001_state : public chessbase_state
{
public:
	ch2001_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_dac(*this, "dac"),
		m_speaker_off(*this, "speaker_off")
	{ }

	// machine drivers
	void ch2001(machine_config &config);

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_irq_on;
	required_device<dac_bit_interface> m_dac;
	required_device<timer_device> m_speaker_off;

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	TIMER_DEVICE_CALLBACK_MEMBER(speaker_off) { m_dac->write(0); }

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_READ8_MEMBER(input_r);
};



/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL

WRITE8_MEMBER(ch2001_state::speaker_w)
{
	// 74ls109 clock pulse to speaker
	m_dac->write(1);
	m_speaker_off->adjust(attotime::from_usec(200)); // not accurate
}

WRITE8_MEMBER(ch2001_state::leds_w)
{
	// d0-d7: 74ls273 (WR to CLK)
	// 74ls273 Q1-Q4: 74ls145 A-D
	// 74ls145 0-9: input mux/led select
	m_inp_mux = 1 << (data & 0xf) & 0x3ff;

	// 74ls273 Q5-Q8: MC14028 A-D
	// MC14028 Q0-Q7: led data, Q8,Q9: N/C
	u8 led_data = 1 << (data >> 4 & 0xf) & 0xff;
	display_matrix(8, 10, led_data, m_inp_mux);
}

READ8_MEMBER(ch2001_state::input_r)
{
	// d0-d7: multiplexed inputs
	return ~read_inputs(10);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void ch2001_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).mirror(0x3800).ram();
	map(0x8000, 0x8000).mirror(0x3fff).rw(FUNC(ch2001_state::input_r), FUNC(ch2001_state::leds_w));
	map(0xc000, 0xc000).mirror(0x3fff).w(FUNC(ch2001_state::speaker_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( ch2001 )
	PORT_INCLUDE( generic_cb_magnets )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Black")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Knight")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("White")

	PORT_START("IN.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Set up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Take Back")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Forward")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Hint")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Move")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void ch2001_state::ch2001(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ch2001_state::main_map);

	const attotime irq_period = attotime::from_hz(484); // theoretical frequency from 555 timer (22nF, 100K+33K, 1K2), measurement was 568Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(ch2001_state::irq_on<INPUT_LINE_IRQ0>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(18300)); // active for 18.3us
	TIMER(config, "irq_off").configure_periodic(FUNC(ch2001_state::irq_off<INPUT_LINE_IRQ0>), irq_period);

	TIMER(config, m_speaker_off).configure_generic(FUNC(ch2001_state::speaker_off));

	TIMER(config, "display_decay").configure_periodic(FUNC(ch2001_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cxg_ch2001);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ch2001 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ch2001.bin", 0x0000, 0x4000, CRC(b3485c73) SHA1(f405c6f67fe70edf45dcc383a4049ee6bad387a9) ) // D27128D, no label
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME      FLAGS */
CONS( 1984, ch2001, 0,      0,      ch2001,  ch2001, ch2001_state, empty_init, "CXG",   "Chess 2001", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
