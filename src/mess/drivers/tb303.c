// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Roland TB-303
  * NEC uCOM-43 MCU, labeled D650C 133
  
  x

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "tb303.lh"


class tb303_state : public driver_device
{
public:
	tb303_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
};


static INPUT_PORTS_START( tb303 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tb303_state::machine_start()
{
}


static MACHINE_CONFIG_START( tb303, tb303_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D650, 454545) // LC circuit, 2.2us pulse

	MCFG_DEFAULT_LAYOUT(layout_tb303)

	/* no video! */

	/* sound hardware */
	// discrete...
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tb303 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-133.ic8", 0x0000, 0x0800, CRC(dd2f26ae) SHA1(7f5e37f38d970219dc9e5d49a20dc5335a5c0b30) )
ROM_END


CONS( 1982, tb303, 0, 0, tb303, tb303, driver_device, 0, "Roland", "TB-303", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
