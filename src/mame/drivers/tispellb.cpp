// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **
  
  Texas Instruments Spelling B hardware
  
  The Spelling B was introduced together with the Speak & Spell. It is a
  handheld educational toy with booklet. Two revisions of the hardware exist.
  (* indicates not dumped)
  
  1st revision:
  
  Spelling B (US), 1978
  - TMS0270 MCU TMC0272 (die labeled 0272A T0270B)
  - TMS1980 MCU TMC1984 (die labeled 1980A 84A)
  - 8-digit cyan VFD display (seen with and without apostrophe)
  
  Spelling ABC (UK), 1979: exact same hardware as US version

  2nd revision:
  
  Spelling B (US), 1980
  - TMS0270 MCU TMC0274*
  - TMC0355 4KB VSM ROM CD2602*
  - 8-digit cyan VFD display
  - 1-bit sound (indicated by a music note symbol on the top-right of the casing)

  Spelling ABC (UK), 1980: exact same hardware as US version

  Spelling ABC (Germany), 1980: different VSM
  - TMC0355 4KB VSM ROM CD2607*
  
  Mr. Challenger (US), 1980
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
  - spellb numbers don't match with picture book
  - spellb random lockups


***************************************************************************/

#include "includes/hh_tms1k.h"
#include "machine/tms6100.h"

// internal artwork
#include "spellb.lh"


class tispellb_state : public hh_tms1k_state
{
public:
	tispellb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_tms6100(*this, "tms6100")
	{ }

	// devices
	optional_device<cpu_device> m_subcpu;
	optional_device<tms6100_device> m_tms6100;

	UINT8 m_rev1_ctl;
	UINT16 m_sub_o;
	UINT16 m_sub_r;

	virtual DECLARE_INPUT_CHANGED_MEMBER(power_button) override;
	void power_off();
	void prepare_display();

	DECLARE_READ8_MEMBER(main_read_k);
	DECLARE_WRITE16_MEMBER(main_write_o);
	DECLARE_WRITE16_MEMBER(main_write_r);

	DECLARE_READ8_MEMBER(rev1_ctl_r);
	DECLARE_WRITE8_MEMBER(rev1_ctl_w);
	DECLARE_READ8_MEMBER(sub_read_k);
	DECLARE_WRITE16_MEMBER(sub_write_o);
	DECLARE_WRITE16_MEMBER(sub_write_r);

	DECLARE_WRITE16_MEMBER(rev2_write_o);
	DECLARE_WRITE16_MEMBER(rev2_write_r);

protected:
	virtual void machine_start() override;
};


void tispellb_state::machine_start()
{
	hh_tms1k_state::machine_start();
	memset(m_display_segmask, ~0, sizeof(m_display_segmask)); // !

	// zerofill
	m_rev1_ctl = 0;
	m_sub_o = 0;
	m_sub_r = 0;

	// register for savestates
	save_item(NAME(m_rev1_ctl));
	save_item(NAME(m_sub_o));
	save_item(NAME(m_sub_r));
}



/***************************************************************************

  I/O

***************************************************************************/

// common

void tispellb_state::power_off()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	if (m_subcpu)
		m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_power_on = false;
}

void tispellb_state::prepare_display()
{
	// same as snspell
	UINT16 gridmask = (m_display_decay[15][16] != 0) ? 0xffff : 0x8000;
	display_matrix_seg(16+1, 16, m_plate | 0x10000, m_grid & gridmask, 0x3fff);
}

WRITE16_MEMBER(tispellb_state::main_write_o)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15, same as snspell
	m_plate = BITSWAP16(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	prepare_display();
}

WRITE16_MEMBER(tispellb_state::main_write_r)
{
	// R13: power-off request, on falling edge
	if ((m_r >> 13 & 1) && !(data >> 13 & 1))
		power_off();

	// R0-R6: input mux
	// R0-R7: select digit
	// R15: filament on
	m_r = data;
	m_inp_mux = data & 0x7f;
	m_grid = data & 0x80ff;
	prepare_display();
}

READ8_MEMBER(tispellb_state::main_read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[7]->read() | read_inputs(7);
}


// 1st revision mcu/mcu comms

WRITE8_MEMBER(tispellb_state::rev1_ctl_w)
{
	// main CTL write data
	m_rev1_ctl = data & 0xf;
}

READ8_MEMBER(tispellb_state::sub_read_k)
{
	// sub K3210 <- main CTL3201
	return BITSWAP8(m_rev1_ctl,7,6,5,4,3,2,0,1);
}

WRITE16_MEMBER(tispellb_state::sub_write_o)
{
	// sub O write data
	m_sub_o = data;
}

READ8_MEMBER(tispellb_state::rev1_ctl_r)
{
	// main CTL3210 <- sub O6043
	return BITSWAP8(m_sub_o,7,5,2,1,6,0,4,3) & 0xf;
}

WRITE16_MEMBER(tispellb_state::sub_write_r)
{
	// sub R: unused?
	m_sub_r = data;
}


// 2nd revision specifics

WRITE16_MEMBER(tispellb_state::rev2_write_o)
{
	// SEG DP: speaker out
	m_speaker->level_w(data >> 15 & 1);
	
	// SEG DP and SEG AP are not connected to VFD, rest is same as rev1
	main_write_o(space, offset, data & 0x6fff);
}

WRITE16_MEMBER(tispellb_state::rev2_write_r)
{
	// R12: TMC0355 CS
	// R4: TMC0355 M1
	// R6: TMC0355 M0
	if (data & 0x1000)
	{
		m_tms6100->m1_w(data >> 4 & 1);
		m_tms6100->m0_w(data >> 6 & 1);
		m_tms6100->romclock_w(1);
		m_tms6100->romclock_w(0);
	}
	
	// rest is same as rev1
	main_write_r(space, offset, data);
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(tispellb_state::power_button)
{
	int on = (int)(FPTR)param;

	if (on && !m_power_on)
	{
		m_power_on = true;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		
		if (m_subcpu)
			m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else if (!on && m_power_on)
		power_off();
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Memory")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Clue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_HOME) PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Level")

	PORT_START("IN.7") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Missing Letter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Mystery Word")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Scramble")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Spelling B/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispellb_state, power_button, (void *)true)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Starts With")
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Mystery Word/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispellb_state, power_button, (void *)true)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Replay")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( rev1, tispellb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, 350000) // approximation
	MCFG_TMS1XXX_READ_K_CB(READ8(tispellb_state, main_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispellb_state, main_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispellb_state, main_write_r))
	MCFG_TMS0270_READ_CTL_CB(READ8(tispellb_state, rev1_ctl_r))
	MCFG_TMS0270_WRITE_CTL_CB(WRITE8(tispellb_state, rev1_ctl_w))

	MCFG_CPU_ADD("subcpu", TMS1980, 350000) // approximation
	MCFG_TMS1XXX_READ_K_CB(READ8(tispellb_state, sub_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispellb_state, sub_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispellb_state, sub_write_r))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_spellb)

	/* no sound! */
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( rev2, tispellb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, 350000) // approximation
	MCFG_TMS1XXX_READ_K_CB(READ8(tispellb_state, main_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispellb_state, rev2_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispellb_state, rev2_write_r))
	MCFG_TMS0270_READ_CTL_CB(DEVREAD8("tms6100", tms6100_device, data_r))
	MCFG_TMS0270_WRITE_CTL_CB(DEVWRITE8("tms6100", tms6100_device, addr_w))

	MCFG_DEVICE_ADD("tms6100", TMS6100, 350000)
	MCFG_TMS6100_4BIT_MODE()

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_spellb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



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


ROM_START( mrchalgr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tmc0273nll", 0x0000, 0x1000, CRC(ef6d23bd) SHA1(194e3b022c299e99a731bbcfba5bf8a3a9f0d07e) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_mrchalgr_output.pla", 0, 1246, CRC(4785289c) SHA1(60567af0ea120872a4ccf3128e1365fe84722aa8) )

	ROM_REGION( 0x4000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cd2601.vsm", 0x0000, 0x1000, CRC(a9fbe7e9) SHA1(9d480cb30313b8cbce2d048140c1e5e6c5b92452) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE INPUT      INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1978, spellb,    0,        0,  rev1,   spellb,    driver_device, 0, "Texas Instruments", "Spelling B (1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )

COMP( 1980, mrchalgr,  0,        0,  rev2,   mrchalgr,  driver_device, 0, "Texas Instruments", "Mr. Challenger", MACHINE_SUPPORTS_SAVE )
