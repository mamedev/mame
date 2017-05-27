// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:David Viens, Sean Riddle
/***************************************************************************

  x

  TODO:
  - x

***************************************************************************/

#include "emu.h"
#include "cpu/tms1000/tms1000.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"
#include "speaker.h"


class eva_state : public driver_device
{
public:
	eva_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void eva_state::machine_start()
{
	// zerofill

	// register for savestates
}

void eva_state::machine_reset()
{
}



/***************************************************************************

  I/O

***************************************************************************/



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( eva11 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( tms5110_route )

	/* sound hardware */
	MCFG_TMS5110_M0_CB(DEVWRITELINE("tms6100", tms6100_device, m0_w))
	MCFG_TMS5110_M1_CB(DEVWRITELINE("tms6100", tms6100_device, m1_w))
	MCFG_TMS5110_ADDR_CB(DEVWRITE8("tms6100", tms6100_device, add_w))
	MCFG_TMS5110_DATA_CB(DEVREADLINE("tms6100", tms6100_device, data_line_r))
	MCFG_TMS5110_ROMCLK_CB(DEVWRITELINE("tms6100", tms6100_device, clk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( eva11 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, XTAL_640kHz/2)

	/* sound hardware */
	MCFG_DEVICE_ADD("tms6100", TMS6100, XTAL_640kHz/4)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5100", TMS5110A, XTAL_640kHz)
	MCFG_FRAGMENT_ADD(tms5110_route)
MACHINE_CONFIG_END



/***************************************************************************

  ROM Defs, Game driver(s)

***************************************************************************/

ROM_START( eva11 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "32045b", 0x0000, 0x0400, CRC(eea36ebe) SHA1(094755b60965654ddc3e57cbd69f4749abd3b526) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_eva11_micro.pla", 0, 867, CRC(38fcc108) SHA1(be265300e0828b6ac83832b3279985a43817bb64) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_eva11_output.pla", 0, 365, CRC(f0f36970) SHA1(a6ad1f5e804ac98e5e1a1d07466b3db3a8d6c256) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cm73002.vsm", 0x0000, 0x1000, CRC(d5340bf8) SHA1(81195e8f870275d39a1abe1c8e2a6afdfdb15725) )
ROM_END



//    YEAR  NAME  PARENT CMP MACHINE INPUT STATE   INIT  COMPANY, FULLNAME, FLAGS
SYST( 1983, eva11,  0,      0, eva11,    eva11,  eva_state, 0, "Chrysler", "Electronic Voice Alert (11-function)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
