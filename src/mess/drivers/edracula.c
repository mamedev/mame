// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Epoch Dracula (manufactured in Japan)
  * NEC uCOM-43 MCU, labeled D553C 206
  * cyan/red/green VFD display NEC FIP8BM20T


***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "edracula.lh" // this is a test layout, external artwork is necessary


class edracula_state : public driver_device
{
public:
	edracula_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	
	virtual void machine_start();
};



static INPUT_PORTS_START( edracula )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void edracula_state::machine_start()
{
}


static MACHINE_CONFIG_START( edracula, edracula_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)

	MCFG_DEFAULT_LAYOUT(layout_edracula)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( edracula )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-206", 0x0000, 0x0800, CRC(b524857b) SHA1(c1c89ed5dd4bb1e6e98462dc8fa5af2aa48d8ede) )
ROM_END


CONS( 1982, edracula, 0, 0, edracula, edracula, driver_device, 0, "Epoch", "Dracula", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
