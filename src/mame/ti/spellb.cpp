// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Texas Instruments Spelling B hardware

The Spelling B was introduced together with the Speak & Spell. It is a handheld
educational toy with booklet. Two revisions of the hardware exist.

1st version:

Spelling B (US), 1978
- TMS0270 MCU TMC0272 (die label: 0272A T0270B)
- TMS1980 MCU TMC1984 (die label: 1980A 84A)
- 8-digit cyan VFD (seen with and without apostrophe)

2nd version:

Spelling B (US), 1980
- TMS0270 MCU TMC0274
- TMC0355 4KB VSM ROM CD2602
- 8-digit cyan VFD
- 1-bit sound (indicated by a music note symbol on the top-right of the casing)
- note: much rarer than the 1978 version, not much luck finding one on eBay.
  The words/indexes from the documentation are the same as the older version.

Spelling ABC (UK), 1980: exact same hardware as US 2nd version (the 1st version
was also sold in the UK earlier, but not renamed)

Spelling ABC (Germany), 1980: different VSM
- TMC0355 4KB VSM ROM CD2607

Mr. Challenger (US), 1979
- TMS0270 MCU TMC0273
- TMC0355 4KB VSM ROM CD2601
- 8-digit cyan VFD
- 1-bit sound

Letterlogic (UK), 1980: exact same hardware as US Mr. Challenger
- note: stylized as "LETTERlogic", same for other language versions

Letterlogic (France), 1980: different VSM
- TMC0355 4KB VSM ROM CD2603

Letterlogic (Germany), 1980: different VSM
- TMC0355 4KB VSM ROM CD2604

*******************************************************************************/

#include "emu.h"

#include "cpu/tms1000/tms0270.h"
#include "cpu/tms1000/tms0980.h"
#include "machine/tms6100.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "spellb.lh"


namespace {

class spellb_state : public driver_device
{
public:
	spellb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_display(*this, "display"),
		m_tms6100(*this, "tms6100"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void rev1(machine_config &config);
	void rev2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<tms0270_cpu_device> m_maincpu;
	optional_device<tms1980_cpu_device> m_subcpu;
	required_device<pwm_display_device> m_display;
	optional_device<tms6100_device> m_tms6100;
	optional_device<speaker_sound_device> m_speaker;
	required_ioport_array<8> m_inputs;

	bool m_power_on = false;
	u32 m_r = 0;
	u16 m_grid = 0;
	u16 m_plate = 0;
	u16 m_sub_o = 0;
	u16 m_sub_r = 0;
	u8 m_rev1_ctl = 0;

	void power_off();
	void power_subcpu();
	void update_display();

	u8 main_read_k();
	void main_write_o(u16 data);
	void main_write_r(u32 data);

	u8 rev1_ctl_r();
	void rev1_ctl_w(u8 data);
	u8 sub_read_k();
	void sub_write_o(u16 data);
	void sub_write_r(u16 data);

	void rev2_write_o(u16 data);
	void rev2_write_r(u32 data);
};

void spellb_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power_on));
	save_item(NAME(m_r));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
	save_item(NAME(m_sub_o));
	save_item(NAME(m_sub_r));
	save_item(NAME(m_rev1_ctl));
}



/*******************************************************************************
    Power
*******************************************************************************/

void spellb_state::machine_reset()
{
	m_power_on = true;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	power_subcpu();
}

INPUT_CHANGED_MEMBER(spellb_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void spellb_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	power_subcpu();

	m_display->clear();
}

void spellb_state::power_subcpu()
{
	if (m_subcpu)
		m_subcpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common

void spellb_state::update_display()
{
	// almost same as snspell
	u16 gridmask = m_display->row_on(15) ? 0xffff : 0x8000;
	m_display->matrix(m_grid & gridmask, m_plate);
}

void spellb_state::main_write_o(u16 data)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15, same as snspell
	m_plate = bitswap<16>(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	update_display();
}

void spellb_state::main_write_r(u32 data)
{
	// R0-R7: select digit
	// R15: filament on
	m_grid = data & 0x80ff;
	update_display();

	// R13: power-off request, on falling edge
	if (~data & m_r & 0x2000)
		power_off();

	// R0-R6: input mux
	m_r = data;
}

u8 spellb_state::main_read_k()
{
	u8 data = 0;

	// K: multiplexed inputs
	for (int i = 0; i < 7; i++)
		if (BIT(m_r, i))
			data |= m_inputs[i]->read();

	// Vss row is always on
	return data | m_inputs[7]->read();
}


// 1st revision mcu/mcu comms

void spellb_state::rev1_ctl_w(u8 data)
{
	// main CTL write data
	m_rev1_ctl = data & 0xf;
}

u8 spellb_state::sub_read_k()
{
	// sub K8421 <- main CTL3210 (does not use external CS)
	if (m_r & 0x1000)
		return m_sub_o | m_rev1_ctl;
	else
		return m_sub_o | (m_plate & 0xe) | (m_plate >> 6 & 1);
}

void spellb_state::sub_write_o(u16 data)
{
	// sub O write data
	m_sub_o = bitswap<4>(data,6,0,4,3);
}

u8 spellb_state::rev1_ctl_r()
{
	// main CTL3210 <- sub O6043
	return m_sub_o;
}

void spellb_state::sub_write_r(u16 data)
{
	// sub R: unused?
	m_sub_r = data;
}


// 2nd revision specifics

void spellb_state::rev2_write_o(u16 data)
{
	// SEG DP: speaker out
	m_speaker->level_w(data >> 15 & 1);

	// SEG DP and SEG AP are not connected to VFD, rest is same as rev1
	main_write_o(data & 0x6fff);
}

void spellb_state::rev2_write_r(u32 data)
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



/*******************************************************************************
    Input Ports
*******************************************************************************/

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_NAME("Memory")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("Clue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Erase")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_CHAR('1') PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("Level")

	PORT_START("IN.7") // Vss
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("Missing Letter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME("Mystery Word")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("Scramble")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_CHAR('6') PORT_NAME("Spelling B/On") PORT_CHANGED_MEMBER(DEVICE_SELF, spellb_state, power_on, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("Starts With")
INPUT_PORTS_END

static INPUT_PORTS_START( spellabc )
	PORT_INCLUDE( spellb )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_NAME("Speicher")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("Rat")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Tilgen")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Eingabe")

	PORT_MODIFY("IN.6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("Stufe")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("Was Fehlt?")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME(u8"Wörter Rätsel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("Wirr Warr")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_CHAR('6') PORT_NAME("Lerne ABC/On") PORT_CHANGED_MEMBER(DEVICE_SELF, spellb_state, power_on, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("Anfang Mit")
INPUT_PORTS_END


static INPUT_PORTS_START( mrchalgr )
	PORT_INCLUDE( spellb ) // same key layout as spellb

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_NAME("2nd Player")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Score")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("Crazy Letters")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME("Letter Guesser")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("Word Challenge")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_CHAR('6') PORT_NAME("Mystery Word/On") PORT_CHANGED_MEMBER(DEVICE_SELF, spellb_state, power_on, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("Replay")
INPUT_PORTS_END

static INPUT_PORTS_START( letterlf )
	PORT_INCLUDE( mrchalgr )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_NAME("Joueur 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("Aide")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Effacez")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Marque")

	PORT_MODIFY("IN.6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("Niveau")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("Suite Folle")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME("Devin")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("Duel")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_CHAR('6') PORT_NAME("Mot Mystere/On") PORT_CHANGED_MEMBER(DEVICE_SELF, spellb_state, power_on, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("Rejouez")
INPUT_PORTS_END

static INPUT_PORTS_START( letterlg )
	PORT_INCLUDE( mrchalgr )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_NAME("Spieler 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("Rat")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Tilgen")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Punkte")

	PORT_MODIFY("IN.6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("Stufe")

	PORT_MODIFY("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("Lettern Salat")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME("Lettern Rater")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("Wettstreit")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F1) PORT_CHAR('6') PORT_NAME(u8"Wörter Rätsel/On") PORT_CHANGED_MEMBER(DEVICE_SELF, spellb_state, power_on, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("Wiedergabe")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void spellb_state::rev1(machine_config &config)
{
	// basic machine hardware
	TMS0270(config, m_maincpu, 320000); // approximation
	m_maincpu->read_k().set(FUNC(spellb_state::main_read_k));
	m_maincpu->write_o().set(FUNC(spellb_state::main_write_o));
	m_maincpu->write_r().set(FUNC(spellb_state::main_write_r));
	m_maincpu->read_ctl().set(FUNC(spellb_state::rev1_ctl_r));
	m_maincpu->write_ctl().set(FUNC(spellb_state::rev1_ctl_w));

	TMS1980(config, m_subcpu, 320000); // approximation
	m_subcpu->read_k().set(FUNC(spellb_state::sub_read_k));
	m_subcpu->write_o().set(FUNC(spellb_state::sub_write_o));
	m_subcpu->write_r().set(FUNC(spellb_state::sub_write_r));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 16);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_spellb);

	// no sound!
}

void spellb_state::rev2(machine_config &config)
{
	// basic machine hardware
	TMS0270(config, m_maincpu, 320000); // approximation
	m_maincpu->read_k().set(FUNC(spellb_state::main_read_k));
	m_maincpu->write_o().set(FUNC(spellb_state::rev2_write_o));
	m_maincpu->write_r().set(FUNC(spellb_state::rev2_write_r));
	m_maincpu->read_ctl().set(m_tms6100, FUNC(tms6100_device::data_r));
	m_maincpu->write_ctl().set(m_tms6100, FUNC(tms6100_device::add_w));

	TMS6100(config, m_tms6100, 320000);
	m_tms6100->enable_4bit_mode(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 16);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_spellb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( spellb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0274n2l", 0x0000, 0x1000, CRC(98e3bd32) SHA1(e79b59ac29b0183bf1ee8d84b2944450c5e5d8fb) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_spellb_output.pla", 0, 1246, CRC(b95e35e6) SHA1(430917486856c9e6c28af10ff3758242048096c4) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cd2602", 0x0000, 0x1000, CRC(dd1fff8c) SHA1(f1760b29aa50fc96a1538db814cc73289654ac25) )
ROM_END

ROM_START( spellabc )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0274n2l", 0x0000, 0x1000, CRC(98e3bd32) SHA1(e79b59ac29b0183bf1ee8d84b2944450c5e5d8fb) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_spellb_output.pla", 0, 1246, CRC(b95e35e6) SHA1(430917486856c9e6c28af10ff3758242048096c4) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cd2607", 0x0000, 0x1000, CRC(875090c0) SHA1(73b87fff64054f6ab3b7e69d89585582145dbaa7) )
ROM_END

ROM_START( spellba )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0272nl", 0x0000, 0x1000, CRC(f90318ff) SHA1(7cff03fafbc66b0e07b3c70a513fbb0b11eef4ea) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_spellba_output.pla", 0, 1246, CRC(3e021cbd) SHA1(c9bdfe10601b8a5a70442fe4805e4bfed8bbed35) )

	ROM_REGION( 0x1000, "subcpu", 0 )
	ROM_LOAD( "tmc1984nl", 0x0000, 0x1000, CRC(78c9c83a) SHA1(6307fe2a0228fd1b8d308fcaae1b8e856d40fe57) )

	ROM_REGION( 1246, "subcpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "subcpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "subcpu:opla", 0 )
	ROM_LOAD( "tms1980_spellba_output.pla", 0, 525, CRC(1e26a719) SHA1(eb031aa216fe865bc9e40b070ca5de2b1509f13b) )
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
	ROM_LOAD( "cd2601", 0x0000, 0x1000, CRC(a9fbe7e9) SHA1(9d480cb30313b8cbce2d048140c1e5e6c5b92452) )
ROM_END

ROM_START( letterlf )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0273nll", 0x0000, 0x1000, CRC(ef6d23bd) SHA1(194e3b022c299e99a731bbcfba5bf8a3a9f0d07e) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_mrchalgr_output.pla", 0, 1246, CRC(4785289c) SHA1(60567af0ea120872a4ccf3128e1365fe84722aa8) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cd2603", 0x0000, 0x1000, CRC(70ac954b) SHA1(5593a5844063acdf399600e3e842f0fbe712ba69) )
ROM_END

ROM_START( letterlg )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0273nll", 0x0000, 0x1000, CRC(ef6d23bd) SHA1(194e3b022c299e99a731bbcfba5bf8a3a9f0d07e) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_mrchalgr_output.pla", 0, 1246, CRC(4785289c) SHA1(60567af0ea120872a4ccf3128e1365fe84722aa8) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cd2604", 0x0000, 0x1000, CRC(cdb6f039) SHA1(56f512720c5e80cd74b65e31d5a19bf1260017fb) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE  INPUT     CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, spellb,   0,        0,      rev2,    spellb,   spellb_state, empty_init, "Texas Instruments", "Spelling B (US, 1980 version)", MACHINE_SUPPORTS_SAVE )
SYST( 1978, spellba,  spellb,   0,      rev1,    spellb,   spellb_state, empty_init, "Texas Instruments", "Spelling B (US, 1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1980, spellabc, spellb,   0,      rev2,    spellabc, spellb_state, empty_init, "Texas Instruments", "Spelling ABC (Germany)", MACHINE_SUPPORTS_SAVE )

SYST( 1979, mrchalgr, 0,        0,      rev2,    mrchalgr, spellb_state, empty_init, "Texas Instruments", "Mr. Challenger (US)", MACHINE_SUPPORTS_SAVE )
SYST( 1980, letterlf, mrchalgr, 0,      rev2,    letterlf, spellb_state, empty_init, "Texas Instruments", "Letterlogic (France)", MACHINE_SUPPORTS_SAVE )
SYST( 1980, letterlg, mrchalgr, 0,      rev2,    letterlg, spellb_state, empty_init, "Texas Instruments", "Letterlogic (Germany)", MACHINE_SUPPORTS_SAVE )
