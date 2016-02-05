// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **

  Coleco Talking Teacher
  *

***************************************************************************/

#include "includes/hh_tms1k.h"


class ctteach_state : public hh_tms1k_state
{
public:
	ctteach_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

protected:
	virtual void machine_start() override;
};


/***************************************************************************

  I/O

***************************************************************************/

WRITE16_MEMBER(ctteach_state::write_r)
{
}

WRITE16_MEMBER(ctteach_state::write_o)
{
}

READ8_MEMBER(ctteach_state::read_k)
{
	return 0;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( ctteach )

INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void ctteach_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill

	// register for savestates
}


static MACHINE_CONFIG_START( ctteach, ctteach_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 400000) // approximation
	MCFG_TMS1XXX_READ_K_CB(READ8(ctteach_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(ctteach_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(ctteach_state, write_o))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ctteach )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7324", 0x0000, 0x1000, CRC(08d15ab6) SHA1(5b0f6c53e6732a362c4bb25d966d4072fdd33db8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_ctteach_output.pla", 0, 557, CRC(3a5c7005) SHA1(3fe5819c138a90e7fc12817415f2622ca81b40b2) )

	ROM_REGION( 0x8000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cm62084.vsm", 0x0000, 0x4000, CRC(cd1376f7) SHA1(96fa484c392c451599bc083b8376cad9c998df7d) )
ROM_END


COMP( 1987, ctteach, 0, 0, ctteach, ctteach, driver_device, 0, "Coleco", "Talking Teacher", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
