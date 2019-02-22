// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard
/******************************************************************************
*
* fidel_dames.cpp, subdriver of machine/fidelbase.cpp, machine/chessbase.cpp

*******************************************************************************

Fidelity Dame Sensory Challenger (DSC) overview:
- Z80A CPU @ 3.9MHz
- 8KB ROM(MOS 2364), 1KB RAM(2*TMM314APL)
- 4-digit 7seg panel, sensory board with 50 buttons
- PCB label 510-1030A01

It's a checkers game for once instead of chess

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/z80/z80.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_dsc.lh" // clickable


namespace {

class dsc_state : public fidelbase_state
{
public:
	dsc_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag)
	{ }

	// machine drivers
	void dsc(machine_config &config);

private:
	// address maps
	void main_map(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(select_w);
	DECLARE_READ8_MEMBER(input_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL

void dsc_state::prepare_display()
{
	// 4 7seg leds
	set_display_segmask(0xf, 0x7f);
	display_matrix(8, 4, m_7seg_data, m_led_select);
}

WRITE8_MEMBER(dsc_state::control_w)
{
	// d0-d7: input mux, 7seg data
	m_inp_mux = ~data;
	m_7seg_data = data;
	prepare_display();
}

WRITE8_MEMBER(dsc_state::select_w)
{
	// d4: speaker out
	m_dac->write(BIT(~data, 4));

	// d0-d3: digit select
	m_led_select = data & 0xf;
	prepare_display();
}

READ8_MEMBER(dsc_state::input_r)
{
	// d0-d7: multiplexed inputs (active low)
	return ~read_inputs(8);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void dsc_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x4000).mirror(0x1fff).w(FUNC(dsc_state::control_w));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(dsc_state::select_w));
	map(0x8000, 0x8000).mirror(0x1fff).r(FUNC(dsc_state::input_r));
	map(0xa000, 0xa3ff).mirror(0x1c00).ram();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( dsc )
	PORT_INCLUDE( generic_cb_buttons )

	PORT_MODIFY("IN.4")
	PORT_BIT(0x8f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("IN.6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Black King")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Black")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("White King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("White")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("RV")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("RE")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("PB")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("LV")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("CL")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void dsc_state::dsc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3.9_MHz_XTAL); // 3.9MHz resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &dsc_state::main_map);

	const attotime irq_period = attotime::from_hz(523); // from 555 timer (22nF, 120K, 2.7K)
	TIMER(config, m_irq_on).configure_periodic(FUNC(dsc_state::irq_on<INPUT_LINE_IRQ0>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_usec(41)); // active for 41us
	TIMER(config, "irq_off").configure_periodic(FUNC(dsc_state::irq_off<INPUT_LINE_IRQ0>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(dsc_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_dsc);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( damesc ) // model DSC
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "101-1027a01", 0x0000, 0x2000, CRC(d86c985c) SHA1(20f923a24420050fd16e1172f5e889f144d17ac9) ) // MOS 2364
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1981, damesc, 0,      0, dsc,     dsc,   dsc_state, empty_init, "Fidelity Electronics", "Dame Sensory Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
