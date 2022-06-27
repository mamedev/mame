// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **

  Texas Instruments Spelling B hardware

  The Spelling B was introduced together with the Speak & Spell. It is a
  handheld educational toy with booklet. Two revisions of the hardware exist.
  (* indicates not dumped)

  1st revision:

  Spelling B (US), 1978
  - TMS0270 MCU TMC0272 (die label 0272A T0270B)
  - TMS1980 MCU TMC1984 (die label 1980A 84A)
  - 8-digit cyan VFD display (seen with and without apostrophe)

  Spelling ABC (UK), 1979: exact same hardware as US version

  2nd revision:

  Spelling B (US), 1979
  - TMS0270 MCU TMC0274
  - TMC0355 4KB VSM ROM CD2602
  - 8-digit cyan VFD display
  - 1-bit sound (indicated by a music note symbol on the top-right of the casing)
  - note: much rarer than the 1978 version, not much luck finding one on eBay

  Spelling ABC (UK), 1979: exact same hardware as US version

  Spelling ABC (Germany), 1979: different VSM
  - TMC0355 4KB VSM ROM CD2607*

  Mr. Challenger (US), 1979
  - TMS0270 MCU TMC0273
  - TMC0355 4KB VSM ROM CD2601
  - 8-digit cyan VFD display
  - 1-bit sound

  Letterlogic (UK), 1980: exact same hardware as US Mr. Challenger

  Letterlogic (France), 1980: different VSM
  - TMC0355 4KB VSM ROM CD2603*

  Letterlogic (Germany), 1980: different VSM
  - TMC0355 4KB VSM ROM CD2604*


----------------------------------------------------------------------------

  TODO:
  - spellb fetches wrong word sometimes (on lv1 SPOON and ANT) - roms were doublechecked


***************************************************************************/

#include "emu.h"
#include "includes/hh_tms1k.h"

#include "machine/tms6100.h"
#include "speaker.h"

// internal artwork
#include "spellb.lh"


class tispellb_state : public hh_tms1k_state
{
public:
	tispellb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_tms6100(*this, "tms6100")
	{ }

	void rev1(machine_config &config);
	void rev2(machine_config &config);

	virtual DECLARE_INPUT_CHANGED_MEMBER(power_button) override;

private:
	// devices
	optional_device<tms1k_base_device> m_subcpu;
	optional_device<tms6100_device> m_tms6100;

	u8 m_rev1_ctl = 0;
	u16 m_sub_o = 0;
	u16 m_sub_r = 0;

	virtual void power_off() override;
	void power_subcpu();
	void update_display();

	u8 main_read_k();
	void main_write_o(u16 data);
	void main_write_r(u16 data);

	u8 rev1_ctl_r();
	void rev1_ctl_w(u8 data);
	u8 sub_read_k();
	void sub_write_o(u16 data);
	void sub_write_r(u16 data);

	void rev2_write_o(u16 data);
	void rev2_write_r(u16 data);

	virtual void machine_start() override;
};

void tispellb_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_rev1_ctl));
	save_item(NAME(m_sub_o));
	save_item(NAME(m_sub_r));
}



/***************************************************************************

  I/O

***************************************************************************/

// common

void tispellb_state::power_subcpu()
{
	if (m_subcpu)
		m_subcpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}

void tispellb_state::power_off()
{
	hh_tms1k_state::power_off();
	power_subcpu();
}

void tispellb_state::update_display()
{
	// almost same as snspell
	u16 gridmask = m_display->row_on(15) ? 0xffff : 0x8000;
	m_display->matrix(m_grid & gridmask, m_plate);
}

void tispellb_state::main_write_o(u16 data)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15, same as snspell
	m_plate = bitswap<16>(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	update_display();
}

void tispellb_state::main_write_r(u16 data)
{
	// R13: power-off request, on falling edge
	if (~data & m_r & 0x2000)
		power_off();

	// R0-R6: input mux
	// R0-R7: select digit
	// R15: filament on
	m_r = data;
	m_inp_mux = data & 0x7f;
	m_grid = data & 0x80ff;
	update_display();
}

u8 tispellb_state::main_read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[7]->read() | read_inputs(7);
}


// 1st revision mcu/mcu comms

void tispellb_state::rev1_ctl_w(u8 data)
{
	// main CTL write data
	m_rev1_ctl = data & 0xf;
}

u8 tispellb_state::sub_read_k()
{
	// sub K8421 <- main CTL3210
	return m_rev1_ctl;
}

void tispellb_state::sub_write_o(u16 data)
{
	// sub O write data
	m_sub_o = data;
}

u8 tispellb_state::rev1_ctl_r()
{
	// main CTL3210 <- sub O6043
	return bitswap<4>(m_sub_o,6,0,4,3);
}

void tispellb_state::sub_write_r(u16 data)
{
	// sub R: unused?
	m_sub_r = data;
}


// 2nd revision specifics

void tispellb_state::rev2_write_o(u16 data)
{
	// SEG DP: speaker out
	m_speaker->level_w(data >> 15 & 1);

	// SEG DP and SEG AP are not connected to VFD, rest is same as rev1
	main_write_o(data & 0x6fff);
}

void tispellb_state::rev2_write_r(u16 data)
{
	// R12: TMC0355 CS
	// R4: TMC0355 M1
	// R6: TMC0355 M0
	m_tms6100->cs_w(data >> 12 & 1);
	m_tms6100->m1_w(data >> 4 & 1);
	m_tms6100->m0_w(data >> 6 & 1);
	m_tms6100->clk_w(1);
	m_tms6100->clk_w(0);

	// rest is same as rev1
	main_write_r(data);
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(tispellb_state::power_button)
{
	hh_tms1k_state::power_button(field, param, oldval, newval);
	power_subcpu();
}

static INPUT_PORTS_START( spellb )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Memory") PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Clue") PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase") PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter") PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go") PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Level") PORT_CHAR('/')

	PORT_START("IN.7") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Missing Letter") PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Mystery Word") PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Scramble") PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_NAME("Spelling B/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispellb_state, power_button, true)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Starts With") PORT_CHAR('2')
INPUT_PORTS_END


static INPUT_PORTS_START( mrchalgr )
	PORT_INCLUDE( spellb ) // same key layout as spellb

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("2nd Player")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Score")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Crazy Letters")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Letter Guesser")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Word Challenge")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_NAME("Mystery Word/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispellb_state, power_button, true)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Replay")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tispellb_state::rev1(machine_config &config)
{
	// basic machine hardware
	TMS0270(config, m_maincpu, 350000); // approximation
	m_maincpu->k().set(FUNC(tispellb_state::main_read_k));
	m_maincpu->o().set(FUNC(tispellb_state::main_write_o));
	m_maincpu->r().set(FUNC(tispellb_state::main_write_r));
	m_maincpu->read_ctl().set(FUNC(tispellb_state::rev1_ctl_r));
	m_maincpu->write_ctl().set(FUNC(tispellb_state::rev1_ctl_w));

	TMS1980(config, m_subcpu, 350000); // approximation
	m_subcpu->k().set(FUNC(tispellb_state::sub_read_k));
	m_subcpu->o().set(FUNC(tispellb_state::sub_write_o));
	m_subcpu->r().set(FUNC(tispellb_state::sub_write_r));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 16);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_spellb);

	// no sound!
}


void tispellb_state::rev2(machine_config &config)
{
	// basic machine hardware
	TMS0270(config, m_maincpu, 350000); // approximation
	m_maincpu->k().set(FUNC(tispellb_state::main_read_k));
	m_maincpu->o().set(FUNC(tispellb_state::rev2_write_o));
	m_maincpu->r().set(FUNC(tispellb_state::rev2_write_r));
	m_maincpu->read_ctl().set(m_tms6100, FUNC(tms6100_device::data_r));
	m_maincpu->write_ctl().set(m_tms6100, FUNC(tms6100_device::add_w));

	TMS6100(config, m_tms6100, 350000);
	m_tms6100->enable_4bit_mode(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 16);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_spellb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( spellb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0272nl", 0x0000, 0x1000, CRC(f90318ff) SHA1(7cff03fafbc66b0e07b3c70a513fbb0b11eef4ea) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_spellb_output.pla", 0, 1246, CRC(3e021cbd) SHA1(c9bdfe10601b8a5a70442fe4805e4bfed8bbed35) )

	ROM_REGION( 0x1000, "subcpu", 0 )
	ROM_LOAD( "tmc1984nl", 0x0000, 0x1000, CRC(78c9c83a) SHA1(6307fe2a0228fd1b8d308fcaae1b8e856d40fe57) )

	ROM_REGION( 1246, "subcpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "subcpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "subcpu:opla", 0 )
	ROM_LOAD( "tms1980_spellb_output.pla", 0, 525, CRC(1e26a719) SHA1(eb031aa216fe865bc9e40b070ca5de2b1509f13b) )
ROM_END

ROM_START( spellb79 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0274n2l", 0x0000, 0x1000, CRC(98e3bd32) SHA1(e79b59ac29b0183bf1ee8d84b2944450c5e5d8fb) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_spellb79_output.pla", 0, 1246, CRC(b95e35e6) SHA1(430917486856c9e6c28af10ff3758242048096c4) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cd2602.vsm", 0x0000, 0x1000, CRC(dd1fff8c) SHA1(f1760b29aa50fc96a1538db814cc73289654ac25) )
ROM_END


ROM_START( mrchalgr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0273nll", 0x0000, 0x1000, CRC(ef6d23bd) SHA1(194e3b022c299e99a731bbcfba5bf8a3a9f0d07e) ) // matches patent US4421487

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_mrchalgr_output.pla", 0, 1246, CRC(4785289c) SHA1(60567af0ea120872a4ccf3128e1365fe84722aa8) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cd2601.vsm", 0x0000, 0x1000, CRC(a9fbe7e9) SHA1(9d480cb30313b8cbce2d048140c1e5e6c5b92452) )
ROM_END



//    YEAR  NAME      PARENT CMP MACHINE  INPUT     CLASS           INIT        COMPANY              FULLNAME                     FLAGS
COMP( 1978, spellb,   0,      0, rev1,    spellb,   tispellb_state, empty_init, "Texas Instruments", "Spelling B (1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1979, spellb79, spellb, 0, rev2,    spellb,   tispellb_state, empty_init, "Texas Instruments", "Spelling B (1979 version)", MACHINE_SUPPORTS_SAVE )

COMP( 1979, mrchalgr, 0,      0, rev2,    mrchalgr, tispellb_state, empty_init, "Texas Instruments", "Mr. Challenger",            MACHINE_SUPPORTS_SAVE )
