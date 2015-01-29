// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Wildfire
  * AMI S2150, labeled C10641


***************************************************************************/

#include "emu.h"
#include "cpu/amis2000/amis2000.h"
#include "sound/speaker.h"

#include "wildfire.lh"

// master clock is MCU internal, default frequency of 850kHz
#define MASTER_CLOCK (850000)


class wildfire_state : public driver_device
{
public:
	wildfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

//..



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( wildfire )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void wildfire_state::machine_start()
{
}


static MACHINE_CONFIG_START( wildfire, wildfire_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", AMI_S2150, MASTER_CLOCK)

	MCFG_DEFAULT_LAYOUT(layout_wildfire)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wildfire )
	ROM_REGION( 0x0600, "maincpu", 0 )
	ROM_LOAD( "us4341385", 0x0000, 0x0600, CRC(46877cef) SHA1(fc84c893cf0bdb12a5a002b921ce3120263e0081) ) // from patent US4334679, data should be correct (it included checksums)
ROM_END


CONS( 1979, wildfire, 0, 0, wildfire, wildfire, driver_device, 0, "Parker Brothers", "Wildfire (prototype)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
