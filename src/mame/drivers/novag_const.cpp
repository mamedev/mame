// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Novag Constellation (model 831)

Hardware notes (1st version):
- MOS MPS6502A @ 2MHz
- 2KB RAM (daughterboard with 4*2114), 2*8KB ROM
- TTL, buzzer, 24 LEDs, 8*8 chessboard buttons

3.6MHz version:
- G65SC02P-3 @ 3.6MHz (7.2MHz XTAL)
- 2KB RAM (TC5516AP), 16KB ROM (custom label, assumed TMM23128)
- PCB supports "Memory Save", but components aren't installed

TODO:
- add Quattro version, another small update, this time 4MHz

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "novag_const.lh" // clickable


namespace {

class const_state : public driver_device
{
public:
	const_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void nconst(machine_config &config);
	void nconst36(machine_config &config);

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
	void const_map(address_map &map);
	void const36_map(address_map &map);

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

void const_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_led_select = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));

	// game relies on RAM filled with FF at power-on
	for (int i = 0; i < 0x800; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i, 0xff);
}



/******************************************************************************
    I/O
******************************************************************************/

// TTL

void const_state::update_display()
{
	m_display->matrix(m_led_select, m_inp_mux);
}

WRITE8_MEMBER(const_state::mux_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = data;
	update_display();
}

WRITE8_MEMBER(const_state::control_w)
{
	// d0-d3: ?
	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	update_display();

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

READ8_MEMBER(const_state::input1_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7, true);

	return ~data;
}

READ8_MEMBER(const_state::input2_r)
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

void const_state::const36_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram();
	map(0x6000, 0x6000).rw(FUNC(const_state::input2_r), FUNC(const_state::mux_w));
	map(0x8000, 0x8000).rw(FUNC(const_state::input1_r), FUNC(const_state::control_w));
	map(0xc000, 0xffff).rom();
}

void const_state::const_map(address_map &map)
{
	const36_map(map);

	map(0xa000, 0xbfff).rom();
	map(0xc000, 0xdfff).unmapr(); // checks for bookrom? but doesn't have any
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( nconst )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Multi Move / Player/Player / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move / Random / Queen")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Knight")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Rook")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Pawn")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Show Moves")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void const_state::nconst(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::const_map);

	const attotime irq_period = attotime::from_hz(2_MHz_XTAL / 0x2000); // through 4020 IC, ~244Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(const_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(17100)); // assume same as const36
	TIMER(config, "irq_off").configure_periodic(FUNC(const_state::irq_off<M6502_IRQ_LINE>), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_const);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2_MHz_XTAL / 0x800); // ~976Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void const_state::nconst36(machine_config &config)
{
	nconst(config);

	/* basic machine hardware */
	M65SC02(config.replace(), m_maincpu, 7.2_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::const36_map);

	const attotime irq_period = attotime::from_hz(7.2_MHz_XTAL/2 / 0x2000); // through 4020 IC, ~439Hz
	TIMER(config.replace(), m_irq_on).configure_periodic(FUNC(const_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(17100)); // active for ~17.1us
	TIMER(config.replace(), "irq_off").configure_periodic(FUNC(const_state::irq_off<M6502_IRQ_LINE>), irq_period);

	m_board->set_delay(attotime::from_msec(200));

	BEEP(config.replace(), m_beeper, 7.2_MHz_XTAL/2 / 0x800); // ~1758Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( const )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("8315_white",  0xa000, 0x2000, CRC(76e6c97b) SHA1(55645e08f9f1258366c29a4ea2033bb86d860227) ) // TMM2364P
	ROM_LOAD("8314_orange", 0xe000, 0x2000, CRC(89395a86) SHA1(4807f196fec70abdaabff5bfc479a64d5cf2b0ad) ) // "
ROM_END

ROM_START( const36 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag-831a_6133-8316.u2",  0xc000, 0x4000, CRC(7da760f3) SHA1(6172e0fa03377e911141a86747849bf25f20613f) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT CMP MACHINE   INPUT   STATE        INIT        COMPANY, FULLNAME, FLAGS
CONS( 1983, const,   0,      0, nconst,   nconst, const_state, empty_init, "Novag", "Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1984, const36, const,  0, nconst36, nconst, const_state, empty_init, "Novag", "Constellation 3.6MHz", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
