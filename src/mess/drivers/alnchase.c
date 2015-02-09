// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Tomy Alien Chase (manufactured in Japan)
  * boards are labeled TN-16
  * NEC uCOM-43 MCU, labeled D553C 258
  * red/green VFD display with color overlay, 2-sided (opposing player sees a mirrored image)


***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "alnchase.lh" // this is a test layout, external artwork is necessary


class alnchase_state : public driver_device
{
public:
	alnchase_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	
	virtual void machine_start();
};



static INPUT_PORTS_START( alnchase )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void alnchase_state::machine_start()
{
}


static MACHINE_CONFIG_START( alnchase, alnchase_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)

	MCFG_DEFAULT_LAYOUT(layout_alnchase)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( alnchase )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-258", 0x0000, 0x0800, CRC(c5284ff5) SHA1(6a20aaacc9748f0e0335958f3cea482e36153704) )
ROM_END


CONS( 1984, alnchase, 0, 0, alnchase, alnchase, driver_device, 0, "Tomy", "Alien Chase", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
