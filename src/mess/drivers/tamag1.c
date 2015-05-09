// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Bandai Tamagotchi generation 1 hardware
  
  ** SKELETON Driver - feel free to add notes here, but driver itself is WIP

***************************************************************************/

#include "emu.h"
#include "cpu/e0c6200/e0c6s46.h"
#include "sound/speaker.h"


class tamag1_state : public driver_device
{
public:
	tamag1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
};




static INPUT_PORTS_START( tama )
INPUT_PORTS_END



static MACHINE_CONFIG_START( tama, tamag1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", E0C6S46, 32768)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tama )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "tama.b", 0x0000, 0x3000, CRC(5c864cb1) SHA1(4b4979cf92dc9d2fb6d7295a38f209f3da144f72) )
ROM_END


CONS( 1997, tama, 0, 0, tama, tama, driver_device, 0, "Bandai", "Tamagotchi (USA)", GAME_NOT_WORKING )
