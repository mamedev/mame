// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Kenner Star Wars: Electronic Battle Command Game
  * TMS1100 MCU, marked MP3438A


***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "starwbc.lh"


class starwbc_state : public driver_device
{
public:
	starwbc_state(const machine_config &mconfig, device_type type, const char *tag)
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

READ8_MEMBER(starwbc_state::read_k)
{
	return 0;
}

WRITE16_MEMBER(starwbc_state::write_r)
{
	m_r = data;
}

WRITE16_MEMBER(starwbc_state::write_o)
{
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( starwbc )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void starwbc_state::machine_start()
{
	m_r = 0;
	m_o = 0;
	
	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( starwbc, starwbc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 400000)
	MCFG_TMS1XXX_READ_K_CB(READ8(starwbc_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(starwbc_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(starwbc_state, write_r))
	
	MCFG_DEFAULT_LAYOUT(layout_starwbc)

	/* no video! */

	/* sound hardware */
//	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( starwbc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3438a", 0x0000, 0x0800, CRC(c12b7069) SHA1(d1f39c69a543c128023ba11cc6228bacdfab04de) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_starwbc_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_opla.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END


CONS( 1979, starwbc, 0, 0, starwbc, starwbc, driver_device, 0, "Kenner", "Star Wars: Electronic Battle Command Game", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
