// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Tomy Tennis (manufactured in Japan)
  * board labeled TOMY TN-04 TENNIS
  * NEC uCOM-44 MCU, labeled D552C 048
  * VFD display NEC FIP11AM15T (FIP=fluorescent indicator panel)


***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "tmtennis.lh"

// master clock is from an LC circuit oscillating by default at 360kHz,
// the difficulty switch puts a capacitor across it to slow it down to 260kHz
#define MASTER_CLOCK_PRO1 (260000)
#define MASTER_CLOCK_PRO2 (360000)


class tmtennis_state : public driver_device
{
public:
	tmtennis_state(const machine_config &mconfig, device_type type, const char *tag)
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



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( tmtennis )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tmtennis_state::machine_start()
{
}


static MACHINE_CONFIG_START( tmtennis, tmtennis_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, MASTER_CLOCK_PRO2)

	MCFG_DEFAULT_LAYOUT(layout_tmtennis)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tmtennis )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-048", 0x0000, 0x0400, CRC(78702003) SHA1(4d427d4dbeed901770c682338867f58c7b54eee3) )
ROM_END


CONS( 1980, tmtennis, 0, 0, tmtennis, tmtennis, driver_device, 0, "Tomy", "Tomytronic Tennis", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
