// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Texas Instruments Speak & Spell hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "tispeak.lh"

// master clock is unknown
#define MASTER_CLOCK (500000)


class tispeak_state : public driver_device
{
public:
	tispeak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<tms0270_cpu_device> m_maincpu;

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

READ8_MEMBER(tispeak_state::read_k)
{
	return 0;
}

WRITE16_MEMBER(tispeak_state::write_r)
{
	m_r = data;
}

WRITE16_MEMBER(tispeak_state::write_o)
{
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( tispeak )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tispeak_state::machine_start()
{
	m_r = 0;
	m_o = 0;

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( tispeak, tispeak_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(tispeak_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispeak_state, write_r))
	
	MCFG_DEFAULT_LAYOUT(layout_tispeak)

	/* no video! */

	/* sound hardware */
//	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( snmath )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4946391_t2074", 0x0000, 0x1000, CRC(011f0c2d) SHA1(d2e14d72e03ca864abd51da78ffb71a9da82f624) ) // from patent 4946391, verified with source code

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tmc0270_cd2708_mpla.pla", 0, 2127, BAD_DUMP CRC(94333005) SHA1(1583444c73637d859632dd5186cd7e1a2588c78a) ) // taken from cd2708, need to verify if it's same as cd2704
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tmc0270_cd2708_opla.pla", 0, 1246, BAD_DUMP CRC(e70836e2) SHA1(70e7dcdf81ae2052874fb21c504fcc06b2649f9a) ) // "
ROM_END


COMP( 1980, snmath,  0, 0, tispeak, tispeak, driver_device, 0, "Texas Instruments", "Speak & Math (US, prototype)", GAME_NO_SOUND | GAME_NOT_WORKING )
