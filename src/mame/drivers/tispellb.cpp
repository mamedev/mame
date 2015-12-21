// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **
  
  Texas Instruments Spelling B hardware
  
  The Spelling B was introduced together with the Speak & Spell. It is a
  handheld educational toy. Two revisions of the hardware exist. (* indicates not dumped)
  
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
//#include "spellb.lh"


class tispellb_state : public hh_tms1k_state
{
public:
	tispellb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_start() override;
};


void tispellb_state::machine_start()
{
	hh_tms1k_state::machine_start();
	memset(m_display_segmask, ~0, sizeof(m_display_segmask)); // !
}



/***************************************************************************

  I/O

***************************************************************************/



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( spellb )

INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( spellb, tispellb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, 300000) // guessed

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

	ROM_REGION( 0x1000, "sub", 0 )
	ROM_LOAD( "tmc1984nl", 0x0000, 0x1000, CRC(ad417878) SHA1(d02ca44db104d34e8089037ddd514958eb007e27) )

	ROM_REGION( 1246, "sub:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "sub:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "sub:opla", 0 )
	ROM_LOAD( "tms1980_spellb_output.pla", 0, 525, CRC(1e26a719) SHA1(eb031aa216fe865bc9e40b070ca5de2b1509f13b) )
ROM_END






/*    YEAR  NAME       PARENT COMPAT MACHINE   INPUT      INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1978, spellb,    0,        0, spellb,    spellb,    driver_device, 0, "Texas Instruments", "Spelling B (1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
