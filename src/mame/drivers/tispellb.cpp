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
  - unexpected pigeon


***************************************************************************/

#include "includes/hh_tms1k.h"

// internal artwork
#include "spellb.lh"


class tispellb_state : public hh_tms1k_state
{
public:
	tispellb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu")
	{ }

	// devices
	optional_device<cpu_device> m_subcpu;

	UINT8 m_rev1_ctl;
	UINT16 m_sub_o;
	UINT16 m_sub_r;

	DECLARE_READ8_MEMBER(main_read_k);
	DECLARE_WRITE16_MEMBER(main_write_o);
	DECLARE_WRITE16_MEMBER(main_write_r);

	DECLARE_READ8_MEMBER(rev1_ctl_r);
	DECLARE_WRITE8_MEMBER(rev1_ctl_w);
	DECLARE_READ8_MEMBER(sub_read_k);
	DECLARE_WRITE16_MEMBER(sub_write_o);
	DECLARE_WRITE16_MEMBER(sub_write_r);

	void prepare_display();

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

void tispellb_state::prepare_display()
{
	display_matrix_seg(16, 8, m_plate, m_grid, 0x3fff);
}

WRITE16_MEMBER(tispellb_state::main_write_o)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15, same as snspell
	m_plate = BITSWAP16(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);
	prepare_display();
}

WRITE16_MEMBER(tispellb_state::main_write_r)
{
	// R0-R6: input mux
	// R0-R7: select digit
	m_r = data;
	m_inp_mux = data & 0x7f;
	m_grid = data & 0xff;
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



/***************************************************************************

  Inputs

***************************************************************************/

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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Spelling B/On") //PORT_CHANGED_MEMBER(DEVICE_SELF, tispellb_state, power_button, (void *)true)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Starts With")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( rev1, tispellb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, 300000) // approximation
	MCFG_TMS1XXX_READ_K_CB(READ8(tispellb_state, main_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispellb_state, main_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispellb_state, main_write_r))
	MCFG_TMS0270_READ_CTL_CB(READ8(tispellb_state, rev1_ctl_r))
	MCFG_TMS0270_WRITE_CTL_CB(WRITE8(tispellb_state, rev1_ctl_w))

	MCFG_CPU_ADD("subcpu", TMS1980, 300000) // approximation
	MCFG_TMS1XXX_READ_K_CB(READ8(tispellb_state, sub_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispellb_state, sub_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispellb_state, sub_write_r))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_spellb)

	/* no sound! */
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
	ROM_LOAD( "tmc1984nl", 0x0000, 0x1000, CRC(ad417878) SHA1(d02ca44db104d34e8089037ddd514958eb007e27) )

	ROM_REGION( 1246, "subcpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "subcpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "subcpu:opla", 0 )
	ROM_LOAD( "tms1980_spellb_output.pla", 0, 525, CRC(1e26a719) SHA1(eb031aa216fe865bc9e40b070ca5de2b1509f13b) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE INPUT      INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1978, spellb,    0,        0,  rev1,   spellb,    driver_device, 0, "Texas Instruments", "Spelling B (1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
