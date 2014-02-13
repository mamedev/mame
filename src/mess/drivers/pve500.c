// license:MAME|GPL-2.0+
// copyright-holders:Felipe Correa
/***************************************************************************

  SONY PVE-500 Editing Control Unit
  "A/B roll edit controller for professional video editing applications"

  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>
  Technical info at https://www.garoa.net.br/wiki/PVE-500

  Licensed under GPLv2 or later.

  NOTE: Even though the MAME/MESS project has been adopting a non-commercial additional licensing clause, I do allow commercial usage
  of my portion of the code according to the plain terms of the GPL license (version 2 or later). This is useful if you happen to use
  my code in another project or in case the other MAME/MESS developers happen to drop the non-comercial clause completely. I suggest
  that other developers consider doing the same. --Felipe Sanches

  Changelog:

   2014 JAN 14 [Felipe Sanches]:
   * Initial driver skeleton
*/

#include "emu.h"
#include "cpu/z80/z80.h"

class pve500_state : public driver_device
{
public:
	pve500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
	{ }

	DECLARE_WRITE8_MEMBER(io_expander_w);
	DECLARE_READ8_MEMBER(io_expander_r);
	DECLARE_DRIVER_INIT(pve500);
private:
	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
};

static ADDRESS_MAP_START(maincpu_prg, AS_PROGRAM, 8, pve500_state)
	AM_RANGE (0x0000, 0xBFFF) AM_ROM // ICB7: 48kbytes EEPROM
	AM_RANGE (0xC000, 0xDFFF) AM_RAM // ICD6: 8kbytes of RAM
	AM_RANGE (0xE000, 0xE7FF) AM_MIRROR(0x1800) AM_RAM AM_SHARE("sharedram") //  ICF5: 2kbytes of RAM shared between the two CPUs
ADDRESS_MAP_END

static ADDRESS_MAP_START(subcpu_prg, AS_PROGRAM, 8, pve500_state)
	AM_RANGE (0x0000, 0x7FFF) AM_ROM // ICG5: 32kbytes EEPROM
	AM_RANGE (0x8000, 0xBFFF) AM_READWRITE(io_expander_r, io_expander_w) // ICG3: I/O Expander
	AM_RANGE (0xC000, 0xC7FF) AM_MIRROR(0x3800) AM_RAM AM_SHARE("sharedram") //  ICF5: 2kbytes of RAM shared between the two CPUs
ADDRESS_MAP_END

DRIVER_INIT_MEMBER( pve500_state, pve500 )
{
}

static INPUT_PORTS_START( pve500 )
	PORT_START("keyboard")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("TODO") PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

void pve500_state::machine_start()
{
}

void pve500_state::machine_reset()
{
}

READ8_MEMBER(pve500_state::io_expander_r)
{
	/* Implement-me ! */
	return 0;
}

WRITE8_MEMBER(pve500_state::io_expander_w)
{
	/* Implement-me !*/
}

static MACHINE_CONFIG_START( pve500, pve500_state )
/*
I think we should emulate the TOSHIBA TLCS-Z80 and instantiate it here.
TLCS-Z80 = Z80 CPU + internal CTC + internal SIO + some other things

The PVE-500 board uses both the internal and additional external CTCs and SIOs
*/
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz / 2) /* Actual chip is TMPZ84C015BF-6 (TOSHIBA TLCS-Z80) */
	MCFG_CPU_PROGRAM_MAP(maincpu_prg)
//	MCFG_Z80CTC_ADD("ctc", XTAL_?MHz, maincpu_ctc_intf)
//	MCFG_Z80SIO_ADD("sio", XTAL_12MHz / 2, maincpu_sio_intf)

	MCFG_CPU_ADD("subcpu", Z80, XTAL_12MHz / 2) /* Actual chip is TMPZ84C015BF-6 (TOSHIBA TLCS-Z80) */
	MCFG_CPU_PROGRAM_MAP(subcpu_prg)
//	MCFG_Z80CTC_ADD("ctc", XTAL_?MHz, subcpu_ctc_intf)
//	MCFG_Z80SIO_ADD("sio", XTAL_12MHz / 2, subcpu_sio_intf)

/* TODO:
-> There are a few LEDs and a sequence of 7-seg displays with atotal of 27 digits
-> Sound hardware consists of a buzzer connected to a signal of the maincpu SIO and a logic-gate that attaches/detaches it from the
   system clock Which apparently means you can only beep the buzzer to a certain predefined tone or keep it mute.
*/
MACHINE_CONFIG_END

ROM_START( pve500 )
	ROM_REGION( 0xC000, "maincpu", 0 )
	ROM_LOAD("pve500.icb7",  0x00000, 0xC000, CRC(7b1dc6ef) SHA1(141e2baa10e27e1d1bdb15ed3401bc9d0a775f22) ) //48kbyte main-cpu program

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD("pve500.icg5",  0x00000, 0x8000, CRC(28cca60a) SHA1(308d70062653769250327ede7a4e1a8a76fc9ab9) ) //32kbyte sub-cpu program
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   CLASS           INIT   COMPANY    FULLNAME                    FLAGS */
COMP( 1995, pve500, 0,      0,      pve500,     pve500, pve500_state, pve500, "SONY", "PVE-500", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND)
