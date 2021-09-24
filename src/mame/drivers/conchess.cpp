// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Conchess, a series of modular chess computers by Consumenta Computer.
Hardware development by Loproc (Germany), manufactured at Wallharn Electronics
(Ireland). The core people involved were Ulf Rathsman for the chess engine,
and Johan Enroth. After Consumenta went under in 1983, the Conchess brand was
continued by Systemhuset, Enroth's company.

TODO:
- dump/add princhess module
- dump/add plymate original 2MHz module
- dump/add concplyv library module (L/L16 don't work, manual says it has its own add-on)
- concplyv unmapped reads/writes
- verify irq/beeper for concplyv, though it is probably correct
- official rom labels for concply5, concply8

-------------------------------------------------------------------------------

Hardware notes:

Chess boards released were Escorter, Ambassador, and Monarch, each should be the
same hardware, they just differ in size and material.
- TTL, 2 module slots
- 16+64 leds, 16 buttons, reed sensors for magnet chesspieces

All chess modules appear to be on similar PCBs, with room a 6502/65C02,
and 8 ROM/RAM chips.

For the newer programs, higher clocked versions were also available, the CPU speed
was labeled on a sticker on the module (eg. A2/5.5MHz). A German redistributor
named them "S"(speed) or "T"(top speed).

A0 (untitled standard pack-in module):
- SY6502A @ 2MHz (4MHz XTAL)
- 3*8KB ROM, 4KB RAM(2*TMM2016P)
- TTL, beeper

note: XTAL goes to 4020, 4020 /2 goes to CPU clock, and other dividers to
IRQ and beeper. On A0, IRQ is active for ~31.2us.

A1(P) + A0(M) (Princhess, aka Glasgow)
- dual-module, each module has its own 6502 - need verification, more likely
  2nd module has no CPU (according to the manual, A0 is modified and won't
  work stand-alone)

A2(C) (Plymate, aka Amsterdam)
- R65C02P4 @ 5.5MHz (11MHz XTAL) (5.5MHz version)
- 32KB ROM, rest similar to A0

A3 (Plymate Victoria)
- W65C02S8P-14(14? probably replaced chip from a repair) @ 6.144MHz (12.288MHz XTAL)
- 32KB ROM, rest similar to A0

Library modules:
- L: small PCB, PCB label: CCL L-2, 8KB EPROM no label
- L16: 2*8KB EPROM (have no photo of PCB)

******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/sensorboard.h"
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
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void concstd(machine_config &config);
	void concply5(machine_config &config);
	void concply8(machine_config &config);
	void concplyv(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<beep_device> m_beeper;
	required_ioport_array<2> m_inputs;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	void clear_irq();
	u8 input_r();
	void leds_w(offs_t offset, u8 data);
	void sound_w(u8 data);

	u8 m_inp_mux = 0;
};

void conchess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

void conchess_state::clear_irq()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

u8 conchess_state::input_r()
{
	clear_irq();

	u8 data = 0;

	// read side panel buttons
	if (m_inp_mux == 0 || m_inp_mux == 9)
		data = m_inputs[m_inp_mux & 1]->read();

	// read chessboard sensors
	else
		data = m_board->read_file((m_inp_mux - 1) ^ 7);

	return ~data;
}

void conchess_state::leds_w(offs_t offset, u8 data)
{
	clear_irq();

	// a0-a3: CD4028B to led select/input mux
	m_inp_mux = offset;
	if (m_inp_mux & 8)
		m_inp_mux &= 9;

	// d0-d7: led data
	m_display->matrix(1 << m_inp_mux, data);
}

void conchess_state::sound_w(u8 data)
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
	map(0x1000, 0x1000).mirror(0x07ff).r(FUNC(conchess_state::input_r));
	map(0x1000, 0x100f).mirror(0x07f0).w(FUNC(conchess_state::leds_w));
	map(0x1800, 0x1800).mirror(0x07ff).w(FUNC(conchess_state::sound_w));
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0xffff).rom();
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
	M6502(config, m_maincpu, 4_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(4_MHz_XTAL/2 / 0x1000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "conchess_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("conchess_standard");

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	config.set_default_layout(layout_conchess);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 4_MHz_XTAL/2 / 0x200);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void conchess_state::concply5(machine_config &config)
{
	concstd(config);

	/* basic machine hardware */
	R65C02(config.replace(), m_maincpu, 11_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(11_MHz_XTAL/2 / 0x2000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	SOFTWARE_LIST(config.replace(), "cart_list").set_original("conchess_plymate");

	/* sound hardware */
	BEEP(config.replace(), m_beeper, 11_MHz_XTAL/2 / 0x400);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void conchess_state::concply8(machine_config &config)
{
	concply5(config);

	/* basic machine hardware */
	R65C02(config.replace(), m_maincpu, 16_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(16_MHz_XTAL/2 / 0x2000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	/* sound hardware */
	BEEP(config.replace(), m_beeper, 16_MHz_XTAL/2 / 0x800);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void conchess_state::concplyv(machine_config &config)
{
	concstd(config);

	/* basic machine hardware */
	M65C02(config.replace(), m_maincpu, 12.288_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL/2 / 0x2000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	SOFTWARE_LIST(config.replace(), "cart_list").set_original("conchess_victoria");

	/* sound hardware */
	BEEP(config.replace(), m_beeper, 12.288_MHz_XTAL/2 / 0x800);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( concstd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("c87011.b3", 0xa000, 0x2000, CRC(915e414c) SHA1(80c94712d1c79fa469576c37b80ab66f77c77cc4) )
	ROM_LOAD("c87010.b2", 0xc000, 0x2000, CRC(088c8737) SHA1(9f841b3c47de9ef1da8ce98c0a33a919cba873c6) )
	ROM_LOAD("c87009.b1", 0xe000, 0x2000, CRC(e1c648e2) SHA1(725a6ac1c69f788a7bba0573e5609b55b12899ac) )
ROM_END

ROM_START( concply5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("plymate_5.5mhz.bin", 0x8000, 0x8000, CRC(9a9d1ec1) SHA1(75dbd1f96502775ed304f6b085d958f1b07d08f9) )
ROM_END

ROM_START( concply8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("plymate_8mhz.bin", 0x8000, 0x8000, CRC(85005b73) SHA1(edbc18d07552cab5951d8a6b738b2eacd73331c1) )
ROM_END

ROM_START( concplyv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cc8-f.a2", 0x8000, 0x8000, CRC(5b0a1d09) SHA1(07cbc970a8dfbca386396ce5d5cc8ce77ad4ee1b) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1982, concstd,  0,        0,      concstd,  conchess, conchess_state, empty_init, "Consumenta Computer / Loproc", "Conchess (standard)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1985, concply5, 0,        0,      concply5, conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate 5.5MHz", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1985, concply8, concply5, 0,      concply8, conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate 8.0MHz", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, concplyv, 0,        0,      concplyv, conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate Victoria", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
