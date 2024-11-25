// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Mr. Lars
/*******************************************************************************

Conchess, a series of modular chess computers by Consumenta Computer.

Hardware development by Loproc (Germany), manufactured at Wallharn Electronics
(Ireland). The core people involved were Ulf Rathsman for the chess engine,
and Johan Enroth. After Consumenta went under in 1983, the Conchess brand was
continued by Systemhuset.

TODO:
- concglap/concglapa rom labels
- dump/add concvicp library module (L/L16 don't work, manual says it has its own add-on)
- concvicp unmapped reads/writes
- verify irq/beeper for concvicp, though it is probably correct

================================================================================

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
- dual-module, 2nd module has no CPU (according to the manual, A0 is modified and
  won't work stand-alone)
- dual-module Amsterdam version also exists

A2(C) (Plymate, aka Amsterdam)
- R65C02P4 @ 5.5MHz (11MHz XTAL) (5.5MHz version)
- 32KB ROM, rest similar to A0

A3 (Plymate Victoria)
- W65C02S8P-14(14? probably replaced chip from a repair) @ 6.144MHz (12.288MHz XTAL)
- 32KB ROM, rest similar to A0

Library modules:
- L: small PCB, PCB label: CCL L-2, 8KB EPROM no label
- L16: 2*8KB EPROM (have no photo of PCB)

There are also modern bootleg boards from SteveUK in circulation. The library ROM
is integrated.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "conchess.lh"


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
	void conc(machine_config &config);
	void concgla(machine_config &config);
	void concams(machine_config &config);
	void concams5(machine_config &config);
	void concvicp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<beep_device> m_beeper;
	required_ioport_array<2> m_inputs;

	u8 m_inp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void clear_irq();
	u8 input_r();
	void leds_w(offs_t offset, u8 data);
	void sound_w(u8 data);
};

void conchess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void conchess_state::clear_irq()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

u8 conchess_state::input_r()
{
	if (!machine().side_effects_disabled())
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



/*******************************************************************************
    Address Maps
*******************************************************************************/

void conchess_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1000).mirror(0x07ff).r(FUNC(conchess_state::input_r));
	map(0x1000, 0x100f).mirror(0x07f0).w(FUNC(conchess_state::leds_w));
	map(0x1800, 0x1800).mirror(0x07ff).w(FUNC(conchess_state::sound_w));
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

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



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void conchess_state::conc(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 4_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(4_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "conchess_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("conchess").set_filter("l");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	config.set_default_layout(layout_conchess);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 4_MHz_XTAL / 0x400);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void conchess_state::concgla(machine_config &config)
{
	conc(config);

	// basic machine hardware
	R65C02(config.replace(), m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(4_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	subdevice<software_list_device>("cart_list")->set_filter("none");

	// sound hardware
	BEEP(config.replace(), m_beeper, 4_MHz_XTAL / 0x400);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void conchess_state::concams(machine_config &config)
{
	concgla(config);

	// basic machine hardware
	subdevice<software_list_device>("cart_list")->set_filter("l16");
}

void conchess_state::concams5(machine_config &config)
{
	concams(config);

	// basic machine hardware
	m_maincpu->set_clock(11_MHz_XTAL/2);

	const attotime irq_period = attotime::from_hz(11_MHz_XTAL / 0x4000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	// sound hardware
	BEEP(config.replace(), m_beeper, 11_MHz_XTAL / 0x800);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void conchess_state::concvicp(machine_config &config)
{
	concams5(config);

	// basic machine hardware
	M65C02(config.replace(), m_maincpu, 12.288_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &conchess_state::main_map);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL / 0x4000);
	m_maincpu->set_periodic_int(FUNC(conchess_state::irq0_line_assert), irq_period);

	subdevice<software_list_device>("cart_list")->set_filter("l1024");

	// sound hardware
	BEEP(config.replace(), m_beeper, 12.288_MHz_XTAL / 0x1000);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( conc ) // 2 bytes different (bookrom fix)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("ccp2.a", 0xa000, 0x1000, CRC(fddd155b) SHA1(ccb67543f4d680b47d2956b284d5c6b7a8d21520) ) // MBM2732A-30
	ROM_LOAD("ccp2.b", 0xb000, 0x1000, CRC(81cf6b20) SHA1(7f6e30e50c6f9b633767a04b7d7a9c291d301614) ) // "
	ROM_LOAD("ccp2.c", 0xc000, 0x1000, CRC(151eca1a) SHA1(40b97bce2ff0cae528efa3a9bab5767d5d15d369) ) // "
	ROM_LOAD("ccp2.d", 0xd000, 0x1000, CRC(ed5dafeb) SHA1(3f0cea45bf26d9a1d120963480fc883a00185224) ) // "
	ROM_LOAD("ccp2.e", 0xe000, 0x1000, CRC(f07bf0e2) SHA1(e4f2f415144ed5e43f4940c7c9618e89707d19e9) ) // "
	ROM_LOAD("ccp2.f", 0xf000, 0x1000, CRC(7a04b000) SHA1(72d702b91adc643201ac6cea7aeb9394a3afc8be) ) // "
ROM_END

ROM_START( conca ) // 6 EPROMs version also exists, same ROM contents
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("c87011_ccp2ab.b3", 0xa000, 0x2000, CRC(915e414c) SHA1(80c94712d1c79fa469576c37b80ab66f77c77cc4) ) // C2C076 serial also seen, same ROM contents
	ROM_LOAD("c87010_ccp2cd.b2", 0xc000, 0x2000, CRC(088c8737) SHA1(9f841b3c47de9ef1da8ce98c0a33a919cba873c6) )
	ROM_LOAD("c87009_ccp2ef.b1", 0xe000, 0x2000, CRC(e1c648e2) SHA1(725a6ac1c69f788a7bba0573e5609b55b12899ac) )
ROM_END

ROM_START( concgla ) // 8 EPROMs version also exists, same ROM contents
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("9128c-0133", 0x8000, 0x4000, CRC(a6ac88eb) SHA1(d1fcd990e5196c00210d380e0e04155a5ea19824) ) // GI 9128C
	ROM_LOAD("9128c-0134", 0xc000, 0x4000, CRC(b694a275) SHA1(e4e49379b4eb45402ca8bb82c20d0169db62ed7a) ) // "
ROM_END

ROM_START( concglap ) // 2 bytes different
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ccplus", 0x8000, 0x8000, CRC(c62e522b) SHA1(28ea9af35f9f4e96e52c19597d62e0af135c7672) )
ROM_END

ROM_START( concglapa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ccplus", 0x8000, 0x8000, CRC(0bf4d2d0) SHA1(0ce670d3be1ae4da5467be6b062cfe73dcfbf229) )
ROM_END

ROM_START( concams )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("8b_4.0_133", 0x8000, 0x4000, CRC(e021e3d8) SHA1(7a2c079664ed5cbaa7e5c55d4d2e8936b7be4219) ) // SEEQ DQ5143-250
	ROM_LOAD("cf_4.0_134", 0xc000, 0x4000, CRC(fc827c7f) SHA1(51acd881c4d5fe29f2a83d35a48ba323344122db) ) // "
ROM_END

ROM_START( concams5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("s5.a1", 0x8000, 0x8000, CRC(9a9d1ec1) SHA1(75dbd1f96502775ed304f6b085d958f1b07d08f9) ) // AT27C256
ROM_END

ROM_START( concvicp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cc8-f.a2", 0x8000, 0x8000, CRC(5b0a1d09) SHA1(07cbc970a8dfbca386396ce5d5cc8ce77ad4ee1b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, conc,      0,        0,      conc,     conchess, conchess_state, empty_init, "Consumenta Computer / Loproc", "Conchess (standard, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, conca,     conc,     0,      conc,     conchess, conchess_state, empty_init, "Consumenta Computer / Loproc", "Conchess (standard, set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1984, concgla,   0,        0,      concgla,  conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Princhess Glasgow", MACHINE_SUPPORTS_SAVE )
SYST( 1984, concglap,  0,        0,      concgla,  conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate Glasgow Plus (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1984, concglapa, concglap, 0,      concgla,  conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate Glasgow Plus (set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1985, concams,   0,        0,      concams,  conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate Amsterdam", MACHINE_SUPPORTS_SAVE )
SYST( 1985, concams5,  concams,  0,      concams5, conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate Amsterdam 5.5MHz", MACHINE_SUPPORTS_SAVE )

SYST( 1990, concvicp,  0,        0,      concvicp, conchess, conchess_state, empty_init, "Systemhuset / Loproc", "Conchess Plymate Victoria (prototype)", MACHINE_SUPPORTS_SAVE )
