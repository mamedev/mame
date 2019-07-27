// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Novag Super Constellation (model 844)

Hardware notes:
- UMC UM6502C @ 4 MHz (8MHz XTAL)
- 2*2KB RAM TC5516APL-2 battery-backed, 2*32KB ROM custom label
- TTL, buzzer, 24 LEDs, 8*8 chessboard buttons
- external ports for clock and printer, not emulated here

I/O is nearly identical to Constellation (novag_const.cpp), the main difference
is additional outputs to external ports.

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/sensorboard.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "novag_supercon.lh" // clickable


namespace {

class sconst_state : public driver_device
{
public:
	sconst_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void sconst(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_irq_on;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	// address maps
	void main_map(address_map &map);

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// I/O handlers
	void update_display();
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(input2_r);

	u8 m_inp_mux;
	u8 m_led_select;
};

void sconst_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_led_select = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
}



/******************************************************************************
    I/O
******************************************************************************/

// TTL

void sconst_state::update_display()
{
	m_display->matrix(m_led_select, m_inp_mux);
}

WRITE8_MEMBER(sconst_state::mux_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = data;
	update_display();
}

WRITE8_MEMBER(sconst_state::control_w)
{
	// d0-d3: ?
	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	update_display();

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

READ8_MEMBER(sconst_state::input1_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7, true);

	return ~data;
}

READ8_MEMBER(sconst_state::input2_r)
{
	u8 data = 0;

	// d0-d5: ?
	// d6,d7: multiplexed inputs (side panel)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 6;

	return ~data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void sconst_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1c00, 0x1c00).nopw(); // printer/clock?
	map(0x1d00, 0x1d00).nopw(); // printer/clock?
	map(0x1e00, 0x1e00).rw(FUNC(sconst_state::input2_r), FUNC(sconst_state::mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(sconst_state::input1_r), FUNC(sconst_state::control_w));
	map(0x2000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sconst )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Multi Move / Player/Player / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move/Random / Training Level / Queen")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Knight")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Rook")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Form Size")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void sconst_state::sconst(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 8_MHz_XTAL/2); // UM6502C
	m_maincpu->set_addrmap(AS_PROGRAM, &sconst_state::main_map);

	const attotime irq_period = attotime::from_hz(8_MHz_XTAL/4 / 0x1000); // through 4020 IC, ~488Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(sconst_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(1020)); // active for 10.2us
	TIMER(config, "irq_off").configure_periodic(FUNC(sconst_state::irq_off<M6502_IRQ_LINE>), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_supercon);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 8_MHz_XTAL/4 / 0x800); // ~976Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( supercon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8443", 0x0000, 0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0) )
	ROM_LOAD("novag_8442", 0x8000, 0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE  INPUT   STATE         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1984, supercon, 0,      0, sconst,  sconst, sconst_state, empty_init, "Novag", "Super Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
