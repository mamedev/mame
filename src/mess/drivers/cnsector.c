// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Code Name: Sector
  * MP0905BNL ZA0379 (die labeled 0970F-05B)


***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "cnsector.lh"


class cnsector_state : public driver_device
{
public:
	cnsector_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	UINT16 m_r;
	UINT16 m_o;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(cnsector_state::read_k)
{
	return 0;
}

WRITE16_MEMBER(cnsector_state::write_r)
{
	m_r = data;
}

WRITE16_MEMBER(cnsector_state::write_o)
{
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cnsector )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void cnsector_state::machine_start()
{
	m_r = 0;
	m_o = 0;
	
	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( cnsector, cnsector_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, 250000)
	MCFG_TMS1XXX_READ_K_CB(READ8(cnsector_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(cnsector_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(cnsector_state, write_r))
	
	MCFG_DEFAULT_LAYOUT(layout_cnsector)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cnsector )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0905bnl_za0379", 0x0000, 0x0400, CRC(564fe1a0) SHA1(825840a73175eee12e9712c871799f00e3be2c53) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_cnsector_mpla.pla", 0, 860, CRC(059f5bb4) SHA1(2653766f9fd74d41d44013bb6f54c0973a6080c9) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_cnsector_opla.pla", 0, 352, CRC(7c0bdcd6) SHA1(dade774097e8095dca5deac7b2367d0c701aca51) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_cnsector_spla.pla", 0, 157, CRC(56c37a4f) SHA1(18ecc20d2666e89673739056483aed5a261ae927) )
ROM_END


CONS( 1977, cnsector, 0, 0, cnsector, cnsector, driver_device, 0, "Parker Brothers", "Code Name: Sector", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
