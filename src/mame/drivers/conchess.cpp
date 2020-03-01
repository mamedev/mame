// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Conchess, a series of modular chess computers by Consumenta Computer.
Hardware development by Loproc (Germany), manufactured at Wallharn Electronics
(Ireland). The core people involved were Ulf Rathsman for the chess engine,
and Johan Enroth. After Consumenta went under in 1983, the Conchess brand was
continued by Systemhuset, Enroth's company.

Hardware notes:

Chess boards released were Escorter, Ambassador, and Monarch, each should be the
same hardware, they just differ in size and material.
- TTL, 2 module slots
- 16+64 leds, 16 buttons, reed sensors for magnet chesspieces

All chess modules appear to be on similar PCBs, with room a 6502/65C02,
and 8 ROM/RAM chips.

A0 (untitled standard pack-in module):
- SY6502A @ 2MHz (4MHz XTAL)
- 3*8KB ROM, 4KB RAM(2*TMM2016P)
- TTL, beeper

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "conchess.lh" // clickable


namespace {

class conchess_state : public driver_device
{
public:
	conchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void concstd(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_irq_on;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<beep_device> m_beeper;
	required_ioport_array<2> m_inputs;

	// address maps
	void main_map(address_map &map);

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// I/O handlers
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(sound_w);

	u8 m_inp_mux = 0;
};

void conchess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

READ8_MEMBER(conchess_state::input_r)
{
	u8 data = 0;

	// read side panel buttons
	if (m_inp_mux == 0 || m_inp_mux == 9)
		data = m_inputs[m_inp_mux & 1]->read();

	// read chessboard sensors
	else
		data = m_board->read_file((m_inp_mux - 1) ^ 7);

	return ~data;
}

WRITE8_MEMBER(conchess_state::leds_w)
{
	// a0-a3: CD4028B to led select/input mux
	m_inp_mux = offset;
	if (m_inp_mux & 8)
		m_inp_mux &= 9;

	// d0-d7: led data
	m_display->matrix(1 << m_inp_mux, data);
}

WRITE8_MEMBER(conchess_state::sound_w)
{
	// d7: enable beeper
	m_beeper->set_state(BIT(data, 7));
}



/******************************************************************************
    Address Maps
******************************************************************************/

void conchess_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1050, 0x1050).r(FUNC(conchess_state::input_r));
	map(0x1060, 0x106f).w(FUNC(conchess_state::leds_w));
	map(0x1800, 0x1800).w(FUNC(conchess_state::sound_w));
	map(0xa000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( conchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("O. (Clear)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Dice Symbol (Alternate)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("?-Sign (Analyze)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Section Sign (Referee)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("4-Way Arrow (Piece)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("2-Way Arrow (Level)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME(". (Continue)")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("White")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Black")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void conchess_state::concstd(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(4_MHz_XTAL / 0x2000); // through 4020 IC, ~488Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(conchess_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(31200)); // active for ~31.2us
	TIMER(config, "irq_off").configure_periodic(FUNC(conchess_state::irq_off<M6502_IRQ_LINE>), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	config.set_default_layout(layout_conchess);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 4_MHz_XTAL / 0x400);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( concstd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c87011.b3", 0xa000, 0x2000, CRC(915e414c) SHA1(80c94712d1c79fa469576c37b80ab66f77c77cc4) )
	ROM_LOAD("c87010.b2", 0xc000, 0x2000, CRC(088c8737) SHA1(9f841b3c47de9ef1da8ce98c0a33a919cba873c6) )
	ROM_LOAD("c87009.b1", 0xe000, 0x2000, CRC(e1c648e2) SHA1(725a6ac1c69f788a7bba0573e5609b55b12899ac) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1982, concstd, 0,      0,      concstd, conchess, conchess_state, empty_init, "Consumenta Computer / Loproc", "Conchess (standard)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
