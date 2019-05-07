// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

* novag_scon.cpp, subdriver of machine/novagbase.cpp, machine/chessbase.cpp

TODO:
- verify IRQ and beeper frequency

*******************************************************************************

Novag Super Constellation Chess Computer (model 844) overview:
- UMC UM6502C @ 4 MHz (8MHz XTAL), 600Hz? IRQ(source unknown?)
- 2*2KB RAM TC5516APL-2 battery-backed, 2*32KB ROM custom label
- TTL, buzzer, 24 LEDs, 8*8 chessboard buttons
- external ports for clock and printer, not emulated here

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "cpu/m6502/m6502.h"
#include "machine/nvram.h"
#include "speaker.h"

// internal artwork
#include "novag_supercon.lh" // clickable


namespace {

class scon_state : public novagbase_state
{
public:
	scon_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag)
	{ }

	// machine drivers
	void scon(machine_config &config);

private:
	// address maps
	void main_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(input2_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL

WRITE8_MEMBER(scon_state::mux_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = m_led_data = data;
	display_matrix(8, 3, m_led_data, m_led_select);
}

WRITE8_MEMBER(scon_state::control_w)
{
	// d0-d3: ?
	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	display_matrix(8, 3, m_led_data, m_led_select);

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

READ8_MEMBER(scon_state::input1_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}

READ8_MEMBER(scon_state::input2_r)
{
	// d0-d5: ?
	// d6,d7: multiplexed inputs (side panel)
	return (read_inputs(8) >> 2 & 0xc0) ^ 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void scon_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1c00, 0x1c00).nopw(); // printer/clock?
	map(0x1d00, 0x1d00).nopw(); // printer/clock?
	map(0x1e00, 0x1e00).rw(FUNC(scon_state::input2_r), FUNC(scon_state::mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(scon_state::input1_r), FUNC(scon_state::control_w));
	map(0x2000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( scon )
	PORT_INCLUDE( generic_cb_buttons )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Multi Move / Player/Player / King")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move/Random / Training Level / Queen")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Knight")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Rook")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Form Size")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void scon_state::scon(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 8_MHz_XTAL/2); // UM6502C
	m_maincpu->set_addrmap(AS_PROGRAM, &scon_state::main_map);
	m_maincpu->set_periodic_int(FUNC(scon_state::irq0_line_hold), attotime::from_hz(600)); // guessed

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "display_decay").configure_periodic(FUNC(scon_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_supercon);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1024); // guessed
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( supercon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8441", 0x0000, 0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0) ) // label obscured by Q.C. sticker
	ROM_LOAD("novag_8442", 0x8000, 0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE  INPUT  STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1984, supercon, 0,      0, scon,    scon,  scon_state, empty_init, "Novag", "Super Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
